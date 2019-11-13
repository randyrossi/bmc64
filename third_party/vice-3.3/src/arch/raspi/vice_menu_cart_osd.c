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

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// VICE includes
#include "cartridge.h"
#include "keyboard.h"

// RASPI includes
#include "emux_api.h"
#include "menu.h"
#include "raspi_machine.h"
#include "ui.h"

static void menu_item_changed(struct menu_item *item) {
  switch (item->id) {
  case MENU_SAVE_EASYFLASH:
    ui_pop_all_and_toggle();
    if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
      ui_error("Problem saving");
    }
    break;
  case MENU_CART_FREEZE:
    cartridge_freeze();
    ui_pop_all_and_toggle();
    break;
  default:
    break;
  }
}

// VICE specific implementation
void emux_show_cart_osd_menu(void) {
  // We only show OSD when the emulator is running. (not in the trap)
  if (ui_enabled) {
    ui_dismiss_osd_if_active();
    return;
  }

  struct menu_item *root = ui_push_menu(20, 2);
  root->on_popped_off = glob_osd_popped;

  struct menu_item *child;
  if (emux_machine_class == BMC64_MACHINE_CLASS_C64 || 
      emux_machine_class == BMC64_MACHINE_CLASS_C128) {
    child = ui_menu_add_button(MENU_SAVE_EASYFLASH, root, "Save EasyFlash Now");
    child->on_value_changed = menu_item_changed;
    child = ui_menu_add_button(MENU_CART_FREEZE, root, "Cartridge Freeze");
    child->on_value_changed = menu_item_changed;
  } else {
    child = ui_menu_add_button(MENU_TEXT, root, "No functions...");
  }

  // This will turn on ui rendering from the emuation side which will
  // see the OSD we just created.
  ui_enable_osd();
}

int emux_attach_cart(int menu_id, char* filename) {
  ui_info("Attaching...");

  int cart_type;
  switch (menu_id) {
     case MENU_C64_CART_FILE:
       cart_type = CARTRIDGE_CRT;
       break;
     case MENU_C64_CART_8K_FILE:
       cart_type = CARTRIDGE_GENERIC_8KB;
       break;
     case MENU_C64_CART_16K_FILE:
       cart_type = CARTRIDGE_GENERIC_16KB;
       break;
     case MENU_C64_CART_ULTIMAX_FILE:
       cart_type = CARTRIDGE_ULTIMAX;
       break;
     case MENU_VIC20_CART_DETECT_FILE:
       cart_type = CARTRIDGE_VIC20_DETECT;
       break;
     case MENU_VIC20_CART_GENERIC_FILE:
       cart_type = CARTRIDGE_VIC20_GENERIC;
       break;
     case MENU_VIC20_CART_16K_2000_FILE:
       cart_type = CARTRIDGE_VIC20_16KB_2000;
       break;
     case MENU_VIC20_CART_16K_4000_FILE:
       cart_type = CARTRIDGE_VIC20_16KB_4000;
       break;
     case MENU_VIC20_CART_16K_6000_FILE:
       cart_type = CARTRIDGE_VIC20_16KB_6000;
       break;
     case MENU_VIC20_CART_8K_A000_FILE:
       cart_type = CARTRIDGE_VIC20_8KB_A000;
       break;
     case MENU_VIC20_CART_4K_B000_FILE:
       cart_type = CARTRIDGE_VIC20_4KB_B000;
       break;
     case MENU_VIC20_CART_BEHRBONZ_FILE:
       cart_type = CARTRIDGE_VIC20_BEHRBONZ;
       break;
     case MENU_VIC20_CART_UM_FILE:
       cart_type = CARTRIDGE_VIC20_UM;
       break;
     case MENU_VIC20_CART_FP_FILE:
       cart_type = CARTRIDGE_VIC20_FP;
       break;
     case MENU_VIC20_CART_MEGACART_FILE:
       cart_type = CARTRIDGE_VIC20_MEGACART;
       break;
     case MENU_VIC20_CART_FINAL_EXPANSION_FILE:
       cart_type = CARTRIDGE_VIC20_FINAL_EXPANSION;
       break;
     case MENU_PLUS4_CART_FILE:
       cart_type = CARTRIDGE_PLUS4_DETECT;
       break;
     case MENU_PLUS4_CART_C0_LO_FILE:
       cart_type = CARTRIDGE_PLUS4_16KB_C0LO;
       break;
     case MENU_PLUS4_CART_C0_HI_FILE:
       cart_type = CARTRIDGE_PLUS4_16KB_C0HI;
       break;
     case MENU_PLUS4_CART_C1_LO_FILE:
       cart_type = CARTRIDGE_PLUS4_16KB_C1LO;
       break;
     case MENU_PLUS4_CART_C1_HI_FILE:
       cart_type = CARTRIDGE_PLUS4_16KB_C1HI;
       break;
     case MENU_PLUS4_CART_C2_LO_FILE:
       cart_type = CARTRIDGE_PLUS4_16KB_C2LO;
       break;
     case MENU_PLUS4_CART_C2_HI_FILE:
       cart_type = CARTRIDGE_PLUS4_16KB_C2HI;
       break;
     default:
       cart_type = CARTRIDGE_CRT;
       break;
  }

  if (cartridge_attach_image(cart_type, filename) < 0) {
     ui_pop_menu();
     ui_error("Failed to attach cart image");
     return 1;
  } else {
     ui_pop_all_and_toggle();
  }
  return 0;
}
