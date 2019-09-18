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

#include "menu_plus4.h"

#include <memory.h>

#include "plus4/plus4.h"
#include "plus4/plus4model.h"
#include "resources.h"
#include "cartridge.h"
#include "menu.h"
#include "ui.h"

static void menu_value_changed(struct menu_item *item) {
  plus4model_set(item->sub_id);
  ui_pop_all_and_toggle();
}

unsigned long calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
    return PLUS4_PAL_CYCLES_PER_LINE * PLUS4_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
    return PLUS4_NTSC_CYCLES_PER_LINE * PLUS4_NTSC_SCREEN_LINES * fps;
  } else {
    return 0;
  }
}

void set_color_brightness(int display_num, int value) {
  resources_set_int("TEDColorBrightness", value);
}

void set_color_contrast(int display_num, int value) {
  resources_set_int("TEDColorContrast", value);
}

void set_color_gamma(int display_num, int value) {
  resources_set_int("TEDColorGamma", value);
}

void set_color_tint(int display_num, int value) {
  resources_set_int("TEDColorTint", value);
}

void set_video_cache(int value) {
  resources_set_int("TEDVideoCache", value);
}

void set_hw_scale(int value) {
  resources_set_int("TEDHwScale", value);
}

int get_color_brightness(int display_num) {
  int value;
  resources_get_int("TEDColorBrightness", &value);
  return value;
}

int get_color_contrast(int display_num) {
  int value;
  resources_get_int("TEDColorContrast", &value);
  return value;
}

int get_color_gamma(int display_num) {
  int value;
  resources_get_int("TEDColorGamma", &value);
  return value;
}

int get_color_tint(int display_num) {
  int value;
  resources_get_int("TEDColorTint", &value);
  return value;
}

void raspi_cartridge_trigger_freeze(void) {
  cartridge_trigger_freeze();
}

struct menu_item* menu_build_palette_options(int menu_id, struct menu_item* parent) {
  struct menu_item* palette_item =
      ui_menu_add_multiple_choice(menu_id, parent, "Color Palette");
  palette_item->num_choices = 2;
  palette_item->value = 0;
  strcpy(palette_item->choices[0], "Yape-Pal");
  strcpy(palette_item->choices[1], "Yape-Ntsc");
  return palette_item;
}

int cartridge_flush_image(int type) { }
void cartridge_set_default(void) { }
void cartridge_trigger_freeze(void) { }

void menu_build_machine(struct menu_item* parent) {
  struct menu_item* model_parent = ui_menu_add_folder(parent, "Model...");
  int timing = circle_get_machine_timing();

  struct menu_item* item;
  if (timing == MACHINE_TIMING_PAL_HDMI ||
      timing == MACHINE_TIMING_PAL_COMPOSITE ||
      timing == MACHINE_TIMING_PAL_CUSTOM) {
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
}

struct menu_item* menu_build_cartridge(struct menu_item* root) {
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
