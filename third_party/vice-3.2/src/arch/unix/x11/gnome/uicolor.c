/*
 * uicolor.c - X11 color routines.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *  Andreas Boose <viceteam@t-online.de>
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

/* #define DEBUG_GNOMEUI */

#include "vice.h"

#include <stdlib.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "color.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "types.h"
#include "uicolor.h"
#include "video.h"
#include "videoarch.h"

#ifdef DEBUG_GNOMEUI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/* UI color constants shared by GUI elements */
GdkColor drive_led_on_red_pixel, drive_led_on_green_pixel;
GdkColor drive_led_off_pixel, motor_running_pixel, tape_control_pixel;
GdkColor drive_led_on_red_pixels[16];
GdkColor drive_led_on_green_pixels[16];

void uicolor_init_video_colors(void);

/*-----------------------------------------------------------------------*/

int uicolor_alloc_colors(video_canvas_t *c)
{
    int i;

    drive_led_off_pixel.red = 0;
    drive_led_off_pixel.green = 0;
    drive_led_off_pixel.blue = 0;

    drive_led_on_red_pixel.red = 0xff00;
    drive_led_on_red_pixel.green = 0;
    drive_led_on_red_pixel.blue = 0;

    drive_led_on_green_pixel.red = 0;
    drive_led_on_green_pixel.green = 0xff00;
    drive_led_on_green_pixel.blue = 0;

    motor_running_pixel.red = 0xff00;
    motor_running_pixel.green = 0xff00;
    motor_running_pixel.blue = 0x7f00;

    tape_control_pixel.red = 0xaf00;
    tape_control_pixel.green = 0xaf00;
    tape_control_pixel.blue = 0xaf00;

    /* different colors intensities for drive leds */
    for (i = 0; i < 16; i++) {
        drive_led_on_red_pixels[i].red = 0x1000 * i + 0xf00;
        drive_led_on_red_pixels[i].green = 0;
        drive_led_on_red_pixels[i].blue = 0;

        drive_led_on_green_pixels[i].red = 0;
        drive_led_on_green_pixels[i].green =  0x1000 * i + 0xf00;
        drive_led_on_green_pixels[i].blue = 0;
    }
    return 0;
}


static unsigned int endian_swap(unsigned int color, unsigned int bpp, unsigned int swap) {
    if (!swap) {
        return color;
    }

    if (bpp == 8) {
        return color;
    }

    if (bpp == 16) {
        return ((color >> 8) & 0x00ff) | ((color << 8) & 0xff00);
    }

    if (bpp == 24) {
        return color; /* 24 bpp output special case at renderer level, always writes out with LSB order regardless of host CPU, handled elsewhere */
    }

    if (bpp == 32) {
        return ((color >> 24) & 0x000000ff) | ((color >>  8) & 0x0000ff00) | ((color <<  8) & 0x00ff0000) | ((color << 24) & 0xff000000);
    }

    /* err? */
    return color;
}

int uicolor_set_palette(struct video_canvas_s *c, const palette_t *palette)
{
    unsigned int i, bpp, swap;
    gint rs, gs, bs, rb, gb, bb;

    /* Hwscaled colours are expected in GL_RGB order. 24 bpp renderers are
     * special, they always seem to expect color order to be logically ABGR,
     * which they write out in RGB memory order. (Glorious, eh?) */
    if (c->videoconfig->hwscale) {
        bpp = 24;
        rb = 8;
        gb = 8;
        bb = 8;
        rs = 0;
        gs = 8;
        bs = 16;
        swap = 0;
    } else {
#if !defined(HAVE_CAIRO)
        /* GdkImage is deprecated since 2.22 and removed in 3.0 */
        GdkVisual *vis = gdk_image_get_visual(c->gdk_image);
        bpp = gdk_visual_get_depth(vis);
        gdk_visual_get_red_pixel_details(vis, NULL, &rs, &rb);
        gdk_visual_get_green_pixel_details(vis, NULL, &gs, &gb);
        gdk_visual_get_blue_pixel_details(vis, NULL, &bs, &bb);

#ifdef WORDS_BIGENDIAN
        swap = (gdk_visual_get_byte_order(vis) == GDK_LSB_FIRST);
#else
        swap = (gdk_visual_get_byte_order(vis) == GDK_MSB_FIRST);
#endif

#else
        if (gdk_pixbuf_get_colorspace(c->gdk_pixbuf) == GDK_COLORSPACE_RGB) {
            bpp = gdk_pixbuf_get_n_channels(c->gdk_pixbuf) * gdk_pixbuf_get_bits_per_sample(c->gdk_pixbuf);
            rb = gb = bb = gdk_pixbuf_get_bits_per_sample(c->gdk_pixbuf);
            rs = 0;
            gs = 8;
            bs = 16;
            swap = 0;
        } else {
            log_error(ui_log, "uicolor_set_palette: unsupported color space");
            bpp = 24;
            rb = 8;
            gb = 8;
            bb = 8;
            rs = 0;
            gs = 8;
            bs = 16;
            swap = 0;
        }
#endif
        /* 24 bpp modes do not really work with the existing
         * arrangement as they have been written to assume the A component is
         * in the 32-bit longword bits 24-31. If any arch needs 24 bpp, that
         * code must be specially written for it. */
    }

    DBG(("bpp:%d rb:%d gb:%d bb:%d rs:%d gs:%d bs:%d swap:%d", bpp, rb, gb, bb, rs, gs, bs, swap));

    for (i = 0; i < palette->num_entries; i++) {
        palette_entry_t color = palette->entries[i];
        /* scale 256 color palette for Gdk terms, then shift to precision,
         * then move component where it needs to be. */
        uint32_t color_pixel = endian_swap(color.red << 8 >> (16 - rb) << rs | color.green << 8 >> (16 - gb) << gs | color.blue  << 8 >> (16 - bb) << bs, bpp, swap);

        video_render_setphysicalcolor(c->videoconfig, i, color_pixel, bpp);
    }

    for (i = 0; i < 256; i ++) {
        video_render_setrawrgb(i, endian_swap(i << 8 >> (16 - rb) << rs, bpp, swap), endian_swap(i << 8 >> (16 - gb) << gs, bpp, swap), endian_swap(i << 8 >> (16 - bb) << bs, bpp, swap));
    }

    video_render_initraw(c->videoconfig);

    return 0;
}
