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

struct CanvasState {
  struct video_canvas_s *canvas;
  struct video_draw_buffer_callback_s draw_buffer_callback;
  int palette_index;
  // Just the gfx area (no border)
  int gfx_w;
  int gfx_h;
  // How much border is available
  int max_border_w;
  int max_border_h;
  // How much of the border we want to see
  int border_w;
  int border_h;

  int src_off_x;
  int src_off_y;
  // The total visiible pixels in each dimension
  int vis_w;
  int vis_h;
  // For our src region
  int top;
  int left;

  // Where does the status overlay show up?
  int overlay_x;
  int overlay_y;
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

void video_canvas_change_palette(int display_num, int palette_index);

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

// Make sure ui also sets this when turning on/off warp
extern int raspi_warp;

// Called by menu after color setting changed (brightness, contrast...)
void video_color_setting_changed(int display_num);

void video_init_overlay(int padding, int c40_80_state, int vkbd_transparency);

void apply_video_adjustments(int layer, int hcenter, int vcenter,
                             double hborder, double vborder,
                             double aspect, double lpad, double rpad,
                             double tpad, double bpad, int zlayer);

void enable_vic(int enabled);
void enable_vdc(int enabled);

// Makes sure whatever canvas should be visible actually is.
void ensure_video(void);

palette_t *raspi_video_load_palette(int num_entries, char *name);
void main_exit(void);

// If layer is visible right now, make the ui transparent and tell the
// ui only to render the current item. This is used to assist the user
// in making video adjustments in real time (color, aspect ratio,
// etc). Only takes effect while the user remains on the current menu
// item.
void video_canvas_reveal_temp(int layer);

#endif
