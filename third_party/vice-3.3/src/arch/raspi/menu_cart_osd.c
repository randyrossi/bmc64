/*
 * menu_cart_osd.c
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

#include "menu_cart_osd.h"

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cartridge.h"
#include "keyboard.h"
#include "machine.h"
#include "menu.h"
#include "raspi_machine.h"
#include "ui.h"

static void popped(struct menu_item *item) { osd_active = 0; }

static void menu_item_changed(struct menu_item *item) {
  switch (item->id) {
  case MENU_SAVE_EASYFLASH:
    if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
      ui_pop_menu();
      ui_error("Problem saving");
      // Treat this as OSD too
      osd_active = 1;
    } else {
      ui_pop_all_and_toggle();
    }
    break;
  case MENU_CART_FREEZE:
    keyboard_clear_keymatrix();
    raspi_cartridge_trigger_freeze();
    ui_pop_all_and_toggle();
    break;
  default:
    break;
  }
}

void show_cart_osd_menu(void) {
  // We only show OSD when the emulator is running. (not in the trap)
  if (ui_activated) {
    if (osd_active) {
      ui_pop_all_and_toggle();
      osd_active = 0;
    }
    return;
  }

  struct menu_item *root = ui_push_menu(20, 2);
  root->on_value_changed = popped;

  struct menu_item *child;
  if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C128) {
    child = ui_menu_add_button(MENU_SAVE_EASYFLASH, root, "Save EasyFlash Now");
    child->on_value_changed = menu_item_changed;
    child = ui_menu_add_button(MENU_CART_FREEZE, root, "Cartridge Freeze");
    child->on_value_changed = menu_item_changed;
  } else {
    child = ui_menu_add_button(MENU_TEXT, root, "No functions...");
  }

  // This will turn on ui rendering from the emuation side which will
  // now see the OSD we just created.
  ui_activated = 1;
  osd_active = 1;
}
