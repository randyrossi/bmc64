/*
 * nativedrv.c - Common code for native screenshots.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "archdep.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "mem.h"
#include "gfxoutput.h"
#include "nativedrv.h"
#include "palette.h"
#include "resources.h"
#include "screenshot.h"
#include "types.h"
#include "uiapi.h"
#include "util.h"
#include "vsync.h"

void native_smooth_scroll_borderize_colormap(native_data_t *source, uint8_t bordercolor, uint8_t xcover, uint8_t ycover)
{
    int i, j, k;
    int xstart = 0;
    int xsize;
    int xendamount = 0;
    int ystart = 0;
    int ysize;
    int yendamount = 0;

    if (xcover == 255) {
        xstart = 0;
        xsize = source->xsize;
        xendamount = 0;
    } else {
        xstart = 7 - xcover;
        xsize = source->xsize - 16;
        xendamount = 16 - xstart;
    }

    if (ycover == 255) {
        ystart = 0;
        ysize = source->ysize;
        yendamount = 0;
    } else {
        ystart = 7 - ycover;
        ysize = source->ysize - 8;
        yendamount = 8 - ystart;
    }

    k = 0;

    /* render top border if needed */
    for (i = 0; i < ystart; i++) {
        for (j = 0; j < source->xsize; j++) {
            source->colormap[k++] = bordercolor;
        }
    }

    for (i = 0; i < ysize; i++) {
        /* render left border if needed */
        for (j = 0; j < xstart; j++) {
            source->colormap[k++] = bordercolor;
        }

        /* skip screen data */
        k += xsize;

        /* render right border if needed */
        for (j = 0; j < xendamount; j++) {
            source->colormap[k++] = bordercolor;
        }
    }

    /* render bottom border if needed */
    for (i = 0; i < yendamount; i++) {
        for (j = 0; j < source->xsize; j++) {
            source->colormap[k++] = bordercolor;
        }
    }
}

native_data_t *native_borderize_colormap(native_data_t *source, uint8_t bordercolor, int xsize, int ysize)
{
    int i, j, k, l;
    int xstart = 0;
    int xendamount = 0;
    int ystart = 0;
    int yendamount = 0;
    native_data_t *dest = lib_malloc(sizeof(native_data_t));

    dest->filename = source->filename;

    if (source->xsize < xsize) {
        dest->xsize = xsize;
        xstart = ((xsize - source->xsize) / 16) * 8;
        xendamount = xsize - xstart - source->xsize;
    } else {
        dest->xsize = source->xsize;
    }

    if (source->ysize < ysize) {
        dest->ysize = ysize;
        ystart = ((ysize - source->ysize) / 16) * 8;
        yendamount = ysize - ystart - source->ysize;
    } else {
        dest->ysize = source->ysize;
    }

    dest->colormap = lib_malloc(dest->xsize * dest->ysize);

    k = 0;
    l = 0;

    /* render top border if needed */
    for (i = 0; i < ystart; i++) {
        for (j = 0; j < dest->xsize; j++) {
            dest->colormap[k++] = bordercolor;
        }
    }

    for (i = 0; i < source->ysize; i++) {
        /* render left border if needed */
        for (j = 0; j < xstart; j++) {
            dest->colormap[k++] = bordercolor;
        }

        /* copy screen data */
        for (j = 0; j < source->xsize; j++) {
            dest->colormap[k++] = source->colormap[l++];
        }

        /* render right border if needed */
        for (j = 0; j < xendamount; j++) {
            dest->colormap[k++] = bordercolor;
        }
    }

    /* render bottom border if needed */
    for (i = 0; i < yendamount; i++) {
        for (j = 0; j < dest->xsize; j++) {
            dest->colormap[k++] = bordercolor;
        }
    }

    lib_free(source->colormap);
    lib_free(source);

    return dest;
}

native_data_t *native_crop_and_borderize_colormap(native_data_t *source, uint8_t bordercolor, int xsize, int ysize, int oversize_handling)
{
    int startx;
    int starty;
    int skipxstart = 0;
    int skipxend = 0;
    int skipystart = 0;
    int i, j, k, l;
    native_data_t *dest = lib_malloc(sizeof(native_data_t));

    dest->filename = source->filename;

    startx = (xsize - source->xsize) / 2;
    starty = (ysize - source->ysize) / 2;

    if (source->xsize > xsize) {
        dest->xsize = xsize;
    } else {
        dest->xsize = source->xsize;
    }

    if (source->ysize > ysize) {
        dest->ysize = ysize;
    } else {
        dest->ysize = source->ysize;
    }

    dest->colormap = lib_malloc(dest->xsize * dest->ysize);

    if (startx < 0) {
        switch (oversize_handling) {
            default:
            case NATIVE_SS_OVERSIZE_CROP_LEFT_TOP:
            case NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER:
            case NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM:
                skipxend = source->xsize - xsize;
                break;
            case NATIVE_SS_OVERSIZE_CROP_CENTER_TOP:
            case NATIVE_SS_OVERSIZE_CROP_CENTER:
            case NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM:
                skipxstart = 0 - startx;
                skipxend = source->xsize - xsize - skipxstart;
                break;
            case NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP:
            case NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER:
            case NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM:
                skipxstart = source->xsize - xsize;
                break;
        }
        startx = 0;
    } else {
        startx = ((xsize - source->xsize) / 16) * 8;
    }

    if (starty < 0) {
        switch (oversize_handling) {
            default:
            case NATIVE_SS_OVERSIZE_CROP_LEFT_TOP:
            case NATIVE_SS_OVERSIZE_CROP_CENTER_TOP:
            case NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP:
                break;
            case NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER:
            case NATIVE_SS_OVERSIZE_CROP_CENTER:
            case NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER:
                skipystart = 0 - starty;
                break;
            case NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM:
            case NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM:
            case NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM:
                skipystart = source->ysize - ysize;
                break;
        }
        starty = 0;
    } else {
        starty = ((ysize - source->ysize) / 16) * 8;
    }

    k = 0;
    l = 0;

    /* skip top lines for cropping if needed */
    for (i = 0; i < skipystart; i++) {
        for (j = 0; j < source->ysize; j++) {
            l++;
        }
    }

    /* render top border if needed */
    for (i = 0; i < starty; i++) {
        for (j = 0; j < xsize; j++) {
            dest->colormap[k++] = bordercolor;
        }
    }

    for (i = starty; i < starty + dest->ysize; i++) {
        /* skip right part for cropping if needed */
        for (j = 0; j < skipxstart; j++) {
            l++;
        }

        /* render left border if needed */
        for (j = 0; j < startx; j++) {
            dest->colormap[k++] = bordercolor;
        }

        /* copy main body */
        for (j = startx; j < startx + dest->xsize; j++) {
            dest->colormap[k++] = source->colormap[l++];
        }

        /* render right border if needed */
        for (j = startx + dest->xsize; j < xsize; j++) {
            dest->colormap[k++] = bordercolor;
        }

        /* skip right part for cropping if needed */
        for (j = 0; j < skipxend; j++) {
            l++;
        }
    }

    /* render bottom border if needed */
    for (i = starty + dest->ysize; i < ysize; i++) {
        for (j = 0; j < xsize; j++) {
            dest->colormap[k++] = bordercolor;
        }
    }

    lib_free(source->colormap);
    lib_free(source);

    return dest;
}

native_data_t *native_scale_colormap(native_data_t *source, int xsize, int ysize)
{
    native_data_t *dest = lib_malloc(sizeof(native_data_t));
    int i, j;
    int xmult, ymult;

    dest->filename = source->filename;

    dest->xsize = xsize;
    dest->ysize = ysize;

    dest->colormap = lib_malloc(xsize * ysize);

    xmult = (source->xsize << 8) / xsize;
    ymult = (source->ysize << 8) / ysize;

    for (i = 0; i < ysize; i++) {
        for (j = 0; j < xsize; j++) {
            dest->colormap[(i * xsize) + j] = source->colormap[(((i * ymult) >> 8) * source->xsize) + ((j * xmult) >> 8)];
        }
    }

    lib_free(source->colormap);
    lib_free(source);

    return dest;
}

native_data_t *native_resize_colormap(native_data_t *source, int xsize, int ysize, uint8_t bordercolor, int oversize_handling, int undersize_handling)
{
    native_data_t *data = source;
    int mc_data_present = source->mc_data_present;

    if (data->xsize > xsize) {
        if (oversize_handling == NATIVE_SS_OVERSIZE_SCALE) {
            data = native_scale_colormap(data, xsize, data->ysize);
        } else {
            data = native_crop_and_borderize_colormap(data, bordercolor, xsize, data->ysize, oversize_handling);
        }
    }

    if (data->xsize < xsize) {
        if (undersize_handling == NATIVE_SS_UNDERSIZE_SCALE) {
            data = native_scale_colormap(data, xsize, data->ysize);
        } else {
            data = native_borderize_colormap(data, bordercolor, xsize, data->ysize);
        }
    }

    if (data->ysize > ysize) {
        if (oversize_handling == NATIVE_SS_OVERSIZE_SCALE) {
            data = native_scale_colormap(data, xsize, ysize);
        } else {
            data = native_crop_and_borderize_colormap(data, bordercolor, xsize, ysize, oversize_handling);
        }
    }

    if (data->ysize < ysize) {
        if (undersize_handling == NATIVE_SS_UNDERSIZE_SCALE) {
            data = native_scale_colormap(data, xsize, ysize);
        } else {
            data = native_borderize_colormap(data, bordercolor, xsize, ysize);
        }
    }

    data->mc_data_present = mc_data_present;

    return data;
}

native_color_sort_t *native_sort_colors_colormap(native_data_t *source, int color_amount)
{
    int i, j;
    uint8_t color;
    int highest;
    int amount;
    int highestindex = 0;
    native_color_sort_t *colors = lib_malloc(sizeof(native_color_sort_t) * color_amount);

    for (i = 0; i < color_amount; i++) {
        colors[i].color = i;
        colors[i].amount = 0;
    }

    /* count the colors used */
    for (i = 0; i < (source->xsize * source->ysize); i++) {
        colors[source->colormap[i]].amount++;
    }

    /* sort colors from highest to lowest */
    for (i = 0; i < color_amount; i++) {
        highest = 0;
        for (j = i; j < color_amount; j++) {
            if (colors[j].amount >= highest) {
                highest = colors[j].amount;
                highestindex = j;
            }
        }
        color = colors[i].color;
        amount = colors[i].amount;
        colors[i].color = colors[highestindex].color;
        colors[i].amount = colors[highestindex].amount;
        colors[highestindex].color = color;
        colors[highestindex].amount = amount;
    }
    return colors;
}

static uint8_t vicii_color_bw_translate[16] = {
    0,    /* vicii black       (0) -> vicii black (0) */
    1,    /* vicii white       (1) -> vicii white (1) */
    0,    /* vicii red         (2) -> vicii black (0) */
    1,    /* vicii cyan        (3) -> vicii white (1) */
    1,    /* vicii purple      (4) -> vicii white (1) */
    0,    /* vicii green       (5) -> vicii black (0) */
    0,    /* vicii blue        (6) -> vicii black (0) */
    1,    /* vicii yellow      (7) -> vicii white (1) */
    0,    /* vicii orange      (8) -> vicii black (0) */
    0,    /* vicii brown       (9) -> vicii black (0) */
    1,    /* vicii light red   (A) -> vicii white (1) */
    0,    /* vicii dark gray   (B) -> vicii black (0) */
    1,    /* vicii medium gray (C) -> vicii white (1) */
    1,    /* vicii light green (D) -> vicii white (1) */
    1,    /* vicii light blue  (E) -> vicii white (1) */
    1     /* vicii light gray  (F) -> vicii white (1) */
};

static inline uint8_t vicii_color_to_bw(uint8_t color)
{
    return vicii_color_bw_translate[color];
}

void vicii_color_to_vicii_bw_colormap(native_data_t *source)
{
    int i, j;

    for (i = 0; i < 200; i++) {
        for (j = 0; j < 320; j++) {
            source->colormap[(i * 320) + j] = vicii_color_to_bw(source->colormap[(i * 320) + j]);
        }
    }
}

static uint8_t vicii_color_gray_translate[16] = {
    0x0,    /* vicii black       (0) -> vicii black       (0) */
    0xF,    /* vicii white       (1) -> vicii light gray  (F) */
    0xB,    /* vicii red         (2) -> vicii dark gray   (B) */
    0xC,    /* vicii cyan        (3) -> vicii medium gray (C) */
    0xC,    /* vicii purple      (4) -> vicii medium gray (C) */
    0xB,    /* vicii green       (5) -> vicii dark gray   (B) */
    0xB,    /* vicii blue        (6) -> vicii dark gray   (B) */
    0xC,    /* vicii yellow      (7) -> vicii medium gray (C) */
    0xC,    /* vicii orange      (8) -> vicii medium gray (C) */
    0xB,    /* vicii brown       (9) -> vicii dark gray   (B) */
    0xC,    /* vicii light red   (A) -> vicii medium gray (C) */
    0xB,    /* vicii dark gray   (B) -> vicii dark gray   (B) */
    0xC,    /* vicii medium gray (C) -> vicii medium gray (C) */
    0xF,    /* vicii light green (D) -> vicii light gray  (F) */
    0xC,    /* vicii light blue  (E) -> vicii medium gray (C) */
    0xF     /* vicii light gray  (F) -> vicii light gray  (F) */
};

static inline uint8_t vicii_color_to_gray(uint8_t color)
{
    return vicii_color_gray_translate[color];
}

void vicii_color_to_vicii_gray_colormap(native_data_t *source)
{
    int i, j;

    for (i = 0; i < 200; i++) {
        for (j = 0; j < 320; j++) {
            source->colormap[(i * 320) + j] = vicii_color_to_gray(source->colormap[(i * 320) + j]);
        }
    }
}

static uint8_t vicii_closest_color[16][16] = {
    /* vicii black (0) */
    { 0, 9, 11, 2, 6, 8, 5, 12, 4, 10, 14, 3, 13, 15, 7, 1 },

    /* vicii white (1) */
    { 1, 15, 13, 7, 3, 10, 14, 12, 4, 5, 11, 8, 6, 2, 9, 0 },

    /* vicii red (2) */
    { 2, 8, 9, 11, 0, 10, 12, 5, 4, 6, 7, 14, 15, 3, 13, 1 },

    /* vicii cyan (3) */
    { 3, 13, 14, 15, 12, 10, 5, 7, 4, 11, 1, 6, 8, 9, 2, 0 },

    /* vicii purple (4) */
    { 4, 10, 12, 11, 15, 14, 6, 8, 2, 3, 13, 9, 7, 5, 1, 0 },

    /* vicii green (5) */
    { 5, 11, 12, 8, 9, 3, 10, 2, 13, 7, 14, 15, 0, 4, 6, 1 },

    /* vicii blue (6) */
    { 6, 11, 9, 0, 4, 12, 14, 2, 8, 10, 3, 5, 13, 15, 7, 1 },

    /* vicii yellow (7) */
    { 7, 13, 15, 10, 3, 12, 1, 5, 8, 4, 14, 11, 2, 9, 6, 0 },

    /* vicii orange (8) */
    { 8, 2, 9, 11, 10, 5, 12, 4, 0, 7, 6, 15, 3, 14, 13, 1 },

    /* vicii brown (9) */
    { 9, 11, 2, 0, 8, 6, 5, 12, 4, 10, 14, 3, 15, 13, 7, 1 },

    /* vicii light red (10) */
    { 10, 12, 4, 15, 7, 8, 3, 11, 13, 14, 2, 5, 9, 1, 6, 0 },

    /* vicii dark gray (11) */
    { 11, 9, 12, 6, 2, 8, 5, 0, 4, 10, 14, 3, 15, 13, 7, 1 },

    /* vicii medium gray (12) */
    { 12, 10, 4, 3, 14, 11, 15, 5, 13, 8, 9, 6, 7, 2, 1, 0 },

    /* vicii light green (13) */
    { 13, 3, 15, 7, 12, 15, 1, 10, 5, 4, 11, 8, 9, 2, 6, 0 },

    /* vicii light blue (14) */
    { 14, 3, 12, 11, 4, 13, 6, 11, 10, 5, 9, 1, 7, 8, 2, 0 },

    /* vicii light gray (15) */
    { 15, 13, 3, 12, 14, 10, 7, 1, 4, 5, 11, 8, 6, 9, 2, 0 }
};

static inline uint8_t vicii_color_to_nearest_color(uint8_t color, native_color_sort_t *altcolors)
{
    int i, j;

    for (i = 0; i < 16; i++) {
        for (j = 0; altcolors[j].color != 255; j++) {
            if (vicii_closest_color[color][i] == altcolors[j].color) {
                return vicii_closest_color[color][i];
            }
        }
    }
    return 0;
}

void vicii_color_to_nearest_vicii_color_colormap(native_data_t *source, native_color_sort_t *colors)
{
    int i, j;

    for (i = 0; i < source->ysize; i++) {
        for (j = 0; j < source->xsize; j++) {
            source->colormap[(i * source->xsize) + j] = vicii_color_to_nearest_color(source->colormap[(i * source->xsize) + j], colors);
        }
    }
}

/* ------------------------------------------------------------------------ */

native_data_t *native_vicii_text_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = 320;
    data->ysize = 200;
    data->colormap = lib_malloc(320 * 200);

    bgcolor = regs[0x21] & 0xf;

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            fgcolor = screenshot->color_ram_ptr[(i * 40) + j] & 0xf;
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->chargen_ptr[(screenshot->screen_ptr[(i * 40) + j] * 8) + k];
                for (l = 0; l < 8; l++) {
                    if (bitmap & (1 << (7 - l))) {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = fgcolor;
                    } else {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = bgcolor;
                    }
                }
            }
        }
    }
    if (((regs[0x16] & 8) == 0) || ((regs[0x11] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, (uint8_t)(regs[0x20] & 0xf), (uint8_t)((regs[0x16] & 8) ? 255 : regs[0x16] & 7), (uint8_t)((regs[0x11] & 8) ? 255 : regs[0x11] & 7));
    }
    return data;
}

native_data_t *native_vicii_extended_background_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = 320;
    data->ysize = 200;
    data->colormap = lib_malloc(320 * 200);

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            fgcolor = screenshot->color_ram_ptr[(i * 40) + j] & 0xf;
            bgcolor = regs[0x21 + ((screenshot->screen_ptr[(i * 40) + j] & 0xc0) >> 6)] & 0xf;
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->chargen_ptr[((screenshot->screen_ptr[(i * 40) + j] & 0x3f) * 8) + k];
                for (l = 0; l < 8; l++) {
                    if (bitmap & (1 << (7 - l))) {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = fgcolor;
                    } else {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = bgcolor;
                    }
                }
            }
        }
    }
    if (((regs[0x16] & 8) == 0) || ((regs[0x11] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, (uint8_t)(regs[0x20] & 0xf), (uint8_t)((regs[0x16] & 8) ? 255 : regs[0x16] & 7), (uint8_t)((regs[0x11] & 8) ? 255 : regs[0x11] & 7));
    }
    return data;
}

native_data_t *native_vicii_multicolor_text_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t color0;
    uint8_t color1;
    uint8_t color2;
    uint8_t color3;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = 320;
    data->ysize = 200;
    data->colormap = lib_malloc(320 * 200);

    color0 = regs[0x21] & 0xf;
    color1 = regs[0x22] & 0xf;
    color2 = regs[0x23] & 0xf;

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            color3 = screenshot->color_ram_ptr[(i * 40) + j] & 0xf;
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->chargen_ptr[(screenshot->screen_ptr[(i * 40) + j] * 8) + k];
                if (color3 & 8) {
                    for (l = 0; l < 4; l++) {
                        data->mc_data_present = 1;
                        switch ((bitmap & (3 << ((3 - l) * 2))) >> ((3 - l) * 2)) {
                            case 0:
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color0;
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color0;
                                break;
                            case 1:
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color1;
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color1;
                                break;
                            case 2:
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color2;
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color2;
                                break;
                            case 3:
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color3 & 7;
                                data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color3 & 7;
                                break;
                        }
                    }
                } else {
                    for (l = 0; l < 8; l++) {
                        if (bitmap & (1 << (7 - l))) {
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = color3;
                        } else {
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = color0;
                        }
                    }
                }
            }
        }
    }
    if (((regs[0x16] & 8) == 0) || ((regs[0x11] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, (uint8_t)(regs[0x20] & 0xf), (uint8_t)((regs[0x16] & 8) ? 255 : regs[0x16] & 7), (uint8_t)((regs[0x11] & 8) ? 255 : regs[0x11] & 7));
    }
    return data;
}

native_data_t *native_vicii_hires_bitmap_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = 320;
    data->ysize = 200;
    data->colormap = lib_malloc(320 * 200);

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            fgcolor = (screenshot->screen_ptr[(i * 40) + j] & 0xf0) >> 4;
            bgcolor = screenshot->screen_ptr[(i * 40) + j] & 0xf;
            for (k = 0; k < 8; k++) {
                if (((i * 40 * 8) + (j * 8) + k) < 4096) {
                    bitmap = screenshot->bitmap_low_ptr[(i * 40 * 8) + (j * 8) + k];
                } else {
                    bitmap = screenshot->bitmap_high_ptr[((i * 40 * 8) + (j * 8) + k) - 4096];
                }
                for (l = 0; l < 8; l++) {
                    if (bitmap & (1 << (7 - l))) {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = fgcolor;
                    } else {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = bgcolor;
                    }
                }
            }
        }
    }
    if (((regs[0x16] & 8) == 0) || ((regs[0x11] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, (uint8_t)(regs[0x20] & 0xf), (uint8_t)((regs[0x16] & 8) ? 255 : regs[0x16] & 7), (uint8_t)((regs[0x11] & 8) ? 255 : regs[0x11] & 7));
    }
    return data;
}

native_data_t *native_vicii_multicolor_bitmap_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t color0;
    uint8_t color1;
    uint8_t color2;
    uint8_t color3;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 1;

    data->xsize = 320;
    data->ysize = 200;
    data->colormap = lib_malloc(320 * 200);

    color0 = regs[0x21] & 0xf;
    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            color1 = (screenshot->screen_ptr[(i * 40) + j] & 0xf0) >> 4;
            color2 = screenshot->screen_ptr[(i * 40) + j] & 0xf;
            color3 = screenshot->color_ram_ptr[(i * 40) + j] & 0xf;
            for (k = 0; k < 8; k++) {
                if (((i * 40 * 8) + (j * 8) + k) < 4096) {
                    bitmap = screenshot->bitmap_low_ptr[(i * 40 * 8) + (j * 8) + k];
                } else {
                    bitmap = screenshot->bitmap_high_ptr[((i * 40 * 8) + (j * 8) + k) - 4096];
                }
                for (l = 0; l < 4; l++) {
                    switch ((bitmap & (3 << ((3 - l) * 2))) >> ((3 - l) * 2)) {
                        case 0:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color0;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color0;
                            break;
                        case 1:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color1;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color1;
                            break;
                        case 2:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color2;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color2;
                            break;
                        case 3:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color3;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color3;
                            break;
                    }
                }
            }
        }
    }
    if (((regs[0x16] & 8) == 0) || ((regs[0x11] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, (uint8_t)(regs[0x20] & 0xf), (uint8_t)((regs[0x16] & 8) ? 255 : regs[0x16] & 7), (uint8_t)((regs[0x11] & 8) ? 255 : regs[0x11] & 7));
    }
    return data;
}

/* ------------------------------------------------------------------------ */

static uint8_t ted_vicii_translate[16] = {
    0x0,    /* ted black        (0) -> vicii black       (0) */
    0x1,    /* ted white        (1) -> vicii white       (1) */
    0x2,    /* ted red          (2) -> vicii red         (2) */
    0x3,    /* ted cyan         (3) -> vicii cyan        (3) */
    0x4,    /* ted purple       (4) -> vicii purple      (4) */
    0x5,    /* ted green        (5) -> vicii green       (5) */
    0x6,    /* ted blue         (6) -> vicii blue        (6) */
    0x7,    /* ted yellow       (7) -> vicii yellow      (7) */
    0x8,    /* ted orange       (8) -> vicii orange      (8) */
    0x9,    /* ted brown        (9) -> vicii brown       (9) */
    0xD,    /* ted yellow-green (A) -> vicii light green (D) */
    0xA,    /* ted pink         (B) -> vicii light red   (A) */
    0xE,    /* ted blue-green   (C) -> vicii light blue  (E) */
    0xE,    /* ted light blue   (D) -> vicii light blue  (E) */
    0x6,    /* ted dark blue    (E) -> vicii blue        (6) */
    0xD     /* ted light green  (F) -> vicii light green (D) */
};

static uint8_t ted_to_vicii_color(uint8_t color)
{
    return ted_vicii_translate[color];
}

static uint8_t ted_lum_vicii_translate[16 * 8] = {
    0x0,    /* ted black L0        (0) -> vicii black     (0) */
    0x9,    /* ted white L0        (1) -> vicii brown     (9) */
    0x2,    /* ted red L0          (2) -> vicii red       (2) */
    0xB,    /* ted cyan L0         (3) -> vicii dark gray (B) */
    0x6,    /* ted purple L0       (4) -> vicii blue      (6) */
    0x0,    /* ted green L0        (5) -> vicii black     (0) */
    0x6,    /* ted blue L0         (6) -> vicii blue      (6) */
    0x9,    /* ted yellow L0       (7) -> vicii brown     (9) */
    0x9,    /* ted orange L0       (8) -> vicii brown     (9) */
    0x9,    /* ted brown L0        (9) -> vicii brown     (9) */
    0x9,    /* ted yellow-green L0 (A) -> vicii brown     (9) */
    0x9,    /* ted pink L0         (B) -> vicii brown     (9) */
    0x0,    /* ted blue-green L0   (C) -> vicii black     (0) */
    0x6,    /* ted light blue L0   (D) -> vicii blue      (6) */
    0x6,    /* ted dark blue L0    (E) -> vicii blue      (6) */
    0x9,    /* ted light green L0  (F) -> vicii brown     (9) */

    0x0,    /* ted black L1        (0) -> vicii black     (0) */
    0xB,    /* ted white L1        (1) -> vicii dark gray (B) */
    0x2,    /* ted red L1          (2) -> vicii red       (2) */
    0xB,    /* ted cyan L1         (3) -> vicii dark gray (B) */
    0x6,    /* ted purple L1       (4) -> vicii blue      (6) */
    0x9,    /* ted green L1        (5) -> vicii brown     (9) */
    0x6,    /* ted blue L1         (6) -> vicii blue      (6) */
    0x9,    /* ted yellow L1       (7) -> vicii brown     (9) */
    0x2,    /* ted orange L1       (8) -> vicii red       (2) */
    0x9,    /* ted brown L1        (9) -> vicii brown     (9) */
    0x9,    /* ted yellow-green L1 (A) -> vicii brown     (9) */
    0xB,    /* ted pink L1         (B) -> vicii dark gray (B) */
    0xB,    /* ted blue-green L1   (C) -> vicii dark gray (B) */
    0x6,    /* ted light blue L1   (D) -> vicii blue      (6) */
    0x6,    /* ted dark blue L1    (E) -> vicii blue      (6) */
    0x9,    /* ted light green L1  (F) -> vicii brown     (9) */

    0x0,    /* ted black L2        (0) -> vicii black     (0) */
    0xB,    /* ted white L2        (1) -> vicii dark gray (B) */
    0x2,    /* ted red L2          (2) -> vicii red       (2) */
    0xB,    /* ted cyan L2         (3) -> vicii dark gray (B) */
    0x4,    /* ted purple L2       (4) -> vicii purple    (4) */
    0x9,    /* ted green L2        (5) -> vicii brown     (9) */
    0x6,    /* ted blue L2         (6) -> vicii blue      (6) */
    0x9,    /* ted yellow L2       (7) -> vicii brown     (9) */
    0x2,    /* ted orange L2       (8) -> vicii red       (2) */
    0x9,    /* ted brown L2        (9) -> vicii brown     (9) */
    0x9,    /* ted yellow-green L2 (A) -> vicii brown     (9) */
    0xB,    /* ted pink L2         (B) -> vicii dark gray (B) */
    0xB,    /* ted blue-green L2   (C) -> vicii dark gray (B) */
    0x6,    /* ted light blue L2   (D) -> vicii blue      (6) */
    0x6,    /* ted dark blue L2    (E) -> vicii blue      (6) */
    0x9,    /* ted light green L2  (F) -> vicii brown     (9) */

    0x0,    /* ted black L3        (0) -> vicii black     (0) */
    0xB,    /* ted white L3        (1) -> vicii dark gray (B) */
    0x2,    /* ted red L3          (2) -> vicii red       (2) */
    0xB,    /* ted cyan L3         (3) -> vicii dark gray (B) */
    0x4,    /* ted purple L3       (4) -> vicii purple    (4) */
    0x9,    /* ted green L3        (5) -> vicii brown     (9) */
    0x6,    /* ted blue L3         (6) -> vicii blue      (6) */
    0x9,    /* ted yellow L3       (7) -> vicii brown     (9) */
    0x8,    /* ted orange L3       (8) -> vicii orange    (8) */
    0x8,    /* ted brown L3        (9) -> vicii orange    (8) */
    0x9,    /* ted yellow-green L3 (A) -> vicii brown     (9) */
    0x4,    /* ted pink L3         (B) -> vicii purple    (4) */
    0xB,    /* ted blue-green L3   (C) -> vicii dark gray (B) */
    0x6,    /* ted light blue L3   (D) -> vicii blue      (6) */
    0x6,    /* ted dark blue L3    (E) -> vicii blue      (6) */
    0x9,    /* ted light green L3  (F) -> vicii brown     (9) */

    0x0,    /* ted black L4        (0) -> vicii black       (0) */
    0xC,    /* ted white L4        (1) -> vicii medium gray (C) */
    0xA,    /* ted red L4          (2) -> vicii light red   (A) */
    0xE,    /* ted cyan L4         (3) -> vicii light blue  (E) */
    0x4,    /* ted purple L4       (4) -> vicii purple      (4) */
    0x5,    /* ted green L4        (5) -> vicii green       (5) */
    0xE,    /* ted blue L4         (6) -> vicii light blue  (E) */
    0x5,    /* ted yellow L4       (7) -> vicii green       (5) */
    0xA,    /* ted orange L4       (8) -> vicii light red   (A) */
    0x8,    /* ted brown L4        (9) -> vicii orange      (8) */
    0x5,    /* ted yellow-green L4 (A) -> vicii green       (5) */
    0x4,    /* ted pink L4         (B) -> vicii purple      (4) */
    0xC,    /* ted blue-green L4   (C) -> vicii medium gray (C) */
    0xE,    /* ted light blue L4   (D) -> vicii light blue  (E) */
    0xE,    /* ted dark blue L4    (E) -> vicii light blue  (E) */
    0x5,    /* ted light green L4  (F) -> vicii green       (5) */

    0x0,    /* ted black L5        (0) -> vicii black       (0) */
    0xC,    /* ted white L5        (1) -> vicii medium gray (C) */
    0xA,    /* ted red L5          (2) -> vicii light red   (A) */
    0x3,    /* ted cyan L5         (3) -> vicii cyan        (3) */
    0xF,    /* ted purple L5       (4) -> vicii light gray  (F) */
    0x5,    /* ted green L5        (5) -> vicii green       (5) */
    0xE,    /* ted blue L5         (6) -> vicii light blue  (E) */
    0x5,    /* ted yellow L5       (7) -> vicii green       (5) */
    0xA,    /* ted orange L5       (8) -> vicii light red   (A) */
    0xA,    /* ted brown L5        (9) -> vicii light red   (A) */
    0x5,    /* ted yellow-green L5 (A) -> vicii green       (5) */
    0xA,    /* ted pink L5         (B) -> vicii light red   (A) */
    0x3,    /* ted blue-green L5   (C) -> vicii cyan        (3) */
    0xE,    /* ted light blue L5   (D) -> vicii light blue  (E) */
    0xE,    /* ted dark blue L5    (E) -> vicii light blue  (E) */
    0x5,    /* ted light green L5  (F) -> vicii green       (5) */

    0x0,    /* ted black L6        (0) -> vicii black       (0) */
    0xF,    /* ted white L6        (1) -> vicii light gray  (F) */
    0xA,    /* ted red L6          (2) -> vicii light red   (A) */
    0x3,    /* ted cyan L6         (3) -> vicii cyan        (3) */
    0xF,    /* ted purple L6       (4) -> vicii light gray  (F) */
    0xD,    /* ted green L6        (5) -> vicii light green (D) */
    0xF,    /* ted blue L6         (6) -> vicii light gray  (F) */
    0x7,    /* ted yellow L6       (7) -> vicii yellow      (7) */
    0xA,    /* ted orange L6       (8) -> vicii light red   (A) */
    0xA,    /* ted brown L6        (9) -> vicii light red   (A) */
    0x7,    /* ted yellow-green L6 (A) -> vicii yellow      (7) */
    0xF,    /* ted pink L6         (B) -> vicii light gray  (F) */
    0x3,    /* ted blue-green L6   (C) -> vicii cyan        (3) */
    0xF,    /* ted light blue L6   (D) -> vicii light gray  (F) */
    0xF,    /* ted dark blue L6    (E) -> vicii light gray  (F) */
    0x7,    /* ted light green L6  (F) -> vicii yellow      (7) */

    0x0,    /* ted black L7        (0) -> vicii black       (0) */
    0x1,    /* ted white L7        (1) -> vicii white       (1) */
    0x1,    /* ted red L7          (2) -> vicii white       (1) */
    0x1,    /* ted cyan L7         (3) -> vicii white       (1) */
    0x1,    /* ted purple L7       (4) -> vicii white       (1) */
    0xD,    /* ted green L7        (5) -> vicii light green (D) */
    0x1,    /* ted blue L7         (6) -> vicii white       (1) */
    0x7,    /* ted yellow L7       (7) -> vicii yellow      (7) */
    0xF,    /* ted orange L7       (8) -> vicii light gray  (F) */
    0x7,    /* ted brown L7        (9) -> vicii yellow      (7) */
    0x7,    /* ted yellow-green L7 (A) -> vicii yellow      (7) */
    0x1,    /* ted pink L7         (B) -> vicii white       (1) */
    0xD,    /* ted blue-green L7   (C) -> vicii light green (D) */
    0x1,    /* ted light blue L7   (D) -> vicii white       (1) */
    0x1,    /* ted dark blue L7    (E) -> vicii white       (1) */
    0xD     /* ted light green L7  (F) -> vicii light green (D) */
};

static uint8_t ted_lum_to_vicii_color(uint8_t color, uint8_t lum)
{
    return ted_lum_vicii_translate[(lum * 16) + color];
}

void ted_color_to_vicii_color_colormap(native_data_t *source, int ted_lum_handling)
{
    int i, j;
    uint8_t colorbyte;

    for (i = 0; i < source->ysize; i++) {
        for (j = 0; j < source->xsize; j++) {
            colorbyte = source->colormap[(i * source->xsize) + j];
            if (ted_lum_handling == NATIVE_SS_TED_LUM_DITHER) {
                source->colormap[(i * source->xsize) + j] = ted_lum_to_vicii_color((uint8_t)(colorbyte & 0xf), (uint8_t)(colorbyte >> 4));
            } else {
                source->colormap[(i * source->xsize) + j] = ted_to_vicii_color((uint8_t)(colorbyte & 0xf));
            }
        }
    }
}

native_data_t *native_ted_text_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    uint8_t brdrcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = 320;
    data->ysize = 200;

    data->colormap = lib_malloc(320 * 200);

    bgcolor = regs[0x15] & 0x7f;

    brdrcolor = regs[0x19] & 0x7f;

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            fgcolor = screenshot->color_ram_ptr[(i * 40) + j] & 0x7f;
            for (k = 0; k < 8; k++) {
                if (regs[0x07] & 0x80) {
                    bitmap = screenshot->chargen_ptr[(screenshot->screen_ptr[(i * 40) + j] * 8) + k];
                } else {
                    bitmap = screenshot->chargen_ptr[((screenshot->screen_ptr[(i * 40) + j] & 0x7f) * 8) + k];
                    if (screenshot->screen_ptr[(i * 40) + j] & 0x80) {
                        bitmap = ~bitmap;
                    }
                }
                for (l = 0; l < 8; l++) {
                    if (bitmap & (1 << (7 - l))) {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = fgcolor;
                    } else {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = bgcolor;
                    }
                }
            }
        }
    }
    if (((regs[0x07] & 8) == 0) || ((regs[0x06] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, brdrcolor, (uint8_t)((regs[0x07] & 8) ? 255 : regs[0x07] & 7), (uint8_t)((regs[0x06] & 8) ? 255 : regs[0x06] & 7));
    }
    return data;
}

native_data_t *native_ted_extended_background_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    uint8_t brdrcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = 320;
    data->ysize = 200;

    data->colormap = lib_malloc(320 * 200);

    brdrcolor = regs[0x19] & 0x7f;

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            fgcolor = screenshot->color_ram_ptr[(i * 40) + j] & 0x7f;
            bgcolor = regs[0x15 + ((screenshot->screen_ptr[(i * 40) + j] & 0xc0) >> 6)] & 0x7f;
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->chargen_ptr[((screenshot->screen_ptr[(i * 40) + j] & 0x3f) * 8) + k];
                if ((regs[0x07] & 0x80) && (screenshot->screen_ptr[(i * 40) + j] & 0x80)) {
                    bitmap = ~bitmap;
                }
                for (l = 0; l < 8; l++) {
                    if (bitmap & (1 << (7 - l))) {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = fgcolor;
                    } else {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = bgcolor;
                    }
                }
            }
        }
    }
    if (((regs[0x07] & 8) == 0) || ((regs[0x06] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, brdrcolor, (uint8_t)((regs[0x07] & 8) ? 255 : regs[0x07] & 7), (uint8_t)((regs[0x06] & 8) ? 255 : regs[0x06] & 7));
    }
    return data;
}

native_data_t *native_ted_hires_bitmap_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    uint8_t brdrcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = 320;
    data->ysize = 200;

    data->colormap = lib_malloc(320 * 200);

    brdrcolor = regs[0x19] & 0x7f;

    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            fgcolor = (screenshot->screen_ptr[(i * 40) + j] & 0xf0) >> 4;
            fgcolor |= (screenshot->screen_ptr[(i * 40) + j] & 0x70);
            bgcolor = screenshot->screen_ptr[(i * 40) + j] & 0xf;
            bgcolor |= ((screenshot->screen_ptr[(i * 40) + j] & 0x7) << 4);
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->bitmap_ptr[(i * 40 * 8) + j + (k * 40)];
                for (l = 0; l < 8; l++) {
                    if (bitmap & (1 << (7 - l))) {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = fgcolor;
                    } else {
                        data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + l] = bgcolor;
                    }
                }
            }
        }
    }
    if (((regs[0x07] & 8) == 0) || ((regs[0x06] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, brdrcolor, (uint8_t)((regs[0x07] & 8) ? 255 : regs[0x07] & 7), (uint8_t)((regs[0x06] & 8) ? 255 : regs[0x06] & 7));
    }
    return data;
}

native_data_t *native_ted_multicolor_bitmap_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t color0;
    uint8_t color1;
    uint8_t color2;
    uint8_t color3;
    uint8_t brdrcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 1;

    data->xsize = 320;
    data->ysize = 200;
    data->colormap = lib_malloc(320 * 200);

    brdrcolor = regs[0x19] & 0x7f;

    color0 = regs[0x15];
    color3 = regs[0x16];
    for (i = 0; i < 25; i++) {
        for (j = 0; j < 40; j++) {
            color1 = (screenshot->screen_ptr[(i * 40) + j] & 0xf0) >> 4;
            color1 |= (screenshot->screen_ptr[(i * 40) + j] & 0x70);
            color2 = screenshot->screen_ptr[(i * 40) + j] & 0xf;
            color2 |= ((screenshot->screen_ptr[(i * 40) + j] & 0x7) << 4);
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->bitmap_ptr[(i * 40 * 8) + j + (k * 40)];
                for (l = 0; l < 4; l++) {
                    switch ((bitmap & (3 << ((3 - l) * 2))) >> ((3 - l) * 2)) {
                        case 0:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color0;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color0;
                            break;
                        case 1:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color1;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color1;
                            break;
                        case 2:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color2;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color2;
                            break;
                        case 3:
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2)] = color3;
                            data->colormap[(i * 320 * 8) + (j * 8) + (k * 320) + (l * 2) + 1] = color3;
                            break;
                    }
                }
            }
        }
    }
    if (((regs[0x07] & 8) == 0) || ((regs[0x06] & 8) == 0)) {
        native_smooth_scroll_borderize_colormap(data, brdrcolor, (uint8_t)((regs[0x07] & 8) ? 255 : regs[0x07] & 7), (uint8_t)((regs[0x06] & 8) ? 255 : regs[0x06] & 7));
    }
    return data;
}

/* ------------------------------------------------------------------------ */

static uint8_t vic_vicii_translate[16] = {
    0x0,    /* vic black        (0) -> vicii black       (0) */
    0x1,    /* vic white        (1) -> vicii white       (1) */
    0x2,    /* vic red          (2) -> vicii red         (2) */
    0x3,    /* vic cyan         (3) -> vicii cyan        (3) */
    0x4,    /* vic purple       (4) -> vicii purple      (4) */
    0x5,    /* vic green        (5) -> vicii green       (5) */
    0x6,    /* vic blue         (6) -> vicii blue        (6) */
    0x7,    /* vic yellow       (7) -> vicii yellow      (7) */
    0x8,    /* vic orange       (8) -> vicii orange      (8) */
    0x8,    /* vic light orange (9) -> vicii orange      (8) */
    0x8,    /* vic pink         (A) -> vicii orange      (8) */
    0xD,    /* vic light cyan   (B) -> vicii light green (D) */
    0x4,    /* vic light purple (C) -> vicii purple      (4) */
    0xD,    /* vic light green  (D) -> vicii light green (D) */
    0xE,    /* vic light blue   (E) -> vicii light blue  (E) */
    0x7     /* vic light yellow (F) -> vicii yellow      (7) */
};

static inline uint8_t vic_to_vicii_color(uint8_t color)
{
    return vic_vicii_translate[color];
}

void vic_color_to_vicii_color_colormap(native_data_t *source)
{
    int i, j;

    for (i = 0; i < source->ysize; i++) {
        for (j = 0; j < source->xsize; j++) {
            source->colormap[(i * source->xsize) + j] = vic_to_vicii_color(source->colormap[(i * source->xsize) + j]);
        }
    }
}

native_data_t *native_vic_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    uint8_t brdrcolor;
    uint8_t auxcolor;
    int i, j, k, l;
    native_data_t *data;
    uint8_t xsize;
    uint8_t ysize;

    xsize = regs[0x02] & 0x7f;
    ysize = (regs[0x03] & 0x7e) >> 1;

    if (xsize == 0 || ysize == 0) {
        ui_error("Screen is blank, no save will be done");
        return NULL;
    }

    if (screenshot->chargen_ptr == NULL) {
        ui_error("Character generator memory is illegal");
        return NULL;
    }

    data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    data->xsize = xsize * 8;
    data->ysize = ysize * 8;

    data->colormap = lib_malloc(data->xsize * data->ysize);

    bgcolor = (regs[0xf] & 0xf0) >> 4;
    auxcolor = (regs[0xe] & 0xf0) >> 4;
    brdrcolor = regs[0xf] & 3;
    for (i = 0; i < ysize; i++) {
        for (j = 0; j < xsize; j++) {
            fgcolor = screenshot->color_ram_ptr[(i * xsize) + j] & 7;
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->chargen_ptr[(screenshot->screen_ptr[(i * xsize) + j] * 8) + k];
                if (!(regs[0xf] & 8)) {
                    bitmap = ~bitmap;
                }
                if (screenshot->color_ram_ptr[(i * xsize) + j] & 8) {
                    for (l = 0; l < 4; l++) {
                        data->mc_data_present = 1;
                        switch ((bitmap & (3 << ((3 - l) * 2))) >> ((3 - l) * 2)) {
                            case 0:
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2)] = bgcolor;
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2) + 1] = bgcolor;
                                break;
                            case 1:
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2)] = brdrcolor;
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2) + 1] = brdrcolor;
                                break;
                            case 2:
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2)] = fgcolor;
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2) + 1] = fgcolor;
                                break;
                            case 3:
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2)] = auxcolor;
                                data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + (l * 2) + 1] = auxcolor;
                                break;
                        }
                    }
                } else {
                    for (l = 0; l < 8; l++) {
                        if (bitmap & (1 << (7 - l))) {
                            data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + l] = fgcolor;
                        } else {
                            data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + l] = bgcolor;
                        }
                    }
                }
            }
        }
    }
    return data;
}

/* ------------------------------------------------------------------------ */

#define MA_WIDTH        64
#define MA_LO           (MA_WIDTH - 1)          /* 6 bits */
#define MA_HI           (~MA_LO)

native_data_t *native_crtc_render(screenshot_t *screenshot, const char *filename, int crtc_fgcolor)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t *petdww_ram = screenshot->bitmap_ptr;
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    int x, y, k, l;
    native_data_t *data;
    uint8_t xsize;
    uint8_t ysize;
    uint8_t invert;
    uint8_t charheight;
    int base;
    int shiftand;
    int chars = 1;
    int hre = 0;
    int col80;
    int scr_rel;

    switch (screenshot->bitmap_low_ptr[0]) {
        default:
        case 40:
            xsize = regs[0x01];
            base = ((regs[0x0c] & 3) << 8) + regs[0x0d];
            shiftand = 0x3ff;
            col80 = 0;
            break;
        case 60:
            xsize = regs[0x01];
            base = ((regs[0x0c] & 3) << 8) + regs[0x0d];
            shiftand = 0x7ff;
            col80 = 0;
            break;
        case 80:
            xsize = regs[0x01] << 1;
            base = (((regs[0x0c] & 3) << 9) + regs[0x0d]) << 1;
            shiftand = 0x7ff;
            col80 = 1;
            break;
    }

    ysize = regs[0x06];
    invert = (regs[0x0c] & 0x10) >> 4;

    if (!invert) {      /* On 8296 only! */
        hre = 1;
        chars = 0;
        invert = 1;
    }

    if (!hre && (xsize == 0 || ysize == 0)) {
        ui_error("Screen is blank, no save will be done");
        return NULL;
    }

    charheight = screenshot->bitmap_high_ptr[0];

    data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    if (hre) {
        data->xsize = 512;
        data->ysize = 256;
    } else {
        data->xsize = xsize * 8;
        data->ysize = ysize * charheight;
    }

    data->colormap = lib_malloc(data->xsize * data->ysize);

    bitmap = 0;
    bgcolor = 0;
    fgcolor = crtc_fgcolor;
    scr_rel = base;

    if (hre) {
        int ma_hi = scr_rel & MA_HI;    /* MA<9...6> */
        int ma_lo = scr_rel & MA_LO;    /* MA<5...0> */
        /* Form <MA 9-6><RA 2-0><MA 5-0> */
        int addr = ((ma_hi << 3) + ma_lo) >> 1;

        for (l = 0; l < 16384; l++) {
            bitmap = screenshot->screen_ptr[addr + l];
            for (k = 0; k < 8; k++) {
                if (bitmap & (1 << (7 - k))) {
                    data->colormap[(l * 8) + k] = fgcolor;
                } else {
                    data->colormap[(l * 8) + k] = bgcolor;
                }
            }
        }
    } else {
        for (y = 0; y < ysize; y++) {
            for (x = 0; x < xsize; x++) {
                for (k = 0; k < charheight; k++) {
                    bitmap = 0;
                    if (chars) {
                        uint8_t chr = screenshot->screen_ptr[scr_rel & shiftand];
                        bitmap = screenshot->chargen_ptr[(chr * 16) + k];
                    }
                    if (petdww_ram && k < 8) {
                        int addr = (k * 1024) + ((scr_rel >> col80) & 0x3FF);
                        uint8_t b = petdww_ram[addr];
                        /*
                         * If we're in an 80 column screen we need to
                         * horizontally double all pixels, since DWW
                         * only has 40 characters worth of pixels.
                         * Fetch the same byte for odd and even matrix
                         * addresses (above), then double the bits from
                         * first the left half (low nybble) and then the
                         * right half (high nybble).
                         */
                        if (col80) {
                            if (x & 1) {
                                b >>= 4;        /* show right half */
                            } else {
                                b &= 0x0F;      /* show left half */
                            }
                            /*
                             * Double the bits by interleaving with self
                             * http://graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
                             */
                            b = (b | (b << 2)) & 0x33;
                            b = (b | (b << 1)) & 0x55;
                            b |= b << 1;
                        }
                        /*
                        * Now reverse the bits...
                        * http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
                        */
                        b = ((b * 0x0802U & 0x22110U) | (b * 0x8020U & 0x88440U)) * 0x10101U >> 16;
                        bitmap |= b;
                    }
                }
                if (!invert) {
                    bitmap = ~bitmap;
                }
                for (l = 0; l < 8; l++) {
                    int color;

                    if (bitmap & (1 << (7 - l))) {
                        color = fgcolor;
                    } else {
                        color = bgcolor;
                    }
                    data->colormap[(y * data->xsize * charheight) + (x * 8) + (k * data->xsize) + l] = color;
                }
            }
            scr_rel++;
        }
    }
    return data;
}

/* ------------------------------------------------------------------------ */

static uint8_t vdc_vicii_translate[16] = {
    0x0,    /* vdc black        (0) -> vicii black       (0) */
    0xB,    /* vdc dark gray    (1) -> vicii dark gray   (B) */
    0x6,    /* vdc dark blue    (2) -> vicii blue        (6) */
    0xE,    /* vdc light blue   (3) -> vicii light blue  (E) */
    0x5,    /* vdc dark green   (4) -> vicii green       (5) */
    0xD,    /* vdc light green  (5) -> vicii light green (D) */
    0x6,    /* vdc dark cyan    (6) -> vicii blue        (6) */
    0x3,    /* vdc light cyan   (7) -> vicii cyan        (3) */
    0x2,    /* vdc dark red     (8) -> vicii red         (2) */
    0x8,    /* vdc light red    (9) -> vicii orange      (8) */
    0x4,    /* vdc dark purple  (A) -> vicii purple      (4) */
    0xA,    /* vdc light purple (B) -> vicii light red   (A) */
    0x7,    /* vdc dark yellow  (C) -> vicii yellow      (7) */
    0x7,    /* vdc light yellow (D) -> vicii yellow      (7) */
    0xF,    /* vdc light gray   (E) -> vicii light gray  (F) */
    0x1     /* vdc white        (F) -> vicii white       (1) */
};

static inline uint8_t vdc_to_vicii_color(uint8_t color)
{
    return vdc_vicii_translate[color];
}

void vdc_color_to_vicii_color_colormap(native_data_t *source)
{
    int i, j;

    for (i = 0; i < source->ysize; i++) {
        for (j = 0; j < source->xsize; j++) {
            source->colormap[(i * source->xsize) + j] = vdc_to_vicii_color(source->colormap[(i * source->xsize) + j]);
        }
    }
}

native_data_t *native_vdc_text_mode_render(screenshot_t *screenshot, const char *filename)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t displayed_chars_h = regs[1];
    uint8_t displayed_chars_v = regs[6];
    /* BYTE scanlines_per_char = (regs[9] & 0x1f) + 1;
    BYTE char_h_size_alloc = regs[22] & 0xf;
    BYTE char_h_size_displayed = (regs[22] & 0xf0) >> 4; */
    uint8_t bitmap;
    uint8_t fgcolor;
    uint8_t bgcolor;
    int i, j, k, l;
    native_data_t *data = lib_malloc(sizeof(native_data_t));

    data->filename = filename;
    data->mc_data_present = 0;

    /* X size calculation is not completely correct,
       char x and double pixel is not taken into account yet,
       correct calculation will become : data->xsize = displayed_chars_h * char_h_size_displayed * ((regs[25] & 0x10) ? 2 : 1);
     */
    data->xsize = displayed_chars_h * 8;

    /* Y size calculation is not completely correct,
       scanlines per char is not taken into account yet,
       correct calculation will become : data->ysize = displayed_chars_v * scanlines_per_char;
     */
    data->ysize = displayed_chars_v * 8;

    data->colormap = lib_malloc(data->xsize * data->ysize);

    fgcolor = 1;
    bgcolor = regs[0x26] & 0xf;

    if (!(regs[25] & 0x40)) {
        fgcolor = (regs[26] & 0xf0) >> 4;
    }

    /* bitmap filling is not completely correct,
      char x allocation, char x display, skipped chars, smooth scrolling and
      blanked borders are not taken into account yet.
     */
    for (i = 0; i < data->ysize / 8; i++) {
        for (j = 0; j < data->xsize / 8; j++) {
            if (regs[25] & 0x40) {
                fgcolor = screenshot->color_ram_ptr[(i * (data->xsize / 8)) + j] & 0x7f;
            }
            for (k = 0; k < 8; k++) {
                bitmap = screenshot->chargen_ptr[(screenshot->screen_ptr[(i * data->xsize / 8) + j] * 16) + k];
                for (l = 0; l < 8; l++) {
                    if (bitmap & (1 << (7 - l))) {
                        data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + l] = fgcolor;
                    } else {
                        data->colormap[(i * data->xsize * 8) + (j * 8) + (k * data->xsize) + l] = bgcolor;
                    }
                }
            }
        }
    }
    return data;
}

/*
                              ------------------------
                              | Register Definitions |
                              ------------------------

Reg#     7    6    5    4    3    2    1    0     Description              Notes
------- ---- ---- ---- ---- ---- ---- ---- ----   ------------------------ -----
  1     HzD7 HzD6 HzD5 HzD4 HzD3 HzD2 HzD1 HzD0   Horizontal Displayed      ^1
  6     VeD7 VeD6 VeD5 VeD4 VeD3 VeD2 VeD1 VeD0   Vertical Displayed        ^3
  9     .... .... .... CTV4 CTV3 CTV2 CTV1 CTV0   Character Total Vertical  ^5
 12     Ds15 Ds14 Ds13 Ds12 Ds11 Ds10 Ds09 Ds08   Display Start Adrs (Hi)   ^7
 13     Ds07 Ds06 Ds05 Ds04 Ds03 Ds02 Ds01 Ds00   Display Start Adrs (Lo)   ^7
 20     At15 At14 At13 At12 At11 At10 At09 At08   Attribute Start Adrs (Hi) ^7
 21     At07 At06 At05 At04 At03 At02 At01 At00   Attribute Start Adrs (Lo) ^7
 22     HcP3 HcP2 HcP1 HcP0 IcS3 IcS2 IcS1 IcS0   Hz Chr Pxl Ttl/IChar Spc  ^A
 23     .... .... .... VcP4 VcP3 VcP2 VcP1 VcP0   Vert. Character Pxl Spc   ^5
 24     BlkM RvsS Vss5 Vss4 Vss3 Vss2 Vss1 Vss0   Block/Rvs Scr/V. Scroll ^9^B^C
 25     Text Atri Semi Dble Hss3 Hss2 Hss1 Hss0   Diff. Mode Sw/H. Scroll  ^D,^E
 26     Fgd3 Fgd2 Fgd1 Fgd0 Bgd3 Bgd2 Bgd1 Bgd0   ForeGround/BackGround Col ^F
 27     Rin7 Rin6 Rin5 Rin4 Rin3 Rin2 Rin1 Rin0   Row/Adrs. Increment       ^G
 28     CSa2 CSa1 CSa0 RamT .... .... .... ....   Character Set Addrs/Ram  ^H,^I
 34     DeB7 DeB6 DeB5 DeB4 DeB3 DeB2 DeB1 DeB0   Display Enable Begin      ^J
 35     DeE7 DeE6 DeE5 DeE4 DeE3 DeE2 DeE1 DeE0   Display Enable End        ^J

                             +-----------------+
                             | Register Usage: |
                             +-----------------+

---  Register #1:     Horizontal Displayed

  These two register function to define the display width of the screen.
Register 0 will contain the number of characters minus 1 between sucessive
horizontal sync pulses, the horizontal border and the interval between
horizontal sync pulses. The normal value for this is usually set to 126.
Register 1 specifies how many of the positions as specified in register 0 can
actually be used to display characters.  The default value for this is 80.
The VDC can take values less than 80 and thus, will only display that many
characters. A useful effect can be a sweep from the right by incrementing
the value here from 1 to 80. Register #2 specifies the starting character
position at which the vertical sync pulse begins. Thus, it also determines
where on the active screen characters appear. A default value of 102,
increasing the value moves the screen to the left, decreasing it moves it to
the right.

     Register #6:     Vertical Displayed

  Register #4 of this register determines the total number of screen rows,
including the rows for the active display, and the top and bottom borders in
addition to that of the vertical sync width. The value held here is normally
a value of 32 for NTSC systems (US Standard) or 39 for PAL(European) systems.
Register #5 holds in bits 0-4 a "fine-adjust" where any extra scan lines that
are necessary to make up the display can be specified here. The value here is
normally a 0 in both the NTSC and PAL initializations by the kernal and bits
5-7 are unused, always returning a binary 1111. Register #6 specifies the total
number of the vertical character positions (as set in Register 4) that can be
used for actual display of characters. Thus, this register usually holds a
value of 25 for a standard 25-row display.

^5 : Register #9:     Total Scan Lines Per Character
----

  Bits 0-4 of this register are the only relevant ones, the rest returning a
binary 1. Bits 0-4 determine the character height in scan-lines of displayed
characters and allow up to scan-line heights of 32 scan lines. The VDC normally
sets aside 16 bytes for each character (normally, each byte is equivlent to
1 scan line) so the value here could be increased to 16-1 and a double-height
character set could be loaded in. Note, however that values less than 16 will
tell the VDC to use a 8,192 byte character set (normal) while specifying values
greater than 16 will make it use 32 bytes per character even if some of the
bytes are not used.

^7 : Register #12:    Display Start Address (Hi)
---- Register #13:    Display Start Address (Lo)
     Register #20:    Attribute Start Addrs (Hi)
     Register #21:    Attribute Start Addrs (Lo)

  Note first, that all of these registers are grouped in Hi byte, Lo byte order
which is usually different from the 6502 convention of low byte, hi byte (ie:
in normal 6502 ml, $c000 is stored as $00 $c0, however in the 8563 it would be
stored as $c0 $00).  Registers 12 and 13 determine, where in VDC memory the
8563 is the start of the screen. Incrementing this value by 80 (the number of
characters per line) and with a little additional work can provide a very
effecient way of having a screen that "seems" to be larger than just 80x25.
The cursor position in registers 14 and 15 reflect the actual character in
memory that the cursor currently lies over. If it's not on the display screen,
then it is not displayed. Registers 20 and 21 reflect where in the 8563 memory
attribute memory is held. Attribute memory refers to the character attributes
such as flash, inverse video, color etc that can be set for each character.

^A : Register #22:    Character Horizontal Size Control
----

  Bits 0-3 of this register determines how many horizontal pixels are used
for each displayed character. Values greater than 8 here result in apparent
gaps in the display. Inter-character spacing can be achieved by setting this
value greater than that of bits 4-7. Bits 4-7 determine the width of each
character position in pixels. Thus, while bits 0-3 allocate n-pixels, bits
4-7 specify how many of those pixels are used for character display.

^B : Register #24:5   Reverse Screen Bit
---- Register #24:6   Blink Rate for Characters.

  Bit #6 specifies for the VDC for all pixels normally unset on the VDC screen
to be set, and all set pixels to be unset.  Bit #5 specifies the blink rate
for all characters with the blink attribute. Setting this to a binary 1
specifies a blink rate of 1/32 the refresh rate, while a binary 0 is equivlant
to a blink rate 1/16th of the refresh rate.

^C : Register #24:0-4 Vertical Smooth Scroll
----

  The 8563 provides for a smooth scroll, allowing bits 0-4 to function as an
indicator of the number of bits to scroll the screen vertically upward.

^D : Register #25:7   Text or Graphics Mode Indicator Bit
---- Register #25:6   Monochrome Mode Bit
     Register #25:5   Semi-Graphics Mode
     Register #25:4   Double-Pixel Mode

  The 8563 allows the implementation of a graphics mode, in where all of the 16k
of the screen may be bit-mapped sequentially resulting in a resolution of
640x200 (see Craig Bruce's 8563 Line-Plotting routine in the first issue for a
more detailed explanation of this feature). Setting this bit to 1 specifies
graphics mode, binary 0 indicates text mode.  Bit 6 indicates to the 8563 where
to obtain its color information etc, about the characters. Bit 6 when it is a
binary 0 results in the 8563 taking it's color information from bits 4-7 of
register 26. When this bit is a binary 1, the attribute memory is used to
obtain color, flash, reverse information. Also note than when this bit is a
binary 1 that only the first of the two character sets is available. Bit #5
indicates a semi-graphics mode that allows the rightmost pixel of any characters
to be repeated through-out the intercharacter spacing gap. Activating it on the
normal display will result in what appears to be a "digital" character font. The
8563 with bit #4 allows a pixel-double feature which results in all displayed
horizontal pixels having twice their usual size. Thus, a 40 column screen is
easily obtainable although the values in registers #00-#02 must be halved.

^E : Register #25:    Horizontal Smooth Control
----

  This register is analogous to register #24 Vertical Smooth Control and
functions similairly. Increasing this bits moves the screen one pixel to the
right, while decreasing them moves the screen one pixel to the left.

^F : Register #26:    ForeGround / BackGround Color Register
----

  This register, in bits 0-3 specifies the background color of the display while
bits 4-7 specify the foreground character colors when attributes are disabled
(via bit 6 of register #25).  Note, these are not the usual C= colors but are
instead organized as follows:

   Bit Value  Decimal Value   Color
   ----------------------------------       +-----------------------------+
    %0000       0 / $00       Black         |  Note: Bit 0 = Intensity    |
    %0001       1 / $01       Dark Gray     |        Bit 1 = Blue         |
    %0010       2 / $02       Dark Blue     | RGBI   Bit 2 = Green        |
    %0011       3 / $03       Light Blue    |        Bit 3 = Red          |
    %0100       4 / $04       Dark Green    |                             |
    %0101       5 / $05       Light Green   +-----------------------------+
    %0110       6 / $06       Dark Cyan
    %0111       7 / $07       Light Cyan
    %1000       8 / $08       Dark Red
    %1001       9 / $09       Light Red
    %1010      10 / $0A       Dark Purple
    %1011      11 / $0B       Light Purple
    %1100      12 / $0C       Dark Yellow
    %1101      13 / $0D       Light Yellow
    %1110      14 / $0E       Light Gray (Dark White)
    %1111      15 / $0F       White

^G : Register #27:    Row Address Display Increment
----

  This register specifies the number of bytes to skip, when displaying
characters on the 8563 screen. Normally, this byte holds a value of $00
indicating no bytes to skip; however typically programs that "scroll" the
screen do so by setting this to 80 or 160 allowing the program to then alter
the Screen Start (Registers #12 and #13) and appear to "scroll". Note the
normal C= 128 Kernal Screen Editor does not support this function.

^H : Register #28:7-5 Character Set Address
----

  These bits indicate the address of screen memory * 8k. Thus the values in
these bits may be multiplied by 8192 to obtain the starting character set
position (normall these bits hold a value of $01 indicating the character
set begins at 8192).  Note that the character set is not in ROM, but is usually
copied to 8192 when the computer is first turned on and the 8563 is initialized.
(Examine the INIT80 routine at $CE0C in bank 15).

                           +--------------------------+
                           | 8563 Memory Organization |
                           +--------------------------+

  Normally, the extra memory of the C=128's equipped with 64k goes unused (48k
worth) unless programs like Basic-8 etc, take advantage of it. There are various
mod files describing the upgrade from 16k to 64k and it is _strongly_ advised
(although the author has not yet done so) and be aware that ***OPENING YOUR
COMPUTER JUST TO LOOK, YOU MAY MESS IT UP*** and it is _strongly_ advised that
you contact a person experienced with electronics to perform the upgrade for
you.  Note also that some mail order companies are offering an "up-grade board"
which plugs into the 8563 slot and does not involve desoldering the RAM chips.

  Now, the 8563 uses the 16k of memory (it ignores the extra 48k of memory when
it's got 64k, thus the following applies also to the 8563's equipped with 64k
of memory) and normally, has the following memory map:

  $0000 - $07ff - Screen Memory
  $0800 - $0fff - Attribute Memory
  $1000 - $1fff - Unused
  $2000 - $2fff - UpperCase / Graphic Character Set   (Char Set #1)
  $3000 - $3fff - LowerCase / UpperCase Character Set (Char Set #2)
*/
