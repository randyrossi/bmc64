/*
 * menu_pet.c
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
#include "pet/pet.h"
#include "pet/petmodel.h"
#include "resources.h"
#include "keyboard.h"
#include "cartridge.h"

// RASPI includes
#include "emux_api.h"
#include "menu.h"
#include "ui.h"
#include "keycodes.h"

extern struct video_canvas_s *vic_canvas;

static void menu_value_changed(struct menu_item *item) {
  switch (item->id) {
     case MENU_MODEL_PET_2001:
     case MENU_MODEL_PET_3008:
     case MENU_MODEL_PET_3016:
     case MENU_MODEL_PET_3032:
     case MENU_MODEL_PET_3032B:
     case MENU_MODEL_PET_4016:
     case MENU_MODEL_PET_4032:
     case MENU_MODEL_PET_4032B:
     case MENU_MODEL_PET_8032:
     case MENU_MODEL_PET_8096:
     case MENU_MODEL_PET_8296:
     case MENU_MODEL_PET_SUPERPET:
        petmodel_set(item->sub_id);
        ui_pop_all_and_toggle();
        break;
     default:
        break;
  }
}

unsigned long emux_calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
    return PET_PAL_CYCLES_PER_LINE * PET_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
    return PET_NTSC_CYCLES_PER_LINE * PET_NTSC_SCREEN_LINES * fps;
  } else {
    return 0;
  }
}

double emux_calculate_fps() {
  if (is_ntsc()) {
     return (double)circle_cycles_per_sec() / (PET_NTSC_CYCLES_PER_LINE * PET_NTSC_SCREEN_LINES);
  }
  return (double)circle_cycles_per_sec() / (PET_PAL_CYCLES_PER_LINE * PET_PAL_SCREEN_LINES);
}

void emux_set_color_brightness(int display_num, int value) {
  resources_set_int("CrtcColorBrightness", value);
}

void emux_set_color_contrast(int display_num, int value) {
  resources_set_int("CrtcColorContrast", value);
}

void emux_set_color_gamma(int display_num, int value) {
  resources_set_int("CrtcColorGamma", value);
}

void emux_set_color_tint(int display_num, int value) {
  resources_set_int("CrtcColorTint", value);
}

void emux_set_color_saturation(int display_num, int value) {
  resources_set_int("CrtcSaturation", value);
}

void emux_set_video_cache(int value) {
  resources_set_int("CrtcVideoCache", value);
}

void emux_set_hw_scale(int value) {
  resources_set_int("CrtcHwScale", value);
}

int emux_get_color_brightness(int display_num) {
  int value;
  resources_get_int("CrtcColorBrightness", &value);
  return value;
}

int emux_get_color_contrast(int display_num) {
  int value;
  resources_get_int("CrtcColorContrast", &value);
  return value;
}

int emux_get_color_gamma(int display_num) {
  int value;
  resources_get_int("CrtcColorGamma", &value);
  return value;
}

int emux_get_color_tint(int display_num) {
  int value;
  resources_get_int("CrtcColorTint", &value);
  return value;
}

int emux_get_color_saturation(int display_num) {
  int value;
  resources_get_int("CrtcColorSaturation", &value);
  return value;
}

int cartridge_flush_image(int type) { }
void cartridge_set_default(void) { }
void cartridge_freeze(void) { }
void cartridge_detach_image (int type) { }

struct menu_item* emux_add_palette_options(int menu_id, struct menu_item* parent) {
  struct menu_item* palette_item =
      ui_menu_add_multiple_choice(menu_id, parent, "Color Palette");
  palette_item->num_choices = 3;
  palette_item->value = 0;
  strcpy(palette_item->choices[0], "Green");
  strcpy(palette_item->choices[1], "Amber");
  strcpy(palette_item->choices[2], "Black/White");
  return palette_item;
}

void emux_add_machine_options(struct menu_item* parent) {
  struct menu_item* model_parent = ui_menu_add_folder(parent, "Model...");
  int timing = circle_get_machine_timing();

  struct menu_item* item;

  int num_models = 12;
  static const char* labels[] = { "2001-NB", "3008", "3016", "3032", "3032B", "4016", "4032","4032B","8032","8096","8296","SUPERPET" };
  static int menu_values[] = { MENU_MODEL_PET_2001, MENU_MODEL_PET_3008, MENU_MODEL_PET_3016, MENU_MODEL_PET_3032, MENU_MODEL_PET_3032B, MENU_MODEL_PET_4016, MENU_MODEL_PET_4032, MENU_MODEL_PET_4032B, MENU_MODEL_PET_8032, MENU_MODEL_PET_8096, MENU_MODEL_PET_8296, MENU_MODEL_PET_SUPERPET };
  static int id_values[] = { PETMODEL_2001, PETMODEL_3008, PETMODEL_3016, PETMODEL_3032, PETMODEL_3032B, PETMODEL_4016, PETMODEL_4032, PETMODEL_4032B, PETMODEL_8032, PETMODEL_8096, PETMODEL_8296, PETMODEL_SUPERPET };

  for (int i=0;i<num_models;i++) {
    item = ui_menu_add_button(menu_values[i], model_parent, labels[i]);
    item->sub_id = id_values[i];
    item->on_value_changed = menu_value_changed;
  }
}

struct menu_item* emux_add_cartridge_options(struct menu_item* root) {
  return NULL;
}

void emux_machine_load_settings_done(void) {
}

void machine_keymap_changed(int row, int col, signed long sym) {
  if (row == 0 && col == 0) {
     // Different keyboards so just be consistent and use these.
     commodore_key_sym = KEYCODE_LeftControl;
     ctrl_key_sym = KEYCODE_Tab;
     restore_key_sym = KEYCODE_PageUp;
  }
}

