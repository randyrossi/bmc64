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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datasette.h"
#include "drive.h"
#include "machine.h"
#include "menu.h"
#include "resources.h"
#include "ui.h"
#include "uiapi.h"
#include "virtual_keyboard.h"
#include "circle.h"
#include "keycodes.h"

#define ARGB(a,r,g,b) ((uint32_t)((uint8_t)(a)<<24 | (uint8_t)(r)<<16 | (uint8_t)(g)<<8 | (uint8_t)(b)))

#define TICKS_PER_SECOND 1000000L

// These were added when we switch to hi-res overlay
// Existing coordinated are scaled up 4x so we get
// large letters for the status bar but get higher res
// for keyboard overlay.
#define SCALE_XY 2

// Font advance both horizontal and vertical
#define FONT_ADVANCE (8*SCALE_XY)

// Status bar height is the font height + 2 pixels padding above and
// below then scaled.
#define STATUS_BAR_HEIGHT (FONT_ADVANCE + 2 * SCALE_XY)

static int drive_x[4];
static int tape_x;
static int tape_controls_x;
static int tape_motor_x;
static int warp_x;
static int joyswap_x;
static int columns_x;

static int drive_led_types[DRIVE_NUM];
static unsigned int current_drive_leds[DRIVE_NUM][2];
static int tape_counter = -1;

static unsigned long statusbar_delay = 0;
static unsigned long statusbar_start = 0;

static int inset_x;
static int inset_y;

uint8_t *overlay_buf;
static int overlay_buf_pitch;

#define BG_COLOR 0
#define FG_COLOR 1
#define BLACK_COLOR 2
#define RED_COLOR 3
#define GREEN_COLOR 4
#define LIGHT_RED_COLOR 5
#define LIGHT_GREEN_COLOR 6
#define TRANSPARENT_COLOR 7

// Defines the first 8 overlay palette entries
static uint32_t overlay_palette[8] = {
  ARGB(0xFF, 0x6c, 0x5e, 0xb5), // bg
  ARGB(0xFF, 0xFF, 0xFF, 0xFF), // fg
  ARGB(0xFF, 0x00, 0x00, 0x00), // black
  ARGB(0xFF, 0x68, 0x37, 0x2b), // red
  ARGB(0xFF, 0x58, 0x8d, 0x43), // green
  ARGB(0xFF, 0x9a, 0x67, 0x59), // light red
  ARGB(0xFF, 0x9a, 0xd2, 0x84), // light green
  ARGB(0x00, 0x00, 0x00, 0x00), // transparent
};

// The index into the virtual keyboard of the cursor
int vkbd_cursor;

int vkbd_enabled;
int vkbd_showing;
int vkbd_press;

int statusbar_enabled;
int statusbar_showing;

static int last_c480_80_state;
static char *template;

int overlay_dirty;

static void draw_statusbar() {
  // Now draw the bg for the status bar
  ui_draw_rect_buf(0, OVERLAY_HEIGHT - STATUS_BAR_HEIGHT,
                   OVERLAY_WIDTH, STATUS_BAR_HEIGHT,
                   BG_COLOR, 1, overlay_buf, overlay_buf_pitch);


  ui_draw_text_buf(template, inset_x, inset_y, FG_COLOR, overlay_buf,
                   overlay_buf_pitch, SCALE_XY);

  ui_draw_text_buf("-", warp_x + inset_x, inset_y, FG_COLOR, overlay_buf,
                   overlay_buf_pitch, SCALE_XY);

  if (machine_class == VICE_MACHINE_C128) {
     overlay_40_80_columns_changed(last_c480_80_state);
  }
  overlay_dirty = 1;
}

static void clear_statusbar() {
  ui_draw_rect_buf(0, OVERLAY_HEIGHT - STATUS_BAR_HEIGHT,
                   OVERLAY_WIDTH, STATUS_BAR_HEIGHT,
                   TRANSPARENT_COLOR, 1, overlay_buf, overlay_buf_pitch);
  overlay_dirty = 1;
}

// Create a new overlay buffer
uint8_t *overlay_init(int padding, int c40_80_state) {
  last_c480_80_state = c40_80_state;
  circle_alloc_fbl(FB_LAYER_STATUS,
                   &overlay_buf, OVERLAY_WIDTH, OVERLAY_HEIGHT,
                   &overlay_buf_pitch);
  // Use negative aspect here so our overlay is stretched to the full
  // horizontal resolution rather than vertical.
  circle_set_aspect_fbl(FB_LAYER_STATUS,
      -(double)OVERLAY_WIDTH/(double)OVERLAY_HEIGHT);
  // We want our status bar to show up at the bottom of the screen with
  // padding set by user.
  circle_set_valign_fbl(FB_LAYER_STATUS, 1 /* BOTTOM */, padding);

  // Start with complete transparent overlay
  memset(overlay_buf, TRANSPARENT_COLOR, overlay_buf_pitch * OVERLAY_HEIGHT);

  // Figure out inset that will center our template.
  if (machine_class == VICE_MACHINE_VIC20) {
     template = "8:  9:  10:  11:  T:000 STP   W:  J:1";
  } else if (machine_class == VICE_MACHINE_C128) {
     template = "8:  9:  10:  11:  T:000 STP   W:  J:12 C:  ";
  } else {
     template = "8:  9:  10:  11:  T:000 STP   W:  J:12";
  }

  inset_x = OVERLAY_WIDTH / 2 - (strlen(template) * FONT_ADVANCE) / 2;
  inset_y = OVERLAY_HEIGHT - 1 - STATUS_BAR_HEIGHT + 1*SCALE_XY;

  // Positions relative to start of text (before inset)
  drive_x[0] = 2 * FONT_ADVANCE;
  drive_x[1] = 6 * FONT_ADVANCE;
  drive_x[2] = 11 * FONT_ADVANCE;
  drive_x[3] = 16 * FONT_ADVANCE;
  tape_x = 20 * FONT_ADVANCE;
  tape_controls_x = 24 * FONT_ADVANCE;
  tape_motor_x = 28 * FONT_ADVANCE;
  warp_x = 32 * FONT_ADVANCE;
  joyswap_x = 36 * FONT_ADVANCE;
  columns_x = 41 * FONT_ADVANCE;

  // Setup colors for this layer
  for (int p = 0; p < 8; p++) {
     circle_set_palette32_fbl(FB_LAYER_STATUS, p, overlay_palette[p]);
  }
  circle_update_palette_fbl(FB_LAYER_STATUS);

  overlay_dirty = 1;
  return overlay_buf;
}

void overlay_statusbar_enable(void) {
  statusbar_enabled = 1;
  draw_statusbar();
}

void overlay_statusbar_disable(void) {
  statusbar_enabled = 0;
  clear_statusbar();
}

// Some activity means statusbar should show (if menu option set)
static void statusbar_triggered_by_activity() {
  if (!overlay_buf) return;

  statusbar_start = circle_get_ticks();
  statusbar_delay = 5 * TICKS_PER_SECOND;
  if (!statusbar_never()) {
     overlay_statusbar_enable();
  }
}

// Called by VICE to enable a drive status lights
void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color) {
  if (!overlay_buf)
    return;

  statusbar_triggered_by_activity();

  if (!statusbar_enabled) return;

  int i, enabled = state;

  for (i = 0; i < DRIVE_NUM; ++i) {
    if (overlay_buf) {
      ui_draw_rect_buf(drive_x[i] + FONT_ADVANCE * 0 + inset_x, inset_y + 2, 6*SCALE_XY, 4*SCALE_XY,
                       BG_COLOR, 1, overlay_buf, overlay_buf_pitch);
      // The second LED never seems to go on.  Removing it.
      //ui_draw_rect_buf(drive_x[i] + FONT_ADVANCE * 1 + inset_x, inset_y + 2, 6*SCALE_XY, 4*SCALE_XY,
      //                 BG_COLOR, 1, overlay_buf, overlay_buf_pitch);
    }
    if (enabled & 1) {
      drive_led_types[i] = drive_led_color[i];
      current_drive_leds[i][0] = 0;
      current_drive_leds[i][1] = 0;
      ui_draw_rect_buf(drive_x[i] + FONT_ADVANCE * 0 + inset_x, inset_y + 2*SCALE_XY, 6*SCALE_XY, 4*SCALE_XY, BLACK_COLOR,
                       1, overlay_buf, overlay_buf_pitch);
      // The second LED never seems to go on.  Removing it.
      //ui_draw_rect_buf(drive_x[i] + FONT_ADVANCE * 1 + inset_x, inset_y + 2*SCALE_XY, 6*SCALE_XY, 4*SCALE_XY, BLACK_COLOR,
      //                 1, overlay_buf, overlay_buf_pitch);
    }
    enabled >>= 1;
  }
  overlay_dirty = 1;
}

// Show drive led
void ui_display_drive_led(int drive, unsigned int pwm1, unsigned int pwm2) {
  if (!overlay_buf)
    return;

  statusbar_triggered_by_activity();

  if (!statusbar_enabled) return;

  // Was i < 2, disabled 2nd LED since it never seems to turn on.
  for (int i = 0; i < 1; i++) {
    unsigned int pwm = i == 0 ? pwm1 : pwm2;
    int led_color = drive_led_types[drive] & (1 << i);
    int led;
    if (led_color) {
      if (pwm < 333)
        led = BLACK_COLOR;
      else if (pwm < 666)
        led = GREEN_COLOR;
      else
        led = LIGHT_GREEN_COLOR;
    } else {
      if (pwm < 333)
        led = BLACK_COLOR;
      else if (pwm < 666)
        led = RED_COLOR;
      else
        led = LIGHT_RED_COLOR;
    }

    // draw only 4 pixels in height and 6 wide (centered in cell)
    ui_draw_rect_buf(drive_x[drive] + FONT_ADVANCE * i + inset_x, inset_y + 2 * SCALE_XY, // x,y
                     6 * SCALE_XY, 4 * SCALE_XY, led, 1,                    // w,h,color,fill
                     overlay_buf, overlay_buf_pitch); // dest
  }
  overlay_dirty = 1;
}

// Show tape counter text
void ui_display_tape_counter(int counter) {
  if (!overlay_buf)
    return;

  if (counter != tape_counter) {
    statusbar_triggered_by_activity();

    if (!statusbar_enabled) return;

    char tmp[16];
    sprintf(tmp, "%03d", counter % 1000);
    ui_draw_rect_buf(tape_x + inset_x, inset_y, FONT_ADVANCE * 3, FONT_ADVANCE, BG_COLOR, 1,
                     overlay_buf, overlay_buf_pitch);
    ui_draw_text_buf(tmp, tape_x + inset_x, inset_y, FG_COLOR, overlay_buf,
                     overlay_buf_pitch, SCALE_XY);
    tape_counter = counter;
    overlay_dirty = 1;
  }
}

// Show tape control text
void ui_display_tape_control_status(int control) {
  if (!overlay_buf)
    return;

  statusbar_triggered_by_activity();

  if (!statusbar_enabled) return;

  ui_draw_rect_buf(tape_controls_x + inset_x, inset_y, FONT_ADVANCE * 3, FONT_ADVANCE, BG_COLOR, 1,
                   overlay_buf, overlay_buf_pitch);
  const char *txt;
  int col = FG_COLOR;
  switch (control) {
  case DATASETTE_CONTROL_STOP:
    txt = "STP";
    break;
  case DATASETTE_CONTROL_START:
    col = GREEN_COLOR;
    txt = "PLY";
    break;
  case DATASETTE_CONTROL_FORWARD:
    txt = "FWD";
    break;
  case DATASETTE_CONTROL_REWIND:
    txt = "REW";
    break;
  case DATASETTE_CONTROL_RECORD:
    col = RED_COLOR;
    txt = "REC";
    break;
  default:
    txt = "";
    break;
  }

  ui_draw_text_buf(txt, tape_controls_x + inset_x, inset_y, col, overlay_buf,
                   overlay_buf_pitch, SCALE_XY);
  overlay_dirty = 1;
}

// Draw tape motor status light
void ui_display_tape_motor_status(int motor) {
  if (!overlay_buf)
    return;

  statusbar_triggered_by_activity();

  if (!statusbar_enabled) return;

  int led = motor ? RED_COLOR : BG_COLOR;
  ui_draw_rect_buf(tape_motor_x + inset_x, inset_y + 2 * SCALE_XY, 6 * SCALE_XY, 4 * SCALE_XY, led,
                   1, // w,h,color,fill
                   overlay_buf, overlay_buf_pitch);
  overlay_dirty = 1;
}

void overlay_warp_changed(int warp) {
  if (!overlay_buf)
    return;

  statusbar_triggered_by_activity();

  if (!statusbar_enabled) return;

  ui_draw_rect_buf(warp_x + inset_x, inset_y, FONT_ADVANCE, FONT_ADVANCE, BG_COLOR, 1, overlay_buf,
                   overlay_buf_pitch);
  ui_draw_text_buf(warp ? "!" : "-", warp_x + inset_x, inset_y, FG_COLOR,
                   overlay_buf, overlay_buf_pitch, SCALE_XY);
  overlay_dirty = 1;
}

void overlay_joyswap_changed(int swap) {
  if (!overlay_buf)
    return;

  statusbar_triggered_by_activity();

  if (!statusbar_enabled) return;

  ui_draw_rect_buf(joyswap_x + inset_x, inset_y, FONT_ADVANCE * 2, FONT_ADVANCE, BG_COLOR, 1,
                   overlay_buf, overlay_buf_pitch);
  ui_draw_text_buf(swap ? "21" : "12", joyswap_x + inset_x, inset_y, FG_COLOR,
                   overlay_buf, overlay_buf_pitch, SCALE_XY);
  overlay_dirty = 1;
}

// Checks whether a showing overlay due to activity should no longer be showing
void overlay_check(void) {
  // Rollover safe way of checking duration
  if (statusbar_enabled && circle_get_ticks() - statusbar_start >= statusbar_delay) {
      overlay_statusbar_dismiss();
  }
}

void overlay_statusbar_dismiss(void) {
  if (!statusbar_always()) {
     overlay_statusbar_disable();
  }
}

void overlay_change_padding(int padding) {
  circle_hide_fbl(FB_LAYER_STATUS);
  statusbar_showing = 0;
  vkbd_showing = 0;
  circle_set_valign_fbl(FB_LAYER_STATUS, 1 /* BOTTOM */, padding);
}

void overlay_40_80_columns_changed(int value) {
  if (!overlay_buf)
    return;

  statusbar_triggered_by_activity();

  if (!statusbar_enabled) return;

  ui_draw_rect_buf(columns_x + inset_x, inset_y, FONT_ADVANCE * 2, FONT_ADVANCE, BG_COLOR, 1,
                   overlay_buf, overlay_buf_pitch);
  ui_draw_text_buf(value ? "40" : "80", columns_x + inset_x, inset_y, FG_COLOR,
                   overlay_buf, overlay_buf_pitch, SCALE_XY);

  last_c480_80_state = value;
  overlay_dirty = 1;
}

static void overlay_clear_virtual_keyboard() {
  int cx = (OVERLAY_WIDTH - VKBD_WIDTH) / 2;
  int cy = (OVERLAY_HEIGHT - VKBD_HEIGHT) / 2;

  // Clear background for keyboard
  ui_draw_rect_buf(cx-1, cy-1, VKBD_WIDTH+2, VKBD_HEIGHT+2,
                   TRANSPARENT_COLOR, 1, overlay_buf, overlay_buf_pitch);

  overlay_dirty = 1;
}

static void overlay_draw_virtual_keyboard() {
  // Draw keys
  int cx = (OVERLAY_WIDTH - VKBD_WIDTH) / 2;
  int cy = (OVERLAY_HEIGHT - VKBD_HEIGHT) / 2;

  // Clear background for keyboard
  ui_draw_rect_buf(cx-1, cy-1, VKBD_WIDTH+2, VKBD_HEIGHT+2,
                   BG_COLOR, 1, overlay_buf, overlay_buf_pitch);

  for (int i=0; i < NUM_KEYS; i++) {
     // Show current key
     int color = (i == vkbd_cursor ? GREEN_COLOR : FG_COLOR);

     if (vkbd[i].state) {
        ui_draw_rect_buf(vkbd[i].x+cx, vkbd[i].y+cy, vkbd[i].w, vkbd[i].h,
                      GREEN_COLOR, 1 /* fill */, overlay_buf, overlay_buf_pitch);
     }
     ui_draw_rect_buf(vkbd[i].x+cx, vkbd[i].y+cy, vkbd[i].w, vkbd[i].h,
                      color, 0 /* fill */, overlay_buf, overlay_buf_pitch);

     int labelx = (vkbd[i].x+cx + vkbd[i].w / 2);
     int labely = (vkbd[i].y+cy + vkbd[i].h / 2);
     if (vkbd[i].code >= 0) {
        // Center our 2x scaled character
        labelx -= 8;
        labely -= 8;
        ui_draw_char_raw(vkbd[i].code, labelx, labely,
                      FG_COLOR, overlay_buf,
                      overlay_buf_pitch, 2);
     } else {
        
        switch (vkbd[i].code) {
          case VKBD_KEY_HOME:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("HOM", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_DEL:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("DEL", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_F1:
             labelx -= (8*2)/2;
             labely -= 4;
             ui_draw_text_buf("F1", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_F3:
             labelx -= (8*2)/2;
             labely -= 4;
             ui_draw_text_buf("F3", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_F5:
             labelx -= (8*2)/2;
             labely -= 4;
             ui_draw_text_buf("F5", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_F7:
             labelx -= (8*2)/2;
             labely -= 4;
             ui_draw_text_buf("F7", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_CNTRL:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("CTL", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_RESTORE:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("RES", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_RUNSTOP:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("RST", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_SHIFTLOCK:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("LCK", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_RETURN:
             labelx -= (8*2)/2;
             labely -= 4;
             ui_draw_text_buf("RET", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_COMMODORE:
             labelx -= (8*2)/2;
             labely -= 4;
             ui_draw_text_buf("C=", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_SHIFT:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("SHF", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;
          case VKBD_CURSDOWN:
          case VKBD_CURSRIGHT:
             labelx -= (8*3)/2;
             labely -= 4;
             ui_draw_text_buf("CRS", labelx, labely, FG_COLOR, overlay_buf, overlay_buf_pitch, 1);
             break;

          default:
             break;
        }
     }
  }
  overlay_dirty = 1;
}

void vkbd_enable() {
   vkbd_enabled = 1;
   overlay_draw_virtual_keyboard();
}

void vkbd_disable() {
   vkbd_enabled = 0;
   overlay_clear_virtual_keyboard();
}

void vkbd_nav_up(void) {
   vkbd_cursor = vkbd[vkbd_cursor].up;
   overlay_draw_virtual_keyboard();
}

void vkbd_nav_down(void) {
   vkbd_cursor = vkbd[vkbd_cursor].down;
   overlay_draw_virtual_keyboard();
}

void vkbd_nav_left(void) {
   vkbd_cursor = vkbd[vkbd_cursor].left;
   overlay_draw_virtual_keyboard();
}

void vkbd_nav_right(void) {
   vkbd_cursor = vkbd[vkbd_cursor].right;
   overlay_draw_virtual_keyboard();
}

void vkbd_nav_press(int pressed) {
   if (vkbd[vkbd_cursor].toggle) {
      // Only toggle on the press
      if (pressed) {
        vkbd[vkbd_cursor].state = 1 - vkbd[vkbd_cursor].state;
        circle_keyboard_set_latch_keyarr(vkbd[vkbd_cursor].col,
                                    vkbd[vkbd_cursor].row,
                                    vkbd[vkbd_cursor].state);
      }
   } else {
      // Handle restore special case
      if (vkbd[vkbd_cursor].row == 0 && vkbd[vkbd_cursor].col == -3) {
         if (pressed) {
            circle_key_pressed(KEYCODE_PageUp);
         } else {
            circle_key_released(KEYCODE_PageUp);
         }
      } else {
        circle_keyboard_set_latch_keyarr(vkbd[vkbd_cursor].col,
                                    vkbd[vkbd_cursor].row,
                                    pressed);
      }
      vkbd[vkbd_cursor].state = pressed;
   }
   vkbd_press = pressed;
   overlay_draw_virtual_keyboard();
}

