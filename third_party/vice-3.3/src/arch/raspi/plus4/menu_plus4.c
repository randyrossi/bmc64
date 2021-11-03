/*
 * menu_plus4.c
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
#include "plus4/plus4.h"
#include "plus4/plus4model.h"
#include "resources.h"
#include "keyboard.h"
#include "cartridge.h"

// RASPI includes
#include "circle.h"
#include "emux_api.h"
#include "menu.h"
#include "ui.h"
#include "keycodes.h"

static struct menu_item* memory_item;

static void update_memory_item() {
  int value;
  resources_get_int("RamSize", &value);
  switch (value) {
    case 16:
      memory_item->value = 0;
      break;
    case 32:
      memory_item->value = 1;
      break;
    case 64:
    default:
      memory_item->value = 2;
      break;
  }
}

static void menu_value_changed(struct menu_item *item) {
  switch (item->id) {
     case MENU_MODEL_C16_PAL:
     case MENU_MODEL_PLUS4_PAL:
     case MENU_MODEL_C16_NTSC:
     case MENU_MODEL_PLUS4_NTSC:
     case MENU_MODEL_V364_NTSC:
     case MENU_MODEL_C232_NTSC:
        plus4model_set(item->sub_id);
        update_memory_item();
        ui_pop_all_and_toggle();
        break;
     case MENU_MEMORY:
        resources_set_int("RamSize", item->choice_ints[item->value]);
        break;
     default:
        break;
  }
}

unsigned long emux_calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
    return PLUS4_PAL_CYCLES_PER_LINE * PLUS4_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
    return PLUS4_NTSC_CYCLES_PER_LINE * PLUS4_NTSC_SCREEN_LINES * fps;
  } else {
    return 0;
  }
}

double emux_calculate_fps() {
  if (is_ntsc()) {
     return (double)circle_cycles_per_sec() / (PLUS4_NTSC_CYCLES_PER_LINE * PLUS4_NTSC_SCREEN_LINES);
  }
  return (double)circle_cycles_per_sec() / (PLUS4_PAL_CYCLES_PER_LINE * PLUS4_PAL_SCREEN_LINES);
}

void emux_set_color_brightness(int display_num, int value) {
  resources_set_int("TEDColorBrightness", value);
}

void emux_set_color_contrast(int display_num, int value) {
  resources_set_int("TEDColorContrast", value);
}

void emux_set_color_gamma(int display_num, int value) {
  resources_set_int("TEDColorGamma", value);
}

void emux_set_color_tint(int display_num, int value) {
  resources_set_int("TEDColorTint", value);
}

void emux_set_color_saturation(int display_num, int value) {
  resources_set_int("TEDColorSaturation", value);
}

void emux_set_video_cache(int value) {
  resources_set_int("TEDVideoCache", value);
}

void emux_set_hw_scale(int value) {
  resources_set_int("TEDHwScale", value);
}

int emux_get_color_brightness(int display_num) {
  int value;
  resources_get_int("TEDColorBrightness", &value);
  return value;
}

int emux_get_color_contrast(int display_num) {
  int value;
  resources_get_int("TEDColorContrast", &value);
  return value;
}

int emux_get_color_gamma(int display_num) {
  int value;
  resources_get_int("TEDColorGamma", &value);
  return value;
}

int emux_get_color_tint(int display_num) {
  int value;
  resources_get_int("TEDColorTint", &value);
  return value;
}

int emux_get_color_saturation(int display_num) {
  int value;
  resources_get_int("TEDColorSaturation", &value);
  return value;
}

struct menu_item* emux_add_palette_options(int menu_id, struct menu_item* parent) {
  struct menu_item* palette_item =
      ui_menu_add_multiple_choice(menu_id, parent, "Color Palette");
  palette_item->num_choices = 3;
  palette_item->value = 0;
  strcpy(palette_item->choices[0], "YAPE (PAL)");
  strcpy(palette_item->choices[1], "YAPE (NTSC)");
  strcpy(palette_item->choices[2], "Colodore (PAL)");
  return palette_item;
}

int cartridge_flush_image(int type) { }
void cartridge_set_default(void) { }
void cartridge_freeze(void) { }

void emux_add_machine_options(struct menu_item* parent) {
  struct menu_item* model_parent = ui_menu_add_folder(parent, "Model...");
  int timing = circle_get_machine_timing();

  struct menu_item* item;
  if (!is_ntsc()) {
    item = ui_menu_add_button(MENU_MODEL_C16_PAL, model_parent, "C16");
    item->sub_id = PLUS4MODEL_C16_PAL;
    item->on_value_changed = menu_value_changed;
    item = ui_menu_add_button(MENU_MODEL_PLUS4_PAL, model_parent, "Plus/4");
    item->sub_id = PLUS4MODEL_PLUS4_PAL;
    item->on_value_changed = menu_value_changed;
  } else {
    item = ui_menu_add_button(MENU_MODEL_C16_NTSC, model_parent, "C16");
    item->sub_id = PLUS4MODEL_C16_NTSC;
    item->on_value_changed = menu_value_changed;
    item = ui_menu_add_button(MENU_MODEL_PLUS4_NTSC, model_parent, "Plus/4");
    item->sub_id = PLUS4MODEL_PLUS4_NTSC;
    item->on_value_changed = menu_value_changed;
    item = ui_menu_add_button(MENU_MODEL_V364_NTSC, model_parent, "V364");
    item->sub_id = PLUS4MODEL_V364_NTSC;
    item->on_value_changed = menu_value_changed;
    item = ui_menu_add_button(MENU_MODEL_C232_NTSC, model_parent, "C232");
    item->sub_id = PLUS4MODEL_232_NTSC;
    item->on_value_changed = menu_value_changed;
  }

  memory_item =
      ui_menu_add_multiple_choice(MENU_MEMORY, parent, "Memory");
  memory_item->num_choices = 3;

  update_memory_item();

  strcpy(memory_item->choices[0], "16k");
  strcpy(memory_item->choices[1], "32k");
  strcpy(memory_item->choices[2], "64k");
  memory_item->choice_ints[0] = 16;
  memory_item->choice_ints[1] = 32;
  memory_item->choice_ints[2] = 64;
  memory_item->on_value_changed = menu_value_changed;
}

struct menu_item* emux_add_cartridge_options(struct menu_item* root) {
  struct menu_item* parent = ui_menu_add_folder(root, "Cartridge");
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART, parent, "Attach cart...");
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C0_LO, parent, "Attach C0 LO...");
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C0_HI, parent, "Attach C0 HI...");
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C1_LO, parent, "Attach C1 LO...");
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C1_HI, parent, "Attach C1 HI...");
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C2_LO, parent, "Attach C2 LO...");
  ui_menu_add_button(MENU_PLUS4_ATTACH_CART_C2_HI, parent, "Attach C2 HI...");
  ui_menu_add_button(MENU_DETACH_CART, parent, "Detach cartridge");
  return parent;
}

void emux_machine_load_settings_done(void) {
}

void machine_keymap_changed(int row, int col, signed long sym) {
  if (row == 7 && col == 5 && !commodore_key_sym_set) {
     commodore_key_sym = sym;
     commodore_key_sym_set = 1;
  } else if (row == 7 && col == 2 && !ctrl_key_sym_set) {
     ctrl_key_sym = sym;
     ctrl_key_sym_set = 1;
  } else if (row == 7 && col == 0 && !restore_key_sym_set) {
     // No restore, use as HOME key
     restore_key_sym = sym;
     restore_key_sym_set = 1;
  }
}
