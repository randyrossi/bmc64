/*
 * overlay.c
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

#include "overlay.h"

#include "datasette.h"
#include "drive.h"
#include "machine.h"
#include "menu.h"
#include "resources.h"
#include "ui.h"
#include "uiapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLACK 0
#define GREEN 5
#define RED 2
#define LIGHT_GREEN 13
#define LIGHT_RED 10

#define TICKS_PER_SECOND 1000000L

static int drive_x[4];
static int tape_x;
static int tape_controls_x;
static int tape_motor_x;
static int warp_x;
static int joyswap_x;
static int columns_x;
static int fg_color;
static int bg_color;

static int drive_led_types[DRIVE_NUM];
static unsigned int current_drive_leds[DRIVE_NUM][2];
static int tape_counter = -1;

static unsigned long overlay_delay = 0;
static unsigned long overlay_start = 0;

static int inset_x;
static int inset_y;

uint8_t *overlay_buf;
static int overlay_buf_pitch;

#define OVERLAY_HEIGHT 10

// Create a new overlay buffer
uint8_t *overlay_init(int width, int height, int padding, int c40_80_state) {
  bg_color = 0;
  fg_color = 1;
  switch (machine_class) {
     case VICE_MACHINE_VIC20:
        bg_color = 3;
        fg_color = 6;
        break;
     case VICE_MACHINE_C64:
     case VICE_MACHINE_C128:
        bg_color = 14;
        fg_color = 1;
        break;
     default:
        break;
  }

  circle_alloc_fb2(FB_LAYER_STATUS,
                   &overlay_buf, width, OVERLAY_HEIGHT, &overlay_buf_pitch);
  // Use negative aspect here so our overlay is stretched to the full
  // horizontal resolution rather than vertical.
  circle_set_aspect_fb2(FB_LAYER_STATUS, -(double)width/(double)OVERLAY_HEIGHT);
  // We want our status bar to show up at the bottom of the screen with
  // padding set by user.
  circle_set_valign_fb2(FB_LAYER_STATUS, 1 /* BOTTOM */, padding);
  memset(overlay_buf, bg_color, overlay_buf_pitch * OVERLAY_HEIGHT);

  // Figure out inset that will center.
  char *template;
  if (machine_class == VICE_MACHINE_VIC20) {
     template = "8:  9:  10:  11:  T:000 STP   W:  J:1";
  } else if (machine_class == VICE_MACHINE_C128) {
     template = "8:  9:  10:  11:  T:000 STP   W:  J:12 C:  ";
  } else {
     template = "8:  9:  10:  11:  T:000 STP   W:  J:12";
  }
  inset_x = width / 2 - (strlen(template) * 8) / 2;
  inset_y = 1;

  ui_draw_text_buf(template, inset_x, inset_y, fg_color, overlay_buf,
                   overlay_buf_pitch);

  // Positions relative to start of text (before inset)
  drive_x[0] = 2 * 8;
  drive_x[1] = 6 * 8;
  drive_x[2] = 11 * 8;
  drive_x[3] = 16 * 8;
  tape_x = 20 * 8;
  tape_controls_x = 24 * 8;
  tape_motor_x = 28 * 8;
  warp_x = 32 * 8;
  joyswap_x = 36 * 8;
  columns_x = 41 * 8;

  ui_draw_text_buf("-", warp_x + inset_x, inset_y, fg_color, overlay_buf,
                   overlay_buf_pitch);

  overlay_40_80_columns_changed(c40_80_state);

  return overlay_buf;
}

// Some activity means overlay should show (if menu option set)
void overlay_activate() {
  if (!overlay_buf) return;

  overlay_start = circle_get_ticks();
  overlay_delay = 5 * TICKS_PER_SECOND;
  if (!overlay_never()) {
     overlay_enabled = 1;
  }
}

// Called by VICE to enable a drive status lights
void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color) {
  if (!overlay_buf)
    return;

  overlay_activate();

  if (!overlay_enabled) return;

  int i, enabled = state;

  for (i = 0; i < DRIVE_NUM; ++i) {
    if (overlay_buf) {
      ui_draw_rect_buf(drive_x[i] + 8 * 0 + inset_x, inset_y + 2, 6, 4,
                       bg_color, 1, overlay_buf, overlay_buf_pitch);
      // The second LED never seems to go on.  Removing it.
      //ui_draw_rect_buf(drive_x[i] + 8 * 1 + inset_x, inset_y + 2, 6, 4,
      //                 bg_color, 1, overlay_buf, overlay_buf_pitch);
    }
    if (enabled & 1) {
      drive_led_types[i] = drive_led_color[i];
      current_drive_leds[i][0] = 0;
      current_drive_leds[i][1] = 0;
      ui_draw_rect_buf(drive_x[i] + 8 * 0 + inset_x, inset_y + 2, 6, 4, BLACK,
                       1, overlay_buf, overlay_buf_pitch);
      // The second LED never seems to go on.  Removing it.
      //ui_draw_rect_buf(drive_x[i] + 8 * 1 + inset_x, inset_y + 2, 6, 4, BLACK,
      //                 1, overlay_buf, overlay_buf_pitch);
    }
    enabled >>= 1;
  }
}

// Show drive led
void ui_display_drive_led(int drive, unsigned int pwm1, unsigned int pwm2) {
  if (!overlay_buf)
    return;

  overlay_activate();

  if (!overlay_enabled) return;

  for (int i = 0; i < 2; i++) {
    unsigned int pwm = i == 0 ? pwm1 : pwm2;
    int led_color = drive_led_types[drive] & (1 << i);
    int led;
    if (led_color) {
      if (pwm < 333)
        led = BLACK;
      else if (pwm < 666)
        led = GREEN;
      else
        led = LIGHT_GREEN;
    } else {
      if (pwm < 333)
        led = BLACK;
      else if (pwm < 666)
        led = RED;
      else
        led = LIGHT_RED;
    }

    // draw only 4 pixels in height and 6 wide (centered in cell)
    ui_draw_rect_buf(drive_x[drive] + 8 * i + inset_x, inset_y + 2, // x,y
                     6, 4, led, 1,                    // w,h,color,fill
                     overlay_buf, overlay_buf_pitch); // dest
  }
}

// Show tape counter text
void ui_display_tape_counter(int counter) {
  if (!overlay_buf)
    return;

  if (counter != tape_counter) {
    overlay_activate();

    if (!overlay_enabled) return;

    char tmp[16];
    sprintf(tmp, "%03d", counter % 1000);
    ui_draw_rect_buf(tape_x + inset_x, inset_y, 8 * 3, 8, bg_color, 1,
                     overlay_buf, overlay_buf_pitch);
    ui_draw_text_buf(tmp, tape_x + inset_x, inset_y, fg_color, overlay_buf,
                     overlay_buf_pitch);
    tape_counter = counter;
  }
}

// Show tape control text
void ui_display_tape_control_status(int control) {
  if (!overlay_buf)
    return;

  overlay_activate();

  if (!overlay_enabled) return;

  ui_draw_rect_buf(tape_controls_x + inset_x, inset_y, 8 * 3, 8, bg_color, 1,
                   overlay_buf, overlay_buf_pitch);
  const char *txt;
  int col = fg_color;
  switch (control) {
  case DATASETTE_CONTROL_STOP:
    txt = "STP";
    break;
  case DATASETTE_CONTROL_START:
    col = GREEN;
    txt = "PLY";
    break;
  case DATASETTE_CONTROL_FORWARD:
    txt = "FWD";
    break;
  case DATASETTE_CONTROL_REWIND:
    txt = "REW";
    break;
  case DATASETTE_CONTROL_RECORD:
    col = RED;
    txt = "REC";
    break;
  default:
    txt = "";
    break;
  }

  ui_draw_text_buf(txt, tape_controls_x + inset_x, inset_y, col, overlay_buf,
                   overlay_buf_pitch);
}

// Draw tape motor status light
void ui_display_tape_motor_status(int motor) {
  if (!overlay_buf)
    return;

  overlay_activate();

  if (!overlay_enabled) return;

  int led = motor ? RED : bg_color;
  ui_draw_rect_buf(tape_motor_x + inset_x, inset_y + 2, 6, 4, led,
                   1, // w,h,color,fill
                   overlay_buf, overlay_buf_pitch);
}

void overlay_warp_changed(int warp) {
  if (!overlay_buf)
    return;

  overlay_activate();

  if (!overlay_enabled) return;

  ui_draw_rect_buf(warp_x + inset_x, inset_y, 8, 8, bg_color, 1, overlay_buf,
                   overlay_buf_pitch);
  ui_draw_text_buf(warp ? "!" : "-", warp_x + inset_x, inset_y, fg_color,
                   overlay_buf, overlay_buf_pitch);
}

void overlay_joyswap_changed(int swap) {
  if (!overlay_buf)
    return;

  overlay_activate();

  if (!overlay_enabled) return;

  ui_draw_rect_buf(joyswap_x + inset_x, inset_y, 8 * 2, 8, bg_color, 1,
                   overlay_buf, overlay_buf_pitch);
  ui_draw_text_buf(swap ? "21" : "12", joyswap_x + inset_x, inset_y, fg_color,
                   overlay_buf, overlay_buf_pitch);
}

// Checks whether a showing overlay due to activity should no longer be showing
void overlay_check(void) {
  // Rollover safe way of checking duration
  if (overlay_enabled && circle_get_ticks() - overlay_start >= overlay_delay) {
      overlay_dismiss();
  }
}

void overlay_dismiss(void) {
  if (!overlay_always()) {
     overlay_enabled = 0;
  }
}

void overlay_force_enabled(void) {
  overlay_enabled = 1;
}

void overlay_change_padding(int padding) {
  circle_hide_fb2(FB_LAYER_STATUS);
  overlay_showing = 0;
  circle_set_valign_fb2(FB_LAYER_STATUS, 1 /* BOTTOM */, padding);
}

void overlay_40_80_columns_changed(int value) {
  if (!overlay_buf)
    return;

  overlay_activate();

  if (!overlay_enabled) return;

  ui_draw_rect_buf(columns_x + inset_x, inset_y, 8 * 2, 8, bg_color, 1,
                   overlay_buf, overlay_buf_pitch);
  ui_draw_text_buf(value ? "40" : "80", columns_x + inset_x, inset_y, fg_color,
                   overlay_buf, overlay_buf_pitch);
}
