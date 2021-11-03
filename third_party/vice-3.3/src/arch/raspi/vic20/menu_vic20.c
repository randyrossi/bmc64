/*
 * menu_vic20.c
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

#include "../raspi_machine.h"

#include <memory.h>

// VICE includes
#include "resources.h"
#include "vic20/vic20.h"
#include "keyboard.h"

// RASPI includes
#include "emux_api.h"
#include "menu.h"
#include "ui.h"
#include "keycodes.h"

unsigned long emux_calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
    return VIC20_PAL_CYCLES_PER_LINE * VIC20_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
    return VIC20_NTSC_CYCLES_PER_LINE * VIC20_NTSC_SCREEN_LINES * fps;
  } else {
    return 0;
  }
}

double emux_calculate_fps() {
  if (is_ntsc()) {
     return (double)circle_cycles_per_sec() / (VIC20_NTSC_CYCLES_PER_LINE * VIC20_NTSC_SCREEN_LINES);
  }
  return (double)circle_cycles_per_sec() / (VIC20_PAL_CYCLES_PER_LINE * VIC20_PAL_SCREEN_LINES);
}

void emux_set_color_brightness(int display_num, int value) {
  resources_set_int("VICColorBrightness", value);
}

void emux_set_color_contrast(int display_num, int value) {
  resources_set_int("VICColorContrast", value);
}

void emux_set_color_gamma(int display_num, int value) {
  resources_set_int("VICColorGamma", value);
}

void emux_set_color_tint(int display_num, int value) {
  resources_set_int("VICColorTint", value);
}

void emux_set_color_saturation(int display_num, int value) {
  resources_set_int("VICColorSaturation", value);
}

void emux_set_video_cache(int value) {
  resources_set_int("VICVideoCache", value);
}

void emux_set_hw_scale(int value) {
  resources_set_int("VICHwScale", value);
}

int emux_get_color_brightness(int display_num) {
  int value;
  resources_get_int("VICColorBrightness", &value);
  return value;
}

int emux_get_color_contrast(int display_num) {
  int value;
  resources_get_int("VICColorContrast", &value);
  return value;
}

int emux_get_color_gamma(int display_num) {
  int value;
  resources_get_int("VICColorGamma", &value);
  return value;
}

int emux_get_color_tint(int display_num) {
  int value;
  resources_get_int("VICColorTint", &value);
  return value;
}

int emux_get_color_saturation(int display_num) {
  int value;
  resources_get_int("VICColorSaturation", &value);
  return value;
}

void cartridge_freeze(void) {
  // Not available on this machine.
}

struct menu_item* emux_add_palette_options(int menu_id, struct menu_item* parent) {
  struct menu_item* palette_item =
      ui_menu_add_multiple_choice(menu_id, parent, "Color Palette");
  palette_item->num_choices = 4;
  palette_item->value = 0;
  strcpy(palette_item->choices[0], "VICE");
  strcpy(palette_item->choices[1], "Mike (PAL)");
  strcpy(palette_item->choices[2], "Mike (NTSC)");
  strcpy(palette_item->choices[3], "Colodore (PAL)");
  return palette_item;
}

void emux_add_machine_options(struct menu_item* parent) {
  struct menu_item* roms_parent = ui_menu_add_folder(parent, "ROMs...");
  ui_menu_add_button(MENU_LOAD_KERNAL, roms_parent, "Load Kernal ROM...");
  ui_menu_add_button(MENU_LOAD_BASIC, roms_parent, "Load Basic ROM...");
  ui_menu_add_button(MENU_LOAD_CHARGEN, roms_parent, "Load Chargen ROM...");

  struct menu_item* mem_parent = ui_menu_add_folder(parent, "Memory");

  int block_value;
  int blocks;
  resources_get_int("RAMBlock0", &block_value);
  blocks = block_value;
  resources_get_int("RAMBlock1", &block_value);
  blocks |= (block_value << 1);
  resources_get_int("RAMBlock2", &block_value);
  blocks |= (block_value << 2);
  resources_get_int("RAMBlock3", &block_value);
  blocks |= (block_value << 3);
  resources_get_int("RAMBlock5", &block_value);
  blocks |= (block_value << 5);

  ui_menu_add_toggle(MENU_VIC20_MEMORY_3K, mem_parent,
                                         "3Kb ($0400)", blocks & VIC20_BLOCK_0 ? 1: 0);
  ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_2000, mem_parent,
                                         "8kb ($2000)", blocks & VIC20_BLOCK_1 ? 1: 0);
  ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_4000, mem_parent,
                                         "8kb ($4000)", blocks & VIC20_BLOCK_2 ? 1 : 0);
  ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_6000, mem_parent,
                                         "8kb ($6000)", blocks & VIC20_BLOCK_3 ? 1 : 0);
  ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_A000, mem_parent,
                                         "8kb ($A000)", blocks & VIC20_BLOCK_5 ? 1 : 0);
}

struct menu_item* emux_add_cartridge_options(struct menu_item* root) {
  struct menu_item* parent = ui_menu_add_folder(root, "Cartridge");

  ui_menu_add_button(MENU_VIC20_ATTACH_CART_GENERIC, parent, "Attach generic cart...");
  ui_menu_add_button(MENU_VIC20_ATTACH_CART_BEHRBONZ, parent, "Attach behrbonz cart...");
  ui_menu_add_button(MENU_VIC20_ATTACH_CART_MEGACART, parent, "Attach megacart cart...");
  ui_menu_add_button(MENU_VIC20_ATTACH_CART_FINAL_EXPANSION, parent, "Attach final expansion cart...");
  ui_menu_add_button(MENU_VIC20_ATTACH_CART_UM, parent, "Attach UM cart...");
  ui_menu_add_button(MENU_VIC20_ATTACH_CART_FP, parent, "Attach FP cart...");
  ui_menu_add_button(MENU_DETACH_CART, parent, "Detach cartridge");

  struct menu_item* p2 = ui_menu_add_folder(parent, "Add to generic cartridge");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_DETECT, p2, "Smart attach...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_16K_2000, p2, "Attach 4/8/16k $2000...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_16K_4000, p2, "Attach 4/8/16k $4000...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_16K_6000, p2, "Attach 4/8k/16k $6000...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_8K_A000, p2, "Attach 4/8k $A000...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_4K_B000, p2, "Attach 4k $B000...");
  return parent;
}

void emux_machine_load_settings_done(void) {
}

void machine_keymap_changed(int row, int col, signed long sym) {
  if (row == 0 && col == 5 && !commodore_key_sym_set) {
     commodore_key_sym = sym;
     commodore_key_sym_set = 1;
  } else if (row == 0 && col == 2 && !ctrl_key_sym_set) {
     ctrl_key_sym = sym;
     ctrl_key_sym_set = 1;
  } else if (row == -3 && col == 0 && !restore_key_sym_set) {
     restore_key_sym = sym;
     restore_key_sym_set = 1;
  }

}
