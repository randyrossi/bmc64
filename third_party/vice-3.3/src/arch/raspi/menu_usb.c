/*
 * menu_usb.c
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

#include "menu_usb.h"

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "joy.h"
#include "machine.h"
#include "menu.h"
#include "ui.h"

extern int usb_pref_0;
extern int usb_pref_1;
extern int usb_x_axis_0;
extern int usb_y_axis_0;
extern int usb_x_axis_1;
extern int usb_y_axis_1;
extern float usb_x_thresh_0;
extern float usb_y_thresh_0;
extern float usb_x_thresh_1;
extern float usb_y_thresh_1;
extern int usb_0_button_assignments[16];
extern int usb_1_button_assignments[16];
extern int usb_0_button_bits[16];
extern int usb_1_button_bits[16];

extern volatile int ui_activated;

struct menu_item *raw_buttons_item;
struct menu_item *raw_hats_item[6];
struct menu_item *raw_axes_item[16];

// For pot x y as buttons
struct menu_item *potx_high_item;
struct menu_item *potx_low_item;
struct menu_item *poty_high_item;
struct menu_item *poty_low_item;

extern int pot_x_high_value;
extern int pot_x_low_value;
extern int pot_y_high_value;
extern int pot_y_low_value;

// Set to one when we are listening for raw usb values for config
int want_raw_usb = 0;
int want_raw_usb_device = 0;

// Conver this value to log2(b) for display purposes
// Shows user which button index is being pressed on raw screen
static int map_button_value(int b) {
  if (b == 0)
    return -1;
  return log2(b);
}

static void raw_popped(struct menu_item *item) {
  // This is our raw monitor being popped.
  // Turn off raw monitoring.
  want_raw_usb = 0;
}

static void show_usb_monitor(int device) {
  int i;
  char scratch[16];

  want_raw_usb = 1;
  want_raw_usb_device = device;

  struct menu_item *root = ui_push_menu(-1, -1);
  // We need to get notified of pop to turn off monitoring
  root->on_value_changed = raw_popped;

  raw_buttons_item =
      ui_menu_add_button_with_value(MENU_TEXT, root, "Button #", 0, "", "");
  // We want to show button index so have to show log base 2 of value
  raw_buttons_item->map_value_func = map_button_value;

  for (i = 0; i < joy_num_hats[device]; i++) {
    sprintf(scratch, "Hat %d", i);
    raw_hats_item[i] =
        ui_menu_add_button_with_value(MENU_TEXT, root, scratch, 0, "", "");
  }

  for (i = 0; i < joy_num_axes[device]; i++) {
    sprintf(scratch, "Axis %d", i);
    raw_axes_item[i] =
        ui_menu_add_button_with_value(MENU_TEXT, root, scratch, 0, "", "");
  }
}

static void menu_usb_value_changed(struct menu_item *item) {
  switch (item->id) {
  case MENU_USB_0_PREF:
    usb_pref_0 = item->value;
    break;
  case MENU_USB_0_X_AXIS:
    usb_x_axis_0 = item->value;
    break;
  case MENU_USB_0_Y_AXIS:
    usb_y_axis_0 = item->value;
    break;
  case MENU_USB_1_PREF:
    usb_pref_1 = item->value;
    break;
  case MENU_USB_1_X_AXIS:
    usb_x_axis_1 = item->value;
    break;
  case MENU_USB_1_Y_AXIS:
    usb_y_axis_1 = item->value;
    break;
  case MENU_USB_0_WATCH_RAW:
    show_usb_monitor(0);
    break;
  case MENU_USB_1_WATCH_RAW:
    show_usb_monitor(1);
    break;
  case MENU_USB_0_BTN_ASSIGN:
    usb_0_button_assignments[item->sub_id] = item->value;
    break;
  case MENU_USB_1_BTN_ASSIGN:
    usb_1_button_assignments[item->sub_id] = item->value;
    break;
  case MENU_POTX_HIGH:
    pot_x_high_value = item->value;
    break;
  case MENU_POTX_LOW:
    pot_x_low_value = item->value;
    break;
  case MENU_POTY_HIGH:
    pot_y_high_value = item->value;
    break;
  case MENU_POTY_LOW:
    pot_y_low_value = item->value;
    break;
  case MENU_USB_0_X_THRESH:
    usb_x_thresh_0 = ((float)item->value) / 100.0f;
    break;
  case MENU_USB_0_Y_THRESH:
    usb_y_thresh_0 = ((float)item->value) / 100.0f;
    break;
  case MENU_USB_1_X_THRESH:
    usb_x_thresh_1 = ((float)item->value) / 100.0f;
    break;
  case MENU_USB_1_Y_THRESH:
    usb_y_thresh_1 = ((float)item->value) / 100.0f;
    break;
  default:
    break;
  }
}

static void add_button_choices(struct menu_item *tmp_item) {
  tmp_item->num_choices = NUM_BUTTON_ASSIGNMENTS;
  strcpy(tmp_item->choices[BTN_ASSIGN_UNDEF], "None");
  strcpy(tmp_item->choices[BTN_ASSIGN_FIRE], "Fire");
  strcpy(tmp_item->choices[BTN_ASSIGN_MENU], "Menu");
  strcpy(tmp_item->choices[BTN_ASSIGN_WARP], "Warp");
  strcpy(tmp_item->choices[BTN_ASSIGN_STATUS_TOGGLE], "Status Toggle");
  strcpy(tmp_item->choices[BTN_ASSIGN_SWAP_PORTS], "Swap Ports");

  // These are only available for USB buttons, not as hotkeys
  strcpy(tmp_item->choices[BTN_ASSIGN_UP], "Up");
  strcpy(tmp_item->choices[BTN_ASSIGN_DOWN], "Down");
  strcpy(tmp_item->choices[BTN_ASSIGN_LEFT], "Left");
  strcpy(tmp_item->choices[BTN_ASSIGN_RIGHT], "Right");
  strcpy(tmp_item->choices[BTN_ASSIGN_POTX], "POT X");
  strcpy(tmp_item->choices[BTN_ASSIGN_POTY], "POT Y");

  // Back to avail for all
  strcpy(tmp_item->choices[BTN_ASSIGN_TAPE_MENU], "Tape OSD");
  strcpy(tmp_item->choices[BTN_ASSIGN_CART_MENU], "Cart OSD");
  strcpy(tmp_item->choices[BTN_ASSIGN_CART_FREEZE], "Cart Freeze");
  strcpy(tmp_item->choices[BTN_ASSIGN_RESET_HARD], "Hard Reset");
  strcpy(tmp_item->choices[BTN_ASSIGN_RESET_SOFT], "Soft Reset");
  strcpy(tmp_item->choices[BTN_ASSIGN_RUN_STOP_BACK], "Menu Back/RunStop");

  if (machine_class == VICE_MACHINE_VIC20) {
    tmp_item->choice_disabled[BTN_ASSIGN_SWAP_PORTS] = 1;
    tmp_item->choice_disabled[BTN_ASSIGN_CART_FREEZE] = 1;
  }
}

void build_usb_menu(int dev, struct menu_item *root) {
  struct menu_item *usb_pref_item;
  struct menu_item *x_axis_item;
  struct menu_item *y_axis_item;
  struct menu_item *x_thresh_item;
  struct menu_item *y_thresh_item;
  struct menu_item *tmp_item;
  struct menu_item *usb_btn_item[16];
  char desc[40];
  char scratch[32];
  int i;
  int j;

  if (dev == 0) {
    strcpy(desc, "USB 1:");
    if (joy_num_pads > 0) {
      strcat(desc, "DETECTED ");
      sprintf(scratch, "%d hats, %d axes", joy_num_hats[0], joy_num_axes[0]);
      strcat(desc, scratch);
    } else {
      strcat(desc, "NOT DETECTED");
    }
    ui_menu_add_button(MENU_TEXT, root, desc);
    ui_menu_add_divider(root);
    usb_pref_item =
        ui_menu_add_multiple_choice(MENU_USB_0_PREF, root, "USB 1 Uses");
    usb_pref_item->value = usb_pref_0;

    x_axis_item = ui_menu_add_range(MENU_USB_0_X_AXIS, root, "USB 1 Analog X #",
                                    0, 12, 1, usb_x_axis_0);
    y_axis_item = ui_menu_add_range(MENU_USB_0_Y_AXIS, root, "USB 1 Analog Y #",
                                    0, 12, 1, usb_y_axis_0);
    x_thresh_item = ui_menu_add_range(MENU_USB_0_X_THRESH, root,
                                      "USB 1 Analog X Threshold %", 10, 90, 1,
                                      (int)(usb_x_thresh_0 * 100.0f));
    y_thresh_item = ui_menu_add_range(MENU_USB_0_Y_THRESH, root,
                                      "USB 1 Analog Y Threshold %", 10, 90, 1,
                                      (int)(usb_y_thresh_0 * 100.0f));

    tmp_item = ui_menu_add_button(MENU_USB_0_WATCH_RAW, root,
                                  "Monitor raw USB 1 data...");
    tmp_item->on_value_changed = menu_usb_value_changed;

    for (i = 0; i < joy_num_buttons[0]; i++) {
      sprintf(scratch, "Button %d Function", i);
      tmp_item =
          ui_menu_add_multiple_choice(MENU_USB_0_BTN_ASSIGN, root, scratch);
      add_button_choices(tmp_item);
      tmp_item->value = usb_0_button_assignments[i];
      tmp_item->on_value_changed = menu_usb_value_changed;
      tmp_item->sub_id = i;
    }
  } else {
    strcpy(desc, "USB 2:");
    if (joy_num_pads > 1) {
      strcat(desc, "DETECTED ");
      sprintf(scratch, "%d hats, %d axes", joy_num_hats[1], joy_num_axes[1]);
      strcat(desc, scratch);
    } else {
      strcat(desc, "NOT DETECTED");
    }
    ui_menu_add_button(MENU_TEXT, root, desc);
    ui_menu_add_divider(root);
    usb_pref_item =
        ui_menu_add_multiple_choice(MENU_USB_1_PREF, root, "USB 2 Uses");
    usb_pref_item->value = usb_pref_1;

    x_axis_item = ui_menu_add_range(MENU_USB_1_X_AXIS, root, "USB 2 Analog X #",
                                    0, 12, 1, usb_x_axis_1);
    y_axis_item = ui_menu_add_range(MENU_USB_1_Y_AXIS, root, "USB 2 Analog Y #",
                                    0, 12, 1, usb_y_axis_1);
    x_thresh_item = ui_menu_add_range(MENU_USB_1_X_THRESH, root,
                                      "USB 2 Analog X Threshold %", 10, 90, 1,
                                      (int)(usb_x_thresh_1 * 100.0f));
    y_thresh_item = ui_menu_add_range(MENU_USB_1_Y_THRESH, root,
                                      "USB 2 Analog Y Threshold %", 10, 90, 1,
                                      (int)(usb_y_thresh_1 * 100.0f));
    tmp_item = ui_menu_add_button(MENU_USB_1_WATCH_RAW, root,
                                  "Monitor raw USB 2 data...");
    tmp_item->on_value_changed = menu_usb_value_changed;

    for (i = 0; i < joy_num_buttons[1]; i++) {
      sprintf(scratch, "Button %d Function", i);
      tmp_item =
          ui_menu_add_multiple_choice(MENU_USB_1_BTN_ASSIGN, root, scratch);
      tmp_item->num_choices = NUM_BUTTON_ASSIGNMENTS;
      add_button_choices(tmp_item);
      tmp_item->value = usb_1_button_assignments[i];
      tmp_item->on_value_changed = menu_usb_value_changed;
      tmp_item->sub_id = i;
    }
  }

  ui_menu_add_divider(root);
  potx_high_item = ui_menu_add_range(MENU_POTX_HIGH, root, "POT X Up Value", 0,
                                     255, 1, pot_x_high_value);
  potx_low_item = ui_menu_add_range(MENU_POTX_LOW, root, "POT X Down Value", 0,
                                    255, 1, pot_x_low_value);
  poty_high_item = ui_menu_add_range(MENU_POTY_HIGH, root, "POT Y Up Value", 0,
                                     255, 1, pot_y_high_value);
  poty_low_item = ui_menu_add_range(MENU_POTY_LOW, root, "POT Y Down Value", 0,
                                    255, 1, pot_y_low_value);

  usb_pref_item->num_choices = 2;
  strcpy(usb_pref_item->choices[0], "Analog Stick");
  strcpy(usb_pref_item->choices[1], "First Hat");

  usb_pref_item->on_value_changed = menu_usb_value_changed;
  x_axis_item->on_value_changed = menu_usb_value_changed;
  y_axis_item->on_value_changed = menu_usb_value_changed;
  x_thresh_item->on_value_changed = menu_usb_value_changed;
  y_thresh_item->on_value_changed = menu_usb_value_changed;
}

int menu_wants_raw_usb(void) { return ui_activated && want_raw_usb; }

void menu_raw_usb(int device, unsigned buttons, const int hats[6],
                  const int axes[16]) {
  // Don't do too much here. This is from an isr.  Just set values for paint to
  // update.
  int i;
  if (device == want_raw_usb_device) {
    raw_buttons_item->value = buttons;
    for (i = 0; i < joy_num_hats[device]; i++) {
      raw_hats_item[i]->value = hats[i];
    }
    for (i = 0; i < joy_num_axes[device]; i++) {
      raw_axes_item[i]->value = axes[i];
    }
  }
}

// Returns the first function found for the button if
// it's bit is set (so only returns something on the
// down event). Expects only one button to be on.
// That is, only the function will be returned for
// the first bit on it finds.
int circle_button_function(int dev, unsigned b) {
  int i;
  if (dev == 0) {
    for (i = 0; i < joy_num_buttons[dev]; i++) {
      if (b & usb_0_button_bits[i]) {
        return usb_0_button_assignments[i];
      }
    }
    return BTN_ASSIGN_UNDEF;
  }

  for (i = 0; i < joy_num_buttons[dev]; i++) {
    if (b & usb_1_button_bits[i]) {
      return usb_1_button_assignments[i];
    }
  }
  return BTN_ASSIGN_UNDEF;
}

int circle_add_button_values(int dev, unsigned b) {
  int i;
  int value = (pot_x_high_value << 5) | (pot_y_high_value << 13);
  if (dev == 0) {
    for (i = 0; i < joy_num_buttons[dev]; i++) {
      if (b & usb_0_button_bits[i]) {
        int j = usb_0_button_assignments[i];
        switch (j) {
        case BTN_ASSIGN_FIRE:
          value |= 0x10;
          break;
        case BTN_ASSIGN_POTX:
          value &= ~(pot_x_high_value << 5);
          value |= (pot_x_low_value << 5);
          break;
        case BTN_ASSIGN_POTY:
          value &= ~(pot_y_high_value << 13);
          value |= (pot_y_low_value << 13);
          break;
        case BTN_ASSIGN_UP:
          value |= 0x1;
          break;
        case BTN_ASSIGN_DOWN:
          value |= 0x2;
          break;
        case BTN_ASSIGN_LEFT:
          value |= 0x4;
          break;
        case BTN_ASSIGN_RIGHT:
          value |= 0x8;
          break;
        }
      }
    }
    return value;
  }

  // TODO: Dedupe this later...
  for (i = 0; i < joy_num_buttons[dev]; i++) {
    if (b & usb_1_button_bits[i]) {
      int j = usb_1_button_assignments[i];
      switch (j) {
      case BTN_ASSIGN_FIRE:
        value |= 0x10;
        break;
      case BTN_ASSIGN_POTX:
        value &= ~(pot_x_high_value << 5);
        value |= (pot_x_low_value << 5);
        break;
      case BTN_ASSIGN_POTY:
        value &= ~(pot_y_high_value << 13);
        value |= (pot_y_low_value << 13);
        break;
      case BTN_ASSIGN_UP:
        value |= 0x1;
        break;
      case BTN_ASSIGN_DOWN:
        value |= 0x2;
        break;
      case BTN_ASSIGN_LEFT:
        value |= 0x4;
        break;
      case BTN_ASSIGN_RIGHT:
        value |= 0x8;
        break;
      }
    }
  }
  return value;
}
