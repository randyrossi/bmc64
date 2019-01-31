/*
 * videoarch.h - X11 graphics routines.
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

#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

#include "vice.h"

#include "ui.h"
#include "uiarch.h"

#include "vice.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

#ifdef USE_MITSHM
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include "types.h"
#include "video.h"

#ifdef HAVE_XVIDEO
#include <X11/extensions/Xvlib.h>
#include "../renderxv.h"
#endif

#include "uiarch.h"

typedef void (*video_refresh_func_t)(struct video_canvas_s *, int, int, int, int, unsigned int, unsigned int);

struct fullscreenconfig_s;
struct palette_s;
struct video_draw_buffer_callback_s;

struct video_canvas_s {
    unsigned int initialized;
    unsigned int created;
    ui_window_t emuwindow;
    struct video_render_config_s *videoconfig;
    struct draw_buffer_s *draw_buffer;
    struct viewport_s *viewport;
    struct geometry_s *geometry;
    struct palette_s *palette;
    Window drawable;
    Colormap colormap;
    XImage *x_image;
    unsigned int depth;
    float refreshrate;
#ifdef HAVE_XVIDEO
    XvImage *xv_image;
    XvPortID xv_port;
    fourcc_t xv_format;
    int is_overlay;
    image_t yuv_image;  /* For architecture independent rendering. */
    struct xywh_s xv_geometry;
#endif

#ifdef USE_MITSHM
    XShmSegmentInfo xshm_info;
    int using_mitshm;     /* True if MITSHM is used for this framebuffer. */
#endif
    GC gc;
    struct video_draw_buffer_callback_s *video_draw_buffer_callback;
#ifdef HAVE_FULLSCREEN
    struct fullscreenconfig_s *fullscreenconfig;
    video_refresh_func_t video_fullscreen_refresh_func;
#endif
    int app_shell;
};
typedef struct video_canvas_s video_canvas_t;

extern int _video_use_xsync;

/* ------------------------------------------------------------------------- */

extern void video_convert_color_table(unsigned int i, uint8_t *data, long col, video_canvas_t *c);

extern void uicolor_init_video_colors(struct video_canvas_s *);

void kbd_event_handler(Widget w, XtPointer client_data, XEvent *report, Boolean *ctd);

#endif
