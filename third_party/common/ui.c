/*
 * ui.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "ui.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// RASPI includes
#include "emux_api.h"
#include "circle.h"
#include "joy.h"
#include "kbd.h"
#include "menu.h"
#include "font.h"
#include "menu_timing.h"

#define COLOR16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)

#ifdef RASPI_LITE

#define BG_COLOR 6
#define FG_COLOR 3
#define DISABLED_COLOR 11
#define HILITE_COLOR 2
#define BORDER_COLOR 14
#define TRANSPARENT_COLOR 16

#else

#define BG_COLOR 0
#define FG_COLOR 1
#define DISABLED_COLOR 11
#define HILITE_COLOR 2
#define BORDER_COLOR 3
#define TRANSPARENT_COLOR 16

#endif

uint8_t *video_font;
uint16_t video_font_translate[256];
uint8_t *raw_video_font;

// Is the UI layer enabled? (either OSD or MENU)
volatile int ui_enabled;
int ui_showing;
// Countdown to toggle menu on/off
int ui_toggle_pending;
// One of the quick functions that can be invoked by button assignments
int pending_emu_quick_func;

static int osd_active;
static int ui_commodore_down;
static int ui_transparent;
static int ui_transparent_layer; // which layer we are revealing for adjustment
static int ui_render_current_item_only;

// Stubs for vice callbacks. Unimplemented for now.
void ui_pause_emulation(int flag) {}
int ui_emulation_is_paused(void) { return 0; }

// Width and height of our text menu in characters
const int menu_width_chars = 40;
const int menu_height_chars = 25;

// Stack of menu screens
static int current_menu = -1;
struct menu_item menu_roots[NUM_MENU_ROOTS];

// Where is our cursor in the menu?
static int menu_cursor[NUM_MENU_ROOTS];
struct menu_item *menu_cursor_item[NUM_MENU_ROOTS];

// Sliding window marking start and stop of what we're showing.
static int menu_window_top[NUM_MENU_ROOTS];
static int menu_window_bottom[NUM_MENU_ROOTS];

// The index of the last item + 1. Can't set cursor to this or higher.
static int max_index[NUM_MENU_ROOTS];

static int pending_ui_key_head = 0;
static int pending_ui_key_tail = 0;
static long pending_ui_key[16];
static int pending_ui_key_pressed[16];

// Global callback for events that happen on menu items
void (*on_value_changed)(struct menu_item *) = NULL;

// Key presses turn into these. Some actions are repeatable and
// the frequency at which they are executed can accelerate the
// longer they are enabled. Key releases will cancel the repeat.
#define ACTION_None 0
#define ACTION_Up 1
#define ACTION_Down 2
#define ACTION_Left 3
#define ACTION_Right 4
#define ACTION_Return 5
#define ACTION_Escape 6
#define ACTION_Exit 7
#define ACTION_MiniLeft 8
#define ACTION_MiniRight 9

#define INITIAL_ACTION_DELAY 24
#define INITIAL_ACTION_REPEAT_DELAY 8

// State variables managing hold and repeat behavior of menu
// actions.  Frequency of repeat will increase as time goes
// on.
static int ui_key_action;
static long ui_key_ticks;
static long ui_key_ticks_next;
static int ui_key_ticks_repeats;
static int ui_key_ticks_repeats_next;

static void ui_action(long action);

static int keyboard_shift = 0;

static uint8_t* ui_fb;
static int ui_fb_pitch;
static int ui_fb_w;
static int ui_fb_h;

void ui_init_menu(void) {
  int i;

  assert(emux_machine_class != BMC64_MACHINE_CLASS_UNKNOWN);

  ui_enabled = 0;
  ui_showing = 0;
  current_menu = -1;

  // Init menu roots
  for (i = 0; i < NUM_MENU_ROOTS; i++) {
    memset(&menu_roots[i], 0, sizeof(struct menu_item));
    menu_roots[i].type = FOLDER;
    menu_roots[i].is_expanded = 1;
    strncpy(menu_roots[i].name, "", MAX_MENU_STR);
  }

  // Root menu is never popped
  struct menu_item *root = ui_push_menu(-2, -2);

  // This also loads our custom settings file. It's safe to have settings
  // here that our videoarch code needs since this is called before any
  // canvases are created.
  build_menu(root);

  ui_key_action = ACTION_None;
  ui_key_ticks = 0;
  ui_key_ticks_next = 0;
  ui_key_ticks_repeats = 0;
  ui_key_ticks_repeats_next = 0;
}

// Draw a single character at x,y coords into the offscreen area
static void ui_draw_char(uint8_t c, int pos_x, int pos_y, int color,
                         uint8_t *dst, int dst_pitch, int stretch,
                         int translate) {
  int x, y, s;
  uint8_t fontchar;
  uint8_t *font_pos;
  uint8_t *draw_pos;

  // Destination is our ui frame buffer if not specified.
  if (dst == NULL) {
    dst_pitch = ui_fb_pitch;
    dst = ui_fb;

    // Don't draw out of bounds
    if (pos_y < 0 || pos_y > ui_fb_h - 8*stretch) {
      return;
    }
    if (pos_x < 0 || pos_x > ui_fb_w - 8*stretch) {
      return;
    }
  }

  if (translate) {
     // Use translation table.
     font_pos = &(video_font[video_font_translate[c]]);
  } else {
     font_pos = &(raw_video_font[c*8]);
  }
  draw_pos = &(dst[pos_x + pos_y * dst_pitch]);

  for (y = 0; y < 8*stretch; ++y) {
    fontchar = *font_pos;
    for (x = 0; x < 8; ++x) {
      if (fontchar & (0x80 >> x)) {
        for (s = 0; s < stretch; s++) {
           draw_pos[x*stretch+s] = color;
        }
      }
    }
    if (y % stretch == stretch-1) ++font_pos;
    draw_pos += dst_pitch;
  }
}

// Draw a string of text at location x,y. Does not word wrap.
void ui_draw_text_buf(const char *text, int x, int y, int color, uint8_t *dst,
                      int dst_pitch, int stretch) {
  int i;
  int x2 = x;
  for (i = 0; i < strlen(text); i++) {
    if (text[i] == '\n') {
      y = y + 8*stretch;
      x2 = x;
    } else {
      ui_draw_char(text[i], x2, y, color, dst, dst_pitch, stretch, 1);
      x2 = x2 + 8*stretch;
    }
  }
}

// No font translation from ascii to petscii
void ui_draw_char_raw(const char singlechar, int x, int y, int color,
                      uint8_t *dst, int dst_pitch, int stretch) {
   ui_draw_char(singlechar, x, y, color, dst, dst_pitch, stretch, 0);
}

void ui_draw_text(const char *text, int x, int y, int color) {
  ui_draw_text_buf(text, x, y, color, NULL, 0, 1);
}

// Draw a rectangle at x/y of given w/h into the offscreen area
void ui_draw_rect_buf(int x, int y, int w, int h, int color, int fill,
                      uint8_t *dst, int dst_pitch) {
  int xx, yy, x2, y2;

  // Destination is ui frame buffer if not specified.
  if (dst == NULL) {
    dst_pitch = ui_fb_pitch;
    dst = ui_fb;
  }
  x2 = x + w;
  y2 = y + h;
  for (xx = x, yy = y; yy < y2; xx++) {
    if (xx >= x2) {
      xx = x - 1;
      yy++;
    } else {
      int p1 = xx + yy * dst_pitch;
      if (fill | (yy == y || yy == (y2 - 1) || (xx == x) || xx == (x2 - 1))) {
        dst[p1] = color;
      }
    }
  }
}

void ui_draw_rect(int x, int y, int w, int h, int color, int fill) {
  ui_draw_rect_buf(x, y, w, h, color, fill, NULL, 0);
}

// Returns the height/width the given text would occupy if drawn
int ui_text_width(const char *text) { return 8 * strlen(text); }

static void do_on_value_changed(struct menu_item *item) {
  if (item->on_value_changed) {
    item->on_value_changed(menu_cursor_item[current_menu]);
  } else if (on_value_changed) {
    on_value_changed(menu_cursor_item[current_menu]);
  }
}

static void ui_type_char(char ch) {
  struct menu_item *cur = menu_cursor_item[current_menu];
  if (cur->type == TEXTFIELD) {
    if (ch == '\b') {
      if (cur->value <= 0)
        return;
      char *str = cur->str_value;
      memmove(str + cur->value - 1, str + cur->value,
              (strlen(str) - cur->value + 1) * sizeof(char));
      cur->value--;
    } else {
      if (strlen(cur->str_value) >= MAX_FN_NAME)
        return;

      char *str = cur->str_value;
      memmove(str + cur->value + 1, str + cur->value,
              (strlen(str) - cur->value + 1) * sizeof(char));
      str[cur->value] = ch;
      cur->value++;
    }
  } else {
    ui_find_first(ch);
  }
}

// Happens on main loop.
static void ui_key_pressed(long key) {
  // Anything other than left/right will reset transparency
  // and render current item only flags. They are applicable
  // only while the user is on the item they were triggered
  // for.
  if (key != KEYCODE_Left && key != KEYCODE_Right) {
    ui_transparent = 0;
    ui_transparent_layer = -1;
    ui_render_current_item_only = 0;
  }

  if (key == commodore_key_sym) {
     ui_commodore_down = 1;
     return;
  }

  switch (key) {
  case KEYCODE_Up:
  case KEYCODE_Down:
  case KEYCODE_Left:
  case KEYCODE_Right:
  case KEYCODE_Comma:
  case KEYCODE_Period:
    switch (key) {
      case KEYCODE_Up:
        ui_key_action = ACTION_Up; break;
      case KEYCODE_Down:
        ui_key_action = ACTION_Down; break;
      case KEYCODE_Left:
        ui_key_action = ACTION_Left; break;
      case KEYCODE_Right:
        ui_key_action = ACTION_Right; break;
      case KEYCODE_Comma:
        ui_key_action = ACTION_MiniLeft; break;
      case KEYCODE_Period:
        ui_key_action = ACTION_MiniRight; break;
      default:
        return;
    }
    ui_key_ticks = INITIAL_ACTION_DELAY;
    ui_key_ticks_next = INITIAL_ACTION_REPEAT_DELAY;
    ui_key_ticks_repeats = 0;
    ui_key_ticks_repeats_next = 8;
    ui_action(ui_key_action);
    return;
  case KEYCODE_Escape:
    return;
  case KEYCODE_LeftShift:
    keyboard_shift |= 1;
    return;
  case KEYCODE_RightShift:
    keyboard_shift |= 2;
    return;
  }

  if (key >= KEYCODE_a && key <= KEYCODE_z) {
    char ch;
    if (keyboard_shift)
      ch = 'A' + key - KEYCODE_a;
    else
      ch = 'a' + key - KEYCODE_a;
    ui_type_char(ch);
  } else if (key >= KEYCODE_1 && key <= KEYCODE_9) {
    char ch = '1' + key - KEYCODE_1;
    ui_type_char(ch);
  } else if (key == KEYCODE_0) {
    ui_type_char('0');
  } else if (key == KEYCODE_Dash) {
    if (keyboard_shift)
      ui_type_char('_');
    else
      ui_type_char('-');
  } else if (key == KEYCODE_Period) {
    ui_type_char('.');
  } else if (key == KEYCODE_Backspace) {
    ui_type_char('\b');
  }
}

// Happens on main loop. Process a key release for the ui.
static void ui_key_released(long key) {
  if (key == commodore_key_sym) {
    ui_commodore_down = 0;
    return;
  }

  switch (key) {
  case KEYCODE_Up:
  case KEYCODE_Down:
  case KEYCODE_Left:
  case KEYCODE_Right:
  case KEYCODE_Comma:
  case KEYCODE_Period:
    ui_key_action = ACTION_None;
    return;
  case KEYCODE_Return:
    ui_action(ACTION_Return);
    return;
  case KEYCODE_Escape:
  case KEYCODE_BackQuote:
    ui_action(ACTION_Escape);
    return;
  case KEYCODE_F12:
    ui_action(ACTION_Exit);
    return;
  // Since FX keys are also used for hotkeys,
  // best not to perform these ui functions if
  // the cntrl key is down. It may trigger a
  // hotkey function and we don't want these
  // to happen as well.
  case KEYCODE_Home:
  case KEYCODE_F1:
    if (!ui_commodore_down) ui_to_top();
    return;
  case KEYCODE_End:
  case KEYCODE_F7:
    if (!ui_commodore_down) ui_to_bottom();
    return;
  case KEYCODE_PageUp:
  case KEYCODE_F3:
    if (!ui_commodore_down) ui_page_up();
    return;
  case KEYCODE_PageDown:
  case KEYCODE_F5:
    if (!ui_commodore_down) ui_page_down();
    return;
  case KEYCODE_LeftShift:
    keyboard_shift &= ~1;
    return;
  case KEYCODE_RightShift:
    keyboard_shift &= ~2;
    return;
  }
}

// Do the next ui action based on key pressed and timeout
static void ui_action_frame() {
  if (ui_key_action != ACTION_None) {
    ui_key_ticks--;
    // When key ticks hits zero, repeat the action.
    if (ui_key_ticks == 0) {
      ui_action(ui_key_action);
      // Set new ticks
      ui_key_ticks = ui_key_ticks_next;
      // Keep track of how many repeats
      ui_key_ticks_repeats++;
      if (ui_key_ticks_repeats >= ui_key_ticks_repeats_next) {
        ui_key_ticks_repeats_next *= 4;
        ui_key_ticks_next /= 2;
        if (ui_key_ticks_next < 2)
          ui_key_ticks_next = 2;
      }
    }
  }
}

void ui_render_single_frame() {
  // Start with transparent
  memset(ui_fb, TRANSPARENT_COLOR, ui_fb_h * ui_fb_pitch);

  for (int msi=0;msi<=current_menu;msi++) {
     ui_render_now(msi);
  }
  circle_frames_ready_fbl(FB_LAYER_UI, -1 /* no 2nd layer */, 1 /* sync */);
  circle_yield();
}

static void ui_toggle(void) {
  ui_enabled = 1 - ui_enabled;
  if (ui_enabled) {
    emux_trap_main_loop_ui();
  }
}

void ui_pop_all_and_toggle() {
  while (current_menu > 0) {
    ui_pop_menu();
  }
  ui_toggle();
}

static void cursor_pos_updated() {
  // Tell listener
  if (menu_roots[current_menu].cursor_listener_func) {
     menu_roots[current_menu].cursor_listener_func(&menu_roots[current_menu],
                                                   menu_cursor[current_menu]);
  }
}

static void ui_action(long action) {
  struct menu_item *cur = menu_cursor_item[current_menu];
  switch (action) {
  case ACTION_Up:
    menu_cursor[current_menu]--;
    cursor_pos_updated();
    if (menu_cursor[current_menu] < 0) {
      menu_cursor[current_menu] = 0;
      cursor_pos_updated();
    }
    if (menu_cursor[current_menu] <= (menu_window_top[current_menu] - 1)) {
      menu_window_top[current_menu]--;
      menu_window_bottom[current_menu]--;
    }
    break;
  case ACTION_Down:
    menu_cursor[current_menu]++;
    cursor_pos_updated();
    if (menu_cursor[current_menu] >= max_index[current_menu]) {
      menu_cursor[current_menu] = max_index[current_menu] - 1;
      cursor_pos_updated();
    }
    if (menu_cursor[current_menu] >= menu_window_bottom[current_menu]) {
      menu_window_top[current_menu]++;
      menu_window_bottom[current_menu]++;
    }
    break;
  case ACTION_Left:
  case ACTION_MiniLeft:
    if (cur->disabled) break;
    if (cur->type == RANGE) {
      if (action == ACTION_MiniLeft)
         cur->value -= cur->ministep;
      else
         cur->value -= cur->step;

      if (cur->value < cur->min) {
        cur->value = cur->min;
      } else {
        do_on_value_changed(menu_cursor_item[current_menu]);
      }
    } else if (cur->type == MULTIPLE_CHOICE) {
      int orig = cur->value;
      cur->value -= 1;
      if (cur->value < 0) {
        cur->value = cur->num_choices - 1;
      }
      // NOTE: This doesn't support the first choice being disabled!
      while (cur->choice_disabled[cur->value] && cur->value != orig) {
        cur->value -= 1;
      }
      if (cur->value < 0) {
        cur->value = cur->num_choices - 1;
      }
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == TOGGLE) {
      cur->value = 1 - cur->value;
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == TEXTFIELD) {
      // Move cursor left
      cur->value--;
      if (cur->value < 0) {
        cur->value = 0;
      }
    }
    break;
  case ACTION_Right:
  case ACTION_MiniRight:
    if (cur->disabled) break;
    if (cur->type == RANGE) {
      if (action == ACTION_MiniRight)
         cur->value += cur->ministep;
      else
         cur->value += cur->step;

      if (cur->value > cur->max) {
        cur->value = cur->max;
      } else {
        do_on_value_changed(menu_cursor_item[current_menu]);
      }
    } else if (cur->type == MULTIPLE_CHOICE) {
      int orig = cur->value;
      cur->value += 1;
      if (cur->value >= cur->num_choices) {
        cur->value = 0;
      }
      while (cur->choice_disabled[cur->value] && cur->value != orig) {
        cur->value = (cur->value + 1) % cur->num_choices;
      }
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == TOGGLE) {
      cur->value = 1 - cur->value;
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == TEXTFIELD) {
      // Move cursor right
      cur->value++;
      if (cur->value >= strlen(cur->str_value)) {
        cur->value = strlen(cur->str_value);
      }
    }
    break;
  case ACTION_Return:
    if (cur->disabled) break;
    if (cur->type == FOLDER) {
      cur->is_expanded = 1 - cur->is_expanded;
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == CHECKBOX) {
      cur->value = 1 - cur->value;
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == TOGGLE) {
      cur->value = 1 - cur->value;
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == BUTTON) {
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == MULTIPLE_CHOICE) {
      int orig = cur->value;
      cur->value += 1;
      if (cur->value >= cur->num_choices) {
        cur->value = 0;
      }
      while (cur->choice_disabled[cur->value] && cur->value != orig) {
        cur->value += 1;
      }
      do_on_value_changed(menu_cursor_item[current_menu]);
    } else if (cur->type == TEXTFIELD) {
      do_on_value_changed(menu_cursor_item[current_menu]);
    }
    break;
  case ACTION_Escape:
    if (current_menu > 0) {
      if (osd_active) {
        ui_pop_all_and_toggle();
        return;
      }
      ui_pop_menu();
    } else {
      ui_toggle();
    }
    break;
  case ACTION_Exit:
    ui_pop_all_and_toggle();
    break;
  }
}

// queue a key for press/release on the UI loop
void emu_ui_key_interrupt(long key, int pressed) {
  circle_lock_acquire();
  int i = pending_ui_key_tail & 0xf;
  pending_ui_key[i] = key;
  pending_ui_key_pressed[i] = pressed;
  pending_ui_key_tail++;
  circle_lock_release();
}

// Do key press/releases on the main loop
void ui_check_key(void) {
  static long process_ui_key[16];
  static int process_ui_key_pressed[16];

  if (!ui_enabled) {
    return;
  }

  // Process ui key event queue
  // Don't hold on to the lock while we call ui handlers.  It causes
  // locking problems with dispmanx calls. Take a copy, then process
  // outside the queue lock.
  circle_lock_acquire();
  int process_index = 0;
  while (pending_ui_key_head != pending_ui_key_tail) {
    int i = pending_ui_key_head & 0xf;
    process_ui_key[process_index] = pending_ui_key[i];
    process_ui_key_pressed[process_index] = pending_ui_key_pressed[i];
    process_index++;
    pending_ui_key_head++;
  }
  circle_lock_release();

  // Now process the ui keys
  for (int i=0;i<process_index;i++) {
    if (process_ui_key_pressed[i]) {
      ui_key_pressed(process_ui_key[i]);
    } else {
      ui_key_released(process_ui_key[i]);
    }
  }

  // Ui action frame tick
  ui_action_frame();
}

void ui_handle_toggle_or_quick_func() {
  // This ensures we transition from emulator to ui only after we've
  // submitted key events and let the emulator process them. Otherwise,
  // we can leave keys in a down state unintentionally. Needs to be set
  // to 2 to ensure we dequeue, then let the emulator process those events.
  if (ui_toggle_pending) {
    ui_toggle_pending--;
    if (ui_toggle_pending == 0) {
      // Even when we are entering the menu, we can't assume there aren't
      // already menus stacked on the root. This will ensure we always enter
      // and leave the menu in a known state (only root menu is on stack).
      ui_pop_all_and_toggle();
    }
  } else if (pending_emu_quick_func) {
    menu_quick_func(pending_emu_quick_func);
    pending_emu_quick_func = 0;
  }
}

void ui_add_all(struct menu_item *src, struct menu_item *dest) {
  assert(src != NULL);
  assert(src->type == FOLDER);
  assert(dest != NULL);
  assert(dest->type == FOLDER);
  struct menu_item *dest_prev = NULL;
  struct menu_item *dest_ptr = dest->first_child;
  struct menu_item *src_ptr = src->first_child;

  // Move to end of dest list
  while (dest_ptr != 0) {
    dest_prev = dest_ptr;
    dest_ptr = dest_ptr->next;
  }

  while (src_ptr != 0) {
    // Children must inheret these properties from new parent.
    src_ptr->menu_width = dest->menu_width;
    src_ptr->menu_height = dest->menu_height;
    src_ptr->menu_top = dest->menu_top;
    src_ptr->menu_left = dest->menu_left;
    src_ptr = src_ptr->next;
  }

  // Put src's children onto dest and cut link from src
  if (dest_prev == NULL) {
    dest->first_child = src->first_child;
  } else {
    dest_prev->next = src->first_child;
  }
  src->first_child = NULL;
}

static char *get_button_display_str(struct menu_item *node) {
  if (node->prefer_str || strlen(node->displayed_value) > 0) {
    return node->displayed_value;
  } else {
    // Turn value into string as fallback
    if (node->map_value_func) {
      sprintf(node->scratch, "%d", node->map_value_func(node->value));
    } else {
      sprintf(node->scratch, "%d", node->value);
    }
    return node->scratch;
  }
}

static void append(struct menu_item *folder, struct menu_item *new_item) {
  assert(folder != NULL);
  assert(folder->type == FOLDER);
  struct menu_item *prev = NULL;
  struct menu_item *ptr = folder->first_child;
  while (ptr != 0) {
    prev = ptr;
    ptr = ptr->next;
  }
  if (prev == NULL) {
    folder->first_child = new_item;
  } else {
    prev->next = new_item;
  }
}

static struct menu_item *ui_new_item(struct menu_item *parent, const char *name,
                                     int id) {
  struct menu_item *new_item =
      (struct menu_item *)malloc(sizeof(struct menu_item));
  memset(new_item, 0, sizeof(struct menu_item));
  strncpy(new_item->name, name, MAX_MENU_STR);
  new_item->id = id;

  // Inherit parent dimensions
  new_item->menu_width = parent->menu_width;
  new_item->menu_height = parent->menu_height;
  new_item->menu_top = parent->menu_top;
  new_item->menu_left = parent->menu_left;
  return new_item;
}

struct menu_item *ui_menu_add_toggle(int id, struct menu_item *folder,
                                     char *name, int initial_state) {
  struct menu_item *new_item = ui_new_item(folder, name, id);
  new_item->type = TOGGLE;
  new_item->value = initial_state;
  append(folder, new_item);
  return new_item;
}

struct menu_item *ui_menu_add_toggle_labels(int id, struct menu_item *folder,
                                     char *name, int initial_state,
                                     char *custom_0, char *custom_1) {
  struct menu_item *new_item =
     ui_menu_add_toggle(id, folder, name, initial_state);
  strcpy(new_item->custom_toggle_label[0], custom_0);
  strcpy(new_item->custom_toggle_label[1], custom_1);
  return new_item;
}

struct menu_item *ui_menu_add_checkbox(int id, struct menu_item *folder,
                                       char *name, int initial_state) {
  struct menu_item *new_item = ui_new_item(folder, name, id);
  new_item->type = CHECKBOX;
  new_item->value = initial_state;
  append(folder, new_item);
  return new_item;
}

struct menu_item *ui_menu_add_multiple_choice(int id, struct menu_item *folder,
                                              char *name) {
  struct menu_item *new_item = ui_new_item(folder, name, id);
  new_item->type = MULTIPLE_CHOICE;
  new_item->num_choices = 0;
  append(folder, new_item);
  return new_item;
}

struct menu_item *ui_menu_add_button(int id, struct menu_item *folder,
                                     const char *name) {
  return ui_menu_add_button_with_value(id, folder, name, 0, " ", " ");
}

struct menu_item *ui_menu_add_button_with_value(int id,
                                                struct menu_item *folder,
                                                const char *name, int value,
                                                const char *str_value,
                                                const char *displayed_value) {
  struct menu_item *new_item = ui_new_item(folder, name, id);
  new_item->type = BUTTON;
  new_item->value = value;
  strncpy(new_item->str_value, str_value, MAX_STR_VAL_LEN);
  strncpy(new_item->displayed_value, displayed_value, MAX_DSP_VAL_LEN);
  append(folder, new_item);
  return new_item;
}

struct menu_item *ui_menu_add_range(int id, struct menu_item *folder,
                                    char *name, int min, int max, int step,
                                    int initial_value) {
  struct menu_item *new_item = ui_new_item(folder, name, id);
  new_item->type = RANGE;
  new_item->min = min;
  new_item->max = max;
  new_item->step = step;
  new_item->ministep = 1;
  new_item->divisor = 1;
  new_item->value = initial_value;
  append(folder, new_item);
  return new_item;
}

struct menu_item *ui_menu_add_folder(struct menu_item *folder, char *name) {
  struct menu_item *new_item = ui_new_item(folder, name, MENU_ID_DO_NOTHING);
  new_item->type = FOLDER;
  append(folder, new_item);
  return new_item;
}

struct menu_item *ui_menu_add_divider(struct menu_item *folder) {
  struct menu_item *new_item = ui_new_item(folder, "", MENU_ID_DO_NOTHING);
  new_item->type = DIVIDER;
  append(folder, new_item);
  return new_item;
}

struct menu_item *ui_menu_add_text_field(int id, struct menu_item *folder,
                                         char *name, char *value_str) {
  struct menu_item *new_item = ui_new_item(folder, name, id);
  new_item->type = TEXTFIELD;
  new_item->value = strlen(value_str);
  strcpy(new_item->str_value, value_str);
  append(folder, new_item);
  return new_item;
}

static void ui_render_children(struct menu_item *node,
                               int stack_index, int *index, int indent) {
  while (node != NULL) {
    node->render_index = *index;

    int colour = node->disabled ? DISABLED_COLOR : FG_COLOR;

    // Render a row
    if (*index >= menu_window_top[stack_index] &&
        *index < menu_window_bottom[stack_index]) {
      int y = (*index - menu_window_top[stack_index]) * 8 + node->menu_top;
      if (*index == menu_cursor[stack_index]) {
        ui_draw_rect(node->menu_left, y, node->menu_width, 8, HILITE_COLOR, 1);
        menu_cursor_item[stack_index] = node;
      }

      // Special symbol drawn on left edge
      if (node->symbol) {
          ui_draw_char_raw(node->symbol,
              node->menu_left+indent*8, y, colour, NULL, 0, 1);
      }

      // Sometimes, we only want to render the current item. Like when we
      // are adjusting things that affect video and we want to see the display
      // underneath the menu while we are making changes.
      if (!ui_render_current_item_only ||
          *index == menu_cursor[stack_index]) {

        ui_draw_text(node->name,
           node->menu_left + (indent + 1) * 8, y, colour);

        if (node->type == FOLDER) {
          if (node->is_expanded)
            ui_draw_text("-", node->menu_left + (indent)*8, y, colour);
          else
            ui_draw_text("+", node->menu_left + (indent)*8, y, colour);
        } else if (node->type == TOGGLE) {
          if (node->value) {
            if (node->custom_toggle_label[1][0] == '\0') {
               ui_draw_text("On",
                         node->menu_left + node->menu_width -
                         ui_text_width("On"), y, colour);
            } else {
               ui_draw_text(node->custom_toggle_label[1],
                         node->menu_left + node->menu_width -
                         ui_text_width(node->custom_toggle_label[1]), y,
                                       colour);
            }
          } else {
            if (node->custom_toggle_label[0][0] == '\0') {
               ui_draw_text("Off", node->menu_left + node->menu_width -
                         ui_text_width("Off"), y, colour);
            } else {
               ui_draw_text(node->custom_toggle_label[0],
                         node->menu_left + node->menu_width -
                         ui_text_width(node->custom_toggle_label[0]), y,
                                       colour);
            }
          }
        } else if (node->type == CHECKBOX) {
          if (node->value)
            ui_draw_text("True", node->menu_left + node->menu_width -
                                     ui_text_width("True"),
                         y, colour);
          else
            ui_draw_text("False", node->menu_left + node->menu_width -
                                      ui_text_width("False"),
                         y, colour);
        } else if (node->type == RANGE) {
          if (node->divisor == 1) {
             sprintf(node->scratch, "%d", node->value);
          } else {
             // TODO: Don't assume 3 decimal places. Use divisor.
             sprintf(node->scratch, "%.3f",
                (float)node->value / (float)node->divisor);
          }
          ui_draw_text(node->scratch, node->menu_left + node->menu_width -
                                          ui_text_width(node->scratch),
                       y, colour);
        } else if (node->type == MULTIPLE_CHOICE) {
          ui_draw_text(node->choices[node->value],
                       node->menu_left + node->menu_width -
                           ui_text_width(node->choices[node->value]),
                       y, colour);
        } else if (node->type == DIVIDER) {
          ui_draw_rect(node->menu_left, y + 3, node->menu_width, 2, BORDER_COLOR, 1);
        } else if (node->type == BUTTON) {
          char *dsp_string = get_button_display_str(node);
          ui_draw_text(dsp_string, node->menu_left + node->menu_width -
                                       ui_text_width(dsp_string),
                       y, colour);
        } else if (node->type == TEXTFIELD) {
          // draw cursor underneath text
          ui_draw_rect(node->menu_left + ui_text_width(node->name) + 8 +
                           node->value * 8,
                       y, 8, 8, BORDER_COLOR, 1);
          ui_draw_text(node->str_value,
                       node->menu_left + ui_text_width(node->name) + 8, y,
                       colour);
        }
      }
    }

    *index = *index + 1;
    if (node->type == FOLDER && node->is_expanded &&
        node->first_child != NULL) {
      ui_render_children(node->first_child, stack_index, index, indent + 1);
    }
    node = node->next;
  }
}

// Make the UI layer fully transparent in preparation for an OSD to
// be displayed.
void ui_make_transparent(void) {
  memset(ui_fb, TRANSPARENT_COLOR, ui_fb_h * ui_fb_pitch);
}

static void ui_draw_shadow_text(const char* txt, int *x, int *y, int col) {
  ui_draw_text(txt, *x+1, *y, 0);
  ui_draw_text(txt, *x-1, *y, 0);
  ui_draw_text(txt, *x, *y+1, 0);
  ui_draw_text(txt, *x, *y-1, 0);
  ui_draw_text(txt, *x, *y, col);
  *x = *x + strlen(txt) *8;
}

void ui_render_now(int menu_stack_index) {
  int index = 0;
  int indent = 0;

  if (menu_stack_index == -1) {
    menu_stack_index = current_menu;
    // When rendering only the top most menu, clear with transparent color
    memset(ui_fb, TRANSPARENT_COLOR, ui_fb_h * ui_fb_pitch);
  }

  struct menu_item *ptr = menu_roots[menu_stack_index].first_child;

  // background conditional upon mode
  if (!ui_transparent) {
     ui_draw_rect(ptr->menu_left, ptr->menu_top,
                  ptr->menu_width, ptr->menu_height,
                  BG_COLOR, 1);
  }

  // border
  ui_draw_rect(ptr->menu_left - 1, ptr->menu_top - 1, ptr->menu_width + 2,
               ptr->menu_height + 2, BORDER_COLOR, 0);

  // menu text
  ui_render_children(ptr, menu_stack_index, &index, indent);

  max_index[menu_stack_index] = index;

  if (menu_cursor[menu_stack_index] >= max_index[menu_stack_index]) {
    menu_cursor[menu_stack_index] = max_index[menu_stack_index] - 1;
    cursor_pos_updated();
  }

  // Reveal dimensions in top left corner
  if (ui_transparent && ui_transparent_layer >= 0) {
    char str1[32];
    char str2[32];
    int dpx, dpy, fbw, fbh, dw, dh, sw, sh;

    // We're drawing into the UI layer so get it's fb dims.
    circle_get_fbl_dimensions(FB_LAYER_UI,
                              &dpx, &dpy,
                              &fbw, &fbh,
                              &sw, &sh,
                              &dw, &dh);

    // We can use the 1st display canvas info because our UI layer
    // mirrors it's dimensions all the time.
    int cx = canvas_state[VIC_INDEX].left + sw / 2 - 18 * 8 / 2;
    int cy = canvas_state[VIC_INDEX].top + sh / 2 - 7 * 10 / 2;

    // Now get info about the layer we are djusting
    circle_get_fbl_dimensions(ui_transparent_layer,
                              &dpx, &dpy,
                              &fbw, &fbh,
                              &sw, &sh,
                              &dw, &dh);

    int qx = cx;
    int qy = cy;

    sprintf (str1,"Display: %dx%d", dpx, dpy);
    ui_draw_shadow_text(str1, &qx, &qy, 1);

    qx = cx; qy+=10;
    // Unscaled frame buffer
    sprintf (str1, "FB: %d x %d", sw, sh);
    ui_draw_shadow_text(str1, &qx, &qy, 1);
    qx = qx + 10;

    // Scaled frame buffer. Show green dimension if it is
    // an even multiple of the unscaled frame buffer.
    qx = cx; qy+=10;
    sprintf (str1, "%d", dw);
    sprintf (str2, "%d", dh);
    ui_draw_shadow_text("SFB:", &qx, &qy, 1);
    qx = qx + 8;
    ui_draw_shadow_text(str1, &qx, &qy, dw % sw == 0 ? 5 : 1);
    ui_draw_shadow_text("x", &qx, &qy, 1);
    ui_draw_shadow_text(str2, &qx, &qy, dh % sh == 0 ? 5 : 1);
    qx = qx + 8;
    if (dw % sw == 0) {
       sprintf (str1, "x%d,", dw/sw);
       ui_draw_shadow_text(str1, &qx, &qy, 5);
    } else {
       ui_draw_shadow_text("*", &qx, &qy, 1);
    }
    if (dh % sh == 0) {
       sprintf (str1, "x%d", dh/sh);
       ui_draw_shadow_text(str1, &qx, &qy, 5);
    } else {
       ui_draw_shadow_text("*", &qx, &qy, 1);
    }

    qx = cx; qy+=20;
    ui_draw_shadow_text("Use , and . for", &qx, &qy, 1);
    qx = cx; qy+=10;
    ui_draw_shadow_text("-/+1 increments.", &qx, &qy, 1);
  }
}

// This function will traverse recursively all nodes in the node list
// starting at 'node'.  It fills in the render_index for each node as it
// goes and records the node that matches the current cursor index into
// menu_cursor_item.  No rendering is done here.  It used when
// we need to find the node matching the cursor position, taking into
// account all items that have been expanded/contracted.
static void ui_traverse_children(struct menu_item *node, int *index) {
  while (node != NULL) {
    node->render_index = *index;

    if (*index >= menu_window_top[current_menu] &&
        *index < menu_window_bottom[current_menu]) {
      if (*index == menu_cursor[current_menu]) {
        menu_cursor_item[current_menu] = node;
      }
    }

    *index = *index + 1;
    if (node->type == FOLDER && node->is_expanded &&
        node->first_child != NULL) {
      ui_traverse_children(node->first_child, index);
    }
    node = node->next;
  }
}

// This function will traverse recursively all child nodes in the current
// active menu. See ui_traverse_child for more details on when this
// is useful.  It also records the max index for the current menu taking
// into account all items that have been expanded/contracted.
static void ui_traverse(void) {
  int index = 0;
  struct menu_item *ptr = menu_roots[current_menu].first_child;

  ui_traverse_children(ptr, &index);

  max_index[current_menu] = index;

  if (menu_cursor[current_menu] >= max_index[current_menu]) {
    menu_cursor[current_menu] = max_index[current_menu] - 1;
    cursor_pos_updated();
  }
}

static void ui_clear_child_menu(struct menu_item *node) {
  if (node != NULL && node->type == FOLDER) {
    ui_clear_child_menu(node->first_child);
  }

  while (node != NULL) {
    struct menu_item *next = node->next;
    free(node);
    node = next;
  }
}

static void ui_clear_menu(int menu_index) {
  struct menu_item *node = &menu_roots[menu_index];
  ui_clear_child_menu(node->first_child);
  node->first_child = NULL;
}

struct menu_item *ui_pop_menu(void) {
  struct menu_item *menu_to_pop = &menu_roots[current_menu];
  ui_clear_menu(current_menu);
  current_menu--;

  if (menu_to_pop->on_popped_off) {
    // Notify pop happened (new_root/old_root)
    menu_to_pop->on_popped_off(&menu_roots[current_menu], menu_to_pop);
  }

  if (menu_roots[current_menu].on_popped_to) {
    // Notify pop happened (new_root/old_root)
    menu_to_pop->on_popped_to(&menu_roots[current_menu], menu_to_pop);
  }

  if (current_menu < 0) {
    printf("FATAL ERROR: tried to pop last menu\n");
    return NULL;
  }
  return &menu_roots[current_menu];
}

// left + border_w brings us to the left of gfx area
// then we center the menu width inside the gfx_w area
static int calc_root_menu_left() {
   return
       canvas_state[VIC_INDEX].left +
          canvas_state[VIC_INDEX].border_w +
             canvas_state[VIC_INDEX].gfx_w / 2 -
                menu_width_chars * 8 / 2;
}

// top + border_h brings us to the top of the gfx area
// then we center the menu height inside the gfx_h area
// BUT must take into account raster_skip since we don't
// double the height of the UI frame buffer like we do
// the main display.
static int calc_root_menu_top() {
   int raster_skip = canvas_state[VIC_INDEX].raster_skip;

   int ui_top = canvas_state[VIC_INDEX].first_displayed_line +
       canvas_state[VIC_INDEX].max_border_h / raster_skip;

   return ui_top + canvas_state[VIC_INDEX].gfx_h / 2 /
                      canvas_state[VIC_INDEX].raster_skip -
                         menu_height_chars * 8 / 2;
}

struct menu_item *ui_push_menu(int w_chars, int h_chars) {

  int menu_width = w_chars * 8;
  int menu_height = h_chars * 8;
  if (w_chars < 0)
    menu_width = menu_width_chars * 8;
  if (h_chars < 0)
    menu_height = menu_height_chars * 8;

  current_menu++;
  if (current_menu >= NUM_MENU_ROOTS) {
    printf("FATAL ERROR: tried to push menu beyond NUM_MENU_ROOTS\n");
    return NULL;
  }
  ui_clear_menu(current_menu);

  // Client must set callback on each push so clear here.
  menu_roots[current_menu].on_value_changed = NULL;
  menu_roots[current_menu].on_popped_off = NULL;
  menu_roots[current_menu].on_popped_to = NULL;

  // Set dimensions
  menu_roots[current_menu].menu_width = menu_width;
  menu_roots[current_menu].menu_height = menu_height;

  if (w_chars == -2) {
    menu_roots[current_menu].menu_left = calc_root_menu_left();
  } else if (w_chars == -1) {
    // Inherit the root menu's left
    menu_roots[current_menu].menu_left = menu_roots[0].menu_left;
  } else {
    // Center this smaller menu inside the bounds of the root
    menu_roots[current_menu].menu_left =
       menu_roots[0].menu_left + (menu_roots[0].menu_width - menu_width) / 2;
  }

  if (h_chars == -2) {
    menu_roots[current_menu].menu_top = calc_root_menu_top();
  } else if (h_chars == -1) {
    // Inherit the root menu's top
    menu_roots[current_menu].menu_top = menu_roots[0].menu_top;
  } else {
    // Center this smaller menu inside the bounds of the root
    menu_roots[current_menu].menu_top =
       menu_roots[0].menu_top + (menu_roots[0].menu_height - menu_height) / 2;
  }

  menu_cursor[current_menu] = 0;
  menu_window_top[current_menu] = 0;
  if (h_chars < 0) {
     menu_window_bottom[current_menu] = menu_height_chars;
  } else {
     menu_window_bottom[current_menu] = h_chars;
  }

  return &menu_roots[current_menu];
}

void ui_set_on_value_changed_callback(void (*callback)(struct menu_item *)) {
  on_value_changed = callback;
}

int emu_is_ui_activated(void) {
  return ui_enabled;
}

static struct menu_item *ui_push_dialog_header(int is_error) {
  struct menu_item *root = ui_push_menu(30, 4);
  if (is_error) {
    ui_menu_add_button(MENU_ERROR_DIALOG, root, "Error");
  } else {
    ui_menu_add_button(MENU_INFO_DIALOG, root, "Info");
  }
  ui_menu_add_divider(root);
  return root;
}

// Attach this callback to any OSD dialog
void glob_osd_popped(struct menu_item *new_root,
                     struct menu_item *old_root) {
  ui_disable_osd();
}

void ui_error(const char *format, ...) {
  struct menu_item *root = ui_push_dialog_header(1);
  // Don't show layer info when we want to show error.
  ui_transparent_layer = 0;
  if (!ui_enabled) {
     // We were called without the UI being up. Make this an OSD.
     ui_enable_osd();
     root->on_popped_off = glob_osd_popped;
  }
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, 255, format, args);
  ui_menu_add_button(MENU_ERROR_DIALOG, root, buffer);
  va_end(args);
  ui_render_single_frame();
}

void ui_info(const char *format, ...) {
  struct menu_item *root = ui_push_dialog_header(0);
  // Don't show layer info when we want to show info.
  ui_transparent_layer = 0;
  if (!ui_enabled) {
     // We were called without the UI being up. Make this an OSD.
     ui_enable_osd();
     root->on_popped_off = glob_osd_popped;
  }
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, 255, format, args);
  ui_menu_add_button(MENU_INFO_DIALOG, root, buffer);
  va_end(args);
  ui_render_single_frame();
}

void ui_confirm_wrapped(char *title, const char *txt, int ok_value, int ok_id) {
  struct menu_item *root = ui_push_menu(30, 10);
  ui_menu_add_button(MENU_ERROR_DIALOG, root, title);

  struct menu_item *child;
  if (ok_value >=0 && ok_id >=0) {
     child = ui_menu_add_button(MENU_CONFIRM_OK, root, "OK");
     child->value = ok_value;
     child->sub_id = ok_id;

     ui_menu_add_button(MENU_CONFIRM_CANCEL, root, "CANCEL");
  }

  ui_menu_add_divider(root);
  char buf[512];
  char line[64];
  strcpy (buf, txt);
  line[0] = '\0';

  int buf_pos = 0;
  int line_pos = 0;
  char* word = strtok(buf," ");
  while (word) {
     int word_len = strlen(word);
     if (buf_pos + word_len < 512) {
        if (line_pos + word_len < 30) {
           strcat(line, word);
           strcat(line, " ");
           line_pos += word_len + 1;
        } else {
           ui_menu_add_button(MENU_INFO_DIALOG, root, line);
           strcpy(line, word);
           strcat(line, " ");
           line_pos = word_len + 1;
        }
        buf_pos += word_len + 1;
     }
     word = strtok(NULL," ");
  }
  if (strlen(line) > 0) {
     ui_menu_add_button(MENU_INFO_DIALOG, root, line);
  }

  ui_render_single_frame();
}

// These nav functions are really inefficient...but oh well.
void ui_page_down() {
  for (int n=0;n<menu_height_chars;n++) {
    ui_action(ACTION_Down);
  }
}

void ui_page_up() {
  for (int n=0;n<menu_height_chars;n++) {
    ui_action(ACTION_Up);
  }
}

void ui_to_top() {
  while (menu_cursor[current_menu] != 0) {
    ui_action(ACTION_Up);
  }
}

void ui_to_bottom() {
  while (menu_cursor[current_menu] < max_index[current_menu] - 1) {
    ui_action(ACTION_Down);
  }
}

void ui_find_first(char letter) {

  int start_index = menu_cursor[current_menu];

  while(1) {
    // Move down or wrap around to the top if we hit the bottom.
    if (menu_cursor[current_menu] >= max_index[current_menu] - 1) {
       ui_to_top();
    } else {
       ui_action(ACTION_Down);
    }

    // Did we get back to where we started? Bail.
    if (menu_cursor[current_menu] == start_index) break;

    // We need to recompute max_index and the cursor after each move.
    ui_traverse();

    // Did this match our criteria? Bail.
    char *name = menu_cursor_item[current_menu]->name;
    if (name[0] != '\0' && tolower(name[0]) == letter) break;
  }
}

// Meant to be called immediately after a menu push to position
// the cursor to a known location. Also useful after a call to
// ui_to_top() to do the same.
void ui_set_cur_pos(int pos) {
  while(menu_cursor[current_menu] < pos &&
        menu_cursor[current_menu] < max_index[current_menu] - 1) {
    ui_action(ACTION_Down);

    // We need to recompute max_index and the cursor after each move.
    ui_traverse();
  }
}

struct menu_item* ui_find_item_by_id(struct menu_item *node, int id) {
  if (node == NULL) {
    return NULL;
  }

  while (node != NULL) {
    if (node->id == id) return node;
    if (node->type == FOLDER) {
       struct menu_item *found = ui_find_item_by_id(node->first_child, id);
       if (found) return found;
    }
    node = node->next;
  }

  return NULL;
}

void ui_enable_osd(void) {
  osd_active = 1;
  ui_enabled = 1;
  ui_make_transparent();
  circle_frames_ready_fbl(FB_LAYER_UI, -1 /* no 2nd layer */, 0 /* nosync */);
  circle_show_fbl(FB_LAYER_UI);
}

void ui_disable_osd(void) {
  osd_active = 0;
  // We don't set ui_enabled to 0 here. We rely on
  // pop and toggle to dismiss OSDs which does the
  // right thing.
  circle_hide_fbl(FB_LAYER_UI);
}

void ui_dismiss_osd_if_active(void) {
  if (osd_active) {
     ui_pop_all_and_toggle();
     ui_disable_osd();
  }
}

void ui_set_render_current_item_only(int v) {
  ui_render_current_item_only = v;
}

void emu_quick_func_interrupt(int button_assignment) {
  pending_emu_quick_func = button_assignment;
}

// These will revert back to 0 when the user moves off the
// current item.
void ui_canvas_reveal_temp(int layer) {
  if (layer == FB_LAYER_VIC && vic_showing) {
    ui_transparent = 1;
    ui_transparent_layer = layer;
    ui_set_render_current_item_only(1);
  }
  else if (layer == FB_LAYER_VDC && vdc_showing) {
    ui_transparent = 1;
    ui_transparent_layer = layer;
    ui_set_render_current_item_only(1);
  }
}

void emu_exit(void) {
  // We should never get here.  If we do, it's probably
  // because essential roms are missing.  So display a message
  // to that effect.
  int i;
  uint8_t *fb;
  int fb_pitch;
  int fb_width = 320;
  int fb_height = 240;

  circle_alloc_fbl(FB_LAYER_VIC, 0 /* indexed */, &fb,
                      fb_width, fb_height, &fb_pitch);
  circle_clear_fbl(FB_LAYER_VIC);
  circle_show_fbl(FB_LAYER_VIC);

  video_font = (uint8_t *)&font8x8_basic;
  for (i = 0; i < 256; ++i) {
    video_font_translate[i] = (8 * (i & 0x7f));
  }

  int x = 0;
  int y = 3;
  switch (emux_machine_class) {
    case BMC64_MACHINE_CLASS_VIC20:
      ui_draw_text_buf("VIC20 (Vice)", x, y, 1, fb, fb_pitch, 1);
      break;
    case BMC64_MACHINE_CLASS_C64:
      ui_draw_text_buf("C64 (Vice)", x, y, 1, fb, fb_pitch, 1);
      break;
    case BMC64_MACHINE_CLASS_C128:
      ui_draw_text_buf("C128 (Vice)", x, y, 1, fb, fb_pitch, 1);
      break;
    case BMC64_MACHINE_CLASS_PLUS4:
      ui_draw_text_buf("PLUS4 (Vice)", x, y, 1, fb, fb_pitch, 1);
      break;
    case BMC64_MACHINE_CLASS_PLUS4EMU:
      ui_draw_text_buf("PLUS4 (Plus4Emu)", x, y, 1, fb, fb_pitch, 1);
      break;
    case BMC64_MACHINE_CLASS_PET:
      ui_draw_text_buf("PET (Vice)", x, y, 1, fb, fb_pitch, 1);
      break;
  }
  y += 8;
  ui_draw_text_buf("Emulator failed to start.", x, y, 1, fb, fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("This most likely means you are missing", x, y, 1, fb,
                   fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("ROM files. Or you have specified an", x, y, 1, fb,
                   fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("invalid kernal, chargen or basic", x, y, 1, fb, fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("ROM.  See the documentation.", x, y, 1, fb,
                   fb_pitch, 1);

  if (emux_machine_class != BMC64_MACHINE_CLASS_C64) {
     y += 16;
     ui_draw_text_buf("Hold Ctrl/Commodore + F7 for 5 seconds,", x, y, 1, fb,
                   fb_pitch, 1);
     y += 8;
     ui_draw_text_buf("then release F7 to reset back to C64.", x, y, 1, fb,
                   fb_pitch, 1);
  }

  circle_set_palette_fbl(FB_LAYER_VIC, 0, COLOR16(0, 0, 0));
  circle_set_palette_fbl(FB_LAYER_VIC, 1, COLOR16(255, 255, 255));
  circle_update_palette_fbl(FB_LAYER_VIC);
  circle_frames_ready_fbl(FB_LAYER_VIC, -1, 0);
}

static void ui_update_children(struct menu_item *node,
                               int top, int left) {
  while (node != NULL) {
    node->menu_top = top;
    node->menu_left = left;

    if (node->type == FOLDER && node->first_child != NULL) {
      ui_update_children(node->first_child, top, left);
    }
    node = node->next;
  }
}

void ui_geometry_changed(int dpx, int dpy,
                         int fbw, int fbh,
                         int sw, int sh,
                         int dw, int dh) {

  // For the UI, we don't want to double the height like we do
  // with the actual display, so we take raster_skip into account
  // here.
  fbh = fbh / canvas_state[VIC_INDEX].raster_skip;

  // When the ui geometry changes, we need to update some menu
  // fields to match.
  if (fbw != ui_fb_w || fbh != ui_fb_h) {
     // Destroy old fb.
     if (ui_fb) {
        circle_free_fbl(FB_LAYER_UI);
     }

     circle_alloc_fbl(FB_LAYER_UI, 0 /* indexed */, &ui_fb,
                      fbw, fbh, &ui_fb_pitch);
     circle_clear_fbl(FB_LAYER_UI);
     ui_fb_w = fbw;
     ui_fb_h = fbh;
   }
   menu_roots[0].menu_top = calc_root_menu_top();
   menu_roots[0].menu_left = calc_root_menu_left();
   ui_update_children(&menu_roots[0],
      menu_roots[0].menu_top, menu_roots[0].menu_left);
}
