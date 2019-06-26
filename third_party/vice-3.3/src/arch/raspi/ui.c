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

#include "circle.h"
#include "interrupt.h"
#include "joy.h"
#include "kbd.h"
#include "menu.h"
#include "menu_timing.h"
#include "videoarch.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

volatile int ui_activated;

int ui_ctrl_down;

// Countdown to toggle menu on/off
int ui_toggle_pending;

// One of the quick functions that can be invoked by button assignments
int pending_emu_quick_func;

extern struct joydev_config joydevs[MAX_JOY_PORTS];

// Stubs for vice callbacks. Unimplemented for now.
void ui_pause_emulation(int flag) {}
int ui_emulation_is_paused(void) { return 0; }

// Width and height of our text menu in characters
const int menu_width_chars = 40;
const int menu_height_chars = 25;

// Stack of menu screens
int current_menu = -1;
struct menu_item menu_roots[NUM_MENU_ROOTS];

// Where is our cursor in the menu?
int menu_cursor[NUM_MENU_ROOTS];
struct menu_item *menu_cursor_item[NUM_MENU_ROOTS];

// Sliding window marking start and stop of what we're showing.
int menu_window_top[NUM_MENU_ROOTS];
int menu_window_bottom[NUM_MENU_ROOTS];

// The index of the last item + 1. Can't set cursor to this or higher.
int max_index[NUM_MENU_ROOTS];

int pending_ui_key_head = 0;
int pending_ui_key_tail = 0;
volatile long pending_ui_key[16];
volatile int pending_ui_key_pressed[16];

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

void ui_init_menu(void) {
  int i;

  ui_activated = 0;
  current_menu = -1;

  // Init menu roots
  for (i = 0; i < NUM_MENU_ROOTS; i++) {
    memset(&menu_roots[i], 0, sizeof(struct menu_item));
    menu_roots[i].type = FOLDER;
    menu_roots[i].is_expanded = 1;
    strncpy(menu_roots[i].name, "", MAX_MENU_STR);
  }

  // Root menu is never popped
  struct menu_item *root = ui_push_menu(-1, -1);
  build_menu(root);

  ui_key_action = ACTION_None;
  ui_key_ticks = 0;
  ui_key_ticks_next = 0;
  ui_key_ticks_repeats = 0;
  ui_key_ticks_repeats_next = 0;
}

// Draw a single character at x,y coords into the offscreen area
static void ui_draw_char(uint8_t c, int pos_x, int pos_y, int color,
                         uint8_t *dst, int dst_pitch) {
  int x, y;
  uint8_t fontchar;
  uint8_t *font_pos;
  uint8_t *draw_pos;

  // Destination is main frame buffer if not specified.
  if (dst == NULL) {
    // Always draw into off screen buffer.
    dst_pitch = video_state.dst_pitch;
    dst = video_state.dst + video_state.offscreen_buffer_y * dst_pitch;
  }

  // Don't draw out of bounds
  if (pos_y < 0 || pos_y > video_state.fb_h - 8) {
    return;
  }
  if (pos_x < 0 || pos_x > video_state.fb_w - 8) {
    return;
  }

  font_pos = &(video_state.font[video_state.font_translate[c]]);
  draw_pos = &(dst[pos_x + pos_y * dst_pitch]);

  for (y = 0; y < 8; ++y) {
    fontchar = *font_pos;
    for (x = 0; x < 8; ++x) {
      if (fontchar & (0x80 >> x)) {
        draw_pos[x] = color;
      }
    }
    ++font_pos;
    draw_pos += dst_pitch;
  }
}

// Draw a string of text at location x,y. Does not word wrap.
void ui_draw_text_buf(const char *text, int x, int y, int color, uint8_t *dst,
                      int dst_pitch) {
  int i;
  int x2 = x;
  for (i = 0; i < strlen(text); i++) {
    if (text[i] == '\n') {
      y = y + 8;
      x2 = x;
    } else {
      ui_draw_char(text[i], x2, y, color, dst, dst_pitch);
      x2 = x2 + 8;
    }
  }
}

void ui_draw_text(const char *text, int x, int y, int color) {
  ui_draw_text_buf(text, x, y, color, NULL, 0);
}

// Draw a rectangle at x/y of given w/h into the offscreen area
void ui_draw_rect_buf(int x, int y, int w, int h, int color, int fill,
                      uint8_t *dst, int dst_pitch) {
  int xx, yy, x2, y2;

  // Destination is main frame buffer if not specified.
  if (dst == NULL) {
    dst_pitch = video_state.dst_pitch;
    dst = video_state.dst + video_state.offscreen_buffer_y * dst_pitch;
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
  switch (key) {
  case KEYCODE_LeftControl:
    ui_ctrl_down = 1;
    return;
  case KEYCODE_Up:
    ui_key_action = ACTION_Up;
    ui_key_ticks = INITIAL_ACTION_DELAY;
    ui_key_ticks_next = INITIAL_ACTION_REPEAT_DELAY;
    ui_key_ticks_repeats = 0;
    ui_key_ticks_repeats_next = 8;
    ui_action(ACTION_Up);
    return;
  case KEYCODE_Down:
    ui_key_action = ACTION_Down;
    ui_key_ticks = INITIAL_ACTION_DELAY;
    ui_key_ticks_next = INITIAL_ACTION_REPEAT_DELAY;
    ui_key_ticks_repeats = 0;
    ui_key_ticks_repeats_next = 8;
    ui_action(ACTION_Down);
    return;
  case KEYCODE_Left:
    ui_key_action = ACTION_Left;
    ui_key_ticks = INITIAL_ACTION_DELAY;
    ui_key_ticks_next = INITIAL_ACTION_REPEAT_DELAY;
    ui_key_ticks_repeats = 0;
    ui_key_ticks_repeats_next = 8;
    ui_action(ACTION_Left);
    return;
  case KEYCODE_Right:
    ui_key_action = ACTION_Right;
    ui_key_ticks = INITIAL_ACTION_DELAY;
    ui_key_ticks_next = INITIAL_ACTION_REPEAT_DELAY;
    ui_key_ticks_repeats = 0;
    ui_key_ticks_repeats_next = 8;
    ui_action(ACTION_Right);
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
  switch (key) {
  case KEYCODE_LeftControl:
    ui_ctrl_down = 0;
    return;
  case KEYCODE_Up:
  case KEYCODE_Down:
  case KEYCODE_Left:
  case KEYCODE_Right:
    ui_key_action = ACTION_None;
    return;
  case KEYCODE_Return:
    ui_action(ACTION_Return);
    return;
  case KEYCODE_Escape:
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
    if (!ui_ctrl_down) ui_to_top();
    return;
  case KEYCODE_End:
  case KEYCODE_F7:
    if (!ui_ctrl_down) ui_to_bottom();
    return;
  case KEYCODE_PageUp:
  case KEYCODE_F3:
    if (!ui_ctrl_down) ui_page_up();
    return;
  case KEYCODE_PageDown:
  case KEYCODE_F5:
    if (!ui_ctrl_down) ui_page_down();
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
        if (ui_key_ticks_next < 1)
          ui_key_ticks_next = 1;
      }
    }
  }
}

static void ui_render_single_frame() {
  ui_render_now();
  videoarch_swap();
}

static void pause_trap(uint16_t addr, void *data) {
  menu_about_to_activate();
  while (ui_activated) {
    circle_check_gpio();
    ui_check_key();

    ui_handle_toggle_or_quick_func();

    ui_render_single_frame();
    circle_wait_vsync();
    hdmi_timing_hook();
  }
  menu_about_to_deactivate();
}

static void ui_toggle(void) {
  ui_activated = 1 - ui_activated;
  if (ui_activated) {
    interrupt_maincpu_trigger_trap(pause_trap, 0);
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
    if (cur->type == RANGE) {
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
      while (cur->choice_disabled[cur->value] && cur->value != orig) {
        cur->value -= 1;
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
    if (cur->type == RANGE) {
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
        cur->value += 1;
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

// queue a key for press/release on the main loop
void circle_ui_key_interrupt(long key, int pressed) {
  circle_lock_acquire();
  int i = pending_ui_key_tail & 0xf;
  pending_ui_key[i] = key;
  pending_ui_key_pressed[i] = pressed;
  pending_ui_key_tail++;
  circle_lock_release();
}

// Do key press/releases on the main loop
void ui_check_key(void) {
  if (!ui_activated) {
    return;
  }

  // Process ui key event queue
  circle_lock_acquire();
  while (pending_ui_key_head != pending_ui_key_tail) {
    int i = pending_ui_key_head & 0xf;
    if (pending_ui_key_pressed[i]) {
      ui_key_pressed(pending_ui_key[i]);
    } else {
      ui_key_released(pending_ui_key[i]);
    }
    pending_ui_key_head++;
  }
  circle_lock_release();

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
  if (strlen(node->displayed_value) > 0) {
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

static struct menu_item *ui_new_item(struct menu_item *parent, char *name,
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
                                     char *name) {
  return ui_menu_add_button_with_value(id, folder, name, 0, " ", " ");
}

struct menu_item *ui_menu_add_button_with_value(int id,
                                                struct menu_item *folder,
                                                char *name, int value,
                                                char *str_value,
                                                char *displayed_value) {
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

static void ui_render_children(struct menu_item *node, int *index, int indent) {
  while (node != NULL) {
    node->render_index = *index;

    // Render a row
    if (*index >= menu_window_top[current_menu] &&
        *index < menu_window_bottom[current_menu]) {
      int y = (*index - menu_window_top[current_menu]) * 8 + node->menu_top;
      if (*index == menu_cursor[current_menu]) {
        ui_draw_rect(node->menu_left, y, node->menu_width, 8, 2, 1);
        menu_cursor_item[current_menu] = node;
      }

      ui_draw_text(node->name, node->menu_left + (indent + 1) * 8, y, 1);
      if (node->type == FOLDER) {
        if (node->is_expanded)
          ui_draw_text("-", node->menu_left + (indent)*8, y, 1);
        else
          ui_draw_text("+", node->menu_left + (indent)*8, y, 1);
      } else if (node->type == TOGGLE) {
        if (node->value)
          ui_draw_text("On",
                       node->menu_left + node->menu_width - ui_text_width("On"),
                       y, 1);
        else
          ui_draw_text("Off", node->menu_left + node->menu_width -
                                  ui_text_width("Off"),
                       y, 1);
      } else if (node->type == CHECKBOX) {
        if (node->value)
          ui_draw_text("True", node->menu_left + node->menu_width -
                                   ui_text_width("True"),
                       y, 1);
        else
          ui_draw_text("False", node->menu_left + node->menu_width -
                                    ui_text_width("False"),
                       y, 1);
      } else if (node->type == RANGE) {
        sprintf(node->scratch, "%d", node->value);
        ui_draw_text(node->scratch, node->menu_left + node->menu_width -
                                        ui_text_width(node->scratch),
                     y, 1);
      } else if (node->type == MULTIPLE_CHOICE) {
        ui_draw_text(node->choices[node->value],
                     node->menu_left + node->menu_width -
                         ui_text_width(node->choices[node->value]),
                     y, 1);
      } else if (node->type == DIVIDER) {
        ui_draw_rect(node->menu_left, y + 3, node->menu_width, 2, 3, 1);
      } else if (node->type == BUTTON) {
        char *dsp_string = get_button_display_str(node);
        ui_draw_text(dsp_string, node->menu_left + node->menu_width -
                                     ui_text_width(dsp_string),
                     y, 1);
      } else if (node->type == TEXTFIELD) {
        // draw cursor underneath text
        ui_draw_rect(node->menu_left + ui_text_width(node->name) + 8 +
                         node->value * 8,
                     y, 8, 8, 3, 1);
        ui_draw_text(node->str_value,
                     node->menu_left + ui_text_width(node->name) + 8, y, 1);
      }
    }

    *index = *index + 1;
    if (node->type == FOLDER && node->is_expanded &&
        node->first_child != NULL) {
      ui_render_children(node->first_child, index, indent + 1);
    }
    node = node->next;
  }
}

void ui_render_now(void) {
  int index = 0;
  int indent = 0;
  struct menu_item *ptr = menu_roots[current_menu].first_child;

  // black background
  ui_draw_rect(ptr->menu_left, ptr->menu_top, ptr->menu_width, ptr->menu_height,
               0, 1);
  // border
  ui_draw_rect(ptr->menu_left - 1, ptr->menu_top - 1, ptr->menu_width + 2,
               ptr->menu_height + 2, 3, 0);

  // menu text
  ui_render_children(ptr, &index, indent);

  max_index[current_menu] = index;

  if (menu_cursor[current_menu] >= max_index[current_menu]) {
    menu_cursor[current_menu] = max_index[current_menu] - 1;
    cursor_pos_updated();
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

struct menu_item *ui_push_menu(int w_chars, int h_chars) {

  if (w_chars == -1)
    w_chars = menu_width_chars;
  if (h_chars == -1)
    h_chars = menu_height_chars;

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
  int menu_width = w_chars * 8;
  int menu_height = h_chars * 8;
  menu_roots[current_menu].menu_width = menu_width;
  menu_roots[current_menu].menu_height = menu_height;
  menu_roots[current_menu].menu_left = (video_state.fb_w - menu_width) / 2;
  menu_roots[current_menu].menu_top = (video_state.fb_h - menu_height) / 2;

  menu_cursor[current_menu] = 0;
  menu_window_top[current_menu] = 0;
  menu_window_bottom[current_menu] = h_chars;

  return &menu_roots[current_menu];
}

void ui_set_on_value_changed_callback(void (*callback)(struct menu_item *)) {
  on_value_changed = callback;
}

int circle_ui_activated(void) { return ui_activated; }

static struct menu_item *ui_push_dialog_header(int is_error) {
  if (!ui_activated) {
    ui_toggle();
  }

  struct menu_item *root = ui_push_menu(30, 4);
  if (is_error) {
    ui_menu_add_button(MENU_ERROR_DIALOG, root, "Error");
  } else {
    ui_menu_add_button(MENU_INFO_DIALOG, root, "Info");
  }
  ui_menu_add_divider(root);
  return root;
}

void ui_error(const char *format, ...) {
  struct menu_item *root = ui_push_dialog_header(1);
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
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, 255, format, args);
  ui_menu_add_button(MENU_INFO_DIALOG, root, buffer);
  va_end(args);
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
