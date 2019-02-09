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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "ui.h"
#include "menu.h"
#include "joy.h"

extern int usb_pref_0;
extern int usb_pref_1;
extern int usb_x_axis_0;
extern int usb_y_axis_0;
extern int usb_x_axis_1;
extern int usb_y_axis_1;

struct menu_item* raw_buttons_item;
struct menu_item* raw_hats_item[6];
struct menu_item* raw_axes_item[16];

// Set to one when we are listening for raw usb values for config
int want_raw_usb = 0;
int want_raw_usb_device = 0;

static void raw_popped(struct menu_item* item) {
   // This is our raw monitor being popped.
   // Turn off raw monitoring.
   want_raw_usb = 0;
}

static void show_usb_monitor(int device) {
  int i;
  char scratch[16];

  want_raw_usb = 1;
  want_raw_usb_device = device;

  struct menu_item* root = ui_push_menu();
  // We need to get notified of pop to turn off monitoring
  root->on_value_changed = raw_popped;

  raw_buttons_item = ui_menu_add_button_with_value(MENU_TEXT, root, "Button", 0, "", "");
  for (i=0;i<joy_num_hats[device];i++) {
     sprintf (scratch,"Hat %d",i);
     raw_hats_item[i] = ui_menu_add_button_with_value(MENU_TEXT, root, scratch, 0, "", "");
  }

  for (i=0;i<joy_num_axes[device];i++) {
     sprintf (scratch,"Axis %d",i);
     raw_axes_item[i] = ui_menu_add_button_with_value(MENU_TEXT, root, scratch, 0, "", "");
  }
}

static void menu_usb_value_changed(struct menu_item* item) {
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
      default:
         break;
   }
}

void build_usb_menu(int dev, struct menu_item* root) {
  struct menu_item* usb_pref_item;
  struct menu_item* x_axis_item;
  struct menu_item* y_axis_item;
  struct menu_item* tmp_item;
  char desc[40];
  char scratch[32];

  if (dev == 0) {
      strcpy (desc, "USB 1:");
      if (joy_num_pads > 0) {
         strcat (desc, "DETECTED ");
         sprintf (scratch,"%d hats, %d axes", joy_num_hats[0], joy_num_axes[0]);
         strcat (desc, scratch);
      } else {
         strcat (desc, "NOT DETECTED");
      }
      ui_menu_add_button(MENU_TEXT, root, desc);
      ui_menu_add_divider(root);
      usb_pref_item = ui_menu_add_multiple_choice(MENU_USB_0_PREF, root, "USB 1 Uses");
      usb_pref_item->value = usb_pref_0;

      x_axis_item = ui_menu_add_range(MENU_USB_0_X_AXIS, root, "USB 1 Analog X #", 0, 12, 1, usb_x_axis_0);
      y_axis_item = ui_menu_add_range(MENU_USB_0_Y_AXIS, root, "USB 1 Analog Y #", 0, 12, 1, usb_y_axis_0);
      tmp_item = ui_menu_add_button(MENU_USB_0_WATCH_RAW, root, "Monitor raw USB 1 data...");
      tmp_item->on_value_changed = menu_usb_value_changed;
  } else {
      strcpy (desc, "USB 2:");
      if (joy_num_pads > 1) {
         strcat (desc, "DETECTED ");
         sprintf (scratch,"%d hats, %d axes", joy_num_hats[1], joy_num_axes[1]);
         strcat (desc, scratch);
      } else {
         strcat (desc, "NOT DETECTED");
      }
      ui_menu_add_button(MENU_TEXT, root, desc);
      ui_menu_add_divider(root);
      usb_pref_item = ui_menu_add_multiple_choice(MENU_USB_1_PREF, root, "USB 2 Uses");
      usb_pref_item->value = usb_pref_1;

      x_axis_item = ui_menu_add_range(MENU_USB_1_X_AXIS, root, "USB 2 Analog X #", 0, 12, 1, usb_x_axis_1);
      y_axis_item = ui_menu_add_range(MENU_USB_1_Y_AXIS, root, "USB 2 Analog Y #", 0, 12, 1, usb_y_axis_1);
      tmp_item = ui_menu_add_button(MENU_USB_1_WATCH_RAW, root, "Monitor raw USB 2 data...");
      tmp_item->on_value_changed = menu_usb_value_changed;
  }

  usb_pref_item->num_choices = 2;
  strcpy (usb_pref_item->choices[0], "Analog Stick");
  strcpy (usb_pref_item->choices[1], "First Hat");

  usb_pref_item->on_value_changed = menu_usb_value_changed;
  x_axis_item->on_value_changed = menu_usb_value_changed;
  y_axis_item->on_value_changed = menu_usb_value_changed;
}

int menu_wants_raw_usb(void) {
   return want_raw_usb;
}

void menu_raw_usb(int device, unsigned buttons, const int hats[6], const int axes[16]) {
  // Don't do too much here. This is from an isr.  Just set values for paint to update.
  int i;
  if (device == want_raw_usb_device) {
     raw_buttons_item->value = buttons;
     for (i=0;i<joy_num_hats[device];i++) {
        raw_hats_item[i]->value = hats[i];
     }
     for (i=0;i<joy_num_axes[device];i++) {
        raw_axes_item[i]->value = axes[i];
     }
  }
}
