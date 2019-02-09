/*
 * joy.h
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

#ifndef VICE_JOY_H
#define VICE_JOY_H

#include "circle.h"

int joy_arch_init(void);

#define JOYDEV_NONE      0
#define JOYDEV_NUMPAD    1
#define JOYDEV_KEYSET1   2
#define JOYDEV_KEYSET2   3
#define JOYDEV_ANALOG_0  4
#define JOYDEV_ANALOG_1  5
#define JOYDEV_ANALOG_2  6
#define JOYDEV_ANALOG_3  7
#define JOYDEV_ANALOG_4  8
#define JOYDEV_ANALOG_5  9
#define JOYDEV_DIGITAL_0 10
#define JOYDEV_DIGITAL_1 11
#define JOYDEV_USB_0     12
#define JOYDEV_USB_1     13
#define JOYDEV_GPIO_0    14
#define JOYDEV_GPIO_1    15
#define JOYDEV_CURS      16
#define JOYDEV_NUMS_1    17
#define JOYDEV_NUMS_2    18

extern void joystick_close(void);
extern void joystick(void);
extern void old_joystick_init(void);
extern void old_joystick_close(void);
extern void old_joystick(void);
extern void new_joystick_init(void);
extern void new_joystick_close(void);
extern void new_joystick(void);

extern int joy_num_pads;
extern int joy_num_axes[2];
extern int joy_num_hats[2];
extern int joy_num_buttons[2];

int joy_key_up(unsigned int device, int key);
int joy_key_down(unsigned int device, int key);

struct axis_config {
   int use;
   int neutral;
   int min;
   int max;
   int dir;
};

struct hat_config {
   int use;
   int dir[9]; // DIR_XX_INDEX
};

// We maintain two joystick devices that can moved
// to different ports.
struct joydev_config {
   // Which port does this device belong to?
   int port;
   int device;

   // Relevant for usb devices
   struct axis_config axes[4];
   struct hat_config hats[2];
};

#endif
