/*
 * menu_key_binding.c
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

#include "menu_key_binding.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// RASPI includes
#include "keycodes.h"
#include "menu.h"
#include "ui.h"

static struct menu_item* binding[6];

static struct menu_item* item_waiting_for_key;

static void keycode_for_keybinding(long keycode) {
  const char* keyname = keycode_to_string(keycode);
  sprintf(item_waiting_for_key->displayed_value, "%s", keyname);
  key_bindings[item_waiting_for_key->value] = keycode;
  raw_keycode_func = NULL;
}

static void menu_usb_value_changed(struct menu_item *item) {
  raw_keycode_func = keycode_for_keybinding;
  item_waiting_for_key = item;
  sprintf(item->displayed_value, "(waiting)");
}

void build_keybinding_menu(struct menu_item *root) {
   char name[16];
   for (int i=0;i<6;i++) {
     sprintf (name, "Binding %d", i+1);
     binding[i] = ui_menu_add_button_with_value(
        MENU_TEXT, root, name, i, "",
        keycode_to_string(key_bindings[i]));
     binding[i]->on_value_changed = menu_usb_value_changed;
   }
}
