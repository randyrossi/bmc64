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

#include "c128/c128.h"
#include "resources.h"

unsigned long calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
    return C128_PAL_CYCLES_PER_LINE * C128_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
    return C128_NTSC_CYCLES_PER_LINE * C128_NTSC_SCREEN_LINES * fps;
  } else {
    return 0;
  }
}

void set_color_brightness(int value) {
  resources_set_int("VICIIColorBrightness", value);
}

void set_color_contrast(int value) {
  resources_set_int("VICIIColorContrast", value);
}

void set_color_gamma(int value) { resources_set_int("VICIIColorGamma", value); }

void set_color_tint(int value) { resources_set_int("VICIIColorTint", value); }

void set_video_cache(int value) { resources_set_int("VICIIVideoCache", value); }

void set_hw_scale(int value) { resources_set_int("VICIIHwScale", value); }

int get_color_brightness() {
  int value;
  resources_get_int("VICIIColorBrightness", &value);
  return value;
}

int get_color_contrast() {
  int value;
  resources_get_int("VICIIColorContrast", &value);
  return value;
}

int get_color_gamma() {
  int value;
  resources_get_int("VICIIColorGamma", &value);
  return value;
}

int get_color_tint() {
  int value;
  resources_get_int("VICIIColorTint", &value);
  return value;
}
