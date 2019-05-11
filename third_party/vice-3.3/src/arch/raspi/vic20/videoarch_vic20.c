/*
 * videoarch_vic20.c
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

#include "videoarch_vic20.h"

#include "vic20/vic20.h"
#include "vic20/vic20mem.h"

#include "../font.h"

void set_refresh_rate(int timing, struct video_canvas_s *canvas) {
  if (timing == MACHINE_TIMING_NTSC_HDMI ||
      timing == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing == MACHINE_TIMING_NTSC_CUSTOM) {
    canvas->refreshrate = VIC20_NTSC_RFSH_PER_SEC;
  } else {
    canvas->refreshrate = VIC20_PAL_RFSH_PER_SEC;
  }
}

void set_video_font(struct VideoData *video_data) {
  int i;
  video_data->font = (uint8_t *)&font8x8_basic;
  for (i = 0; i < 256; ++i) {
    video_data->font_translate[i] = (8 * (i & 0x7f));
  }
}
