/*
 * lightpendrv.h - Lightpen driver for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#ifndef VICE_LIGHTPENDRV_H
#define VICE_LIGHTPENDRV_H

#include "vice.h"
#include "types.h"

struct sdl_lightpen_adjust_s {
    int offset_x, offset_y;
    int max_x, max_y;
    double scale_x, scale_y;
};
typedef struct sdl_lightpen_adjust_s sdl_lightpen_adjust_t;
extern sdl_lightpen_adjust_t sdl_lightpen_adjust;

extern void sdl_lightpen_update(void);

#endif
