/*
 * videoarch.c
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

#include "videoarch.h"

#include "demo.h"
#include "font.h"
#include "joy.h"
#include "joyport/joystick.h"
#include "kbd.h"
#include "kbdbuf.h"
#include "keyboard.h"
#include "machine.h"
#include "mem.h"
#include "menu.h"
#include "menu_tape_osd.h"
#include "monitor.h"
#include "overlay.h"
#include "raspi_machine.h"
#include "resources.h"
#include "sid.h"
#include "ui.h"
#include "video.h"
#include "viewport.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>

#define OVERLAY_H 10

// Increments with each canvas being inited by vice
int canvas_num;

// One struct for each display (can be 2 for C128)
struct CanvasState canvas_state[2];

struct video_canvas_s *vdc_canvas;
struct video_canvas_s *vic_canvas;

static int vdc_canvas_index;
static int vic_canvas_index;

static int vic_enabled;
static int vdc_enabled;

static int vic_showing;
static int vdc_showing;

uint8_t *video_font;
uint16_t video_font_translate[256];

static int vic_first_refresh;
static int vdc_first_refresh;

// We tell vice our clock resolution is the actual vertical
// refresh rate of the machine * some factor. We report our
// tick count when asked for the current time which is incremented
// by that factor after each vertical blank.  Not sure if this
// is really needed anymore since we turned off auto refresh.
// Seems to work fine though.
unsigned long video_ticks = 0;

// So...due to math stuff, whatever value we put for our video tick
// increment here will be how many frames we show before vice decides
// to skip.  Can't really figure out why.  Needs more investigation.
const unsigned long video_tick_inc = 10000;
unsigned long video_freq;
unsigned long video_frame_count;

int raspi_warp = 0;
static int raspi_boot_warp = 1;
static int fix_sid = 0;

extern struct joydev_config joydevs[MAX_JOY_PORTS];

int pending_emu_key_head = 0;
int pending_emu_key_tail = 0;
long pending_emu_key[16];
int pending_emu_key_pressed[16];

int pending_emu_joy_head = 0;
int pending_emu_joy_tail = 0;
int pending_emu_joy_value[128];
int pending_emu_joy_port[128];
int pending_emu_joy_type[128];

extern int pending_emu_quick_func;

#define COLOR16(red, green, blue)                                              \
  (((red)&0x1F) << 11 | ((green)&0x1F) << 6 | ((blue)&0x1F))

// Draw the src buffer into the dst buffer.
void draw(uint8_t *src, int srcw, int srch, int src_pitch, uint8_t *dst,
          int dst_pitch, int dst_off_x, int dst_off_y) {
  int y;
  for (y = 0; y < srch; y++) {
    int p1 = dst_off_x + (y + dst_off_y) * dst_pitch;
    int yp = y * src_pitch;
    memcpy(dst + p1, src + yp, srcw);
  }
}

int is_vic(struct video_canvas_s *canvas) {
  return canvas == vic_canvas;
}

int is_vdc(struct video_canvas_s *canvas) {
  return canvas == vdc_canvas;
}

// Called by menu when palette changes
void video_canvas_change_palette(int display_num, int palette_index) {
  if (!canvas_state[display_num].canvas)
    return;

  canvas_state[display_num].palette_index = palette_index;
  // This will call set_palette below to get called after color controls
  // have been applied to the palette.
  video_color_update_palette(canvas_state[display_num].canvas);
}

// Called when a color setting has changed
void video_color_setting_changed(int display_num) {
  if (!canvas_state[display_num].canvas)
    return;

  // This will call set_palette below to get called after color controls
  // have been applied to the palette.
  video_color_update_palette(canvas_state[display_num].canvas);
}

int video_canvas_set_palette(struct video_canvas_s *canvas, palette_t *p) {
  canvas->palette = p;
  int layer;

  if (is_vic(canvas)) {
    layer = FB_LAYER_VIC;
  } else {
    layer = FB_LAYER_VDC;
  }

  for (int i = 0; i < 16; i++) {
    circle_set_palette_fb2(layer, i,
                     COLOR16(p->entries[i].red >> 3, p->entries[i].green >> 3,
                             p->entries[i].blue >> 3));
  }
  circle_update_palette_fb2(layer);
}

// Draw buffer bridge functions back to kernel
static int draw_buffer_alloc(struct video_canvas_s *canvas, uint8_t **draw_buffer,
                             unsigned int fb_width, unsigned int fb_height,
                             unsigned int *fb_pitch) {
   if (is_vdc(canvas)) {
      vdc_enabled = 0;
      vdc_showing = 0;
      return circle_alloc_fb2(FB_LAYER_VDC, draw_buffer,
                              fb_width, fb_height, fb_pitch);
   } else {
      vic_enabled = 1;
      vic_showing = 0;

      return circle_alloc_fb2(FB_LAYER_VIC, draw_buffer,
                              fb_width, fb_height, fb_pitch);
   }
}

static void draw_buffer_free(struct video_canvas_s *canvas, uint8_t *draw_buffer) {
   if (is_vdc(canvas)) {
      circle_free_fb2(FB_LAYER_VDC);
   } else {
      circle_free_fb2(FB_LAYER_VIC);
   }
}

static void draw_buffer_clear(struct video_canvas_s *canvas, uint8_t *draw_buffer,
                              uint8_t value, unsigned int fb_width,
                              unsigned int fb_height, unsigned int fb_pitch) {
   if (is_vdc(canvas)) {
      circle_clear_fb2(FB_LAYER_VDC);
   } else {
      circle_clear_fb2(FB_LAYER_VIC);
   }
}

// Called for each canvas VICE wants to create.
// For C128, first will be the VDC, followed by VIC.
// For other machines, only one canvas is initialized.
void video_arch_canvas_init(struct video_canvas_s *canvas) {
  if (machine_class == VICE_MACHINE_C128 && canvas_num == 1) {
     vdc_canvas = canvas;
     vdc_canvas_index = canvas_num;
     vdc_first_refresh = 1;
  } else {
     int timing = circle_get_machine_timing();
     set_refresh_rate(timing, canvas);
     vic_first_refresh = 1;
     set_video_font();
     vic_canvas = canvas;
     vic_canvas_index = canvas_num;
     video_freq = canvas->refreshrate * video_tick_inc;
  }

  // Have our fb class allocate draw buffers
  canvas_state[canvas_num].draw_buffer_callback.draw_buffer_alloc =
     draw_buffer_alloc;
  canvas_state[canvas_num].draw_buffer_callback.draw_buffer_free =
     draw_buffer_free;
  canvas_state[canvas_num].draw_buffer_callback.draw_buffer_clear =
     draw_buffer_clear;
  canvas->video_draw_buffer_callback =
     &canvas_state[canvas_num].draw_buffer_callback;

  canvas_num++;
}

void apply_video_adjustments(int layer,
                             double hzoom, double vzoom, double aspect) {
  // Hide the layer. Can't show it here on the same loop so we have to
  // allow ensure_video() to do it for us.  If the canvas is enabled, it
  // will be shown again and our new settings will take effect.
  int index;
  struct video_canvas_s *canvas;

  circle_hide_fb2(layer);
  if (layer == FB_LAYER_VIC) {
     vic_showing = 0;
     index = vic_canvas_index;
     canvas = vic_canvas;
  } else {
     assert (layer == FB_LAYER_VDC);
     vdc_showing = 0;
     index = vdc_canvas_index;
     canvas = vdc_canvas;
  }

  circle_set_aspect_fb2(layer, aspect);

  canvas_state[index].border_w =
     canvas_state[index].max_border_w * hzoom;
  canvas_state[index].border_h =
     canvas_state[index].max_border_h * vzoom;

  canvas_state[index].vis_w =
     canvas_state[index].gfx_w +
        canvas_state[index].border_w*2;
  canvas_state[index].vis_h =
     canvas_state[index].gfx_h +
        canvas_state[index].border_h*2;

  canvas_state[index].src_off_x =
     canvas_state[index].max_border_w -
         canvas_state[index].border_w;

  canvas_state[index].src_off_y =
     canvas_state[index].max_border_h -
         canvas_state[index].border_h;

  canvas_state[index].left =
     canvas->geometry->extra_offscreen_border_left +
         canvas_state[index].src_off_x;

  canvas_state[index].top =
     canvas->geometry->first_displayed_line +
         canvas_state[index].src_off_y;

  // Cut out is defined by top,left,vis_w,vis_h

  canvas_state[index].overlay_y =
     canvas_state[index].top +
          canvas_state[index].max_border_h +
              canvas_state[index].gfx_h + 2;

  canvas_state[index].overlay_x = canvas_state[index].left;

  circle_set_src_rect_fb2(layer,
           canvas_state[index].left,
           canvas_state[index].top,
           canvas_state[index].vis_w,
           canvas_state[index].vis_h);
}

static struct video_canvas_s *video_canvas_create_vic(
       struct video_canvas_s *canvas,
       unsigned int *width,
       unsigned int *height, int mapped) {
  if (machine_class == VICE_MACHINE_VIC20) {
    *width = 448;
    *height = 284;
    canvas_state[vic_canvas_index].gfx_w = 22*8*2;
    canvas_state[vic_canvas_index].gfx_h = 23*8;
  } else {
    assert(machine_class == VICE_MACHINE_C64 ||
           machine_class == VICE_MACHINE_C128);
    *width = 384;
    *height = 272;
    canvas_state[vic_canvas_index].gfx_w = 40*8;
    canvas_state[vic_canvas_index].gfx_h = 25*8;
  }

  canvas->draw_buffer->canvas_physical_width = *width;
  canvas->draw_buffer->canvas_physical_height = *height;
  canvas->videoconfig->external_palette = 1;
  canvas->videoconfig->external_palette_name = "RASPI";

  int timing = circle_get_machine_timing();
  if (machine_class == VICE_MACHINE_VIC20) {
    if (timing == MACHINE_TIMING_NTSC_COMPOSITE ||
        timing == MACHINE_TIMING_NTSC_HDMI ||
        timing == MACHINE_TIMING_NTSC_CUSTOM) {
        canvas_state[vic_canvas_index].max_border_w = 40;
        canvas_state[vic_canvas_index].max_border_h = 22;
    } else {
        canvas_state[vic_canvas_index].max_border_w = 96;
        canvas_state[vic_canvas_index].max_border_h = 48;
    }
  } else {
    assert(machine_class == VICE_MACHINE_C64 ||
           machine_class == VICE_MACHINE_C128);
    if (timing == MACHINE_TIMING_NTSC_COMPOSITE ||
        timing == MACHINE_TIMING_NTSC_HDMI ||
        timing == MACHINE_TIMING_NTSC_CUSTOM) {
        canvas_state[vic_canvas_index].max_border_w = 32;
        canvas_state[vic_canvas_index].max_border_h = 23;
    } else {
        canvas_state[vic_canvas_index].max_border_w = 32;
        canvas_state[vic_canvas_index].max_border_h = 36;
    }
  }

  // Status overlay will only appear on VIC canvas for now.
  overlay_init(*width, OVERLAY_H);

  return canvas;
}


static struct video_canvas_s *video_canvas_create_vdc(
       struct video_canvas_s *canvas,
       unsigned int *width,
       unsigned int *height, int mapped) {
  assert(machine_class == VICE_MACHINE_C128);

  *width = 856;
  *height = 312;
  canvas_state[vdc_canvas_index].gfx_w = 80*8;
  canvas_state[vdc_canvas_index].gfx_h = 25*8;
  canvas->draw_buffer->canvas_physical_width = *width;
  canvas->draw_buffer->canvas_physical_height = *height;
  canvas->videoconfig->external_palette = 1;
  canvas->videoconfig->external_palette_name = "RASPI2";

  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing == MACHINE_TIMING_NTSC_HDMI ||
      timing == MACHINE_TIMING_NTSC_CUSTOM) {
      canvas_state[vdc_canvas_index].max_border_w = 112;
      canvas_state[vdc_canvas_index].max_border_h = 14;
  } else {
      canvas_state[vdc_canvas_index].max_border_w = 112;
      canvas_state[vdc_canvas_index].max_border_h = 38;
  }

  return canvas;
}

struct video_canvas_s *video_canvas_create(struct video_canvas_s *canvas,
                                           unsigned int *width,
                                           unsigned int *height, int mapped) {
  if (is_vic(canvas)) {
     canvas_state[0].canvas = canvas;
     return video_canvas_create_vic(canvas, width, height, mapped);
  } else {
     canvas_state[1].canvas = canvas;
     return video_canvas_create_vdc(canvas, width, height, mapped);
  }
}

void video_canvas_refresh(struct video_canvas_s *canvas, unsigned int xs,
                          unsigned int ys, unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h) {
  // We draw full frames each time so there's little to do here. Just turn on
  // boot warp on first refresh.
  if (is_vic(canvas)) {
     if (vic_first_refresh == 1) {
        // Apply current settings before ensure_video shows anything.
        apply_video_adjustments(FB_LAYER_VIC, 1.0, 1.0, 1.0); // TODO set from menu
        resources_set_int("WarpMode", 1);
        raspi_boot_warp = 1;
        vic_first_refresh = 0;
     }
  } else {
     if (vdc_first_refresh == 1) {
        apply_video_adjustments(FB_LAYER_VDC, 1.0, 1.0, 1.0); // TODO set from menu
        vdc_first_refresh = 0;
     }
  }
}

unsigned long vsyncarch_frequency(void) { return video_freq; }

unsigned long vsyncarch_gettime(void) { return video_ticks; }

void vsyncarch_init(void) {
  // See video refresh code to see why this is necessary.
  int sid_engine;
  resources_get_int("SidEngine", &sid_engine);
  if (sid_engine == SID_ENGINE_RESID) {
    fix_sid = 1;
  }
}

void vsyncarch_presync(void) { kbdbuf_flush(); }

void enable_vic(int enabled) {
  vic_enabled = enabled;
}

void enable_vdc(int enabled) {
  vdc_enabled = enabled;
}

// This makes sure we are showing what the enable flags say we should
// be showing.
void ensure_video(void) {
  if (vic_enabled && !vic_showing) {
     circle_show_fb2(FB_LAYER_VIC);
     vic_showing = 1;
  } else if (!vic_enabled && vic_showing) {
     circle_hide_fb2(FB_LAYER_VIC);
     vic_showing = 0;
  }

  if (vdc_enabled && !vdc_showing) {
     circle_show_fb2(FB_LAYER_VDC);
     vdc_showing = 1;
  } else if (!vdc_enabled && vdc_showing) {
     circle_hide_fb2(FB_LAYER_VDC);
     vdc_showing = 0;
  }
}

void vsyncarch_postsync(void) {
  ensure_video();

  // Sync with display's vertical blank signal.

  circle_frame_ready_fb2(FB_LAYER_VIC);
  if (machine_class == VICE_MACHINE_C128) {
     circle_frame_ready_fb2(FB_LAYER_VDC);
  }

  // This render will handle any OSDs we have. ODSs don't pause emulation.
  if (ui_activated) {
    // The only way we can be here and have ui_activated=1
    // is for an osd to be enabled.
    ui_render_now();
    circle_frame_ready_fb2(FB_LAYER_UI);
    ui_check_key();
  }

  // Always draw overlay on visible buffer
  if (overlay_forced() || (overlay_enabled() && overlay_showing)) {
    overlay_check();

    // Note: We made the overlay as wide as the physical canvas, so use
    // that for the overlay's width and pitch. 
    struct video_canvas_s* canvas = canvas_state[vic_canvas_index].canvas;
    draw(
        overlay_buf,
        canvas->draw_buffer->canvas_physical_width,
        OVERLAY_H,
        canvas->draw_buffer->canvas_physical_width,
        canvas->draw_buffer->draw_buffer + canvas_state[vic_canvas_index].overlay_x,
        canvas->draw_buffer->draw_buffer_pitch,
        0,
        canvas_state[vic_canvas_index].overlay_y);
  }

  video_ticks += video_tick_inc;

  // This yield is important to let the fake kernel 'threads' run.
  circle_yield();

  video_frame_count++;
  if (raspi_boot_warp && video_frame_count > 120) {
    raspi_boot_warp = 0;
    circle_boot_complete();
    resources_set_int("WarpMode", 0);
  }

  // BEGIN UGLY HACK
  // What follows is an ugly hack to get around a small extra delay
  // in the audio buffer when RESID is set and we first boot.  I
  // fought with VICE for a while but eventually just decided to re-init
  // RESID at this point in the boot process to work around the issue.  This
  // gets our audio buffer as close to the 'live' edge as possible.  It's only
  // an issue if RESID is the engine selected for boot.
  if (fix_sid) {
    if (video_frame_count == 121) {
      resources_set_int("SidEngine", SID_ENGINE_FASTSID);
    } else if (video_frame_count == 122) {
      resources_set_int("SidEngine", SID_ENGINE_RESID);
      fix_sid = 0;
    }
  }
  // END UGLY HACK

  // Hold the frame until vsync unless warping
  if (!raspi_boot_warp && !raspi_warp) {
    circle_wait_vsync();
  }

  circle_check_gpio();

  int reset_demo = 0;

  // Do key press/releases and joy latches on the main loop.
  circle_lock_acquire();
  while (pending_emu_key_head != pending_emu_key_tail) {
    int i = pending_emu_key_head & 0xf;
    reset_demo = 1;
    if (pending_emu_key_pressed[i]) {
      keyboard_key_pressed(pending_emu_key[i]);
    } else {
      keyboard_key_released(pending_emu_key[i]);
    }
    pending_emu_key_head++;
  }

  while (pending_emu_joy_head != pending_emu_joy_tail) {
    int i = pending_emu_joy_head & 0x7f;
    reset_demo = 1;
    switch (pending_emu_joy_type[i]) {
    case PENDING_EMU_JOY_TYPE_ABSOLUTE:
      joystick_set_value_absolute(pending_emu_joy_port[i],
                                  pending_emu_joy_value[i] & 0x1f);
      joystick_set_potx((pending_emu_joy_value[i] & POTX_BIT_MASK) >> 5);
      joystick_set_poty((pending_emu_joy_value[i] & POTY_BIT_MASK) >> 13);
      break;
    case PENDING_EMU_JOY_TYPE_AND:
      joystick_set_value_and(pending_emu_joy_port[i],
                             pending_emu_joy_value[i] & 0x1f);
      joystick_set_potx_and((pending_emu_joy_value[i] & POTX_BIT_MASK) >> 5);
      joystick_set_poty_and((pending_emu_joy_value[i] & POTY_BIT_MASK) >> 13);
      break;
    case PENDING_EMU_JOY_TYPE_OR:
      joystick_set_value_or(pending_emu_joy_port[i],
                            pending_emu_joy_value[i] & 0x1f);
      joystick_set_potx_or((pending_emu_joy_value[i] & POTX_BIT_MASK) >> 5);
      joystick_set_poty_or((pending_emu_joy_value[i] & POTY_BIT_MASK) >> 13);
      break;
    default:
      break;
    }
    pending_emu_joy_head++;
  }
  circle_lock_release();

  ui_handle_toggle_or_quick_func();

  if (reset_demo) {
    demo_reset_timeout();
  }

  if (raspi_demo_mode) {
    demo_check();
  }
}

void vsyncarch_sleep(unsigned long delay) {
  // We don't sleep here. Instead, our pace is governed by the
  // wait for vertical blank in vsyncarch_postsync above. This
  // times our machine properly.
}

// queue a key for press/release for the main loop
void circle_emu_key_interrupt(long key, int pressed) {
  circle_lock_acquire();
  int i = pending_emu_key_tail & 0xf;
  pending_emu_key[i] = key;
  pending_emu_key_pressed[i] = pressed;
  pending_emu_key_tail++;
  circle_lock_release();
}

// queue a joy latch change for the main loop
void circle_emu_joy_interrupt(int type, int port, int value) {
  circle_lock_acquire();
  int i = pending_emu_joy_tail & 0x7f;
  pending_emu_joy_type[i] = type;
  pending_emu_joy_port[i] = port;
  pending_emu_joy_value[i] = value;
  pending_emu_joy_tail++;
  circle_lock_release();
}

void circle_emu_quick_func_interrupt(int button_assignment) {
  pending_emu_quick_func = button_assignment;
}

// Called by our special hook in vice to load palettes from
// memory.
palette_t *raspi_video_load_palette(int num_entries, char *name) {
  palette_t *palette = palette_create(16, NULL);
  unsigned int *pal;
  // RASPI2 is for VDC
  if (strcmp(name, "RASPI2") == 0) {
     pal = raspi_get_palette(canvas_state[1].palette_index);
  } else {
     pal = raspi_get_palette(canvas_state[0].palette_index);
  }
  for (int i = 0; i < num_entries; i++) {
    palette->entries[i].red = pal[i * 3];
    palette->entries[i].green = pal[i * 3 + 1];
    palette->entries[i].blue = pal[i * 3 + 2];
    palette->entries[i].dither = 0;
  }
  return palette;
}

// This should be self contained. Don't rely on anything other
// than the frame buffer which is guaranteed to be available.
void main_exit(void) {
  // We should never get here.  If we do, it's probably
  // because essential roms are missing.  So display a message
  // to that effect.

  int i;
  uint8_t *fb = circle_get_fb1();
  int fb_pitch = circle_get_fb1_pitch();
  int h = circle_get_fb1_h();
  bzero(fb, h * fb_pitch);

  video_font = (uint8_t *)&font8x8_basic;
  for (i = 0; i < 256; ++i) {
    video_font_translate[i] = (8 * (i & 0x7f));
  }

  int x = 0;
  int y = 3;
  ui_draw_text_buf("Emulator failed to start.", x, y, 1, fb, fb_pitch);
  y += 8;
  ui_draw_text_buf("This most likely means you are missing", x, y, 1, fb,
                   fb_pitch);
  y += 8;
  ui_draw_text_buf("ROM files. Or you have specified an", x, y, 1, fb,
                   fb_pitch);
  y += 8;
  ui_draw_text_buf("invalid kernal, chargen or basic", x, y, 1, fb, fb_pitch);
  y += 8;
  ui_draw_text_buf("ROM in vice.ini.  See documentation.", x, y, 1, fb,
                   fb_pitch);
  y += 8;

  circle_set_fb1_palette(0, COLOR16(0, 0, 0));
  circle_set_fb1_palette(1, COLOR16(255 >> 3, 255 >> 3, 255 >> 3));
  circle_update_fb1_palette();
}
