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
#include "menu_usb.h"

int emux_machine_class = BMC64_MACHINE_CLASS_UNKNOWN;
int vic_showing;
int vdc_showing;
int vic_enabled;
int vdc_enabled;
int vdc_canvas_index;
int vic_canvas_index;

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
      double hborder, double vborder, double aspect,
      double lpad, double rpad, double tpad, double bpad,
      int zlayer) {
  // Hide the layer. Can't show it here on the same loop so we have to
  // allow emux_ensure_video() to do it for us.  If the canvas is enabled, it
  // will be shown again and our new settings will take effect.
  int index;

  circle_hide_fbl(layer);
  if (layer == FB_LAYER_VIC) {
     vic_showing = 0;
     index = vic_canvas_index;
  } else if (layer == FB_LAYER_VDC) {
     assert (layer == FB_LAYER_VDC);
     vdc_showing = 0;
     index = vdc_canvas_index;
  } else if (layer == FB_LAYER_UI) {
     index = -1;
     ui_showing = 0;
  } else {
     assert(0);
  }

  circle_set_zlayer_fbl(layer, zlayer);
  circle_set_padding_fbl(layer, lpad, rpad, tpad, bpad);

  circle_set_aspect_fbl(layer, aspect);

  if (index >= 0) {
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
       canvas_state[index].extra_offscreen_border_left +
           canvas_state[index].src_off_x;

    canvas_state[index].top =
       canvas_state[index].first_displayed_line +
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
