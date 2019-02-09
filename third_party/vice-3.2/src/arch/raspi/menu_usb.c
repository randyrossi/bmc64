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
#include "menu.h"
#include "joy.h"

extern int usb_pref_0;
extern int usb_pref_1;
extern int usb_x_axis_0;
extern int usb_y_axis_0;
extern int usb_x_axis_1;
extern int usb_y_axis_1;

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
      default:
         break;
   }
}

void build_usb_menu(int dev, struct menu_item* root) {
  struct menu_item* usb_pref_item;
  struct menu_item* x_axis_item;
  struct menu_item* y_axis_item;
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
  }

  usb_pref_item->num_choices = 2;
  strcpy (usb_pref_item->choices[0], "Analog Stick");
  strcpy (usb_pref_item->choices[1], "First Hat");

  usb_pref_item->on_value_changed = menu_usb_value_changed;
  x_axis_item->on_value_changed = menu_usb_value_changed;
  y_axis_item->on_value_changed = menu_usb_value_changed;
}
