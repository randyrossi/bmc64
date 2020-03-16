/*
 * emu_api.c - emulator specific API functions
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

#include "emux_api.h"

#include <assert.h>

#include "circle.h"
#include "overlay.h"
#include "menu.h"
#include "menu_timing.h"
#include "menu_usb.h"

// This maps an ascii char to the charset's index in chargen rom
const uint8_t ascii_to_petscii[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x64,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x5f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x5e};
int emux_machine_class = BMC64_MACHINE_CLASS_UNKNOWN;
int vic_showing;
int vdc_showing;
int vic_enabled = 1;
int vdc_enabled;

// Ring buffer for key latch events
struct pending_emu_key_s pending_emu_key;

// Ring buffer for joy latch events
struct pending_emu_joy_s pending_emu_joy;

struct CanvasState canvas_state[2];

// queue a key for press/release for the main loop
void emux_key_interrupt(long key, int pressed) {
  circle_lock_acquire();
  int i = pending_emu_key.tail & 0xf;
  pending_emu_key.key[i] = key;
  pending_emu_key.pressed[i] = pressed;
  pending_emu_key.tail++;
  circle_lock_release();
}

// Same as above except can call while already holding the lock
void emux_key_interrupt_locked(long key, int pressed) {
  int i = pending_emu_key.tail & 0xf;
  pending_emu_key.key[i] = key;
  pending_emu_key.pressed[i] = pressed;
  pending_emu_key.tail++;
}

// Queue a joy latch change for the main loop
void emux_joy_interrupt(int type, int port, int device, int value) {
  circle_lock_acquire();
  int i = pending_emu_joy.tail & 0x7f;
  pending_emu_joy.type[i] = type;
  pending_emu_joy.port[i] = port;
  pending_emu_joy.device[i] = device;
  pending_emu_joy.value[i] = value;
  pending_emu_joy.tail++;
  circle_lock_release();
}

// This makes sure we are showing what the enable flags say we should
// be showing.
void emux_ensure_video(void) {
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

void emux_apply_video_adjustments(int layer,
      int hcenter, int vcenter,
      int hborder, int vborder, double h_stretch, double v_stretch,
      int hintstr, int vintstr,
      int use_hintstr, int use_vintstr,
      double lpad, double rpad, double tpad, double bpad,
      int zlayer) {
  // Hide the layer. Can't show it here on the same loop so we have to
  // allow emux_ensure_video() to do it for us.  If the canvas is enabled, it
  // will be shown again and our new settings will take effect.
  int index;

  circle_hide_fbl(layer);
  if (layer == FB_LAYER_VIC) {
     vic_showing = 0;
     index = VIC_INDEX;
  } else if (layer == FB_LAYER_VDC) {
     vdc_showing = 0;
     index = VDC_INDEX;
  } else if (layer == FB_LAYER_UI) {
     index = -1;
     ui_showing = 0;
  } else {
     assert(0);
  }

  circle_set_zlayer_fbl(layer, zlayer);
  circle_set_padding_fbl(layer, lpad, rpad, tpad, bpad);
  circle_set_stretch_fbl(layer, h_stretch, v_stretch,
                         hintstr, vintstr, use_hintstr,
                         use_vintstr);

  if (index >= 0) {
    canvas_state[index].border_w = hborder;
    canvas_state[index].border_h = vborder;

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
       canvas_state[index].extra_offscreen_border_left +
           canvas_state[index].src_off_x;

    canvas_state[index].top =
       canvas_state[index].first_displayed_line *
           canvas_state[index].raster_skip +
              canvas_state[index].src_off_y;

    // Cut out is defined by top,left,vis_w,vis_h

    canvas_state[index].overlay_y =
       canvas_state[index].top +
            canvas_state[index].max_border_h +
                canvas_state[index].gfx_h + 2;

    canvas_state[index].overlay_x = canvas_state[index].left;

    circle_set_src_rect_fbl(layer,
           canvas_state[index].left,
           canvas_state[index].top,
           canvas_state[index].vis_w,
           canvas_state[index].vis_h);
  }

  if (layer == FB_LAYER_UI) {
    // Due to raster skip, we won't use top, recalculate it
    // as though raster skip was 1.
    int raster_skip = canvas_state[VIC_INDEX].raster_skip;
    int ui_top = canvas_state[VIC_INDEX].first_displayed_line +
       canvas_state[VIC_INDEX].max_border_h / raster_skip -
           canvas_state[VIC_INDEX].border_h / raster_skip;

    // For the UI, we inherit the same cutout as the VIC (but take
    // in account raster_skip)
    circle_set_src_rect_fbl(layer,
           canvas_state[VIC_INDEX].left,
           ui_top,
           canvas_state[VIC_INDEX].vis_w,
           canvas_state[VIC_INDEX].vis_h / raster_skip);
  }

  circle_set_center_offset(layer,
           hcenter, vcenter);

  emux_geometry_changed(layer);
}

void emu_joy_interrupt_abs(int port, int device,
                           int js_up,
                           int js_down,
                           int js_left,
                           int js_right,
                           int js_fire,
                           int pot_x, int pot_y) {
  int val = 0;
  if (js_up) val |= 0x01;
  if (js_down) val |= 0x02;
  if (js_left) val |= 0x04;
  if (js_right) val |= 0x08;
  if (js_fire) val |= 0x10;
  add_pot_values(&val, pot_x, pot_y);
  emux_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, port, device, val);
}

void emu_pause_trap(uint16_t addr, void *data) {
  menu_about_to_activate();
  circle_show_fbl(FB_LAYER_UI);
  while (ui_enabled) {
    circle_check_gpio();
    ui_check_key();

    ui_handle_toggle_or_quick_func();

    ui_render_single_frame();
    hdmi_timing_hook();
    emux_ensure_video();
  }
  menu_about_to_deactivate();
  circle_hide_fbl(FB_LAYER_UI);
}

int is_ntsc() {
  int timing = circle_get_machine_timing();
  return
      timing == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing == MACHINE_TIMING_NTSC_HDMI ||
      timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI ||
      timing == MACHINE_TIMING_NTSC_DPI ||
      timing == MACHINE_TIMING_NTSC_CUSTOM_DPI;
}

int is_composite() {
  int timing = circle_get_machine_timing();
  return
      timing == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing == MACHINE_TIMING_PAL_COMPOSITE;
}

// Disable shader for composite or models > 3
int allow_shader() {
  return circle_get_model() <= 3 && !is_composite();
}
