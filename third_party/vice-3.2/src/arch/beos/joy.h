/*
 * joy.h - Joystick support for BeOS.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Marcus Sutton <loggedoubt@gmail.com>
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

#include "kbd.h"

/* some thresholds for the analog Joysticks */
#define JOYBORDER_MINX -16384
#define JOYBORDER_MAXX  16384
#define JOYBORDER_MINY -16384
#define JOYBORDER_MAXY  16384

/* devices without the hardware sticks */
#define NUM_OF_SOFTDEVICES      4
#define MAX_HARDWARE_JOYSTICK   16

typedef enum {
    JOYDEV_NONE,
    JOYDEV_NUMPAD,
    JOYDEV_KEYSET1,
    JOYDEV_KEYSET2
} joystick_device_t;

typedef struct _hardware_joystick {
    char device_name[256];
    int device_num;
    int stick;
    int axes;
} hardware_joystick_t;

extern int joystick_close(void);
extern void joystick_update(void);
extern int joystick_handle_key(kbd_code_t kcode, int pressed);

#endif
