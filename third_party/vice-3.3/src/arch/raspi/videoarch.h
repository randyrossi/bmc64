/*
 * videoarch.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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

#ifndef RASPI_VIDEOARCH_H
#define RASPI_VIDEOARCH_H

#include "vice.h"

#include "circle.h"
#include "palette.h"
#include "video.h"

// potx and poty occupy 8 bits in joy int values passed
// to joy update calls
#define POTX_BIT_MASK 0x1fe0
#define POTY_BIT_MASK 0x1fe000

struct video_canvas_s {
  // Core fields required by vice.
  unsigned int initialized;
  unsigned int created;
  struct video_render_config_s *videoconfig;
  struct draw_buffer_s *draw_buffer;
  struct viewport_s *viewport;
  struct geometry_s *geometry;
  struct palette_s *palette;
  unsigned int depth;
  float refreshrate;
  struct video_draw_buffer_callback_s *video_draw_buffer_callback;

  // Set to 2 to double the height of the frame buffer. Allows for
  // 'cheap' raster line simulation that can be turned on/off by
  // setting raster_lines;
  int raster_skip;

  // When non zero, simulates scanlines. Only applicable is raster_lines = 2
  int raster_lines;
};

typedef struct video_canvas_s video_canvas_t;

// Called when video canvas has been created.
struct video_canvas_s *video_canvas_create(struct video_canvas_s *canvas,
                                           unsigned int *width,
                                           unsigned int *height, int mapped);

// Called when a region in the draw buffer has changed.
void video_canvas_refresh(struct video_canvas_s *canvas, unsigned int xs,
                          unsigned int ys, unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h);

int video_canvas_set_palette(struct video_canvas_s *canvas, palette_t *p);

void video_arch_canvas_init(struct video_canvas_s *canvas);

// For timing
unsigned long vsyncarch_frequency(void);
unsigned long vsyncarch_gettime(void);
void vsyncarch_init(void);
void vsyncarch_postsync(void);
void vsyncarch_presync(void);
void vsyncarch_sleep(unsigned long delay);

void videoarch_swap(void);

// Draws a single vice frame into our framebuffer
void draw(uint8_t *src, int srcw, int srch, int src_pitch, uint8_t *dst,
          int dst_pitch, int off_x, int off_y);

palette_t *raspi_video_load_palette(int num_entries, char *name);
void main_exit(void);

// Similar to the public emu_key_interrupt except can be called while
// holding the lock.
void key_interrupt_locked(long key, int pressed);

void set_raster_lines(int v, int v2);
#endif
