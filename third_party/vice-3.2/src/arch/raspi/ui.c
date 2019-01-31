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
#include "kbd.h"
#include "menu.h"

volatile int ui_activated = 0;

// Stubs for vice callbacks. Unimplemented for now.
void ui_pause_emulation(int flag) { }
int ui_emulation_is_paused(void) { return 0; }

// Width and height of our text menu in characters
const int menu_width_chars = 36;
const int menu_height_chars = 20;

int menu_width;
int menu_height;

// Computed top left coordinates of where our menu will begin drawing
// Menu will be centered in frame buffer
int menu_top;
int menu_left;

// We have two menus to use. One for settings, the other for file dialogs.
struct menu_item root_menu;
struct menu_item file_menu;

int current_menu;
struct menu_item* menus[2];

// Where is our cursor in the menu?
int menu_cursor[2] = {0,0};
struct menu_item* menu_cursor_item[2];

// Sliding window marking start and stop of what we're showing.
int menu_window_top[2];
int menu_window_bottom[2];

// The index of the last item + 1. Can't set cursor to this or higher.
int max_index[2];

// Callback for events that happen on menu items
void (*on_value_changed)(struct menu_item*) = NULL;

volatile long pending_ui_key = KEYCODE_NONE;
volatile long pending_ui_key_time = 0;

void ui_init_menu(void) {
   int i;
   memset(&root_menu, 0, sizeof(struct menu_item));
   root_menu.type = FOLDER;
   root_menu.is_expanded = 1;
   strncpy(root_menu.name, "Root", MAX_MENU_STR);

   memset(&file_menu, 0, sizeof(struct menu_item));
   file_menu.type = FOLDER;
   file_menu.is_expanded = 1;
   strncpy(file_menu.name, "File", MAX_MENU_STR);

   build_menu();

   menus[0] = &root_menu;
   menus[1] = &file_menu;

   ui_switch_menu(0);

   menu_width = menu_width_chars * 8;
   menu_height = menu_height_chars * 8;

   // sliding window
   menu_window_top[current_menu] = 0;
   menu_window_bottom[current_menu] = menu_height_chars;
   menu_cursor[0] = 0;
   menu_cursor[1] = 0;

   menu_left = (video_state.scr_w - menu_width) / 2;
   menu_top = (video_state.scr_h - menu_height) / 2;
}

// Draw a single character at x,y coords
static void ui_draw_char(uint8_t c, int pos_x, int pos_y, int color) {
    int x, y;
    uint8_t fontchar;
    uint8_t *font_pos;
    uint8_t *draw_pos;

    // Draw into off screen buffer
    uint8_t *dst = video_state.dst + video_state.offscreen_buffer_y * video_state.dst_pitch;

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

// Draw a rectangle at x/y of given w/h
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
    return 8* strlen(text);
}

static void ui_key(long key) {
   switch (key) {
      case KEYCODE_Up:
         menu_cursor[current_menu]--;
         if (menu_cursor[current_menu] < 0) { menu_cursor[current_menu] = 0; }
         if (menu_cursor[current_menu] <= (menu_window_top[current_menu]-1)) {
            menu_window_top[current_menu]--;
            menu_window_bottom[current_menu]--;
         }
         break;
      case KEYCODE_Down:
         menu_cursor[current_menu]++;
         if (menu_cursor[current_menu] >= max_index[current_menu]) {
            menu_cursor[current_menu] = max_index[current_menu] - 1;
         }
         if (menu_cursor[current_menu] >= menu_window_bottom[current_menu]) {
            menu_window_top[current_menu]++;
            menu_window_bottom[current_menu]++;
         }
         break;
      case KEYCODE_Left:
         if (menu_cursor_item[current_menu]->type == RANGE) {
            menu_cursor_item[current_menu]->value -= menu_cursor_item[current_menu]->step;
            if (menu_cursor_item[current_menu]->value < menu_cursor_item[current_menu]->min) {
                menu_cursor_item[current_menu]->value = menu_cursor_item[current_menu]->min;
            } else if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         }
         else if (menu_cursor_item[current_menu]->type == MULTIPLE_CHOICE) {
            menu_cursor_item[current_menu]->value -= 1;
            if (menu_cursor_item[current_menu]->value < 0) {
               menu_cursor_item[current_menu]->value = menu_cursor_item[current_menu]->num_choices - 1;
            }
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         } else if (menu_cursor_item[current_menu]->type == TOGGLE) {
            menu_cursor_item[current_menu]->value = 1-menu_cursor_item[current_menu]->value;
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         }
         break;
      case KEYCODE_Right:
         if (menu_cursor_item[current_menu]->type == RANGE) {
            menu_cursor_item[current_menu]->value += menu_cursor_item[current_menu]->step;
            if (menu_cursor_item[current_menu]->value > menu_cursor_item[current_menu]->max) {
                menu_cursor_item[current_menu]->value = menu_cursor_item[current_menu]->max;
            } else if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         }
         else if (menu_cursor_item[current_menu]->type == MULTIPLE_CHOICE) {
            menu_cursor_item[current_menu]->value += 1;
            if (menu_cursor_item[current_menu]->value >= menu_cursor_item[current_menu]->num_choices) {
               menu_cursor_item[current_menu]->value = 0;
            }
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         } else if (menu_cursor_item[current_menu]->type == TOGGLE) {
            menu_cursor_item[current_menu]->value = 1-menu_cursor_item[current_menu]->value;
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         }
         break;
      case KEYCODE_Return:
         if (menu_cursor_item[current_menu]->type == FOLDER) {
            menu_cursor_item[current_menu]->is_expanded = 1-menu_cursor_item[current_menu]->is_expanded;
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         } else if (menu_cursor_item[current_menu]->type == CHECKBOX) {
            menu_cursor_item[current_menu]->value = 1-menu_cursor_item[current_menu]->value;
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         } else if (menu_cursor_item[current_menu]->type == TOGGLE) {
            menu_cursor_item[current_menu]->value = 1-menu_cursor_item[current_menu]->value;
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         } else if (menu_cursor_item[current_menu]->type == BUTTON) {
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         } else if (menu_cursor_item[current_menu]->type == MULTIPLE_CHOICE) {
            menu_cursor_item[current_menu]->value += 1;
            if (menu_cursor_item[current_menu]->value >= menu_cursor_item[current_menu]->num_choices) {
               menu_cursor_item[current_menu]->value = 0;
            }
            if (on_value_changed) {
                on_value_changed(menu_cursor_item[current_menu]);
            }
         }
         break;
      case KEYCODE_Escape:
      case KEYCODE_F12:
         if (current_menu == 1) {
            ui_switch_menu(0);
            ui_clear_menu(1);
         } else {
            ui_toggle();
         }
         break;
   }
}

// We can't handle keys that want to call back into vice
// on an interrupt so just capture what the key was and
// pick it up and service it on the main loop
void circle_ui_key_interrupt(long key) {
   if (circle_get_ticks() - pending_ui_key_time > 200000) {
      pending_ui_key = key;
      pending_ui_key_time = circle_get_ticks();
   }
}

void ui_check_key(void) {
   if (pending_ui_key != KEYCODE_NONE) {
      ui_key(pending_ui_key);
      pending_ui_key = KEYCODE_NONE;
   }
}

void ui_toggle(void) {
  ui_activated = 1 - ui_activated;
}

void ui_switch_menu(int menu) {
   current_menu = menu;
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

static struct menu_item* ui_new_item(char* name, int id)  {
   struct menu_item* new_item = (struct menu_item*)
       malloc(sizeof(struct menu_item));
   memset(new_item, 0, sizeof(struct menu_item));
   strncpy(new_item->name, name, MAX_MENU_STR);
   new_item->id = id;
   return new_item;
}

struct menu_item* ui_menu_add_toggle(int id, struct menu_item *folder,
                                     char* name, int initial_state) {
   struct menu_item* new_item = ui_new_item(name,id);
   new_item->type = TOGGLE;
   new_item->value = initial_state;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_checkbox(int id, struct menu_item *folder, char* name, int initial_state) {
   struct menu_item* new_item = ui_new_item(name,id);
   new_item->type = CHECKBOX;
   new_item->value = initial_state;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_multiple_choice(int id, struct menu_item *folder, char *name) {
   struct menu_item* new_item = ui_new_item(name,id);
   new_item->type = MULTIPLE_CHOICE;
   new_item->num_choices = 0;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_button(int id, struct menu_item *folder, char *name) {
   ui_menu_add_button_with_value(id,folder,name,0,"","");
}

struct menu_item* ui_menu_add_button_with_value(int id, struct menu_item *folder, char *name, int int_value, char* str_value, char* displayed_value) {
   struct menu_item* new_item = ui_new_item(name,id);
   new_item->type = BUTTON;
   new_item->int_value = int_value;
   strncpy(new_item->str_value, str_value, 32);
   strncpy(new_item->displayed_value, displayed_value, 32);
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_range(int id, struct menu_item *folder, char *name, int min, int max, int step, int initial_value) {
   struct menu_item* new_item = ui_new_item(name,id);
   new_item->type = RANGE;
   new_item->min = min;
   new_item->max = max;
   new_item->step = step;
   new_item->value = initial_value;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_folder(struct menu_item *folder, char *name) {
   struct menu_item* new_item = ui_new_item(name, MENU_ID_DO_NOTHING);
   new_item->type = FOLDER;
   append(folder, new_item);
   return new_item;
}

struct menu_item* ui_menu_add_divider(struct menu_item *folder) {
   struct menu_item* new_item = ui_new_item("", MENU_ID_DO_NOTHING);
   new_item->type = DIVIDER;
   append(folder, new_item);
   return new_item;
}

static void ui_render_children(struct menu_item* node, int* index, int indent) {
   while (node != NULL) {
      node->render_index = *index;

      // Render a row
      if (*index >= menu_window_top[current_menu] && *index < menu_window_bottom[current_menu]) {
         int y = (*index - menu_window_top[current_menu]) * 8 + menu_top;
         if (*index == menu_cursor[current_menu]) {
             ui_draw_rect(menu_left, y, menu_width, 8, 2, 1);
             menu_cursor_item[current_menu] = node;
         }

         ui_draw_text(node->name, menu_left + (indent+1) * 8, y, 1);
         if (node->type == FOLDER) {
            if (node->is_expanded)
               ui_draw_text("-", menu_left + (indent) * 8, y, 1);
            else
               ui_draw_text("+", menu_left + (indent) * 8, y, 1);
         } else if (node->type == TOGGLE) {
            if (node->value)
               ui_draw_text("On", menu_left + menu_width - 
                         ui_text_width("On"), y, 1);
            else
               ui_draw_text("Off", menu_left + menu_width - 
                         ui_text_width("Off"), y, 1);
         } else if (node->type == CHECKBOX) {
            if (node->value)
               ui_draw_text("True", menu_left + menu_width - 
                         ui_text_width("True"), y, 1);
            else
               ui_draw_text("False", menu_left + menu_width - 
                         ui_text_width("False"), y, 1);
         } else if (node->type == RANGE) {
            sprintf(node->scratch,"%d",node->value);
            ui_draw_text(node->scratch, menu_left + menu_width - 
                         ui_text_width(node->scratch), y, 1);
         } else if (node->type == MULTIPLE_CHOICE) {
            ui_draw_text(node->choices[node->value], menu_left + menu_width - 
                         ui_text_width(node->choices[node->value]), y, 1);
         } else if (node->type == DIVIDER) {
            ui_draw_rect(menu_left, y+3, menu_width, 2, 3, 1);
         } else if (node->type == BUTTON && strlen(node->displayed_value) > 0) {
            ui_draw_text(node->displayed_value, menu_left + menu_width - 
                         ui_text_width(node->displayed_value), y, 1);
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
  struct menu_item* ptr = menus[current_menu]->first_child;

  ui_draw_rect(menu_left, menu_top, menu_width, menu_height, 0, 1);
  ui_render_children(ptr,&index, indent);
  ui_draw_rect(menu_left, menu_top, menu_width, menu_height, 3, 0);

  max_index[current_menu] = index;

  if (menu_cursor[current_menu] >= max_index[current_menu]) { menu_cursor[current_menu] = max_index[current_menu] - 1; }
}

struct menu_item* ui_get_root_menu(void) {
  return &root_menu;
}

struct menu_item* ui_get_file_menu(void) {
  return &file_menu;
}

void ui_set_on_value_changed_callback(void (*callback)(struct menu_item*)) {
   on_value_changed = callback;
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

void ui_clear_menu(int menu_index) {
   struct menu_item* node = menus[menu_index];
   ui_clear_child_menu(node->first_child);
   node->first_child = NULL;
   menu_window_top[menu_index] = 0;
   menu_window_bottom[menu_index] = menu_height_chars;
   menu_cursor[menu_index] = 0;
}

int circle_ui_activated(void) {
   return ui_activated;
}
