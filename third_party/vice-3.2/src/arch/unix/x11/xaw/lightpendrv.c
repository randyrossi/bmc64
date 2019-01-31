/*
 * lightpendrv.c - Xaw
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
 *  Olaf Seibert <rhialto@falu.nl>
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

#ifdef HAVE_MOUSE
#include <stdio.h>
#include <X11/cursorfont.h>
#include "lightpen.h"
#include "ui.h"
#include "uiarch.h"
#include "videoarch.h"
#include "viewport.h"

#include "lightpendrv.h"


static Cursor cursor;
static Display *display;
static int buttons;
static int saved_x, saved_y;
static video_canvas_t *c;

void xaw_init_lightpen(Display *disp) {
    display = disp;
    cursor = XCreateFontCursor(display, XC_pencil);
}

void xaw_lightpen_setbutton(int b, int set)
{
    if (set) {
        buttons |= (b == 1) ? LP_HOST_BUTTON_1 : 0;
        buttons |= (b == 3) ? LP_HOST_BUTTON_2 : 0;
    } else {
        buttons &= (b == 1) ? ~(buttons & LP_HOST_BUTTON_1) : 0xff;
        buttons &= (b == 3) ? ~(buttons & LP_HOST_BUTTON_2) : 0xff;
    }
}

#define LP_DEBUG 0

/* Current mouse coordinates */
void xaw_lightpen_update_xy(int x, int y)
{
    saved_x = x;
    saved_y = y;
}

void x11_lightpen_update(void)
{

    if (c == NULL) {
#if LP_DEBUG
        fprintf(stderr, "x11_lightpen_update: c == NULL\n");
#endif
        return;
    }

    if (display == NULL) {
#if LP_DEBUG
        fprintf(stderr, "x11_lightpen_update: display == NULL\n");
#endif
        return;
    }

    if (lightpen_enabled)  {
        int x, y;
        float scalex, scaley;
        int offx, offy;
        geometry_t *geom = c->geometry;
        draw_buffer_t *draw_buffer = c->draw_buffer;
        viewport_t *viewport = c->viewport;

        XDefineCursor(display, XtWindow(c->emuwindow), cursor);

        /*
         * This information is gleaned from viewport.h,
         * video/video-viewport.c:video_viewport_resize().
         *
         * geom->gfx_size: size inside borders, in C64 pixels
         * geom->screen_size: size including borders, in C64 pixels
         * draw_buffer->canvas_width, ..height: size + black padding, C64
         * c->width, c->height: same size, in X pixels.
         * saved_w, saved_h: should be the same values.
         *
         * Note that in "full" or "debug" border mode the X coordinates
         * will still be too large.
         *
         * To make it more complicated, 80 column screens (128 VDC and PET)
         * are always stretched horizontally. On a real machine the double
         * number of pixels would fit in the same physical width, being
         * narrower pixels, but the emulation pixels remain square and
         * too wide. The lightpen_update() seems to compensate for that
         * itself, though.
         */
#if LP_DEBUG
        fprintf(stderr, "    gfx_size.width=%3d    gfx_size.height=%3d\n"
                        " screen_size.width=%3d screen_size.height=%3d\n"
                        " d_b->canvas_width=%3d      canvas_height=%3d\n"
                        "             width=%3d             height=%3d\n"
                        "      vp->x_offset=%3d           y_offset=%3d border_left=%d\n"
                        "       vp->first_x=%3d     vp->first_line=%3d first_displayed_line=%3d\n"
                        " sclx=%3d        x=%3d     scly=%3d     y=%3d\n",
                geom->gfx_size.width, geom->gfx_size.height,
                geom->screen_size.width, geom->screen_size.height,
                draw_buffer->canvas_width, draw_buffer->canvas_height,
                c->width, c->height,
                viewport->x_offset, viewport->y_offset, geom->extra_offscreen_border_left,
                viewport->first_x, viewport->first_line, geom->first_displayed_line,
                c->videoconfig->scalex, saved_x, c->videoconfig->scaley, saved_y
                );
#endif

#ifdef HAVE_XVIDEO
        if (c->videoconfig->hwscale && c->xv_image) {
            /* display_yuv_image() kindly stored these coordinates for us */
            offx = c->xv_geometry.x;
            offy = c->xv_geometry.y;

            scalex = (float)c->xv_geometry.w / draw_buffer->canvas_width;
            scaley = (float)c->xv_geometry.h / draw_buffer->canvas_height;

            x = (saved_x - offx) / scalex;
            y = (saved_y - offy) / scaley;
        } else
#endif
        {
            /*
             * Either
             *    x_offset is > 0 (if the window is too large: black padding),
             * or first_x is > 0 (if the window is too small: clips).
             */
            offx = viewport->x_offset - viewport->first_x;
            offy = viewport->y_offset - viewport->first_line
                                      + geom->first_displayed_line;

            scalex = (float)draw_buffer->canvas_physical_width  / draw_buffer->canvas_width;
            scaley = (float)draw_buffer->canvas_physical_height / draw_buffer->canvas_height;

            x = (saved_x / scalex) - offx;
            y = (saved_y / scaley) - offy;
        }

#if LP_DEBUG
        fprintf(stderr,"x=%d y=%d b=%02x scalex=%.3f scaley=%.3f offx=%d offy=%d appshell=%d\n",
                x, y, buttons, scalex, scaley, offx, offy, c->app_shell);
#endif

        if (x >= geom->screen_size.width) {
            x = -1;
        }
        if (y >= geom->screen_size.height) {
            y = -1;
        }

        lightpen_update(c->app_shell, x, y, buttons);
    } else {
        if (c->emuwindow) {
            Window w = XtWindow(c->emuwindow);
            if (w) {
                XUndefineCursor(display, w);
            }
        }
        buttons = 0;
    }
}

void xaw_lightpen_update_canvas(struct video_canvas_s *p, int enter) 
{
#if LP_DEBUG
    fprintf(stderr,"xaw_lightpen_update_canvas: video_canvas_t %p %d shell=%d emuwindow=%p\n",
                p, enter, p ? p->app_shell : -1, p ? p->emuwindow : NULL);
#endif
    c = enter ? p : NULL;
}

#endif /* HAVE_MOUSE */
