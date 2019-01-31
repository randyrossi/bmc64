/*
 * uicolor.c - X11 color routines.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Teemu Rantanen <tvr@cs.hut.fi>
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
#include <stdlib.h>
#include <X11/Xlib.h>

#include "color.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "types.h"
#include "uicolor.h"
#include "video.h"
#include "videoarch.h"
#include "x11ui.h"

extern int screen;
extern Colormap colormap;
extern Pixel drive_led_on_red_pixel, drive_led_on_green_pixel;
extern Pixel drive_led_off_pixel;

#define NUM_ENTRIES 3

static int uicolor_alloc_system_colors(void)
{
    palette_t *p = lib_malloc(sizeof(palette_t));
    unsigned long color_return[NUM_ENTRIES];

    p->num_entries = NUM_ENTRIES;
    p->entries = lib_malloc(sizeof(palette_entry_t) * NUM_ENTRIES);
    memset(p->entries, 0, sizeof(palette_entry_t) * NUM_ENTRIES);

    p->entries[0].red = 0;
    p->entries[0].green = 0;
    p->entries[0].blue = 0;

    p->entries[1].red = 0xff;
    p->entries[1].green = 0;
    p->entries[1].blue = 0;

    p->entries[2].red = 0;
    p->entries[2].green = 0xff;
    p->entries[2].blue = 0;

    color_alloc_colors(NULL, p, color_return);

    drive_led_off_pixel = (Pixel)color_return[0];
    drive_led_on_red_pixel = (Pixel)color_return[1];
    drive_led_on_green_pixel = (Pixel)color_return[2];

    lib_free(p->entries);
    lib_free(p);

    return 0;
}

/*-----------------------------------------------------------------------*/

int uicolor_alloc_colors(video_canvas_t *c)
{
    if (uicolor_alloc_system_colors() < 0 || color_alloc_colors(c, c->palette, NULL) < 0) {
        Display *display = x11ui_get_display_ptr();
        if (colormap == DefaultColormap(display, screen)) {
            log_warning(LOG_DEFAULT, "Automatically using a private colormap.");
            colormap = XCreateColormap(display, RootWindow(display, screen), visual, AllocNone);
            XtVaSetValues(_ui_top_level, XtNcolormap, colormap, NULL);
            return color_alloc_colors(c, c->palette, NULL);
        }
    }
    return 0;
}

int uicolor_set_palette(struct video_canvas_s *c, const palette_t *palette)
{
    return color_alloc_colors(c, palette, NULL);
}

/*-----------------------------------------------------------------------*/

int uicolor_alloc_color(unsigned int red, unsigned int green, unsigned int blue, unsigned long *color_pixel, uint8_t *pixel_return)
{
    XColor color;
    XImage *im;
    uint8_t *data = (uint8_t *)malloc(4); /* XDestroyImage will free data.  */
    Display *display = x11ui_get_display_ptr();

    /* This is a kludge to map pixels to zimage values. Is there a better
       way to do this? //tvr */
    im = XCreateImage(display, visual, x11ui_get_display_depth(), ZPixmap, 0, (char *)data, 1, 1, 8, 0);
    if (!im) {
        log_error(LOG_DEFAULT, "XCreateImage failed.");
        free(data);
        return -1;
    }

    color.flags = DoRed | DoGreen | DoBlue;
    color.red =  red << 8;
    color.green =  green << 8;
    color.blue = blue << 8;

    if (!XAllocColor(display, colormap, &color)) {
        log_error(LOG_DEFAULT, "Cannot allocate color \"#%04X%04X%04X\".", color.red, color.green, color.blue);
        XDestroyImage(im);
        return -1;
    }
    XPutPixel(im, 0, 0, color.pixel);

    *pixel_return = *data;
    *color_pixel = color.pixel;

    XDestroyImage(im);

    return 0;
}

void uicolor_free_color(unsigned int red, unsigned int green, unsigned int blue, unsigned long color_pixel)
{
    if (!XFreeColors(x11ui_get_display_ptr(), colormap, &color_pixel, 1, 0)) {
        log_error(LOG_DEFAULT, "XFreeColors failed.");
    }
}

void uicolor_convert_color_table(unsigned int colnr, uint8_t *data, long color_pixel, void *c)
{
    if (c == NULL) {
        return;
    }

    video_convert_color_table(colnr, data, color_pixel, (video_canvas_t *)c);
}

void uicolor_init_video_colors(struct video_canvas_s *c)
{
    short i;
    XColor colorr, colorg, colorb;
    Display *display;

    display = x11ui_get_display_ptr();
    for (i = 0; i < 256; i++) {
        colorr.flags = DoRed | DoGreen | DoBlue;
        colorr.red =  i << 8;
        colorr.green =  0;
        colorr.blue = 0;

        colorg.flags = DoRed | DoGreen | DoBlue;
        colorg.red =  0;
        colorg.green =  i << 8;
        colorg.blue = 0;

        colorb.flags = DoRed | DoGreen | DoBlue;
        colorb.red =  0;
        colorb.green =  0;
        colorb.blue = i << 8;

        if (!XAllocColor(display, colormap, &colorr)) {
            log_error(LOG_DEFAULT, "Cannot allocate color \"#%04X%04X%04X\".", colorr.red, colorr.green, colorr.blue);
        }
        if (!XAllocColor(display, colormap, &colorg)) {
            log_error(LOG_DEFAULT, "Cannot allocate color \"#%04X%04X%04X\".", colorg.red, colorg.green, colorg.blue);
        }
        if (!XAllocColor(display, colormap, &colorb)) {
            log_error(LOG_DEFAULT, "Cannot allocate color \"#%04X%04X%04X\".", colorb.red, colorb.green, colorb.blue);
        }
        video_render_setrawrgb(i, (uint32_t)colorr.pixel, (uint32_t)colorg.pixel, (uint32_t)colorb.pixel);
    }

    video_render_initraw(c->videoconfig);
}
