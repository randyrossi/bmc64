/*
 * videoarch_pet.c
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

#include "videoarch_pet.h"

#include <stdlib.h>

#include "emux_api.h"
#include "pet/pet.h"
#include "pet/petmem.h"
#include "pet/petmodel.h"
#include "resources.h"

static unsigned int white_color_palette[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
};

static unsigned int amber_color_palette[] = {
    0x00, 0x00, 0x00, 0xFF, 0xA8, 0x00,
};

static unsigned int green_color_palette[] = {
    0x00, 0x00, 0x00, 0x41, 0xFF, 0x00,
};

void set_refresh_rate(struct video_canvas_s *canvas) {
  if (is_ntsc()) {
    canvas->refreshrate = PET_NTSC_RFSH_PER_SEC;
  } else {
    canvas->refreshrate = PET_PAL_RFSH_PER_SEC;
  }
}

void set_video_font(void) {
  uint8_t* chargen = malloc(4096); // never freed
  FILE* fp = fopen("PET/CHARGEN", "r");
  fread(chargen,1,2048,fp);
  fseek(fp, 0, SEEK_SET);
  fread(chargen+2048,1,2048,fp);
  fclose(fp);

  int i;
  video_font = chargen + 0x400;
  raw_video_font = chargen;
  for (i = 0; i < 256; ++i) {
    video_font_translate[i] = 8 * ascii_to_petscii[i];
  }
}

unsigned int *raspi_get_palette(int display, int index) {
  switch (index) {
  case 0:
    return green_color_palette;
    break;
  case 1:
    return amber_color_palette;
    break;
  case 2:
    return white_color_palette;
    break;
  default:
    return NULL;
  }
}

void set_canvas_size(int index, int *w, int *h, int *gw, int *gh) {
  int size;
  resources_get_int("VideoSize", &size);
  if (size == 40) {
    *w = 386;
    *h = 288;
    *gw = 40*8;
    *gh = 25*8;
    return;
  }
  *w = 704;
  *h = 288;
  *gw = 80*8;
  *gh = 25*8;
}

void set_canvas_borders(int index, int *w, int *h) {
  *w = 32;
  *h = 40;
}

void set_filter(int display, int value) {
  resources_set_int("CrtcFilter", value);
}

int get_filter(int display) {
  int value;
  resources_get_int("CrtcFilter", &value);
  return value;
}
