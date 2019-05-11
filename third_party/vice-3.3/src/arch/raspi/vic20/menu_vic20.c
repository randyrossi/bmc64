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

#include "menu_vic20.h"

#include "vic20/vic20.h"
#include "resources.h"

unsigned long calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
     return VIC20_PAL_CYCLES_PER_LINE * VIC20_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
     return VIC20_NTSC_CYCLES_PER_LINE * VIC20_NTSC_SCREEN_LINES * fps;
  } else {
     return 0;
  }
}

void set_color_brightness(int value) {
   resources_set_int("VICColorBrightness", value);
}

void set_color_contrast(int value) {
   resources_set_int("VICColorContrast", value);
}

void set_color_gamma(int value) {
   resources_set_int("VICColorGamma", value);
}

void set_color_tint(int value) {
   resources_set_int("VICColorTint", value);
}

void set_video_cache(int value) {
   resources_set_int("VICVideoCache", value);
}

void set_hw_scale(int value) {
   resources_set_int("VICHwScale", value);
}

int get_color_brightness() {
   int value;
   resources_get_int("VICColorBrightness", &value);
   return value;
}

int get_color_contrast() {
   int value;
   resources_get_int("VICColorContrast", &value);
   return value;
}

int get_color_gamma() {
   int value;
   resources_get_int("VICColorGamma", &value);
   return value;
}

int get_color_tint() {
   int value;
   resources_get_int("VICColorTint", &value);
   return value;
}
