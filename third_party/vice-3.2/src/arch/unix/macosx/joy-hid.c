/*
 * joy-hid.c - Mac OS X joystick support using USB HID devices.
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
#include "joy-hid.h"

#ifdef HAS_JOYSTICK

/* ----- Static Data ----- */

static joy_hid_device_array_t *device_array;

joy_hid_axis_info_t joy_hid_axis_infos[] = {
    { "X", kHIDUsage_GD_X },
    { "Y", kHIDUsage_GD_Y },
    { "Z", kHIDUsage_GD_Z },
    { "Rx", kHIDUsage_GD_Rx },
    { "Ry", kHIDUsage_GD_Ry },
    { "Rz", kHIDUsage_GD_Rz },
    { "Slider", kHIDUsage_GD_Slider },
    { NULL, 0 }
};

/* ----- Tools ----- */

/* count devices with same vid:pid */
static int get_device_serial(int size, joy_hid_device_t *devs, int vid, int pid)
{
    int i;
    int serial = 0;
    
    for(i=0;i<size;i++) {
        if((devs->vendor_id == vid) && (devs->product_id == pid)) {
            serial++;
        }
        devs++;
    }
    return serial;
}

static void assign_device_serials(joy_hid_device_array_t *devices)
{
    int num_devices = devices->num_devices;
    joy_hid_device_t *d = devices->devices;
    int i;
    for(i=0;i<num_devices;i++) {
        int serial = get_device_serial(i, devices->devices, d->vendor_id, d->product_id);
        d->serial = serial;
        d++;
    }
}

static int detect_elements(struct joystick_descriptor *joy)
{
    joy->num_hid_axis = 0;
    joy->num_hid_buttons = 0;
    joy->num_hid_hat_switches = 0;
    
    int i;
    joy_hid_device_t *device = joy->hid->device;
    int num_elements = device->num_elements;
    int undetected = 0;
    for(i=0;i<num_elements;i++) {
        joy_hid_element_t *element = &device->elements[i];

        int usage_page = element->usage_page;
        int usage = element->usage;

        if(usage_page == kHIDPage_GenericDesktop) {
            switch(usage) {
                case kHIDUsage_GD_Pointer:
                case kHIDUsage_GD_GamePad:
                case kHIDUsage_GD_Joystick:
                    /* ignore */
                    break;
                case kHIDUsage_GD_X:
                case kHIDUsage_GD_Y:
                case kHIDUsage_GD_Z:
                case kHIDUsage_GD_Rx:
                case kHIDUsage_GD_Ry:
                case kHIDUsage_GD_Rz:
                case kHIDUsage_GD_Slider:
                    /* axis found */
                    if (joy->num_hid_axis == JOYSTICK_DESCRIPTOR_MAX_AXIS) {
                        undetected++;
                    } else {
                        /* check for valid axis */
                        if(element->min_pvalue != element->max_pvalue) {
                            
                            /* check if axis already occured ? 
                               this works around broken HID devices
                               that register multiple times e.g. an x axis
                               but only the last one works actually...
                            */
                            int j;
                            for(j=0;j<joy->num_hid_axis;j++) {
                                if(joy->hid->all_axis[j]->usage == usage)
                                    break;
                            }
                            
                            /* create new or overwrite old axis */
                            joy->hid->all_axis[j] = element;
                            if(j==joy->num_hid_axis) {
                                joy->num_hid_axis++;
                            } else {
                                log_message(LOG_DEFAULT, "joy-hid: ignoring multiple occurrence of axis element (0x%x)! (broken HID device?)", usage);
                            }
                            
                        } else {
                            log_message(LOG_DEFAULT, "joy-hid: ignoring element (0x%x) with invalid range! (broken HID device?)", usage);
                        }
                    }
                    break;
                case kHIDUsage_GD_Hatswitch:
                    if(joy->num_hid_hat_switches == JOYSTICK_DESCRIPTOR_MAX_HAT_SWITCHES) {
                        undetected++;
                    } else {
                        joy->hid->all_hat_switches[joy->num_hid_hat_switches] = element;
                        joy->num_hid_hat_switches++;
                    }
                    break;
                case kHIDUsage_GD_Wheel:
                case kHIDUsage_GD_Dial:
                    undetected++;
                    break;
                default:
                    undetected++;
                    break;
            }
        }
        else if(usage_page == kHIDPage_Button) {
            /* buttons found */
            if (joy->num_hid_buttons == JOYSTICK_DESCRIPTOR_MAX_BUTTONS) {
                undetected++;
            } else {
                joy->hid->all_buttons[joy->num_hid_buttons] = element;
                joy->num_hid_buttons++;
            }
        }
    }
    return undetected;
}

/* ----- API ----- */

int  joy_hid_init(void)
{
    /* first initialize lib */
    int result = joy_hidlib_init();
    if(result != 0) {
        return result;
    }
    
    /* build device array */
    device_array = joy_hidlib_enumerate_devices();
    if(device_array != NULL) {
        assign_device_serials(device_array);
        return device_array->num_devices;
    } else {
        return -1;
    }
}

void joy_hid_exit(void)
{
    /* free device array */
    if(device_array) {
        joy_hidlib_free_devices(device_array);
        device_array = NULL;
    }
    
    /* shutdown lib*/
    joy_hidlib_exit();
}

int  joy_hid_reload(void)
{
    /* reinit */
    joy_hid_exit();    
    return joy_hid_init();
}

const joy_hid_device_array_t *joy_hid_get_devices(void)
{
    return device_array;
}

int  joy_hid_map_device(struct joystick_descriptor *joy, joy_hid_device_t *device)
{
    /* already a device mapped? */
    if(joy->hid->device != NULL) {
        joy_hid_unmap_device(joy);
    }
    
    /* open device */
    int result = joy_hidlib_open_device(device);
    if(result != 0) {
        return result;
    }
    
    /* enumerate all elements */
    result = joy_hidlib_enumerate_elements(device);
    if(result < 0) {
        return result;
    }

    joy->hid->device = device;
    joy->mapped = 1;

    /* return number of undetected elements */
    return detect_elements(joy);
}

void joy_hid_unmap_device(struct joystick_descriptor *joy)
{
    joy_hid_descriptor_t *hid = joy->hid;
    joy_hid_device_t *device = hid->device;
    
    if(device != NULL) {
        joy_hidlib_free_elements(device);
        joy_hidlib_close_device(device);
        hid->device = NULL;
    }
    
    /* clear all */
    memset( hid, 0, sizeof(joy_hid_descriptor_t) );

    joy->mapped = 0;
}

/* ----- axis ----- */

joy_hid_element_t *joy_hid_element_by_usage(struct joystick_descriptor *joy, int usage)
{
    joy_hid_descriptor_t *hid = joy->hid;
    
    /* search axis element with usage */
    int num_axis = joy->num_hid_axis;
    int i;
    for(i=0;i<num_axis;i++) {
        if(hid->all_axis[i]->usage == usage) {
            return hid->all_axis[i];
        }
    }
    return NULL;
}

int joy_hid_reset_axis_range(struct joystick_descriptor *joy, int id, int usage, int logical)
{
    joy_hid_element_t *e = joy_hid_element_by_usage(joy, usage);
    if(e == NULL) {
        joy->axis[id].min = 0;
        joy->axis[id].max = 0;
        return -1;
    }
    
    joy->axis[id].min = logical ? e->min_lvalue : e->min_pvalue;
    joy->axis[id].max = logical ? e->max_lvalue : e->max_pvalue;
    return 0;
}

int  joy_hid_assign_axis(struct joystick_descriptor *joy, int id, int usage, int logical)
{
    joy_hid_descriptor_t *hid = joy->hid;
    
    joy_hid_element_t *e = joy_hid_element_by_usage(joy, usage);
    if(e == NULL) {
        joy->axis[id].mapped = 0;
        return -1;
    }
    
    hid->mapped_axis[id] = e;
    joy->axis[id].mapped = 1;
    
    joy->axis[id].logical = logical;
    
    joy_calc_threshold(joy->axis[id].min,
                       joy->axis[id].max, 
                       joy->axis[id].threshold,
                       &joy->axis[id].min_threshold,
                       &joy->axis[id].max_threshold);
    return 0;
}

int  joy_hid_detect_axis(struct joystick_descriptor *joy, int id, int logical)
{
    int threshold = joy->axis[id].threshold;

    /* search axis that is moved to min or max */
    int num_axis = joy->num_hid_axis;
    int i;
    joy_hid_device_t *device = joy->hid->device;
    for(i=0;i<num_axis;i++) {
        joy_hid_element_t *element = joy->hid->all_axis[i];

        /* calc threshold for this axis */
        int min = logical ? element->min_lvalue : element->min_pvalue;
        int max = logical ? element->max_lvalue : element->max_pvalue;
        int tmin, tmax;
        joy_calc_threshold(min, max, threshold, &tmin, &tmax);

        int value;
        if(joy_hidlib_get_value(device, element, &value, !logical)==0) {
            if(value < tmin) {
                /* auto adjust range */
                if(value < min) {
                    if(logical) {
                        element->min_lvalue = min;
                    } else {
                        element->min_pvalue = min;
                    }
                }
                return element->usage;
            } else if(value > tmax) {
                /* auto adjust range */
                if(value > max) {
                    if(logical) {
                        element->max_lvalue = max;
                    } else {
                        element->max_pvalue = max;
                    }
                }
                return element->usage;
            }
        }
    }
    return -1;
}

int  joy_hid_read_axis(struct joystick_descriptor *joy,int id,int *value, int logical)
{
    joy_hid_device_t *device = joy->hid->device;
    joy_hid_element_t *element = joy->hid->mapped_axis[id];
    return joy_hidlib_get_value(device, element, value, !logical);
}

int joy_hid_info_axis(struct joystick_descriptor *joy,int id,int *min, int *max, int logical)
{
    joy_hid_element_t *element = joy->hid->mapped_axis[id];
    if(element != NULL) {
        *min = logical ? element->min_lvalue : element->min_pvalue;
        *max = logical ? element->max_lvalue : element->max_pvalue;
        return 0;
    } else {
        *min = 0;
        *max = 0;
        return -1;
    }
}

/* ----- buttons ----- */

int  joy_hid_assign_button(struct joystick_descriptor *joy, int id, int usage)
{
    joy_hid_descriptor_t *hid = joy->hid;

    /* search axis element with usage */
    int num_buttons = joy->num_hid_buttons;
    int i;
    for(i=0;i<num_buttons;i++) {
        if(hid->all_buttons[i]->usage == usage) {
            break;
        }
    }
    if(i == num_buttons) {
        joy->buttons[id].mapped = 0;
        return -1;
    }

    hid->mapped_buttons[id] = hid->all_buttons[i];
    joy->buttons[id].mapped = 1;
    return 0;
}

int  joy_hid_detect_button(struct joystick_descriptor *joy)
{
    /* search button that is pressed */
    int num_buttons = joy->num_hid_buttons;
    int i;
    joy_hid_device_t *device = joy->hid->device;
    for(i=0;i<num_buttons;i++) {
        joy_hid_element_t *element = joy->hid->all_buttons[i];

        int value;
        if(joy_hidlib_get_value(device, element, &value, 0)==0) {
            if(value != 0) {
                return element->usage;
            }
        }
    }
    return -1;
}

int  joy_hid_read_button(struct joystick_descriptor *joy, int id, int *value)
{
    joy_hid_device_t *device = joy->hid->device;
    joy_hid_element_t *element = joy->hid->mapped_buttons[id];
    return joy_hidlib_get_value(device, element, value, 0);    
}

/* ----- Hat Switch ----- */

int  joy_hid_assign_hat_switch(struct joystick_descriptor *joy, int serial)
{
    joy_hid_descriptor_t *hid = joy->hid;

    /* search hat switch element with usage */
    int num_hat_switches = joy->num_hid_hat_switches;
    int i;
    for(i=0;i<num_hat_switches;i++) {
        if((i+1) == serial) {
            break;
        }
    }
    if(i == num_hat_switches) {
        joy->hat_switch.mapped = 0;
        return -1;
    }

    hid->mapped_hat_switch = hid->all_hat_switches[i];
    joy->hat_switch.mapped = 1;
    return 0;
}

int  joy_hid_detect_hat_switch(struct joystick_descriptor *joy)
{
    /* search hat switch that is pressed */
    int num_hat_switches = joy->num_hid_hat_switches;
    int i;
    joy_hid_device_t *device = joy->hid->device;
    for(i=0;i<num_hat_switches;i++) {
        joy_hid_element_t *element = joy->hid->all_hat_switches[i];

        int value;
        if(joy_hidlib_get_value(device, element, &value, 0)==0) {
            if((value >= 0)&&(value <= 7)) {
                return i + 1;
            }
        }
    }
    return -1;
}

int  joy_hid_read_hat_switch(struct joystick_descriptor *joy, int *value)
{
    joy_hid_device_t *device = joy->hid->device;
    joy_hid_element_t *element = joy->hid->mapped_hat_switch;
    return joy_hidlib_get_value(device, element, value, 0);
}

/* ----- Tools ----- */

const char *joy_hid_get_axis_name(int usage)
{
    joy_hid_axis_info_t *ptr = joy_hid_axis_infos;;
    while(ptr->name != NULL) {
        if (ptr->usage == usage) {
            return ptr->name;
        }
        ptr++;
    }
    return NULL;
}

int joy_hid_get_axis_usage(const char *name)
{
    if (name == NULL) {
        return -1;
    }
    
    joy_hid_axis_info_t *ptr = joy_hid_axis_infos;;
    while(ptr->name != NULL) {
        if (strcmp(ptr->name, name) == 0) {
            return ptr->usage;
        }
        ptr++;
    }
    return -1;
}

#endif /* HAS_JOYSTICK */
