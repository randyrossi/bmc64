/*
 * joy-hidmgr.c - Mac OS X joystick support using IOHIDManager.
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
#include "joy-hidlib.h"

#ifdef HAS_JOYSTICK
#ifdef HAS_HIDMGR

/* ----- Statics ----- */

static IOHIDManagerRef mgr;

/* ----- Tools ----- */

static void CFSetApplierFunctionCopyToCFArray(const void *value, void *context)
{
    CFArrayAppendValue( ( CFMutableArrayRef ) context, value );
}

static Boolean IOHIDDevice_GetLongProperty( IOHIDDeviceRef inIOHIDDeviceRef, CFStringRef inKey, long * outValue )
{
    Boolean result = FALSE;
    
    if ( inIOHIDDeviceRef ) {
        CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty( inIOHIDDeviceRef, inKey );
        if ( tCFTypeRef ) {
            // if this is a number
            if ( CFNumberGetTypeID() == CFGetTypeID( tCFTypeRef ) ) {
                // get it's value
                result = CFNumberGetValue( ( CFNumberRef ) tCFTypeRef, kCFNumberSInt32Type, outValue );
            }
        }
    }
    return result;
}

/* ----- API ----- */

int  joy_hidlib_init(void)
{
    if ( !mgr ) {
        // create the manager
        mgr = IOHIDManagerCreate( kCFAllocatorDefault, 0L );
    }
    if ( mgr ) {
        // open it
        IOReturn tIOReturn = IOHIDManagerOpen( mgr, 0L);
        if ( kIOReturnSuccess != tIOReturn ) {
            return -1;
        } else {
            IOHIDManagerSetDeviceMatching( mgr, NULL );
            return 0;
        }    
    } else {
        return -1;
    }        
}

void joy_hidlib_exit(void)
{
    if(mgr) {
        IOHIDManagerClose( mgr, 0 );
        mgr = NULL;
    }
}

static int is_joystick(IOHIDDeviceRef ref)
{
    return 
        IOHIDDeviceConformsTo( ref, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick ) ||
        IOHIDDeviceConformsTo( ref, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad );
}

static int count_joysticks(joy_hid_device_array_t *dev_array)
{
    int i;
    int num_devices = dev_array->num_internal_devices;
    int num_joys = 0;
    CFArrayRef devices = dev_array->internal_devices;
    for ( i = 0; i < num_devices ; i++ ) {
        IOHIDDeviceRef dev = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( devices, i );
        if(is_joystick(dev)) {
               num_joys++;
        }
    }
    
    dev_array->num_devices = num_joys;
    return num_joys;
}

static void build_joystick_list(joy_hid_device_array_t *dev_array)
{
    int i;
    int num_devices = dev_array->num_internal_devices;        
    CFArrayRef devices = dev_array->internal_devices;
    joy_hid_device_t *d = dev_array->devices;
    for ( i = 0; i < num_devices ; i++ ) {
        IOHIDDeviceRef dev = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( devices, i );
        if(is_joystick(dev)) {
               
            long vendor_id = 0;
            IOHIDDevice_GetLongProperty( dev, CFSTR( kIOHIDVendorIDKey ), &vendor_id );
            long product_id = 0;
            IOHIDDevice_GetLongProperty( dev, CFSTR( kIOHIDProductIDKey ), &product_id );
            CFStringRef product_key;
            product_key = IOHIDDeviceGetProperty( dev, CFSTR( kIOHIDProductKey ) );
            char *product_name = "N/A";
            if(product_key) {
               char buffer[256];
               if(CFStringGetCString(product_key, buffer, 256, kCFStringEncodingUTF8)) {
                   product_name = strdup(buffer);
               }
            }

            d->internal_device = dev;
            d->vendor_id = (int)vendor_id;
            d->product_id = (int)product_id;
            d->serial = 0; /* will be filled in later */
            d->product_name = product_name;

            d++;
        }
    }
}
            
joy_hid_device_array_t *joy_hidlib_enumerate_devices(void)
{
    if( !mgr ) {
        return NULL;
    }

    /* create set of devices */
    CFSetRef device_set = IOHIDManagerCopyDevices( mgr );
    if ( !device_set ) {
        return NULL;
    }

    /* create array of devices */
    CFMutableArrayRef device_array = CFArrayCreateMutable( kCFAllocatorDefault, 0, 
                                                    & kCFTypeArrayCallBacks );
    if( ! device_array ) {
        CFRelease( device_set );
        return NULL;
    }                                
    CFSetApplyFunction( device_set, CFSetApplierFunctionCopyToCFArray, device_array );
    CFRelease( device_set );

    /* allocate result */
    joy_hid_device_array_t *result = 
        (joy_hid_device_array_t *)lib_malloc(sizeof(joy_hid_device_array_t));
    if(result == NULL) {
        CFRelease( device_array );
        return NULL;
    }

    /* get size */
    CFIndex cnt = CFArrayGetCount( device_array );

    /* fill internal struct */
    result->num_internal_devices = (int)cnt;
    result->internal_devices = device_array;
    result->num_devices = 0;
    result->devices = NULL;
    result->driver_name = "IOHIDManager";

    /* count joysticks -> num devices */
    count_joysticks(result);
    if(result->num_devices == 0)
        return result;
    
    /* allocate our device structs */
    joy_hid_device_t *devices =
        (joy_hid_device_t *)lib_malloc(sizeof(joy_hid_device_t) * result->num_devices);
    if(devices == NULL) {
        lib_free(result);
        CFRelease( device_array );
        return NULL;
    }
    result->devices = devices;
    
    build_joystick_list(result);
    return result;
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
    
    if(devices->internal_devices) {
        CFRelease( devices->internal_devices );
        devices->internal_devices = NULL;
    }

    if(devices != NULL) {
        free(devices);
        devices = NULL;
    }    
}

int  joy_hidlib_open_device(joy_hid_device_t *device)
{
    if( device->internal_device == NULL ) {
        return -2;
    }
    IOReturn result = IOHIDDeviceOpen( device->internal_device, 0 );
    if(result != kIOReturnSuccess) {
        return -1;
    } else {
        return 0;
    }
}

void joy_hidlib_close_device(joy_hid_device_t *device)
{
    /* close old device */
    if(device->internal_device != NULL) {
        IOHIDDeviceClose(device->internal_device, 0);
    }
}

int  joy_hidlib_enumerate_elements(joy_hid_device_t *device)
{
    IOHIDDeviceRef dev = device->internal_device;
    if(dev == NULL) {
        return -1;
    }
    
    /* get all elements of device */
    CFArrayRef internal_elements = IOHIDDeviceCopyMatchingElements( dev, NULL, 0 );    
    if(!internal_elements) {
        return -1;
    }
    
    /* get number of elements */
    CFIndex cnt = CFArrayGetCount( internal_elements );
    device->num_elements = (int)cnt;
    
    /* create elements array */
    joy_hid_element_t *elements = (joy_hid_element_t *)
        lib_malloc(sizeof(joy_hid_element_t) * cnt);
    if(elements == NULL) {
        CFRelease(internal_elements);
        internal_elements = NULL;
        return -1;
    }
    
    /* enumerate and convert all elements */
    CFIndex i;
    joy_hid_element_t *e = elements;
    for(i=0;i<cnt;i++) { 
        IOHIDElementRef internal_element = 
            ( IOHIDElementRef ) CFArrayGetValueAtIndex( internal_elements, i );
        if ( internal_element ) {
            uint32_t usage_page = IOHIDElementGetUsagePage( internal_element );
            uint32_t usage = IOHIDElementGetUsage( internal_element );
            CFIndex pmin = IOHIDElementGetPhysicalMin( internal_element );
            CFIndex pmax = IOHIDElementGetPhysicalMax( internal_element );
            CFIndex lmin = IOHIDElementGetLogicalMin( internal_element );
            CFIndex lmax = IOHIDElementGetLogicalMax( internal_element );
            
            e->usage_page = (int)usage_page;
            e->usage      = (int)usage;
            e->min_pvalue = (int)pmin;
            e->max_pvalue = (int)pmax;
            e->min_lvalue = (int)lmin;
            e->max_lvalue = (int)lmax;
            e->internal_element = internal_element;
        } else {
            e->usage_page = -1;
            e->usage      = -1;
            e->min_pvalue = -1;
            e->max_pvalue = -1;
            e->min_lvalue = -1;
            e->max_lvalue = -1;
            e->internal_element = NULL;
        }
        e++;
    }
    
    /* keep the reference until the elements are free'ed again */
    device->internal_elements = internal_elements;
    device->elements = elements;
    
    return (int)cnt;
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
    if(device->internal_elements) {
        CFRelease(device->internal_elements);
        device->internal_elements = NULL;
    }
}

int  joy_hidlib_get_value(joy_hid_device_t *device, 
                          joy_hid_element_t *element,
                          int *value, int phys)
{
    IOHIDValueRef value_ref;
    IOReturn result = IOHIDDeviceGetValue( device->internal_device, 
                                           element->internal_element,
                                           &value_ref );
    if(result == kIOReturnSuccess) {
        if(phys) {
            *value = (int)IOHIDValueGetScaledValue( value_ref, kIOHIDValueScaleTypePhysical );
        } else {
            *value = (int)IOHIDValueGetIntegerValue( value_ref );
        }
        return 0;
    } else {
        return -1;
    }
}

#endif /* HAS_HIDMGR */
#endif /* HAS_JOYSTICK */
