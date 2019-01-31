/*
 * joy.h - Joystick support for Windows.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

typedef int joystick_device_t;

#define JOYDEV_NONE    0
#define JOYDEV_NUMPAD  1
#define JOYDEV_KEYSET1 2
#define JOYDEV_KEYSET2 3
#define JOYDEV_HW1     4
#define JOYDEV_HW2     5

extern int joystick_close(void);
extern void joystick_update(void);
extern char joystick_uses_direct_input(void);

#endif
