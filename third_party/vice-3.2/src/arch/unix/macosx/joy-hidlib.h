/*
 * joy-hidlib.h - Common interface for the HID implementations.
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

#ifndef VICE_JOY_HIDLIB_H
#define VICE_JOY_HIDLIB_H

#include "vice.h"

#ifdef HAS_JOYSTICK

#ifdef HAS_HIDMGR
/* Use Leopard's IOHIDManager API */
#include <IOKit/hid/IOHIDManager.h>

typedef IOHIDDeviceRef  hid_device_ref_t;
typedef IOHIDElementRef hid_element_ref_t;

#else 
/* NOTE: We use the HID Utilites Library provided by Apple for free

   http://developer.apple.com/samplecode/HID_Utilities_Source/index.html

   Make sure to install this (static) library first!
*/
#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include "HID_Utilities_External.h"

/* HID Mgr Types */
typedef pRecDevice  hid_device_ref_t;
typedef pRecElement hid_element_ref_t;
#endif

/* model a hid element */
struct joy_hid_element {
    int     usage_page;
    int     usage;
    
    int     min_pvalue; /* physical value */
    int     max_pvalue;
    int     min_lvalue; /* logical value */
    int     max_lvalue;
    
    hid_element_ref_t internal_element;
};
typedef struct joy_hid_element joy_hid_element_t;
typedef struct joy_hid_element *joy_hid_element_ptr_t;

/* model a hid device */
struct joy_hid_device {
    int     vendor_id;
    int     product_id;
    int     serial;
    char    *product_name;

    int     num_elements;    /* number of elements in device */
    joy_hid_element_t *elements;

    hid_device_ref_t internal_device; /* pointer to native device */

#ifdef HAS_HIDMGR
    CFArrayRef internal_elements;
#endif
};
typedef struct joy_hid_device joy_hid_device_t;
typedef struct joy_hid_device *joy_hid_device_ptr_t;

/* an array of hid devices */
struct joy_hid_device_array {
    int     num_devices;
    joy_hid_device_t *devices;
    
#ifdef HAS_HIDMGR
    CFArrayRef internal_devices;
    int        num_internal_devices;
#endif

    const char *driver_name;
};
typedef struct joy_hid_device_array joy_hid_device_array_t;

/* ----- API ----- */

extern int  joy_hidlib_init(void); /* return 0=ok !=0 errror */
extern void joy_hidlib_exit(void);

extern joy_hid_device_array_t *joy_hidlib_enumerate_devices(void);
extern void joy_hidlib_free_devices(joy_hid_device_array_t *devices);

extern int  joy_hidlib_open_device(joy_hid_device_t *device);
extern void joy_hidlib_close_device(joy_hid_device_t *device);

extern int  joy_hidlib_enumerate_elements(joy_hid_device_t *device);
extern void joy_hidlib_free_elements(joy_hid_device_t *device);

extern int  joy_hidlib_get_value(joy_hid_device_t *device, 
                                 joy_hid_element_t *element,
                                 int *value, int phys);

#endif

#endif

