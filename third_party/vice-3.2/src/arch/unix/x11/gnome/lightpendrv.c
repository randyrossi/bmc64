/*
 * lightpendrv.c - Gtk+ 
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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

/* #define DEBUG_LIGHTPEN */

#include "vice.h"

#ifdef HAVE_MOUSE
#include <stdio.h>
#include "lightpen.h"
#include "ui.h"
#include "uiarch.h"
#include "videoarch.h"

#include "lightpendrv.h"


/******************************************************************************/

static GdkCursor *cursor;
static int buttons;
static struct video_canvas_s *c;

void gtk_init_lightpen(void) {
    cursor = gdk_cursor_new(GDK_PENCIL);
}

void gtk_lightpen_setbutton(int b, int set)
{
    if (set) {
        buttons |= (b == 1) ? LP_HOST_BUTTON_1 : 0;
        buttons |= (b == 3) ? LP_HOST_BUTTON_2 : 0;
    } else {
        buttons &= (b == 1) ? ~(buttons & LP_HOST_BUTTON_1) : 0xff;
        buttons &= (b == 3) ? ~(buttons & LP_HOST_BUTTON_2) : 0xff;
    }
}

void x11_lightpen_update(void)
{
    int x, y;
    int h, w;
    float fx, fy;
    GtkAllocation allocation;

    if (c && lightpen_enabled)  {
        gdk_window_set_cursor(gtk_widget_get_window(c->emuwindow), cursor);
        gdk_display_get_window_at_pointer(
                gtk_widget_get_display(c->emuwindow), &x, &y);
        /* gdk_drawable_get_size(gtk_widget_get_window(c->emuwindow), &w, &h); */
        gtk_widget_get_allocation(c->emuwindow, &allocation);
        w = allocation.width;
        h = allocation.height;

        fx = w / (float) (c->geometry->screen_size.width);
        fy = h / (float) (c->geometry->last_displayed_line -
                c->geometry->first_displayed_line + 1);

#ifdef DEBUG_LIGHTPEN
        fprintf(stderr,"pre : x = %i, y = %i, b = %02x, w: %d, h:%d, fx = %f, fy = %f divx:%d divy:%d   out: x=%i y=%i (%i-%i)\n",
                x, y, buttons, w, h, fx, fy,
                c->geometry->screen_size.width , c->geometry->last_displayed_line - c->geometry->first_displayed_line + 1,
                (int)(x / fx), (int)(y / fy), c->geometry->first_displayed_line, c->geometry->last_displayed_line
               );
#endif
        x /= fx;
        y /= fy;

        lightpen_update(c->app_shell, x, y, buttons);
    } else {
        if (c) {
            gdk_window_set_cursor(gtk_widget_get_window(c->emuwindow), NULL);
        }
        buttons = 0;
    }
}

void gtk_lightpen_update_canvas(struct video_canvas_s *p, int enter) 
{
    c = enter ? p : NULL;
}

#endif /* HAVE_MOUSE */
