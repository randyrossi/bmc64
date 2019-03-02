/*
 * joy.c
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

#include "joy.h"

#include "ui.h"
#include "menu.h"
#include "kbd.h"

#include <stdio.h>
#include <string.h>
#include "joyport/joystick.h"

int joy_num_pads = 0;
int joy_num_buttons[2];
int joy_num_axes[2];
int joy_num_hats[2];

struct joydev_config joydevs[2];


int joy_key_up(unsigned int device, int key) {
     if (circle_ui_activated()) {
        // When the ui is showing, we want these
        // to turn into navigation for the ui.
        if (joydevs[device].device == JOYDEV_NUMS_1) {
          switch (key) {
            case KEYCODE_KP8:
               circle_ui_key_interrupt(KEYCODE_Up);
               return 1;
            case KEYCODE_KP2:
               circle_ui_key_interrupt(KEYCODE_Down);
               return 1;
            case KEYCODE_KP4:
               circle_ui_key_interrupt(KEYCODE_Left);
               return 1;
            case KEYCODE_KP6:
               circle_ui_key_interrupt(KEYCODE_Right);
               return 1;
            case KEYCODE_KP5:
               circle_ui_key_interrupt(KEYCODE_Return);
               return 1;
            default:
               return 0;
          }
        } else if (joydevs[device].device == JOYDEV_NUMS_2) {
          switch (key) {
            case KEYCODE_KP9:
               circle_ui_key_interrupt(KEYCODE_Up);
               return 1;
            case KEYCODE_KP3:
               circle_ui_key_interrupt(KEYCODE_Down);
               return 1;
            case KEYCODE_KP7:
               circle_ui_key_interrupt(KEYCODE_Left);
               return 1;
            case KEYCODE_KP1:
               circle_ui_key_interrupt(KEYCODE_Right);
               return 1;
            case KEYCODE_KP0:
               circle_ui_key_interrupt(KEYCODE_Return);
               return 1;
            default:
               return 0;
          }
        } else if (joydevs[device].device == JOYDEV_CURS) {
         switch (key) {
            case KEYCODE_Up:
               circle_ui_key_interrupt(KEYCODE_Up);
               return 1;
            case KEYCODE_Down:
               circle_ui_key_interrupt(KEYCODE_Down);
               return 1;
            case KEYCODE_Left:
               circle_ui_key_interrupt(KEYCODE_Left);
               return 1;
            case KEYCODE_Right:
               circle_ui_key_interrupt(KEYCODE_Right);
               return 1;
            case KEYCODE_Space:
               circle_ui_key_interrupt(KEYCODE_Return);
               return 1;
            default:
               return 0;
          }
        }
        return 0;
     }

     int port = joydevs[device].port;
     if (joydevs[device].device == JOYDEV_NUMS_1) {
         switch (key) {
            case KEYCODE_KP8:
               joystick_set_value_and(port, ~0x01);
               return 1;
            case KEYCODE_KP2:
               joystick_set_value_and(port, ~0x02);
               return 1;
            case KEYCODE_KP4:
               joystick_set_value_and(port, ~0x04);
               return 1;
            case KEYCODE_KP6:
               joystick_set_value_and(port, ~0x08);
               return 1;
            case KEYCODE_KP5:
               joystick_set_value_and(port, ~0x10);
               return 1;
            default:
               return 0;
         }
     } else if (joydevs[device].device == JOYDEV_NUMS_2) {
         switch (key) {
            case KEYCODE_KP9:
               joystick_set_value_and(port, ~0x01);
               return 1;
            case KEYCODE_KP3:
               joystick_set_value_and(port, ~0x02);
               return 1;
            case KEYCODE_KP7:
               joystick_set_value_and(port, ~0x04);
               return 1;
            case KEYCODE_KP1:
               joystick_set_value_and(port, ~0x08);
               return 1;
            case KEYCODE_KP0:
               joystick_set_value_and(port, ~0x10);
               return 1;
            default:
               return 0;
         }
     } else if (joydevs[device].device == JOYDEV_CURS) {
         switch (key) {
            case KEYCODE_Up:
               joystick_set_value_and(port, ~0x01);
               return 1;
            case KEYCODE_Down:
               joystick_set_value_and(port, ~0x02);
               return 1;
            case KEYCODE_Left:
               joystick_set_value_and(port, ~0x04);
               return 1;
            case KEYCODE_Right:
               joystick_set_value_and(port, ~0x08);
               return 1;
            case KEYCODE_Space:
               joystick_set_value_and(port, ~0x10);
               return 1;
            default:
               return 0;
         }
     } else {
        return 0;
     }
}

int joy_key_down(unsigned int device, int key) {
     if (circle_ui_activated()) {
        // When the ui is showing, we want these to
        // go nowhere.
        return 1;
     }

     int port = joydevs[device].port;
     if (joydevs[device].device == JOYDEV_NUMS_1) {
         switch (key) {
            case KEYCODE_KP8:
               joystick_set_value_or(port, 0x01);
               return 1;
            case KEYCODE_KP2:
               joystick_set_value_or(port, 0x02);
               return 1;
            case KEYCODE_KP4:
               joystick_set_value_or(port, 0x04);
               return 1;
            case KEYCODE_KP6:
               joystick_set_value_or(port, 0x08);
               return 1;
            case KEYCODE_KP5:
               joystick_set_value_or(port, 0x10);
               return 1;
            default:
               return 0;
         }
     } else if (joydevs[device].device == JOYDEV_NUMS_2) {
         switch (key) {
            case KEYCODE_KP9:
               joystick_set_value_or(port, 0x01);
               return 1;
            case KEYCODE_KP3:
               joystick_set_value_or(port, 0x02);
               return 1;
            case KEYCODE_KP7:
               joystick_set_value_or(port, 0x04);
               return 1;
            case KEYCODE_KP1:
               joystick_set_value_or(port, 0x08);
               return 1;
            case KEYCODE_KP0:
               joystick_set_value_or(port, 0x10);
               return 1;
            default:
               return 0;
         }
     } else if (joydevs[device].device == JOYDEV_CURS) {
         switch (key) {
            case KEYCODE_Up:
               joystick_set_value_or(port, 0x01);
               return 1;
            case KEYCODE_Down:
               joystick_set_value_or(port, 0x02);
               return 1;
            case KEYCODE_Left:
               joystick_set_value_or(port, 0x04);
               return 1;
            case KEYCODE_Right:
               joystick_set_value_or(port, 0x08);
               return 1;
            case KEYCODE_Space:
               joystick_set_value_or(port, 0x10);
               return 1;
            default:
               return 0;
         }
     } else {
        return 0;
     }
}

void circle_joy_usb(unsigned int device, int value) {
     if (device == 0 && joydevs[0].device == JOYDEV_USB_0) {
        joystick_set_value_absolute(joydevs[0].port, value);
     } else if (device == 0 && joydevs[1].device == JOYDEV_USB_0) {
        joystick_set_value_absolute(joydevs[1].port, value);
     } else if (device == 1 && joydevs[0].device == JOYDEV_USB_1) {
        joystick_set_value_absolute(joydevs[0].port, value);
     } else if (device == 1 && joydevs[1].device == JOYDEV_USB_1) {
        joystick_set_value_absolute(joydevs[1].port, value);
     }
}

void circle_joy_gpio(unsigned int device, int value) {
    if (device == 0 && joydevs[0].device == JOYDEV_GPIO_0) {
	joystick_set_value_absolute(joydevs[0].port, value);
    } else if (device == 0 && joydevs[1].device == JOYDEV_GPIO_0) {
        joystick_set_value_absolute(joydevs[1].port, value);
    } else if (device == 1 && joydevs[0].device == JOYDEV_GPIO_1) {
       joystick_set_value_absolute(joydevs[0].port, value);
    } else if (device == 1 && joydevs[1].device == JOYDEV_GPIO_1) {
       joystick_set_value_absolute(joydevs[1].port, value);
    }
}

// Setup stuff
int joy_arch_init(void) {
   int dev;

   circle_joy_init();

   return 0;
}

void joy_set_gamepad_info(int num_pads, int num_buttons[2],
                          int num_axes[2], int num_hats[2]) {
   joy_num_pads = num_pads;
   joy_num_axes[0] = num_axes[0];
   joy_num_axes[1] = num_axes[1];
   joy_num_hats[0] = num_hats[0];
   joy_num_hats[1] = num_hats[1];
   joy_num_buttons[0] = num_buttons[0];
   joy_num_buttons[1] = num_buttons[1];
}

int circle_joy_need_gpio(int device) {
  if (device == 0) {
     return joydevs[0].device == JOYDEV_GPIO_0 || joydevs[1].device == JOYDEV_GPIO_0;
  } else {
     return joydevs[0].device == JOYDEV_GPIO_1 || joydevs[1].device == JOYDEV_GPIO_1;
  }
}

void joystick_close(void) { }
void joystick(void) { }
void old_joystick_init(void) { }
void old_joystick_close(void) { }
void old_joystick(void) { }
void new_joystick_init(void) { }
void new_joystick_close(void) { }
void new_joystick(void) { }

#ifdef HAS_USB_JOYSTICK
int usb_joystick_init(void) { return 0; }
void usb_joystick_close(void) { }
void usb_joystick(void) { }
#endif
