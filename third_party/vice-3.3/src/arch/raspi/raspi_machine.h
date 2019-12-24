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
#ifndef RASPI_MACHINE_H
#define RASPI_MACHINE_H

#include <stdint.h>

#include "videoarch.h"

extern void set_refresh_rate(struct video_canvas_s *canvas);
extern void set_video_font(void);
extern unsigned int *raspi_get_palette(int index);
extern int is_vic(struct video_canvas_s *canvas);
extern int is_vdc(struct video_canvas_s *canvas);
void cartridge_freeze(void);
void set_canvas_size(int* w, int *h, int *gw, int *gh);
void set_canvas_borders(int *w, int *h);
#endif
