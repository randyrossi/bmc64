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

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>

#include "c64/c64.h"
#include "c64/c64mem.h"
#include "monitor.h"
#include "mem.h"
#include "kbd.h"
#include "keyboard.h"
#include "video.h"
#include "viewport.h"
#include "ui.h"
#include "joy.h"
#include "resources.h"
#include "joyport/joystick.h"
#include "demo.h"
#include "overlay.h"
#include "menu.h"

// Keep video state shared between compilation units here
struct VideoData video_state;

// This maps an ascii char to the charset's index in chargen rom
static const uint8_t char_to_screen[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x5f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x5e
};

static unsigned int default_color_palette[] = {
0x00,0x00,0x00,
0xFD,0xFE,0xFC,
0xBE,0x1A,0x24,
0x30,0xE6,0xC6,
0xB4,0x1A,0xE2,
0x1F,0xD2,0x1E,
0x21,0x1B,0xAE,
0xDF,0xF6,0x0A,
0xB8,0x41,0x04,
0x6A,0x33,0x04,
0xFE,0x4A,0x57,
0x42,0x45,0x40,
0x70,0x74,0x6F,
0x59,0xFE,0x59,
0x5F,0x53,0xFE,
0xA4,0xA7,0xA2,
};

static unsigned int vice_color_palette[] = {
0x00,0x00,0x00,
0xFF,0xFF,0xFF,
0x68,0x37,0x2b,
0x70,0xa4,0xb2,
0x6f,0x3d,0x86,
0x58,0x8d,0x43,
0x35,0x28,0x79,
0xb8,0xc7,0x6f,
0x6f,0x4f,0x25,
0x43,0x39,0x00,
0x9a,0x67,0x59,
0x44,0x44,0x44,
0x6c,0x6c,0x6c,
0x9a,0xd2,0x84,
0x6c,0x5e,0xb5,
0x95,0x95,0x95,
};


static unsigned int c64hq_color_palette[] = {
0x0A,0x0A,0x0A,
0xFF,0xF8,0xFF,
0x85,0x1F,0x02,
0x65,0xCD,0xA8,
0xA7,0x3B,0x9F,
0x4D,0xAB,0x19,
0x1A,0x0C,0x92,
0xEB,0xE3,0x53,
0xA9,0x4B,0x02,
0x44,0x1E,0x00,
0xD2,0x80,0x74,
0x46,0x46,0x46,
0x8B,0x8B,0x8B,
0x8E,0xF6,0x8E,
0x4D,0x91,0xD1,
0xBA,0xBA,0xBA,
};


static unsigned int pepto_ntsc_color_palette[] = {
0x00,0x00,0x00,
0xFF,0xFF,0xFF,
0x67,0x37,0x2B,
0x70,0xA3,0xB1,
0x6F,0x3D,0x86,
0x58,0x8C,0x42,
0x34,0x28,0x79,
0xB7,0xC6,0x6E,
0x6F,0x4E,0x25,
0x42,0x38,0x00,
0x99,0x66,0x59,
0x43,0x43,0x43,
0x6B,0x6B,0x6B,
0x9A,0xD1,0x83,
0x6B,0x5E,0xB5,
0x95,0x95,0x95,
};

static unsigned int pepto_pal_color_palette[] = {
0x00,0x00,0x00,
0xFF,0xFF,0xFF,
0x68,0x37,0x2b,
0x70,0xa4,0xb2,
0x6f,0x3d,0x86,
0x58,0x8d,0x43,
0x35,0x28,0x79,
0xb8,0xc7,0x6f,
0x6f,0x4f,0x25,
0x43,0x39,0x00,
0x9a,0x67,0x59,
0x44,0x44,0x44,
0x6c,0x6c,0x6c,
0x9a,0xd2,0x84,
0x6c,0x5e,0xb5,
0x95,0x95,0x95,
};

// This keeps track of the y offset for the region in our virtual
// frame buffer that is NOT visible at the moment. It toggles
// back and forth between 0 and 2x our physical vertical height.
// The idea is, we wait for vsync before telling the video hardware
// to show the pixels we just wrote, avoiding horizontal tearing.
int need_buffer_swap = 0;

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
int raspi_boot_warp = 1;

extern struct joydev_config joydevs[2];

volatile int pending_emu_key_head = 0;
volatile int pending_emu_key_tail = 0;
volatile long pending_emu_key[16];
volatile int pending_emu_key_pressed[16];

volatile int pending_emu_joy_head = 0;
volatile int pending_emu_joy_tail = 0;
volatile int pending_emu_joy_value[128];
volatile int pending_emu_joy_port[128];
volatile int pending_emu_joy_type[128];

#define COLOR16(red, green, blue)         (((red) & 0x1F) << 11 \
                                        | ((green) & 0x1F) << 6 \
                                        | ((blue) & 0x1F))

// Draw the src buffer into the dst buffer.
void draw(uint8_t *src, int srcw, int srch, int src_pitch,
           uint8_t *dst, int dst_pitch, int off_x, int off_y) {
    // x,y are coordinates in src canvas space
    int x,y;
    for(y=0; y < srch; y++){
       int gy = (y+off_y);
       int p1 = off_x+gy*dst_pitch;
       int yp = y*src_pitch;
       memcpy(dst+p1, src+yp, srcw);
    }
}

static unsigned int* get_palette(int index) {
  switch (index) {
     case PALETTE_DEFAULT:
	return default_color_palette;
	break;
     case PALETTE_VICE:
	return vice_color_palette;
	break;
     case PALETTE_C64HQ:
	return c64hq_color_palette;
	break;
     case PALETTE_PEPTO_NTSC:
	return pepto_ntsc_color_palette;
	break;
     case PALETTE_PEPTO_PAL:
	return pepto_pal_color_palette;
	break;
     default:
        return NULL;
  }
}

// Called by menu when palette changes
void video_canvas_change_palette(int index) {
  if (!video_state.canvas) return;

  video_state.palette_index = index;
  // This will call set_palette below to get called after color controls
  // have been applied to the palette.
  video_color_update_palette(video_state.canvas);
}

// Called when a color setting has changed
void video_color_setting_changed() {
  if (!video_state.canvas) return;

  // This will call set_palette below to get called after color controls
  // have been applied to the palette.
  video_color_update_palette(video_state.canvas);
}

int video_canvas_set_palette(struct video_canvas_s *canvas, palette_t *p) {
  canvas->palette = p;

  for (int i=0; i<16;i++) {
    circle_set_palette(i, COLOR16(p->entries[i].red >> 4,
                                  p->entries[i].green >> 4,
                                  p->entries[i].blue >> 4));
  }
  circle_update_palette();
}

struct video_canvas_s *video_canvas_create(struct video_canvas_s *canvas, unsigned int *width, unsigned int *height, int mapped) {
  *width  = circle_get_display_w();
  *height = circle_get_display_h();
  canvas->draw_buffer->canvas_physical_width = *width;
  canvas->draw_buffer->canvas_physical_height = *height;
  canvas->videoconfig->external_palette = 1;
  canvas->videoconfig->external_palette_name = "RASPI";
  video_state.canvas = canvas;
  return canvas;
}

void video_arch_canvas_init(struct video_canvas_s *canvas){
  int i;

  canvas->video_draw_buffer_callback = NULL;

  uint8_t *fb = circle_get_fb();
  int fb_pitch = circle_get_fb_pitch();
  int h = circle_get_display_h();
  bzero(fb, h*fb_pitch);

  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI ||
        timing == MACHINE_TIMING_NTSC_COMPOSITE) {
     canvas->refreshrate = C64_NTSC_RFSH_PER_SEC;
  } else {
     canvas->refreshrate = C64_PAL_RFSH_PER_SEC;
  }

  canvas->off_x = -1;
  canvas->off_y = -1;

  video_freq = canvas->refreshrate * video_tick_inc;

  int scr_w = circle_get_display_w();
  int scr_h = circle_get_display_h();

  video_state.canvas = canvas;
  video_state.scr_w = scr_w;
  video_state.scr_h = scr_h;
  video_state.dst_pitch = fb_pitch;
  video_state.dst = fb;
  video_state.font = mem_chargen_rom + 0x800;
  video_state.palette_index = PALETTE_DEFAULT;
  for (i = 0; i < 256; ++i) {
     video_state.font_translate[i] = 8 * char_to_screen[i];
  }
  video_state.offscreen_buffer_y = 0;
  video_state.onscreen_buffer_y = circle_get_display_h();

  overlay_init(scr_w, 10, scr_w, scr_h);
}

void video_canvas_refresh(struct video_canvas_s *canvas,
                unsigned int xs, unsigned int ys, unsigned int xi,
                unsigned int yi, unsigned int w, unsigned int h) {
  // TODO: Precompute as much of this as possible and store in
  // our canvas struct.
  uint8_t *src = canvas->draw_buffer->draw_buffer;
  int s_pitch = canvas->draw_buffer->draw_buffer_width;
  int sh = canvas->draw_buffer->visible_height;
  int sw = canvas->draw_buffer->visible_width;

  // Figure out what it takes to center our canvas on the display
  if (canvas->off_x == -1) {
     resources_set_int("WarpMode", 1);
     raspi_boot_warp = 1;
     int scr_w  = circle_get_display_w();
     int scr_h = circle_get_display_h();
     canvas->off_x = (scr_w - sw) / 2;
     canvas->off_y = (scr_h - sh) / 2;
     if (canvas->off_x < 0) canvas->off_x = 0;
     if (canvas->off_y < 0) canvas->off_y = 0;
  }

  // Top left calculation used for full frame scaling
  int top_left = (canvas->geometry->first_displayed_line) *
      canvas->draw_buffer->draw_buffer_width +
          canvas->geometry->extra_offscreen_border_left;

  // This will do the whole frame, not the region. Scale into the offscreen
  // area.
  draw(src+top_left, sw, sh, s_pitch,
       video_state.dst + video_state.offscreen_buffer_y*video_state.dst_pitch,
       video_state.dst_pitch, canvas->off_x, canvas->off_y);

  need_buffer_swap = 1;
}

unsigned long vsyncarch_frequency(void) {
    return video_freq;
}

unsigned long vsyncarch_gettime(void) {
    return video_ticks;
}

void vsyncarch_init(void){
}

void vsyncarch_presync(void){
    // Nothing to do.
}

void videoarch_swap() {
  // Show the region we just drew.
  circle_set_fb_y(video_state.offscreen_buffer_y);
  // Swap buffer ptr for next frame.
  video_state.onscreen_buffer_y = video_state.offscreen_buffer_y;
  video_state.offscreen_buffer_y = circle_get_display_h() -
                                      video_state.offscreen_buffer_y;
}

void vsyncarch_postsync(void){
  // Sync with display's vertical blank signal.

  if (need_buffer_swap) {
    videoarch_swap();
    need_buffer_swap = 0;
  }

  // Always draw overlay on visible buffer
  if (overlay_forced() || (overlay_enabled() && overlay_showing)) {
     overlay_check();
     int sh = video_state.canvas->draw_buffer->visible_height;
     int sw = video_state.canvas->draw_buffer->visible_width;
     draw(overlay_buf, sw, 10, sw,
       video_state.dst + video_state.onscreen_buffer_y*video_state.dst_pitch,
       video_state.dst_pitch, video_state.canvas->off_x, video_state.canvas->off_y + sh - 10);
  }

  video_ticks+=video_tick_inc;

  // This yield is important to let the fake kernel 'threads' run.
  circle_yield();

  video_frame_count++;
  if (raspi_boot_warp && video_frame_count > 120) {
     raspi_boot_warp = 0;
     circle_boot_complete();
     resources_set_int("WarpMode", 0);
  }

  // Hold the frame until vsync unless warping
  if (!raspi_boot_warp && !raspi_warp) {
     circle_wait_vsync();
  }

  circle_check_gpio();

  if (joydevs[0].device == JOYDEV_GPIO_0 || joydevs[1].device == JOYDEV_GPIO_0) {
     circle_poll_joysticks(0, 0);
  }
  if (joydevs[0].device == JOYDEV_GPIO_1 || joydevs[1].device == JOYDEV_GPIO_1) {
     circle_poll_joysticks(1, 0);
  }

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
              pending_emu_joy_value[i]);
           break;
        case PENDING_EMU_JOY_TYPE_AND:
           joystick_set_value_and(pending_emu_joy_port[i],
              pending_emu_joy_value[i]);
           break;
        case PENDING_EMU_JOY_TYPE_OR:
           joystick_set_value_or(pending_emu_joy_port[i],
              pending_emu_joy_value[i]);
           break;
        default:
           break;
     }
     pending_emu_joy_head++;
  }

  // This ensures we transition from emulator to ui only after we've
  // submitted key events and let the emulator process them. Otherwise,
  // we can leave keys in a down state unintentionally. Needs to be set
  // to 2 to ensure we dequeue, then let the emulator process those events.
  if (ui_toggle_pending) {
     ui_toggle_pending--;
     if (ui_toggle_pending == 0) {
        ui_toggle();
     }
  }

  circle_lock_release();

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

// Called by our special hook in vice to load palettes from
// memory.
palette_t* raspi_video_load_palette(int num_entries, char* name) {
  palette_t* palette = palette_create(16, NULL);
  unsigned int* pal = get_palette(video_state.palette_index);
  for (int i=0; i<num_entries;i++) {
    palette->entries[i].red = pal[i*3];
    palette->entries[i].green = pal[i*3+1];
    palette->entries[i].blue = pal[i*3+2];
    palette->entries[i].dither = 0;
  }
  return palette;
}
