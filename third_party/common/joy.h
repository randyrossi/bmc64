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

#ifndef RASPI_JOY_H
#define RASPI_JOY_H

#include "circle.h"

#define MAX_USB_BUTTONS 20
#define MAX_USB_AXES 16
#define MAX_USB_HATS 6

int joy_arch_init(void);

extern void joystick_close(void);
extern void joystick(void);
extern void old_joystick_init(void);
extern void old_joystick_close(void);
extern void old_joystick(void);
extern void new_joystick_init(void);
extern void new_joystick_close(void);
extern void new_joystick(void);

extern int joy_num_pads;
extern int joy_num_axes[MAX_USB_DEVICES];
extern int joy_num_hats[MAX_USB_DEVICES];
extern int joy_num_buttons[MAX_USB_DEVICES];
extern unsigned joy_prev_buttons[MAX_USB_DEVICES];

int joy_key_up(unsigned int device, int key);
int joy_key_down(unsigned int device, int key);

#endif
