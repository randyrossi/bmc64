/*
 * statusbar.h
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifndef VICE_STATUSBAR_H
#define VICE_STATUSBAR_H

#include <allegro.h>

#define STATUSBAR_HEIGHT 12
#define STATUSBAR_WIDTH  320

/* current use of the bar:
    speed info:       2 -  98
    tape info:      112 - 166
    drive 1:        180 - 240
    drive 2:        250 - 310
*/

struct statusbar_color_s {
    RGB rgb_color;
    int index;
};

enum {
    STATUSBAR_COLOR_BLACK = 0, 
    STATUSBAR_COLOR_WHITE,
    STATUSBAR_COLOR_GREY,
    STATUSBAR_COLOR_DARKGREY,
    STATUSBAR_COLOR_BLUE,
    STATUSBAR_COLOR_YELLOW,
    STATUSBAR_COLOR_RED,
    STATUSBAR_COLOR_GREEN
};

enum {
    STATUSBAR_MODE_OFF = 0,
    STATUSBAR_MODE_ON,
    STATUSBAR_MODE_AUTO
};

extern void statusbar_update(void);
extern void statusbar_disable(void);
extern int statusbar_init(void);
extern void statusbar_exit(void);
extern int statusbar_enabled(void);
extern void statusbar_reset_bitmaps_to_update(void);
extern void statusbar_append_bitmap_to_update(BITMAP *b);
extern void statusbar_set_width(int w);
extern void statusbar_set_height(int h);
extern void statusbar_prepare(void);
extern void statusbar_register_colors(int next_avail, RGB *colors);
extern int statusbar_get_color(int num);
extern BITMAP *status_bitmap;

#endif
