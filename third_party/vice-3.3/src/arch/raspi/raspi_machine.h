/*
 * videoarch_machine.h
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

// Machine specific routines

#include "videoarch.h"

extern void set_refresh_rate(int timing, struct video_canvas_s *canvas);
extern void set_video_font(struct VideoData *video_state);
extern unsigned long calculate_timing(double fps);

extern void set_color_brightness(int value);
extern void set_color_contrast(int value);
extern void set_color_gamma(int value);
extern void set_color_tint(int value);
extern void set_video_cache(int value);
extern void set_hw_scale(int value);

extern int get_color_brightness(void);
extern int get_color_contrast(void);
extern int get_color_gamma(void);
extern int get_color_tint(void);

extern void raspi_cartridge_trigger_freeze(void);
unsigned int *raspi_get_palette(int index);
struct menu_item* menu_build_palette_options(struct menu_item* parent);

int is_vic(struct video_canvas_s *canvas);
int is_vdc(struct video_canvas_s *canvas);

