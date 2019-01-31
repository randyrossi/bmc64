/*
 * video.h - MS-DOS graphics handling, based on the Allegro library by Shawn
 * Hargreaves (http://www.talula.demon.co.uk/allegro).
 *
 * Written by
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

#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

#include <pc.h>			/* inportb(), outportb() */

#include "types.h"

#include <allegro.h>

#include "palette.h"
#include "types.h"
#include "statusbar.h"
#include "video.h"

#define NUM_AVAILABLE_COLORS 0x100

struct palette_s;
struct video_draw_buffer_callback_s;

struct video_canvas_s {
    unsigned int initialized;
    unsigned int created;
    unsigned int depth, bytes_per_line;
    struct video_render_config_s *videoconfig;
    struct draw_buffer_s *draw_buffer;
    struct viewport_s *viewport;
    struct geometry_s *geometry;
    struct palette_s *palette;
    RGB colors[NUM_AVAILABLE_COLORS];

    /* If set to nonzero, it means we are doing triple buffering on this
       canvas.  */
    int use_triple_buffering;

    /* Pages for triple buffering (the third page is the off-screen frame
       buffer).  */
    BITMAP *pages[2];

    /* Bitmap for the video renderer */
    BITMAP *render_bitmap;

    /* Currently invisible page.  */
    int back_page;

    struct video_draw_buffer_callback_s *video_draw_buffer_callback;
};
typedef struct video_canvas_s video_canvas_t;

#define CANVAS_USES_TRIPLE_BUFFERING(c) (c->use_triple_buffering)

/* ------------------------------------------------------------------------- */

struct _color_def {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
    unsigned char dither;
};
typedef struct _color_def color_def_t;

/* ------------------------------------------------------------------------- */

struct _vga_mode {
    int width, height, depth;
    char *description;
};
typedef struct _vga_mode vga_mode_t;

extern vga_mode_t vga_modes[];

enum vga_mode_id {
    VGA_320x200x8,
    VGA_360x240x8,
    VGA_360x270x8,
    VGA_376x282x8,
    VGA_400x300x8,
    VGA_640x480x8,
    VGA_800x600x8,
    VGA_1024x768x8,
    VGA_320x200x16,
    VGA_400x300x16,
    VGA_640x480x16,
    VGA_800x600x16,
    VGA_1024x768x16,
    VGA_640x480x32,
    VGA_800x600x32,
    VGA_1024x768x32
};

#define NUM_VGA_MODES ((int)VGA_1024x768x32 + 1)

/* ------------------------------------------------------------------------- */

extern void enable_text(void);
extern void disable_text(void);

extern void video_ack_vga_mode(void);
extern int video_in_gfx_mode(void);

extern void canvas_set_border_color(struct video_canvas_s *canvas, uint8_t color);

extern double vsync_get_avg_frame_rate(void);
extern double vsync_get_avg_speed_index(void);

extern video_canvas_t *last_canvas;

#endif
