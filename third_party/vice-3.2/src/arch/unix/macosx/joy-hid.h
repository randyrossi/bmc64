/*
 * joy-hid.h - Joystick support for Mac OS X using USB HID devices.
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

#ifndef VICE_JOY_HID_H
#define VICE_JOY_HID_H

#include "vice.h"

#ifdef HAS_JOYSTICK

#include "joy-hidlib.h"

/* axis map: define names of available axis on a HID device */
struct joy_hid_axis_info {
    const char *name;
    int usage;
};
typedef struct joy_hid_axis_info joy_hid_axis_info_t;

/* public list of axis names */
extern joy_hid_axis_info_t joy_hid_axis_infos[];

/* describe the HID specific parts of the joystick */
struct joy_hid_descriptor  {
    joy_hid_device_ptr_t device; /* what HID device is mapped to this joy */

    joy_hid_element_ptr_t mapped_axis[HID_NUM_AXIS];
    joy_hid_element_ptr_t mapped_buttons[HID_TOTAL_BUTTONS];
    joy_hid_element_ptr_t mapped_hat_switch;
        
    joy_hid_element_ptr_t all_buttons[JOYSTICK_DESCRIPTOR_MAX_BUTTONS];
    joy_hid_element_ptr_t all_axis[JOYSTICK_DESCRIPTOR_MAX_AXIS];
    joy_hid_element_ptr_t all_hat_switches[JOYSTICK_DESCRIPTOR_MAX_HAT_SWITCHES];
};
typedef struct joy_hid_descriptor joy_hid_descriptor_t;

struct joystick_descriptor;

/* ----- API ----- */

extern int  joy_hid_init(void); /* return number of total devices found. <0 error */
extern void joy_hid_exit(void);
extern int  joy_hid_reload(void); /* return number of total device found. <0 error */
extern const joy_hid_device_array_t *joy_hid_get_devices(void);

/* device functions */
extern int  joy_hid_map_device(struct joystick_descriptor *joy, joy_hid_device_t *device);
extern void joy_hid_unmap_device(struct joystick_descriptor *joy);

/* axis functions */
extern int  joy_hid_reset_axis_range(struct joystick_descriptor *joy, int id, int usage, int logical);
extern int  joy_hid_assign_axis(struct joystick_descriptor *joy, int id, int usage, int logical);
extern int  joy_hid_detect_axis(struct joystick_descriptor *joy, int id, int logical);
extern int  joy_hid_read_axis(struct joystick_descriptor *joy,int id,int *value, int logical);
extern int  joy_hid_info_axis(struct joystick_descriptor *joy,int id,int *min, int *max, int logical);

/* button functions */
extern int  joy_hid_assign_button(struct joystick_descriptor *joy, int id, int usage);
extern int  joy_hid_detect_button(struct joystick_descriptor *joy);
extern int  joy_hid_read_button(struct joystick_descriptor *joy, int id, int *value);

/* hat switch functions */
extern int  joy_hid_assign_hat_switch(struct joystick_descriptor *joy, int serial);
extern int  joy_hid_detect_hat_switch(struct joystick_descriptor *joy);
extern int  joy_hid_read_hat_switch(struct joystick_descriptor *joy, int *value);

/* axis map functions */
extern const char *joy_hid_get_axis_name(int usage);
extern int joy_hid_get_axis_usage(const char *name);

#endif
#endif

