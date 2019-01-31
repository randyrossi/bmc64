/*
 * video.h - BEOS graphics handling
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

#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

#ifdef EXACT_TYPE_NEEDED
#include "vicewindow.h"
#endif

#include "types.h"
#include "video.h"

struct video_draw_buffer_callback_s;

struct video_canvas_s {
    unsigned int initialized;
    unsigned int created;
    char *title;
    unsigned int depth;
    struct video_render_config_s *videoconfig;
    struct draw_buffer_s *draw_buffer;
    struct viewport_s *viewport;
    struct geometry_s *geometry;
    struct palette_s *palette;

    /* Pointer to the canvas' ViceWindow */
#ifdef EXACT_TYPE_NEEDED
    ViceWindow *vicewindow;
#else
    void *vicewindow;
#endif
    struct video_draw_buffer_callback_s *video_draw_buffer_callback;
};
typedef struct video_canvas_s video_canvas_t;

#define CANVAS_USES_TRIPLE_BUFFERING(c) 0

/* ------------------------------------------------------------------------- */

struct _color_def {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
    unsigned char dither;
};
typedef struct _color_def color_def_t;

/* ------------------------------------------------------------------------- */

#endif
