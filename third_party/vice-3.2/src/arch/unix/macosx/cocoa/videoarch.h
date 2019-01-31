/*
 * videoarch.h - MacVICE specific canvas structure
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *  Michael Klein <michael.klein@puffin.lb.shuttle.de>
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

#include "video.h"

#ifdef __OBJC__
#import "vicewindow.h"
#else
typedef void VICEWindow;
typedef void VICEGLView;
#endif

struct video_draw_buffer_callback_s;
struct video_resource_chip_s;
struct video_param_s;

/* custom canvas structure for MacVICE */
struct video_canvas_s
{
    unsigned int initialized;
    unsigned int created;
    unsigned int width, height, depth;
    struct video_render_config_s *videoconfig;
    struct draw_buffer_s *draw_buffer;
    struct viewport_s *viewport;
    struct geometry_s *geometry;
    struct palette_s *palette;
    struct video_resource_chip_s *video_resource_chip;
    struct video_draw_buffer_callback_s *video_draw_buffer_callback;

    /* MacVICE */
    int pitch;                  /* pitch width of scanline in byte */
    VICEWindow *window;         /* the associated window */
    VICEGLView *view;           /* the associated view */
    int canvasId;               /* canvas id as registered at the machine */

    struct video_param_s *video_param; /* video parameters */
};
typedef struct video_canvas_s video_canvas_t;

void video_canvas_redraw(video_canvas_t *canvas);

#endif  /* _VIDEOARCH_H */
