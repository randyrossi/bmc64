/*
 * video.c - Video implementation for Win32, using DirectDraw.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@matavnet.hu>
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
#include <string.h>

#include <windows.h>
#include <mmsystem.h>

#include "cmdline.h"
#include "intl.h"
#include "lib.h"
#include "log.h"
#include "fullscrn.h"
#include "fullscreen.h"
#include "palette.h"
#include "res.h"
#include "resources.h"
#include "statusbar.h"
#include "translate.h"
#include "types.h"
#include "ui.h"
#include "video.h"
#include "videoarch.h"
#include "viewport.h"
#include "vsyncapi.h"

void video_resize(void);

#define EXIT_REASON(reason)             \
    {                                   \
        log_debug("Error %08x",reason); \
        return -1;                      \
    }

/* ------------------------------------------------------------------------ */

/* #define DEBUG_VIDEO */

/* Debugging stuff.  */
#ifdef DEBUG_VIDEO
static void video_debug(const char *format, ...)
{
    char tmp[1024];
    va_list args;

    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);
    log_debug(tmp);
}
#define VDEBUG(x) video_debug x
#else
#define VDEBUG(x)
#endif

/* ------------------------------------------------------------------------ */

/* Create a video canvas.  If specified width/height is not possible,
   return an alternative in `*width' and `*height'.  */
video_canvas_t *video_canvas_create_ddraw(video_canvas_t *canvas)
{
    ui_make_resizable(canvas, 0);

    canvas->depth = 24;

    if (video_set_physical_colors(canvas) < 0) {
        goto error;
    }

    video_canvas_add(canvas);

    if (IsFullscreenEnabled()) {
        SwitchToFullscreenMode(canvas->hwnd);
    }

    video_canvas_reset_ddraw(canvas);
    return canvas;

error:
    video_canvas_destroy(canvas);
    return NULL;
}

void video_canvas_reset_ddraw(video_canvas_t *canvas)
{
    canvas->bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    canvas->bmp_info.bmiHeader.biWidth = canvas->draw_buffer->canvas_physical_width;
    canvas->bmp_info.bmiHeader.biHeight = -(LONG)canvas->draw_buffer->canvas_physical_height;
    canvas->bmp_info.bmiHeader.biPlanes = 1;
    canvas->bmp_info.bmiHeader.biBitCount = canvas->depth;
    canvas->bmp_info.bmiHeader.biCompression = BI_RGB;
    canvas->bmp_info.bmiHeader.biSizeImage = canvas->depth / 8 * canvas->draw_buffer->canvas_physical_width * canvas->draw_buffer->canvas_physical_height;
    lib_free(canvas->pixels);
    canvas->pixels = lib_malloc(canvas->bmp_info.bmiHeader.biSizeImage);
}

/* ------------------------------------------------------------------------ */
/*
   Here is where the whole thing gets complicate.  We basically have three
   ways to update the window:

   a) manual/direct framebuffer -> primary_surface copy;

   b) framebuffer -> tmp_surface conversion, and consequent tmp_surface ->
      primary_surface blit;

   c) manual/direct framebuffer -> back_surface copy, and consequent
      back_surface <-> primary_surface flip.

   As the frame buffer is 8bpp, method (a) is only possible when the screen
   is in 8bpp mode.  Unluckily, this cannot be done with some Windows NT
   DirectDraw drivers either, because some of them (e.g. the ET4000 one I am
   using now) do not provide direct access to windows in non-exclusive mode:
   in this case, the only way is to use method (b) (see below).  It would be
   great to just use a DirectDraw surface as the framebuffer, but unluckily
   this is not possible because we might need a framebuffer that is larger
   than the physical screen (to clip sprites), and this is not allowed by
   DirectX 3.  (Maybe future versions?  Does anybody have any info on this?)

   Method (b) is necessary when we work in non-exclusive mode and the depth
   is not 8 bpp: in this case, we convert the framebuffer to 16, 24 or 32 bpp
   into a temporary surface and then Blit the temporary surface into the
   window.

   Method (c) is only possible in full-screen mode: we copy the bitmap into a
   backing surface (without any conversion, because we assume full-screen
   mode is always 8bpp) and then order a flip which will happen at vsync.
   This is the best method because it's faster than the other two and makes
   animations as smooth as possible.

*/

static void clear(HDC hdc, int x1, int y1, int x2, int y2)
{
    static HBRUSH back_color;
    RECT clear_rect;

    if (back_color == NULL) {
        back_color = CreateSolidBrush(0);
    }
    clear_rect.left = x1;
    clear_rect.top = y1;
    clear_rect.right = x2;
    clear_rect.bottom = y2;
    FillRect(hdc, &clear_rect, back_color);
}

void video_canvas_update_ddraw(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h)
{
    video_canvas_t *c;
    int xs;   //  upperleft x in framebuffer
    int ys;   //  upperleft y in framebuffer
    int xi;   //  upperleft x in client space
    int yi;   //  upperleft y in client space
    int window_index;
    RECT rect;
    int safex, safex2, safey, safey2;
    unsigned int pixel_width, pixel_height;

    c = video_canvas_for_hwnd(hwnd);

    if (c == NULL) {
        return;
    }

    pixel_width = c->videoconfig->scalex;
    pixel_height = c->videoconfig->scaley;

    for (window_index = 0; window_index < number_of_windows; window_index++) {
        if (window_handles[window_index] == hwnd) {
            break;
        }
    }

    GetClientRect(hwnd, &rect);

    /*  Calculate upperleft point's framebuffer coords */
    xs = xclient - ((rect.right - c->draw_buffer->canvas_physical_width) / 2) + (c->viewport->first_x - c->viewport->x_offset + c->geometry->extra_offscreen_border_left) * pixel_width;
    ys = yclient - ((rect.bottom - statusbar_get_status_height() - c->draw_buffer->canvas_physical_height) / 2) + (c->viewport->first_line - c->viewport->y_offset) * pixel_height;

    /*  Cut off areas outside of framebuffer and clear them */
    xi = xclient;
    yi = yclient;

    safex = (c->viewport->first_x + c->geometry->extra_offscreen_border_left) * pixel_width;
    safex2 = c->draw_buffer->draw_buffer_width * pixel_width;
    safey = c->viewport->first_line * pixel_height;
    safey2 = (c->viewport->last_line + 1) * pixel_height;

    if (c->draw_buffer->draw_buffer) {
        /*  Check if it's out */
        if ((xs + w <= safex) || (xs >= safex2) || (ys + h <= safey) ||
(ys >= safey2)) {
            clear(hdc, xi, yi, xi + w, yi + h);
            return;
        }

        /*  Cut top */
        if (ys < safey) {
            clear(hdc, xi, yi, xi + w, yi - ys + safey);
            yi -= ys - safey;
            h += ys - safey;
            ys = safey;
        }

        /*  Cut left */
        if (xs < safex) {
            clear(hdc, xi, yi, xi - xs + safex, yi + h);
            xi -= xs - safex;
            w += xs - safex;
            xs = safex;
        }

        /*  Cut bottom */
        if (ys + h > safey2) {
            clear(hdc, xi, yi + safey2 - ys, xi + w, yi + h);
            h = safey2 - ys;
        }

        /*  Cut right */
        if (xs + w > safex2) {
            clear(hdc, xi + safex2 - xs, yi, xi + w, yi + h);
            w = safex2 - xs;
        }

        /*  Update remaining area from framebuffer.... */

        if ((w > 0) && (h > 0) && !IsIconic(c->hwnd)) {
            HDC hdc = GetDC(c->hwnd);

            xs /= pixel_width;
            ys /= pixel_height;
            if (syscolorchanged) {
                ui_error("System colors changed!\n(not implemented yet)");
                syscolorchanged = 0;
            }
            if (displaychanged) {
                displaychanged = 0;
            }
            if (querynewpalette) {
                querynewpalette = 0;
            }
            if (palettechanged) {
                palettechanged = 0;
            }

            video_canvas_render(c, c->pixels, w, h, xs, ys, xi, yi, c->depth / 8 * c->draw_buffer->canvas_physical_width, c->depth);
            SetDIBitsToDevice(hdc,
                xi, yi, w, h, xi, yi, yi, yi+h,
                c->pixels, &c->bmp_info, DIB_RGB_COLORS);
            ReleaseDC(c->hwnd, hdc);
        }
    }
}

void video_canvas_refresh_ddraw(video_canvas_t *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    int window_index;
    RECT rect;

    xi *= canvas->videoconfig->scalex;
    w *= canvas->videoconfig->scalex;

    yi *= canvas->videoconfig->scaley;
    h *= canvas->videoconfig->scaley;

    for (window_index = 0; window_index < number_of_windows; window_index++) {
        if (window_handles[window_index] == canvas->hwnd) {
            break;
        }
    }
    if (window_index == number_of_windows) {
        VDEBUG(("PANIC: can't find window"));
        return;
    }

    GetClientRect(canvas->hwnd, &rect);
    rect.left = xi + (rect.right - canvas->draw_buffer->canvas_physical_width) / 2;
    rect.top = yi + (rect.bottom - statusbar_get_status_height() - canvas->draw_buffer->canvas_physical_height) / 2;
    rect.right = w + rect.left;
    rect.bottom = h + rect.top;

    InvalidateRect(canvas->hwnd, &rect, FALSE);
}

void fullscreen_capability(cap_fullscreen_t *cap_fullscreen)
{
    cap_fullscreen->device_num = 0;
}
