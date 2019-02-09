/*
 * ui.h
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

#ifndef VICE_UI_H_
#define VICE_UI_H_

#include "videoarch.h"

#define NUM_MENU_ROOTS 5
#define MAX_CHOICES    16
#define MAX_MENU_STR   36

// Special menu id for items that do nothing or have no action callback
#define MENU_ID_DO_NOTHING -1 

typedef enum menu_item_type {
   TOGGLE,          // true/false
   CHECKBOX,        // on/off
   MULTIPLE_CHOICE, // one selection among a list of options
   BUTTON,          // an action with optional displayable value to hold
   RANGE,           // something with a min, max and step
   FOLDER,          // contains sub-items/folders
   DIVIDER
} menu_item_type;

struct menu_item {
   // Client defined id.
   int id;

   menu_item_type type;

   // For all
   char name[MAX_MENU_STR];

   // 0/1 for TOGGLE or CHECKBOX, or range value for RANGE
   // index for multple choice
   int value;

   // For MULTIPLE_CHOICE
   int num_choices;
   char choices[MAX_CHOICES][MAX_MENU_STR];
   int choice_ints[MAX_CHOICES];

   // For RANGE
   int min;
   int max;
   int step;

   // For FOLDER
   int is_expanded;
   struct menu_item* first_child;

   // For all
   struct menu_item* next;

   // Always changing, not for external use.
   int render_index;

   // Scratch space for text
   char scratch[64];

   // For buttons - optional values
   int int_value;
   char str_value[32];
   char displayed_value[32];

   // Optional menu item specific value changed function
   void (*on_value_changed)(struct menu_item*);
};

struct menu_item* ui_menu_add_toggle(int id, struct menu_item *folder, char* name, int initial_state);
struct menu_item* ui_menu_add_checkbox(int id, struct menu_item *folder, char* name, int initial_state);
struct menu_item* ui_menu_add_multiple_choice(int id, struct menu_item *folder, char *name);
struct menu_item* ui_menu_add_button(int id, struct menu_item *folder, char *name);
struct menu_item* ui_menu_add_button_with_value(int id, struct menu_item *folder, char *name, int int_value, char* str_value, char* displayed_value);
struct menu_item* ui_menu_add_range(int id, struct menu_item *folder, char *name, int min, int max, int step, int initial_value);
struct menu_item* ui_menu_add_folder(struct menu_item *folder, char *name);
struct menu_item* ui_menu_add_divider(struct menu_item *folder);


// Stubs for vice calls. Unimplemented for now.
void ui_pause_emulation(int flag);
int ui_emulation_is_paused(void);

// Begin raspi ui code
void ui_init_menu(void);

void ui_draw_text(const char* text,
                 int x, int y,
                 int color);

void ui_draw_rect(int x, int y,
                 int w, int h,
                 int color,
                 int fill);

int ui_text_width(const char* text);

void ui_check_key(void);

void ui_toggle(void);

void ui_render_now(void);

struct menu_item* ui_pop_menu(void);

struct menu_item* ui_push_menu(void);

void ui_set_on_value_changed_callback(
    void (*on_value_changed)(struct menu_item*));

void ui_check_key();

volatile int ui_activated;

#endif
