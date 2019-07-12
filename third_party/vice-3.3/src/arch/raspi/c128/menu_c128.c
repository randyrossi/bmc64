/*
 * menu_c128.c
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

#include "menu_c128.h"

#include <memory.h>

#include "c128/c128.h"
#include "resources.h"
#include "cartridge.h"
#include "menu.h"
#include "ui.h"

unsigned long calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
    return C128_PAL_CYCLES_PER_LINE * C128_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
    return C128_NTSC_CYCLES_PER_LINE * C128_NTSC_SCREEN_LINES * fps;
  } else {
    return 0;
  }
}

void set_color_brightness(int display_num, int value) {
  if (display_num == 0) {
    resources_set_int("VICIIColorBrightness", value);
  } else {
    resources_set_int("VDCColorBrightness", value);
  }
}

void set_color_contrast(int display_num, int value) {
  if (display_num == 0) {
    resources_set_int("VICIIColorContrast", value);
  } else {
    resources_set_int("VDCColorContrast", value);
  }
}

void set_color_gamma(int display_num, int value) {
  if (display_num == 0) {
    resources_set_int("VICIIColorGamma", value);
  } else {
    resources_set_int("VDCColorGamma", value);
  }
}

void set_color_tint(int display_num, int value) {
  if (display_num == 0) {
    resources_set_int("VICIIColorTint", value);
  } else {
    resources_set_int("VDCColorTint", value);
  }
}

void set_video_cache(int value) {
  resources_set_int("VICIIVideoCache", value);
}

void set_hw_scale(int value) {
  resources_set_int("VICIIHwScale", value);
}

int get_color_brightness(int display_num) {
  int value;
  if (display_num == 0) {
    resources_get_int("VICIIColorBrightness", &value);
  } else {
    resources_get_int("VDCColorBrightness", &value);
  }
  return value;
}

int get_color_contrast(int display_num) {
  int value;
  if (display_num == 0) {
    resources_get_int("VICIIColorContrast", &value);
  } else {
    resources_get_int("VDCColorContrast", &value);
  }
  return value;
}

int get_color_gamma(int display_num) {
  int value;
  if (display_num == 0) {
    resources_get_int("VICIIColorGamma", &value);
  } else {
    resources_get_int("VDCColorGamma", &value);
  }
  return value;
}

int get_color_tint(int display_num) {
  int value;
  if (display_num == 0) {
    resources_get_int("VICIIColorTint", &value);
  } else {
    resources_get_int("VDCColorTint", &value);
  }
  return value;
}

void raspi_cartridge_trigger_freeze(void) {
  cartridge_trigger_freeze();
}

struct menu_item* menu_build_palette_options(int menu_id, struct menu_item* parent) {
  struct menu_item* palette_item =
      ui_menu_add_multiple_choice(menu_id, parent, "Color Palette");
  palette_item->num_choices = 5;
  palette_item->value = 1;
  strcpy(palette_item->choices[0], "Default");
  strcpy(palette_item->choices[1], "Vice");
  strcpy(palette_item->choices[2], "C64hq");
  strcpy(palette_item->choices[3], "Pepto-Ntsc");
  strcpy(palette_item->choices[4], "Pepto-Pal");
  return palette_item;
}
