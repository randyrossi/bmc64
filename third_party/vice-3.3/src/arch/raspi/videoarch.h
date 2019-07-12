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

#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

#include "vice.h"

#include "circle.h"
#include "palette.h"
#include "video.h"

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
};

typedef struct video_canvas_s video_canvas_t;

// A global struct holding canvas, frame buffer details, state, etc.
struct VideoData {
  struct video_canvas_s *canvas;

  // Information about our dest frame buffer
  uint8_t *dst;
  int dst_pitch;
  // Dest offset when drawing into frame buffer
  // NOTE: x must be multiple of 4
  int dst_off_x;
  int dst_off_y;
  int fb_w;
  int fb_h;

  // Information about our source pixel buffer
  int top_left; // points to top left pixel in emulator canvas
  int vis_w;
  int vis_h;

  // This keeps track of the y offset for the region in our virtual
  // frame buffer that is NOT visible at the moment. It toggles
  // back and forth between 0 and 2x our physical vertical height.
  // The idea is, we wait for vsync before telling the video hardware
  // to show the pixels we just wrote, avoiding horizontal tearing.
  int offscreen_buffer_y;

  // Just the opposite of the above. Will point to on screen buffer.
  int onscreen_buffer_y;

  int palette_index;

  // TODO: Try getting rid of this.
  int first_refresh;

  uint8_t *src;
  int src_pitch;
  int src_off_x;
  int src_off_y;
  int overlay_y;
};

// TODO: Transition from VideoData to CanvasState
struct CanvasState {
  struct video_draw_buffer_callback_s draw_buffer_callback;
};

// Called when video canvas has been created.
struct video_canvas_s *video_canvas_create(struct video_canvas_s *canvas,
                                           unsigned int *width,
                                           unsigned int *height, int mapped);

// Called when a region in the draw buffer has changed.
void video_canvas_refresh(struct video_canvas_s *canvas, unsigned int xs,
                          unsigned int ys, unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h);

int video_canvas_set_palette(struct video_canvas_s *canvas, palette_t *p);

void video_canvas_change_palette(int index);

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

// Make our video state visible
// TODO GET RID OF THIS
extern struct VideoData video_state;

extern uint8_t *video_font;
extern uint16_t video_font_translate[256];

// Make sure ui also sets this when turning on/off warp
extern int raspi_warp;

// May be called from an ISR to change joy value for emulator
void circle_emu_joy_interrupt(int type, int port, int value);

// Called by menu after color setting changed (brightness, contrast...)
void video_color_setting_changed(void);

palette_t *raspi_video_load_palette(int num_entries, char *name);
void main_exit(void);

#endif
