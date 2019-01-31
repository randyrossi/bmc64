/*
 * joy-hidutil.c - Mac OS X joystick support using HID Utility Library.
 *
 * Written by
 *   Christian Vogelgsang <chris@vogelgsang.org>
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

#define JOY_INTERNAL

#include "vice.h"
#include "joy.h"
#include "log.h"
#include "lib.h"

#ifdef HAS_JOYSTICK
#ifndef HAS_HIDMGR

/* ----- Helpers ----- */

static int is_joystick(pRecDevice device)
{
    return  (device->usage == kHIDUsage_GD_Joystick) || 
            (device->usage == kHIDUsage_GD_GamePad);
}

static int count_joysticks(void)
{
    pRecDevice device;
    int num_devices = 0;

    /* iterate through all devices */
    for (device = HIDGetFirstDevice(); 
         device != NULL;
         device = HIDGetNextDevice(device)) {
        /* check if its a joystick or game pad device */
        if (is_joystick(device)) {
            num_devices ++;       
        }
    }
    return num_devices;
}

static void build_device_list(joy_hid_device_array_t *array)
{
    pRecDevice device;
    int num_devices = count_joysticks();

    array->num_devices = num_devices;
    array->devices = NULL;
    
    if(num_devices == 0)
        return;
    
    /* alloc dev array */
    joy_hid_device_t *devices = lib_malloc(sizeof(joy_hid_device_t) * num_devices);
    if(devices == NULL) {
        array->num_devices = 0;
        return;
    }
    array->devices = devices;
    
    /* iterate through all devices */
    joy_hid_device_t *d = devices;
    for (device = HIDGetFirstDevice(); 
         device != NULL;
         device = HIDGetNextDevice(device)) {
        /* check if its a joystick or game pad device */
        if (is_joystick(device)) {
    
            d->internal_device = device;
            d->vendor_id = (int)device->vendorID;
            d->product_id = (int)device->productID;
            d->serial = 0; /* will be filled in later */
            d->product_name = device->product;
    
            d++;
        }
    }
}

/* ----- API ----- */

int  joy_hidlib_init(void)
{
    return 0;
}

void joy_hidlib_exit(void)
{
}

joy_hid_device_array_t *joy_hidlib_enumerate_devices(void)
{
    /* build device list */
    HIDBuildDeviceList(kHIDPage_GenericDesktop, 0);

    /* no device list? -> no joysticks! */
    if (!HIDHaveDeviceList()) {
        return NULL;
    }

    /* alloc device array */
    joy_hid_device_array_t *array = lib_malloc(sizeof(joy_hid_device_array_t));
    if(array == NULL) {
        /* cleanup device list */
        HIDReleaseDeviceList();
        return NULL;
    }

    build_device_list(array);
    
    array->driver_name = "HIDUtils";
    return array;
}

void joy_hidlib_free_devices(joy_hid_device_array_t *devices)
{
    if(devices == NULL) {
        return;
    }
    
    int num_devices = devices->num_devices;
    int i;
    for(i = 0; i<num_devices; i++) {
        joy_hidlib_free_elements(&devices->devices[i]);
    }

    if(devices != NULL) {
        free(devices);
        devices = NULL;
    } 

    HIDReleaseDeviceList();
}

int  joy_hidlib_open_device(joy_hid_device_t *device)
{
    return 0;
}

void joy_hidlib_close_device(joy_hid_device_t *device)
{
}

int  joy_hidlib_enumerate_elements(joy_hid_device_t *device)
{
    pRecElement element;
    int num_elements = 0;

    pRecDevice d = device->internal_device;
    for (element = HIDGetFirstDeviceElement(d, kHIDElementTypeInput);
         element != NULL;
         element = HIDGetNextDeviceElement(element, kHIDElementTypeInput)) {
        num_elements++;
    }

    device->num_elements = num_elements;
    if(num_elements == 0) {
        device->elements = NULL;
        return 0;
    }

    /* alloc my elements */
    joy_hid_element_t *elements = lib_malloc(sizeof(joy_hid_element_t) * num_elements);
    device->elements = elements;
    if(elements == NULL) {
        device->num_elements = 0;
        return -1;
    }

    /* fill my elements */
    joy_hid_element_t *e = elements;
    for (element = HIDGetFirstDeviceElement(d, kHIDElementTypeInput);
         element != NULL;
         element = HIDGetNextDeviceElement(element, kHIDElementTypeInput)) {

        e->usage_page = (int)element->usagePage;
        e->usage      = (int)element->usage;
        e->min_pvalue = (int)element->min;
        e->max_pvalue = (int)element->max;
        e->min_lvalue = (int)element->scaledMin;
        e->max_lvalue = (int)element->scaledMax; 
        e->internal_element = element;
        
        e++;
    }
    return 0;
}

void joy_hidlib_free_elements(joy_hid_device_t *device)
{
    if(device == NULL) {
        return;
    }
    if(device->elements) {
        lib_free(device->elements);
        device->elements = NULL;
    }    
}

int  joy_hidlib_get_value(joy_hid_device_t *device, 
                          joy_hid_element_t *element,
                          int *value, int phys)
{
    pRecDevice d = device->internal_device;
    pRecElement e = element->internal_element;
    if (HIDIsValidElement(d, e)) {
        *value = HIDGetElementValue(d, e);
        return 0;
    } else {
        return -1;
    }
}

#endif /* !HAS_HIDMGR */
#endif /* HAS_JOYSTICK */
