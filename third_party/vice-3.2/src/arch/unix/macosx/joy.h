/*
 * joy.h - Joystick support for Mac OS X.
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#define JOYDEV_NONE    0
#define JOYDEV_NUMPAD  1
#define JOYDEV_KEYSET1 2
#define JOYDEV_KEYSET2 3
#define JOYDEV_HID_0   4
#define JOYDEV_HID_1   5

/* obsolete fallbacks for X11/Gtk */
#define JOYDEV_ANALOG_0 4
#define JOYDEV_ANALOG_1 5
#define JOYDEV_ANALOG_2 6
#define JOYDEV_ANALOG_3 7
#define JOYDEV_ANALOG_4 8
#define JOYDEV_ANALOG_5 9

#define JOYSTICK_DESCRIPTOR_MAX_BUTTONS 32
#define JOYSTICK_DESCRIPTOR_MAX_AXIS    6
#define JOYSTICK_DESCRIPTOR_MAX_HAT_SWITCHES 4

#define HID_FIRE        0
#define HID_ALT_FIRE    1
#define HID_LEFT        2
#define HID_RIGHT       3
#define HID_UP          4
#define HID_DOWN        5
#define HID_NUM_BUTTONS 6

#define HID_AUTO_FIRE   6
#define HID_AUTO_ALT_FIRE 7
#define HID_NUM_AUTO_BUTTONS 2

#define HID_TOTAL_BUTTONS (HID_NUM_BUTTONS + HID_NUM_AUTO_BUTTONS)

#define HID_INVALID_BUTTON  0 /* invalid USB HID button ID */

#define HID_X_AXIS      0
#define HID_Y_AXIS      1
#define HID_NUM_AXIS    2

#ifdef HAS_JOYSTICK

#include "vice.h"
#include "types.h"

#ifndef JOY_INTERNAL
typedef void joy_hid_descriptor_t;
#else
#include "joy-hid.h"
#endif

/* describe an axis */
struct joy_axis {
    char *name;             /* name from joy_hid_axis_map used in VICE */
    int threshold;          /* threshold given in VICE */

    int  min_threshold;     /* calculated internal value */
    int  max_threshold;     /* calculated internal value */
    
    int  logical;            /* read logical values instead of physical values */
    int  min,max;           /* min, max values */
    
    int  mapped;            /* is axis successfully mapped by HID driver? */
};
typedef struct joy_axis joy_axis_t;

/* describe a button */
struct joy_button {
    int id;                 /* id of button in HID device */    
    int press;              /* auto fire press delay */
    int release;            /* auto fire release delay */
    int counter;            /* counter for auto fire */

    int mapped;             /* is button successfully mapped by HID driver? */
};
typedef struct joy_button joy_button_t;

/* describe a hat switch */
struct joy_hat_switch {
    int id;
    
    int mapped;
};
typedef struct joy_hat_switch joy_hat_switch_t;

/* describe a generic joystick HID device */
struct joystick_descriptor  {
    char *device_name;      /* device name: vid:pid:num */
    char *button_mapping;   /* set button mapping */
    char *auto_button_mapping; /* auto fire button mapping */

    joy_axis_t axis[HID_NUM_AXIS];
    joy_button_t buttons[HID_TOTAL_BUTTONS];
    joy_hat_switch_t hat_switch;

    /* number of buttons and axis available in device */
    int num_hid_buttons;
    int num_hid_axis;
    int num_hid_hat_switches;

    int mapped; /* is device mapped ? */

    joy_hid_descriptor_t *hid;
};
typedef struct joystick_descriptor joystick_descriptor_t;

/* access number of joyports and extra joyports for machine */
extern int joy_num_ports;
extern int joy_num_extra_ports;

/* UI accesses joy descriptors */
extern joystick_descriptor_t joy_a;
extern joystick_descriptor_t joy_b;

/* functions */
extern void joystick_close(void);
extern void joystick(void);

extern void joy_reload_device_list(void);
extern void joy_calc_threshold(int min, int max, int threshold, int *min_t, int *max_t);

extern void joy_reset_axis_range(joystick_descriptor_t *joy, int id);

#endif /* HAS_JOYSTICK */

#endif /* VICE_JOY_H */
