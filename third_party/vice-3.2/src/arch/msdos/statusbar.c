/*
 * statusbar.c
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

#include "vice.h"

#include "types.h"

#include <stdio.h>
#include <allegro.h>

#include "log.h"
#include "raster.h"
#include "resources.h"
#include "statusbar.h"
#include "videoarch.h"

/* the bitmap with the status informations */
BITMAP *status_bitmap = NULL;

/* behind the bitmap with the status informations */
BITMAP *behind_status_bitmap = NULL;

/* where has the status-bitmap to be copied? 
   screen or triple-buffer pages? */
BITMAP *bitmaps_to_update[2];
int nr_of_bitmaps = 0;

/* to center the statusbar we need the vga-width */
int vga_width = 0;

/* for auto mode */
int statusbar_possible = 0;

/* colors for the statusbar */
struct statusbar_color_s statusbar_color[] =
{
    { { 00, 00, 00 }, 0 }, /* STATUSBAR_COLOR_BLACK    */
    { { 55, 55, 55 }, 0 }, /* STATUSBAR_COLOR_WHITE    */
    { { 31, 31, 31 }, 0 }, /* STATUSBAR_COLOR_GREY     */
    { { 15, 15, 15 }, 0 }, /* STATUSBAR_COLOR_DARKGREY */
    { { 10, 10, 50 }, 0 }, /* STATUSBAR_COLOR_BLUE     */
    { { 63, 63, 00 }, 0 }, /* STATUSBAR_COLOR_YELLOW   */
    { { 63, 00, 00 }, 0 }, /* STATUSBAR_COLOR_RED      */
    { { 00, 63, 00 }, 0 }, /* STATUSBAR_COLOR_GREEN    */
    { { 00, 00, 00 }, -1 }
};

void statusbar_register_colors(int next_avail, RGB *colors)
{
    int i;

    for (i = 0; statusbar_color[i].index >= 0; i++) {
        if (next_avail < NUM_AVAILABLE_COLORS) {
            /* there is an entry available in the palette */
            colors[next_avail] = statusbar_color[i].rgb_color;
            set_color(next_avail, &colors[next_avail]);
            statusbar_color[i].index = next_avail;
            next_avail++;
        } else {
            /* the palette is full; use the nearest one */
            statusbar_color[i].index = makecol8(statusbar_color[i].rgb_color.r, statusbar_color[i].rgb_color.g, statusbar_color[i].rgb_color.b);
        }
    }
}


int statusbar_get_color(int num)
{
    return statusbar_color[num].index;
}

int statusbar_enabled(void)
{
    int val;

    resources_get_int("ShowStatusbar", &val);

    if (val == STATUSBAR_MODE_AUTO) {
        val = statusbar_possible;
    }

    return val;
}

int statusbar_init(void) 
{
    status_bitmap = create_bitmap(STATUSBAR_WIDTH, STATUSBAR_HEIGHT);
    behind_status_bitmap = create_bitmap(STATUSBAR_WIDTH, STATUSBAR_HEIGHT);
    if (!status_bitmap || !behind_status_bitmap) {
        log_error(LOG_ERR, "Cannot allocate statusbar bitmap");
        return -1;
    } else {
        clear(behind_status_bitmap);
        clear(status_bitmap);
        rect(status_bitmap, 0, 0, STATUSBAR_WIDTH - 1, STATUSBAR_HEIGHT - 1, statusbar_get_color(STATUSBAR_COLOR_WHITE));
    }
    return 0;
}

void statusbar_exit(void) 
{
    if (status_bitmap) {
        destroy_bitmap(status_bitmap);
        status_bitmap = NULL;
    }
    if (behind_status_bitmap) {
        destroy_bitmap(behind_status_bitmap);
        status_bitmap = NULL;
    }
}

void statusbar_reset_bitmaps_to_update(void) 
{
    nr_of_bitmaps = 0;
}

void statusbar_append_bitmap_to_update(BITMAP *b) 
{
    bitmaps_to_update[nr_of_bitmaps++] = b;
}

static void statusbar_to_screen(BITMAP *bitmap, int x_offset, int x_width)
{
    int b;

    if (bitmap == NULL) {
        return;
    }

    for (b = 0; b < nr_of_bitmaps; b++) {
        blit(bitmap, bitmaps_to_update[b], 0, 0, x_offset, 0, x_width, STATUSBAR_HEIGHT);
    }
}

/* print the status_bitmap to screen */
void statusbar_update() 
{
    if (!video_in_gfx_mode() || !statusbar_enabled() || (status_bitmap == NULL)) {
        return;
    }

    statusbar_to_screen(status_bitmap, (vga_width - STATUSBAR_WIDTH) / 2, STATUSBAR_WIDTH);
}

void statusbar_disable()
{
    if (!video_in_gfx_mode() || (behind_status_bitmap == NULL)) {
        return;
    }

    statusbar_to_screen(behind_status_bitmap, (vga_width - STATUSBAR_WIDTH) / 2, STATUSBAR_WIDTH);
    raster_mode_change();
}

void statusbar_set_width(int w)
{
    vga_width = w;
}

void statusbar_set_height(int h)
{
    /* 
    this currently doesn't set the height in fact but decides 
    whether the sb is visible in auto-mode
    */
    if (h >= STATUSBAR_HEIGHT) {
        statusbar_possible = 1;
    } else {
        statusbar_possible = 0;
    }
}

void statusbar_prepare(void)
{
    /*
    this clears the upper bar of screen to avoid
    rubbish remaining at left and right of the bar
    */
    BITMAP *bm_clear;

    if (!video_in_gfx_mode()) {
        return;
    }

    bm_clear = create_bitmap(vga_width, STATUSBAR_HEIGHT);
    if (bm_clear) {
        clear(bm_clear);
        statusbar_to_screen(bm_clear, 0, vga_width);
        destroy_bitmap(bm_clear);
    }
}
