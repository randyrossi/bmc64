/** \file   joy-unix-usb.c
 * \brief   NetBSD/FreeBSD USB joystick support
 *
 * \author  Dieter Baron <dillo@nih.at>
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 */

/*
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

#include "vice.h"

#if defined(UNIX_COMPILE) && !defined(MACOSX_SUPPORT)

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "cmdline.h"
#include "joy.h"
#include "joystick.h"
#include "keyboard.h"
#include "log.h"
#include "resources.h"
#include "types.h"

#if defined(HAS_JOYSTICK) && defined(HAS_USB_JOYSTICK)

#define ITEM_AXIS   0
#define ITEM_BUTTON 1
#define ITEM_HAT    2

int hat_or[] = { 1, 9, 8, 10, 2, 6, 4, 5, };

extern log_t joystick_log;

#ifdef HAVE_USB_H
#include <usb.h>
#endif

#ifdef __DragonFly__
/* sys/param.h contains the __DragonFly_version macro */
# include <sys/param.h>
# if __DragonFly_version >= 300200
/* DragonFly >= 3.2 (USB4BSD stack) */
#  include <bus/u4b/usb.h>
#  include <bus/u4b/usbhid.h>
# else
/* DragonFly < 3.2: old USB stack */
#  include <bus/usb/usb.h>
#  include <bus/usb/usbhid.h>
# endif
#else
# ifdef __FreeBSD__
#  include <sys/ioccom.h>
# endif
# include <dev/usb/usb.h>
# include <dev/usb/usbhid.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_USBHID_H)
#include <usbhid.h>
#elif defined(HAVE_LIBUSB_H)
#include <libusb.h>
#elif defined(HAVE_LIBUSBHID_H)
#include <libusbhid.h>
#endif

#define MAX_DEV 4	/* number of uhid devices to try */

struct usb_joy_item {
    struct hid_item item;
    struct usb_joy_item *next;

    int type;
    int min_or;
    int min_val;
    int max_or;
    int max_val;
};

static struct usb_joy_item *usb_joy_item[2];

static int usb_joy_fd[2] = { -1, -1 };
static int usb_joy_size[2];
static char *usb_joy_buf[2];

static int usb_joy_add_item(struct usb_joy_item **item, struct hid_item *hi, int orval, int type)
{
    struct usb_joy_item *it;
    int w;

    if ((it=malloc(sizeof(*it))) == NULL) {
        /* XXX */
        return -1;
    }

    it->next = *item;
    *item = it;

    memcpy(&it->item, hi, sizeof(*hi));
    it->type = type;
    switch (type) {
        case ITEM_AXIS:
            w = (hi->logical_maximum - hi->logical_minimum) / 3;
            it->min_or = orval;
            it->min_val = hi->logical_minimum + w;
            it->max_or = orval * 2;
            it->max_val = hi->logical_maximum - w;
            break;
        case ITEM_BUTTON:
            it->min_or = 0;
            it->min_val = hi->logical_minimum;
            it->max_or = orval;
            it->max_val = hi->logical_maximum - 1;
            break;
        case ITEM_HAT:
            it->min_val = hi->logical_minimum;
            break;
    }

    return 0;
}

static void usb_free_item(struct usb_joy_item **item)
{
    struct usb_joy_item *it, *it2;
    
    it=*item;
    while (it) {
        it2 = it;
        it = it->next;
        free(it2);
    }
    *item = NULL;
}

int usb_joystick_init(void)
{
    int i, j, id = 0, fd;
    report_desc_t report;
    struct hid_item h;
    struct hid_data *d;
    int is_joy, found;
    char dev[32];

    for (j=i=0; i<2 && j<MAX_DEV; j++) {
        sprintf(dev, "/dev/uhid%d", j);
        fd = open(dev, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            continue;
        }

#if defined(USB_GET_REPORT_ID) && !defined(__DragonFly__)
        if (ioctl(fd, USB_GET_REPORT_ID, &id) < 0) {
            log_warning(joystick_log, "Cannot get report id for joystick device `%s'.", dev);
            close(fd);
        }
#endif

        if ((report=hid_get_report_desc(fd)) == NULL) {
            log_warning(joystick_log, "Cannot report description for joystick device `%s'.", dev);
            close(fd);
            continue;
        }
        usb_joy_size[i] = hid_report_size(report, hid_input, id);

        usb_joy_item[i] = NULL;

        found = 0;
        is_joy = 0;
#if !defined(HAVE_USBHID_H) && !defined(HAVE_LIBUSB_H) && defined(HAVE_LIBUSBHID)
        for (d = hid_start_parse(report, id);
#else
        for (d = hid_start_parse(report, 1 << hid_input, id);
#endif
        hid_get_item(d, &h);) {
            if (h.kind == hid_collection && HID_PAGE(h.usage) == HUP_GENERIC_DESKTOP && (HID_USAGE(h.usage) == HUG_JOYSTICK || HID_USAGE(h.usage) == HUG_GAME_PAD)) {
                is_joy = 1;
                continue;
            }
            if (!is_joy) {
                continue;
            }

            switch (HID_PAGE(h.usage)) {
                case HUP_GENERIC_DESKTOP:
                    switch (HID_USAGE(h.usage)) {
                        case HUG_X:
                        case HUG_RX:
                            if (usb_joy_add_item(usb_joy_item + i, &h, 4, ITEM_AXIS) == 0) {
                                found |= 4;
                            }
                            break;
                        case HUG_Y:
                        case HUG_RY:
                            if (usb_joy_add_item(usb_joy_item + i, &h, 1, ITEM_AXIS) == 0) {
                                found |= 1;
                            }
                            break;
                        case HUG_HAT_SWITCH:
                            if (usb_joy_add_item(usb_joy_item + i, &h, 0, ITEM_HAT) == 0) {
                                found |= 5;
                            }
                            break;
                    }
                    break;
                case HUP_BUTTON:
                    if (usb_joy_add_item(usb_joy_item + i, &h, 16, ITEM_BUTTON) == 0) {
                        found |= 16;
                    }
                    break;
            }
        }
        hid_end_parse(d);

        if (found != 21) {
            close(fd);
            usb_free_item(usb_joy_item + i);
            log_message(joystick_log, "Not all axes found in joystick device `%s'.", dev);
            continue;
        }

        if ((usb_joy_buf[i] = malloc(usb_joy_size[i])) == NULL) {
            log_warning(joystick_log, "Cannot allocate buffer for joystick device `%s'.", dev);
            close(fd);
            usb_free_item(usb_joy_item + i);
            continue;
        }

        log_message(joystick_log, "USB joystick found: `%s'.", dev);
        usb_joy_fd[i] = fd;
        i++;
    }
    return 0;
}

void usb_joystick_close(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (usb_joy_fd[i] < 0) {
            continue;
        }

        close(usb_joy_fd[i]);
        usb_joy_fd[i] = -1;
        usb_free_item(usb_joy_item+i);
    }
}

void usb_joystick(void)
{
    int i, jp, val, ret;
    struct usb_joy_item *it;

    for (i = 0; i < 4; i++) {
        jp = joystick_port_map[i];
        if (jp != JOYDEV_USB_0 && jp != JOYDEV_USB_1) {
            continue;
        }

        jp -= JOYDEV_USB_0;

        if (usb_joy_fd[jp] < 0) {
            continue;
        }

        val = 0;
        while ((ret = read(usb_joy_fd[jp], usb_joy_buf[jp], usb_joy_size[jp])) == usb_joy_size[jp]) {
            val = 1;
        }
        if (ret != -1 && errno != EAGAIN) {
            /* XXX */
            printf("strange read return: %d/%d\n", ret, errno);
            continue;
        }
        if (!val) {
            continue;
        }

        joystick_set_value_absolute(i + 1, 0);

        for (it = usb_joy_item[jp]; it; it = it->next) {
            val = hid_get_data(usb_joy_buf[jp], &it->item);
            if (it->type == ITEM_HAT) {
                val -= it->min_val;
                if (val >= 0 && val <= 7) {
                    joystick_set_value_or(i + 1, hat_or[val]);
                }
            } else {
                if (val <= it->min_val) {
                    joystick_set_value_or(i + 1, it->min_or);
                } else if (val > it->max_val) {
                    joystick_set_value_or(i + 1, it->max_or);
                }
            }
        }
    }
}

#endif /* HAS_JOYSTICK && HAS_USB_JOYSTICK */
#endif

