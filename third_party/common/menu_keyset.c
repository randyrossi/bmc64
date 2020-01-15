/*
 * menu_keyset.c
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

#include "menu_keyset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// RASPI includes
#include "keycodes.h"
#include "menu.h"
#include "ui.h"

static struct menu_item* up;
static struct menu_item* down;
static struct menu_item* left;
static struct menu_item* right;
static struct menu_item* fire;
static struct menu_item* potx;
static struct menu_item* poty;

static int keyset_num;
static struct menu_item* item_waiting_for_key;


typedef enum {
  MENU_KEYSET_DEFINE_UP,
  MENU_KEYSET_DEFINE_DOWN,
  MENU_KEYSET_DEFINE_LEFT,
  MENU_KEYSET_DEFINE_RIGHT,
  MENU_KEYSET_DEFINE_FIRE,
  MENU_KEYSET_DEFINE_POTX,
  MENU_KEYSET_DEFINE_POTY,
} KeysetMenuID;

static void keycode_for_keyset(long keycode) {
  const char* keyname = keycode_to_string(keycode);
  sprintf(item_waiting_for_key->displayed_value, "%s", keyname);
  keyset_codes[keyset_num][item_waiting_for_key->value] = keycode;
  raw_keycode_func = NULL;
}

static void menu_usb_value_changed(struct menu_item *item) {
  raw_keycode_func = keycode_for_keyset;
  item_waiting_for_key = item;
  sprintf(item->displayed_value, "(waiting)");
}

void build_keyset_menu(int num, struct menu_item *root) {
   keyset_num = num;
   up = ui_menu_add_button_with_value(
        MENU_KEYSET_DEFINE_UP, root, "Up", KEYSET_UP, "",
        keycode_to_string(keyset_codes[num][KEYSET_UP]));
   down = ui_menu_add_button_with_value(
        MENU_KEYSET_DEFINE_DOWN, root, "Down", KEYSET_DOWN, "",
        keycode_to_string(keyset_codes[num][KEYSET_DOWN]));
   left = ui_menu_add_button_with_value(
        MENU_KEYSET_DEFINE_LEFT, root, "Left", KEYSET_LEFT, "",
        keycode_to_string(keyset_codes[num][KEYSET_LEFT]));
   right = ui_menu_add_button_with_value(
        MENU_KEYSET_DEFINE_RIGHT, root, "Right", KEYSET_RIGHT, "",
        keycode_to_string(keyset_codes[num][KEYSET_RIGHT]));
   fire = ui_menu_add_button_with_value(
        MENU_KEYSET_DEFINE_FIRE, root, "Fire", KEYSET_FIRE, "",
        keycode_to_string(keyset_codes[num][KEYSET_FIRE]));
   potx = ui_menu_add_button_with_value(
        MENU_KEYSET_DEFINE_POTX, root, "PotX", KEYSET_POTX, "",
        keycode_to_string(keyset_codes[num][KEYSET_POTX]));
   poty = ui_menu_add_button_with_value(
        MENU_KEYSET_DEFINE_POTY, root, "PotY", KEYSET_POTY, "",
        keycode_to_string(keyset_codes[num][KEYSET_POTY]));

   up->on_value_changed = menu_usb_value_changed;
   down->on_value_changed = menu_usb_value_changed;
   left->on_value_changed = menu_usb_value_changed;
   right->on_value_changed = menu_usb_value_changed;
   fire->on_value_changed = menu_usb_value_changed;
   potx->on_value_changed = menu_usb_value_changed;
   poty->on_value_changed = menu_usb_value_changed;
}
