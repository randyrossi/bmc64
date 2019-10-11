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

// Increments with each canvas being inited by vice
int canvas_num;

// One struct for each display (can be 2 for C128)
struct CanvasState canvas_state[2];

struct video_canvas_s *vdc_canvas;
struct video_canvas_s *vic_canvas;

// NOTE: For Plus/4, the vic_* variables are actually ted.
// Maybe rename to pri_?

static int vdc_canvas_index;
static int vic_canvas_index;

static int vic_enabled;
static int vdc_enabled;

static int vic_showing;
static int vdc_showing;

uint8_t *video_font;
uint16_t video_font_translate[256];
uint8_t *raw_video_font;

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

static int vdc_map[] = {0, 12, 6, 14, 5, 13, 11, 3, 2, 10, 8, 4, 9, 7, 15, 1};

extern struct joydev_config joydevs[MAX_JOY_PORTS];

struct pending_emu_key_s {
  int head;
  int tail;
  long key[16];
  int pressed[16];
};

struct pending_emu_key_s pending_emu_key;

struct pending_emu_joy_s {
  int head;
  int tail;
  int value[128];
  int port[128];
  int type[128];
  int device[128];
};

struct pending_emu_joy_s pending_emu_joy;

extern int pending_emu_quick_func;

#define COLOR16(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)

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
    for (int i = 0; i < p->num_entries; i++) {
      circle_set_palette_fbl(layer, i,
                     COLOR16(p->entries[i].red, p->entries[i].green,
                             p->entries[i].blue));
    }
  } else {
    layer = FB_LAYER_VDC;
    for (int i = 0; i < 16; i++) {
      int j = vdc_map[i];
      circle_set_palette_fbl(layer, i,
                     COLOR16(p->entries[j].red, p->entries[j].green,
                             p->entries[j].blue));
    }
  }

  circle_update_palette_fbl(layer);
}

// Draw buffer bridge functions back to kernel
static int draw_buffer_alloc(struct video_canvas_s *canvas,
                             uint8_t **draw_buffer,
                             unsigned int fb_width, unsigned int fb_height,
                             unsigned int *fb_pitch) {
   if (is_vdc(canvas)) {
      return circle_alloc_fbl(FB_LAYER_VDC, draw_buffer,
                              fb_width, fb_height, fb_pitch);
   } else {
      return circle_alloc_fbl(FB_LAYER_VIC, draw_buffer,
                              fb_width, fb_height, fb_pitch);
   }
}

static void draw_buffer_free(struct video_canvas_s *canvas, uint8_t *draw_buffer) {
   if (is_vdc(canvas)) {
      circle_free_fbl(FB_LAYER_VDC);
   } else {
      circle_free_fbl(FB_LAYER_VIC);
   }
}

static void draw_buffer_clear(struct video_canvas_s *canvas, uint8_t *draw_buffer,
                              uint8_t value, unsigned int fb_width,
                              unsigned int fb_height, unsigned int fb_pitch) {
   if (is_vdc(canvas)) {
      circle_clear_fbl(FB_LAYER_VDC);
   } else {
      circle_clear_fbl(FB_LAYER_VIC);
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
     vdc_enabled = 0;
     vdc_showing = 0;
  } else {
     int timing = circle_get_machine_timing();
     set_refresh_rate(timing, canvas);
     vic_first_refresh = 1;
     vic_canvas = canvas;
     vic_canvas_index = canvas_num;
     video_freq = canvas->refreshrate * video_tick_inc;
     vic_enabled = 1;
     vic_showing = 0;
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

void video_init_overlay(int padding, int c40_80_state, int vkbd_transparency) {
  overlay_init(padding, c40_80_state, vkbd_transparency);
}

void apply_video_adjustments(int layer,
      int hcenter, int vcenter,
      double hborder, double vborder, double aspect,
      double lpad, double rpad, double tpad, double bpad,
      int zlayer) {
  // Hide the layer. Can't show it here on the same loop so we have to
  // allow ensure_video() to do it for us.  If the canvas is enabled, it
  // will be shown again and our new settings will take effect.
  int index;
  struct video_canvas_s *canvas;

  circle_hide_fbl(layer);
  if (layer == FB_LAYER_VIC) {
     vic_showing = 0;
     index = vic_canvas_index;
     canvas = vic_canvas;
  } else if (layer == FB_LAYER_VDC) {
     assert (layer == FB_LAYER_VDC);
     vdc_showing = 0;
     index = vdc_canvas_index;
     canvas = vdc_canvas;
  } else if (layer == FB_LAYER_UI) {
     index = -1;
     ui_showing = 0;
     canvas = 0;
  } else {
     assert(0);
  }

  circle_set_zlayer_fbl(layer, zlayer);
  circle_set_padding_fbl(layer, lpad, rpad, tpad, bpad);

  circle_set_aspect_fbl(layer, aspect);

  if (index >= 0 && canvas) {
    canvas_state[index].border_w =
       canvas_state[index].max_border_w * hborder;
    canvas_state[index].border_h =
       canvas_state[index].max_border_h * vborder;

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
  }

  if (layer != FB_LAYER_UI) {
     circle_set_src_rect_fbl(layer,
           canvas_state[index].left,
           canvas_state[index].top,
           canvas_state[index].vis_w,
           canvas_state[index].vis_h);
  }

  circle_set_center_offset(layer,
           hcenter, vcenter);          
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
  } else if (machine_class == VICE_MACHINE_PLUS4) {
    *width = 384;
    *height = 288;
    canvas_state[vic_canvas_index].gfx_w = 40*8;
    canvas_state[vic_canvas_index].gfx_h = 25*8;
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
  } else if (machine_class == VICE_MACHINE_PLUS4) {
    if (timing == MACHINE_TIMING_NTSC_COMPOSITE ||
        timing == MACHINE_TIMING_NTSC_HDMI ||
        timing == MACHINE_TIMING_NTSC_CUSTOM) {
        canvas_state[vic_canvas_index].max_border_w = 32;
        canvas_state[vic_canvas_index].max_border_h = 16;
    } else {
        canvas_state[vic_canvas_index].max_border_w = 32;
        canvas_state[vic_canvas_index].max_border_h = 40;
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
        resources_set_int("WarpMode", 1);
        raspi_boot_warp = 1;
        vic_first_refresh = 0;
     }
        set_video_font(); // !!! FIX THIS
  } else {
     if (vdc_first_refresh == 1) {
        // Nothing to do.  Consider removing.
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
     circle_show_fbl(FB_LAYER_VIC);
     vic_showing = 1;
  } else if (!vic_enabled && vic_showing) {
     circle_hide_fbl(FB_LAYER_VIC);
     vic_showing = 0;
  }

  if (vdc_enabled && !vdc_showing) {
     circle_show_fbl(FB_LAYER_VDC);
     vdc_showing = 1;
  } else if (!vdc_enabled && vdc_showing) {
     circle_hide_fbl(FB_LAYER_VDC);
     vdc_showing = 0;
  }

  if ((statusbar_enabled && !statusbar_showing) ||
         (vkbd_enabled && !vkbd_showing)) {
     if (statusbar_enabled && !statusbar_showing) {
        statusbar_showing = 1;
     }
     if (vkbd_enabled && !vkbd_showing) {
        vkbd_showing = 1;
     }
     if (statusbar_showing || vkbd_showing) {
        circle_show_fbl(FB_LAYER_STATUS);
     }
  } else if ((!statusbar_enabled && statusbar_showing) ||
                 (!vkbd_enabled && vkbd_showing)) {
     if (!statusbar_enabled && statusbar_showing) {
        statusbar_showing = 0;
     }
     if (!vkbd_enabled && vkbd_showing) {
        vkbd_showing = 0;
     }
     if (!statusbar_showing && !vkbd_showing) {
        circle_hide_fbl(FB_LAYER_STATUS);
     }
  }

  if (ui_enabled && !ui_showing) {
     circle_show_fbl(FB_LAYER_UI);
     ui_showing = 1;
  }
}

void vsyncarch_postsync(void) {
  ensure_video();

  // This render will handle any OSDs we have. ODSs don't pause emulation.
  if (ui_enabled) {
    // The only way we can be here and have ui_enabled=1
    // is for an osd to be enabled.
    ui_render_now(-1); // only render top most menu
    circle_frames_ready_fbl(FB_LAYER_UI, -1 /* no 2nd layer */, 0 /* no sync */);
    ui_check_key();
  }

  if (statusbar_showing || vkbd_showing) {
    overlay_check();
    if (overlay_dirty) {
       circle_frames_ready_fbl(FB_LAYER_STATUS, -1 /* no 2nd layer */, 0 /* no sync */);
       overlay_dirty = 0;
    }
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

  // Hold for vsync unless warping or in boot warp.
  circle_frames_ready_fbl(FB_LAYER_VIC,
                         machine_class == VICE_MACHINE_C128 ? FB_LAYER_VDC : -1,
                         !raspi_boot_warp && !raspi_warp);

  circle_check_gpio();

  int reset_demo = 0;

  // Do key press/releases and joy latches on the main loop.
  circle_lock_acquire();
  while (pending_emu_key.head != pending_emu_key.tail) {
    int i = pending_emu_key.head & 0xf;
    reset_demo = 1;
    if (vkbd_enabled) {
      // Kind of nice to have virtual keyboard's state
      // stay in sync with changes happening from USB
      // key events.
      vkbd_sync_event(pending_emu_key.key[i], pending_emu_key.pressed[i]);
    }
    if (pending_emu_key.pressed[i]) {
      keyboard_key_pressed(pending_emu_key.key[i]);
    } else {
      keyboard_key_released(pending_emu_key.key[i]);
    }
    pending_emu_key.head++;
  }

  while (pending_emu_joy.head != pending_emu_joy.tail) {
    int i = pending_emu_joy.head & 0x7f;
    reset_demo = 1;
    if (vkbd_enabled) {
      int value = pending_emu_joy.value[i];
      int devd = pending_emu_joy.device[i];
      switch (pending_emu_joy.type[i]) {
      case PENDING_EMU_JOY_TYPE_ABSOLUTE:
        if (!vkbd_press[devd]) {
           if (value & 0x1 && !vkbd_up[devd]) {
             vkbd_up[devd] = 1;
             vkbd_nav_up();
           } else if (!(value & 0x1) && vkbd_up[devd]) {
             vkbd_up[devd] = 0;
           }
           if (value & 0x2 && !vkbd_down[devd]) {
             vkbd_down[devd] = 1;
             vkbd_nav_down();
           } else if (!(value & 0x2) && vkbd_down[devd]) {
             vkbd_down[devd] = 0;
           }
           if (value & 0x4 && !vkbd_left[devd]) {
             vkbd_left[devd] = 1;
             vkbd_nav_left();
           } else if (!(value & 0x4) && vkbd_left[devd]) {
             vkbd_left[devd] = 0;
           }
           if (value & 0x8 && !vkbd_right[devd]) {
             vkbd_right[devd] = 1;
             vkbd_nav_right();
           } else if (!(value & 0x8) && vkbd_right[devd]) {
             vkbd_right[devd] = 0;
           }
        }
        if (value & 0x10 && !vkbd_press[devd]) vkbd_nav_press(1, devd);
        else if (!(value & 0x10) && vkbd_press[devd]) vkbd_nav_press(0, devd);
        break;
      }
    } else {
      switch (pending_emu_joy.type[i]) {
      case PENDING_EMU_JOY_TYPE_ABSOLUTE:
        joystick_set_value_absolute(pending_emu_joy.port[i],
                                  pending_emu_joy.value[i] & 0x1f);
        joystick_set_potx((pending_emu_joy.value[i] & POTX_BIT_MASK) >> 5);
        joystick_set_poty((pending_emu_joy.value[i] & POTY_BIT_MASK) >> 13);
        break;
      case PENDING_EMU_JOY_TYPE_AND:
        joystick_set_value_and(pending_emu_joy.port[i],
                             pending_emu_joy.value[i] & 0x1f);
        joystick_set_potx_and((pending_emu_joy.value[i] & POTX_BIT_MASK) >> 5);
        joystick_set_poty_and((pending_emu_joy.value[i] & POTY_BIT_MASK) >> 13);
        break;
      case PENDING_EMU_JOY_TYPE_OR:
        joystick_set_value_or(pending_emu_joy.port[i],
                            pending_emu_joy.value[i] & 0x1f);
        joystick_set_potx_or((pending_emu_joy.value[i] & POTX_BIT_MASK) >> 5);
        joystick_set_poty_or((pending_emu_joy.value[i] & POTY_BIT_MASK) >> 13);
        break;
      default:
        break;
      }
    }
    pending_emu_joy.head++;
  }
  circle_lock_release();

  ui_handle_toggle_or_quick_func();

  if (reset_demo) {
    demo_reset_timeout();
  }

  if (raspi_demo_mode) {
    demo_check();
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

}

void vsyncarch_sleep(unsigned long delay) {
  // We don't sleep here. Instead, our pace is governed by the
  // wait for vertical blank in vsyncarch_postsync above. This
  // times our machine properly.
}

// queue a key for press/release for the main loop
void circle_emu_key_interrupt(long key, int pressed) {
  circle_lock_acquire();
  int i = pending_emu_key.tail & 0xf;
  pending_emu_key.key[i] = key;
  pending_emu_key.pressed[i] = pressed;
  pending_emu_key.tail++;
  circle_lock_release();
}

// Same as above except can call while already holding the lock
void circle_emu_key_locked(long key, int pressed) {
  int i = pending_emu_key.tail & 0xf;
  pending_emu_key.key[i] = key;
  pending_emu_key.pressed[i] = pressed;
  pending_emu_key.tail++;
}

// queue a joy latch change for the main loop
void circle_emu_joy_interrupt(int type, int port, int device, int value) {
  circle_lock_acquire();
  int i = pending_emu_joy.tail & 0x7f;
  pending_emu_joy.type[i] = type;
  pending_emu_joy.port[i] = port;
  pending_emu_joy.device[i] = device;
  pending_emu_joy.value[i] = value;
  pending_emu_joy.tail++;
  circle_lock_release();
}

void circle_emu_quick_func_interrupt(int button_assignment) {
  pending_emu_quick_func = button_assignment;
}

// Called by our special hook in vice to load palettes from
// memory.
palette_t *raspi_video_load_palette(int num_entries, char *name) {
  palette_t *palette = palette_create(num_entries, NULL);
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
  uint8_t *fb;
  int fb_pitch;
  int fb_width = 320;
  int fb_height = 240;

  circle_alloc_fbl(FB_LAYER_VIC, &fb,
                      fb_width, fb_height, &fb_pitch);
  circle_clear_fbl(FB_LAYER_VIC);
  circle_show_fbl(FB_LAYER_VIC);

  video_font = (uint8_t *)&font8x8_basic;
  for (i = 0; i < 256; ++i) {
    video_font_translate[i] = (8 * (i & 0x7f));
  }

  int x = 0;
  int y = 3;
  switch (machine_class) {
    case VICE_MACHINE_VIC20:
      ui_draw_text_buf("VIC20", x, y, 1, fb, fb_pitch, 1);
      break;
    case VICE_MACHINE_C64:
      ui_draw_text_buf("C64", x, y, 1, fb, fb_pitch, 1);
      break;
    case VICE_MACHINE_C128:
      ui_draw_text_buf("C128", x, y, 1, fb, fb_pitch, 1);
      break;
    case VICE_MACHINE_PLUS4:
      ui_draw_text_buf("PLUS/4", x, y, 1, fb, fb_pitch, 1);
      break;
  }
  y += 8;
  ui_draw_text_buf("Emulator failed to start.", x, y, 1, fb, fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("This most likely means you are missing", x, y, 1, fb,
                   fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("ROM files. Or you have specified an", x, y, 1, fb,
                   fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("invalid kernal, chargen or basic", x, y, 1, fb, fb_pitch, 1);
  y += 8;
  ui_draw_text_buf("ROM in vice.ini.  See documentation.", x, y, 1, fb,
                   fb_pitch, 1);
  y += 8;

  circle_set_palette_fbl(FB_LAYER_VIC, 0, COLOR16(0, 0, 0));
  circle_set_palette_fbl(FB_LAYER_VIC, 1, COLOR16(255, 255, 255));
  circle_update_palette_fbl(FB_LAYER_VIC);
  circle_frames_ready_fbl(FB_LAYER_VIC, -1, 0);
}

// These will revert back to 0 when the user moves off the
// current item.
void video_canvas_reveal_temp(int layer) {
  if (layer == FB_LAYER_VIC && vic_showing) {
    ui_set_transparent(1);
    ui_set_render_current_item_only(1);
  }
  else if (layer == FB_LAYER_VDC && vdc_showing) {
    ui_set_transparent(1);
    ui_set_render_current_item_only(1);
  }
}
