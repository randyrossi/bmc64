/*
 * videoparam.h - MacVICE specific video parameters
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#ifndef VICE_VIDEOPARAM_H
#define VICE_VIDEOPARAM_H

#define SYNC_DRAW_OFF       0
#define SYNC_DRAW_NEAREST   1
#define SYNC_DRAW_BLEND     2

#define SYNC_DRAW_LAST      2

/* video parameter settings are defined as resources */
struct video_param_s
{
    int sync_draw_mode;
    int sync_draw_buffers;
    int sync_draw_flicker_fix;
    int true_pixel_aspect;
    int show_key_codes;
};
typedef struct video_param_s video_param_t;

#endif
