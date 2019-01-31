/*
 * x11ui.h
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_X11UI_H
#define VICE_X11UI_H

#include "vice.h"

#ifdef USE_GNOMEUI
#include "x11/gnome/uiarch.h"
#endif

#ifdef USE_XAWUI
#ifdef VMS
#include "xawuiarch.h"
#else
#include "x11/xaw/uiarch.h"
#endif
#endif

extern void x11ui_resize_canvas_window(ui_window_t w, int height, int width);
extern void x11ui_move_canvas_window(ui_window_t w, int x, int y);
extern void x11ui_canvas_position(ui_window_t w, int *x, int *y);
extern void x11ui_get_widget_size(ui_window_t win, int *w, int *h);
extern void x11ui_map_canvas_window(ui_window_t w);
extern void x11ui_unmap_canvas_window(ui_window_t w);
extern void x11ui_destroy_widget(ui_window_t w);

#if (defined USE_XF86_EXTENSIONS || defined USE_XAWUI)
extern int x11ui_get_display_depth(void);
extern int x11ui_get_screen(void);
extern int x11ui_fullscreen(int enable);

extern Display *x11ui_get_display_ptr(void);
extern Window x11ui_get_X11_window(void);
#endif

/* required for ui-threads */
#ifdef USE_GNOMEUI
struct video_canvas_s;
struct s_mbufs;
extern void gl_render_canvas(GtkWidget *w, struct video_canvas_s *canvas,
        struct s_mbufs *buffers, int from, int to, int do_swap);
#endif


#endif
