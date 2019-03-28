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

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "kbd.h"
#include "menu.h"
#include "interrupt.h"
#include "videoarch.h"
#include "circle.h"
#include "joy.h"

volatile int ui_activated = 0;

int ui_toggle_pending = 0;

extern struct joydev_config joydevs[2];

// Stubs for vice callbacks. Unimplemented for now.
void ui_pause_emulation(int flag) { }
int ui_emulation_is_paused(void) { return 0; }

// Width and height of our text menu in characters
const int menu_width_chars = 40;
const int menu_height_chars = 25;

// Stack of menu screens
int current_menu = -1;
struct menu_item menu_roots[NUM_MENU_ROOTS];

// Where is our cursor in the menu?
int menu_cursor[NUM_MENU_ROOTS];
struct menu_item* menu_cursor_item[NUM_MENU_ROOTS];

// Sliding window marking start and stop of what we're showing.
int menu_window_top[NUM_MENU_ROOTS];
int menu_window_bottom[NUM_MENU_ROOTS];

// The index of the last item + 1. Can't set cursor to this or higher.
int max_index[NUM_MENU_ROOTS];

int pending_ui_key_head = 0;
int pending_ui_key_tail = 0;
volatile long pending_ui_key[16];
volatile int pending_ui_key_pressed[16];

// Callback for events that happen on menu items
void (*on_value_changed)(struct menu_item*) = NULL;

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
   struct menu_item* root = ui_push_menu(-1, -1);
   build_menu(root);

   ui_key_action = ACTION_None;
   ui_key_ticks = 0;
   ui_key_ticks_next = 0;
   ui_key_ticks_repeats = 0;
   ui_key_ticks_repeats_next = 0;
}

// Draw a single character at x,y coords into the offscreen area
static void ui_draw_char(uint8_t c, int pos_x, int pos_y, int color) {
    int x, y;
    uint8_t fontchar;
    uint8_t *font_pos;
    uint8_t *draw_pos;

    // Draw into off screen buffer
    uint8_t *dst = video_state.dst +
        video_state.offscreen_buffer_y * video_state.dst_pitch;

    // Don't draw out of bounds
    if (pos_y < 0 || pos_y > video_state.scr_h - 8) { return; }
    if (pos_x < 0 || pos_x > video_state.scr_w - 8) { return; }

    font_pos = &(video_state.font[video_state.font_translate[(int)c]]);

    draw_pos = &(dst[pos_x + pos_y * video_state.dst_pitch]);

    for (y = 0; y < 8; ++y) {
        fontchar = *font_pos;
        for (x = 0; x < 8; ++x) {
          if (fontchar & (0x80 >> x)) {
            draw_pos[x] = color;
          }
        }
        ++font_pos;
        draw_pos += video_state.dst_pitch;
    }
}

// Draw a string of text at location x,y. Does not word wrap.
void ui_draw_text(const char* text, int x, int y, int color) {
   int i;
   for (i=0;i<strlen(text);i++) {
      ui_draw_char(text[i], x, y, color); x=x+8;
   }
}

// Draw a rectangle at x/y of given w/h into the offscreen area
void ui_draw_rect(int x,int y, int w, int h, int color, int fill) {
   int xx, yy, x2, y2;
   uint8_t *dst = video_state.dst +
      video_state.offscreen_buffer_y*video_state.dst_pitch;
   x2 = x + w;
   y2 = y + h;
   for(xx= x, yy=y; yy < y2; xx++){
      if(xx >= x2) {
         xx = x - 1; yy++;
      } else {
         int p1 = xx+yy*video_state.dst_pitch;
         if (fill | (yy==y || yy==(y2-1) || (xx == x) || xx == (x2-1))) {
            dst[p1]= color;
         }
      }
   }
}

// Returns the height/width the given text would occupy if drawn
int ui_text_width(const char* text) {
   return 8 * strlen(text);
}

static void do_on_value_changed(struct menu_item* item) {
   if (item->on_value_changed) {
      item->on_value_changed(menu_cursor_item[current_menu]);
   }
   else if (on_value_changed) {
      on_value_changed(menu_cursor_item[current_menu]);
   }
}

static void ui_type_char(char ch) {
   struct menu_item* cur = menu_cursor_item[current_menu];
   if (cur->type == TEXTFIELD) {
      if (ch == '\b') {
        if (cur->value <= 0) return;
        char *str = cur->str_value;
        memmove(str+cur->value-1, str+cur->value,
           (strlen(str) - cur->value + 1)*sizeof(char));
        cur->value--;
      } else {
        if (strlen(cur->str_value) >= MAX_FN_NAME) return;

        char *str = cur->str_value;
        memmove(str+cur->value+1, str+cur->value,
           (strlen(str) - cur->value + 1)*sizeof(char));
        str[cur->value] = ch;
        cur->value++;
      }
   }
}

// Happens on main loop.
static void ui_key_pressed(long key) {
  switch (key) {
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
     if (keyboard_shift) ch = 'A' + key - KEYCODE_a;
     else ch = 'a' + key - KEYCODE_a;
     ui_type_char(ch);
  }
  else if (key >= KEYCODE_1 && key <= KEYCODE_9) {
     char ch = '1' + key - KEYCODE_1;
     ui_type_char(ch);
  }
  else if (key == KEYCODE_0) {
     ui_type_char(0);
  }
  else if (key == KEYCODE_Dash) {
     if (keyboard_shift) ui_type_char('_');
     else ui_type_char('-');
  }
  else if (key == KEYCODE_Period) {
     ui_type_char('.');
  }
  else if (key == KEYCODE_Backspace) {
     ui_type_char('\b');
  }
}

// Happens on main loop. Process a key release for the ui.
static void ui_key_released(long key) {
  switch (key) {
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
    case KEYCODE_F7:
    case KEYCODE_F12:
       ui_action(ACTION_Exit);
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
            if (ui_key_ticks_next < 1) ui_key_ticks_next = 1;
         }
      }
   }
}

void ui_pop_all_and_toggle() {
   while (current_menu > 0) {
      ui_pop_menu();
   }
   ui_toggle();
}

static void ui_action(long action) {
   struct menu_item* cur = menu_cursor_item[current_menu];
   switch (action) {
      case ACTION_Up:
         menu_cursor[current_menu]--;
         if (menu_cursor[current_menu] < 0) { menu_cursor[current_menu] = 0; }
         if (menu_cursor[current_menu] <= (menu_window_top[current_menu]-1)) {
            menu_window_top[current_menu]--;
            menu_window_bottom[current_menu]--;
         }
         break;
      case ACTION_Down:
         menu_cursor[current_menu]++;
         if (menu_cursor[current_menu] >= max_index[current_menu]) {
            menu_cursor[current_menu] = max_index[current_menu] - 1;
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
         }
         else if (cur->type == MULTIPLE_CHOICE) {
            cur->value -= 1;
            if (cur->value < 0) {
               cur->value = cur->num_choices - 1;
            }
            do_on_value_changed(menu_cursor_item[current_menu]);
         } else if (cur->type == TOGGLE) {
            cur->value = 1-cur->value;
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
         }
         else if (cur->type == MULTIPLE_CHOICE) {
            cur->value += 1;
            if (cur->value >= cur->num_choices) {
               cur->value = 0;
            }
            do_on_value_changed(menu_cursor_item[current_menu]);
         } else if (cur->type == TOGGLE) {
            cur->value = 1-cur->value;
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
            cur->is_expanded = 1-cur->is_expanded;
            do_on_value_changed(menu_cursor_item[current_menu]);
         } else if (cur->type == CHECKBOX) {
            cur->value = 1-cur->value;
            do_on_value_changed(menu_cursor_item[current_menu]);
         } else if (cur->type == TOGGLE) {
            cur->value = 1-cur->value;
            do_on_value_changed(menu_cursor_item[current_menu]);
         } else if (cur->type == BUTTON) {
            do_on_value_changed(menu_cursor_item[current_menu]);
         } else if (cur->type == MULTIPLE_CHOICE) {
            cur->value += 1;
            if (cur->value >= cur->num_choices) {
               cur->value = 0;
            }
            do_on_value_changed(menu_cursor_item[current_menu]);
         } else if (cur->type == TEXTFIELD) {
            do_on_value_changed(menu_cursor_item[current_menu]);
         }
         break;
      case ACTION_Escape:
         if (current_menu > 0) {
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

static void ui_render_single_frame() {
   ui_render_now();
   videoarch_swap();
}

static void pause_trap(uint16_t addr, void *data) {
   while (ui_activated) {
      if (joydevs[0].device == JOYDEV_GPIO_0 || joydevs[1].device == JOYDEV_GPIO_0) {
         circle_poll_joysticks(0, 0);
      }
      if (joydevs[0].device == JOYDEV_GPIO_1 || joydevs[1].device == JOYDEV_GPIO_1) {
         circle_poll_joysticks(1, 0);
      }
      circle_check_gpio();
      ui_check_key();
      ui_render_single_frame();
      circle_wait_vsync();
   }
}

void ui_toggle(void) {
  ui_activated = 1 - ui_activated;
  if (ui_activated) {
     interrupt_maincpu_trigger_trap(pause_trap, 0);
  }
}

static void append(struct menu_item *folder, struct menu_item *new_item) {
   assert (folder->type == FOLDER);
   struct menu_item* prev = NULL;
   struct menu_item* ptr = folder->first_child;
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

static struct menu_item* ui_new_item(struct menu_item *parent,
                                     char* name, int id)  {
   struct menu_item* new_item = (struct menu_item*)
       malloc(sizeof(struct menu_item));
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

struct menu_item* ui_menu_add_toggle(int id, struct menu_item *folder,
                                     char* name, int initial_state) {
   struct menu_item* new_item = ui_new_item(folder, name,id);
   new_item->type = TOGGLE;
   new_item->value = initial_state;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_checkbox(int id, struct menu_item *folder, char* name, int initial_state) {
   struct menu_item* new_item = ui_new_item(folder, name,id);
   new_item->type = CHECKBOX;
   new_item->value = initial_state;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_multiple_choice(int id, struct menu_item *folder, char *name) {
   struct menu_item* new_item = ui_new_item(folder, name,id);
   new_item->type = MULTIPLE_CHOICE;
   new_item->num_choices = 0;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_button(int id, struct menu_item *folder, char *name) {
   ui_menu_add_button_with_value(id,folder,name,0," "," ");
}

struct menu_item* ui_menu_add_button_with_value(int id, struct menu_item *folder, char *name, int value, char* str_value, char* displayed_value) {
   struct menu_item* new_item = ui_new_item(folder, name,id);
   new_item->type = BUTTON;
   new_item->value = value;
   strncpy(new_item->str_value, str_value, 32);
   strncpy(new_item->displayed_value, displayed_value, 32);
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_range(int id, struct menu_item *folder, char *name, int min, int max, int step, int initial_value) {
   struct menu_item* new_item = ui_new_item(folder, name, id);
   new_item->type = RANGE;
   new_item->min = min;
   new_item->max = max;
   new_item->step = step;
   new_item->value = initial_value;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_folder(struct menu_item *folder, char *name) {
   struct menu_item* new_item = ui_new_item(folder, name, MENU_ID_DO_NOTHING);
   new_item->type = FOLDER;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_divider(struct menu_item *folder) {
   struct menu_item* new_item = ui_new_item(folder, "", MENU_ID_DO_NOTHING);
   new_item->type = DIVIDER;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_text_field(int id, struct menu_item *folder, char *name, char *value) {
   struct menu_item* new_item = ui_new_item(folder, name, id);
   new_item->type = TEXTFIELD;
   new_item->value = strlen(new_item->str_value);
   strcpy(new_item->str_value, value);
   append(folder, new_item);
   return new_item;
}

static void ui_render_children(struct menu_item* node, int* index, int indent) {
   while (node != NULL) {
      node->render_index = *index;

      // Render a row
      if (*index >= menu_window_top[current_menu] && *index < menu_window_bottom[current_menu]) {
         int y = (*index - menu_window_top[current_menu]) * 8 + node->menu_top;
         if (*index == menu_cursor[current_menu]) {
             ui_draw_rect(node->menu_left, y, node->menu_width, 8, 2, 1);
             menu_cursor_item[current_menu] = node;
         }

         ui_draw_text(node->name, node->menu_left + (indent+1) * 8, y, 1);
         if (node->type == FOLDER) {
            if (node->is_expanded)
               ui_draw_text("-", node->menu_left + (indent) * 8, y, 1);
            else
               ui_draw_text("+", node->menu_left + (indent) * 8, y, 1);
         } else if (node->type == TOGGLE) {
            if (node->value)
               ui_draw_text("On", node->menu_left + node->menu_width -
                         ui_text_width("On"), y, 1);
            else
               ui_draw_text("Off", node->menu_left + node->menu_width -
                         ui_text_width("Off"), y, 1);
         } else if (node->type == CHECKBOX) {
            if (node->value)
               ui_draw_text("True", node->menu_left + node->menu_width -
                         ui_text_width("True"), y, 1);
            else
               ui_draw_text("False", node->menu_left + node->menu_width -
                         ui_text_width("False"), y, 1);
         } else if (node->type == RANGE) {
            sprintf(node->scratch,"%d",node->value);
            ui_draw_text(node->scratch, node->menu_left + node->menu_width -
                         ui_text_width(node->scratch), y, 1);
         } else if (node->type == MULTIPLE_CHOICE) {
            ui_draw_text(node->choices[node->value], node->menu_left + node->menu_width -
                         ui_text_width(node->choices[node->value]), y, 1);
         } else if (node->type == DIVIDER) {
            ui_draw_rect(node->menu_left, y+3, node->menu_width, 2, 3, 1);
         } else if (node->type == BUTTON) {
            if (strlen(node->displayed_value) > 0) {
               // Prefer displayed value if set
               ui_draw_text(node->displayed_value, node->menu_left + node->menu_width -
                         ui_text_width(node->displayed_value), y, 1);
            } else {
               // Turn value into string as fallback
               if (node->map_value_func) {
                  sprintf(node->scratch,"%d",node->map_value_func(node->value));
                  ui_draw_text(node->scratch, node->menu_left + node->menu_width -
                         ui_text_width(node->scratch), y, 1);
               } else {
                  sprintf(node->scratch,"%d",node->value);
                  ui_draw_text(node->scratch, node->menu_left + node->menu_width -
                         ui_text_width(node->scratch), y, 1);
               }
            }
         } else if (node->type == TEXTFIELD) {
            // draw cursor underneath text
            ui_draw_rect(node->menu_left + ui_text_width(node->name) + 8 + node->value * 8,
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
  struct menu_item* ptr = menu_roots[current_menu].first_child;

  // black background
  ui_draw_rect(ptr->menu_left, ptr->menu_top,
               ptr->menu_width, ptr->menu_height, 0, 1);
  // border
  ui_draw_rect(ptr->menu_left-1, ptr->menu_top-1,
               ptr->menu_width+2, ptr->menu_height+2, 3, 0);

  // menu text
  ui_render_children(ptr,&index, indent);

  max_index[current_menu] = index;

  if (menu_cursor[current_menu] >= max_index[current_menu]) {
     menu_cursor[current_menu] = max_index[current_menu] - 1;
  }
}

static void ui_clear_child_menu(struct menu_item* node) {
   if (node != NULL && node->type == FOLDER) {
     ui_clear_child_menu(node->first_child);
   }

   while (node != NULL) {
      struct menu_item* next = node->next;
      free(node);
      node = next;
   }
}

static void ui_clear_menu(int menu_index) {
   struct menu_item* node = &menu_roots[menu_index];
   ui_clear_child_menu(node->first_child);
   node->first_child = NULL;
}

struct menu_item* ui_pop_menu(void) {
  if (menu_roots[current_menu].on_value_changed) {
     // Notify pop happened
     menu_roots[current_menu].on_value_changed(&menu_roots[current_menu]);
  }
  ui_clear_menu(current_menu);
  current_menu--;
  if (current_menu < 0) {
     printf ("FATAL ERROR: tried to pop last menu\n");
     return NULL;
  }
  return &menu_roots[current_menu];
}

struct menu_item* ui_push_menu(int w_chars, int h_chars) {

  if (w_chars == -1) w_chars = menu_width_chars;
  if (h_chars == -1) h_chars = menu_height_chars;

  current_menu++;
  if (current_menu >= NUM_MENU_ROOTS) {
     printf ("FATAL ERROR: tried to push menu beyond NUM_MENU_ROOTS\n");
     return NULL;
  }
  ui_clear_menu(current_menu);

  // Client must set callback on each push so clear here.
  menu_roots[current_menu].on_value_changed = NULL;

  // Set dimensions
  int menu_width = w_chars * 8;
  int menu_height = h_chars * 8;
  menu_roots[current_menu].menu_width = menu_width;
  menu_roots[current_menu].menu_height = menu_height;
  menu_roots[current_menu].menu_left = (video_state.scr_w - menu_width) / 2;
  menu_roots[current_menu].menu_top = (video_state.scr_h - menu_height) / 2;

  menu_cursor[current_menu] = 0;
  menu_window_top[current_menu] = 0;
  menu_window_bottom[current_menu] = h_chars;

  return &menu_roots[current_menu];
}

void ui_set_on_value_changed_callback(void (*callback)(struct menu_item*)) {
   on_value_changed = callback;
}

int circle_ui_activated(void) {
   return ui_activated;
}

static struct menu_item* ui_push_dialog_header(int is_error) {
   if (!ui_activated) {
      ui_toggle();
   }

   struct menu_item* root = ui_push_menu(30, 4);
   if (is_error) {
      ui_menu_add_button(MENU_ERROR_DIALOG, root, "Error");
   } else {
      ui_menu_add_button(MENU_INFO_DIALOG, root, "Info");
   }
   ui_menu_add_divider(root);
   return root;
}

void ui_error(const char *format, ...) {
   struct menu_item* root = ui_push_dialog_header(1);
   char buffer[256];
   va_list args;
   va_start (args, format);
   vsnprintf (buffer, 255, format, args);
   ui_menu_add_button(MENU_ERROR_DIALOG, root, buffer);
   va_end (args);
   ui_render_single_frame();
}

void ui_info(const char *format, ...) {
   struct menu_item* root = ui_push_dialog_header(0);
   char buffer[256];
   va_list args;
   va_start (args, format);
   vsnprintf (buffer, 255, format, args);
   ui_menu_add_button(MENU_INFO_DIALOG, root, buffer);
   va_end (args);
   ui_render_single_frame();
}

