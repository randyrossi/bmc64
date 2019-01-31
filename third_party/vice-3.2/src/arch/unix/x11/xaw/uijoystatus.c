/*
 * uijoystatus.c - Simple Xaw-based graphical user interface.
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
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

#include "vice.h"

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "widgets/Joystick.h"

#include "joystick.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "ui.h"
#include "uiarch.h"
#include "uicontents.h"
#include "uijoystatus.h"
#include "uimenu.h"
#include "userport_joystick.h"
#include "util.h"

#include "x11ui.h"

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

static const char *joynames[JOYSTICK_NUM] = {
    "joystick1",
    "joystick2",
    "joystick3",
    "joystick4",
    "joystick5",
};

void ui_display_joystick_status_widget(int joystick_number, int status)
{
    int i;
    int upjoy = 0, sidjoy = 0, typejoy = USERPORT_JOYSTICK_HUMMER;
    int num_app_shells = get_num_shells();

    DBG(("ui_display_joystick_status_widget (%d, %02x)", joystick_number, status));

    if (machine_class != VICE_MACHINE_CBM5x0) {
        resources_get_int("UserportJoy", &upjoy);
        if (machine_class != VICE_MACHINE_C64DTV) {
            resources_get_int("UserportJoyType", &typejoy);
        }
    }
    if (machine_class == VICE_MACHINE_PLUS4) {
        resources_get_int("SIDCartJoy", &sidjoy);
    }

    if (machine_class != VICE_MACHINE_PLUS4 && machine_class != VICE_MACHINE_C64DTV && joystick_number > 1) {
        switch (typejoy) {
            case USERPORT_JOYSTICK_HUMMER:
            case USERPORT_JOYSTICK_OEM:
                typejoy = 0;
                break;
            default:
                typejoy = 1;
        }
    }

    for (i = 0; i < num_app_shells; i++) {
        Widget ds = app_shells[i].joystick_status[joystick_number];

        if (ds != None) {
            Arg                 args[1];
            Widget              show[JOYSTICK_NUM];
            Widget              hide[JOYSTICK_NUM];
            int                 showcnt, hidecnt;

            XtSetArg(args[0], XtNenableBits, status);
            XtSetValues(ds, args, 1);

            showcnt = hidecnt = 0;

            if (machine_class == VICE_MACHINE_C64DTV) {
                if ((joystick_number == 2) && (upjoy == 0)) {
                    hide[hidecnt++] = ds;
                } else {
                    show[showcnt++] = ds;
                }
            } else {
                if ((joystick_number > 1) && (upjoy == 0)) {
                    hide[hidecnt++] = ds;
                }
                if ((joystick_number == 2) && (upjoy == 1)) {
                    show[showcnt++] = ds;
                }
                if ((joystick_number == 3) && (upjoy == 1) && (typejoy == 0)) {
                    hide[hidecnt++] = ds;
                }
                if ((joystick_number == 3) && (upjoy == 1) && (typejoy == 1)) {
                    show[showcnt++] = ds;
                }
                if ((joystick_number == 4) && (sidjoy == 0)) {
                    hide[hidecnt++] = ds;
                }
                if ((joystick_number == 4) && (sidjoy == 1)) {
                    show[showcnt++] = ds;
                }
            }

            if (hidecnt > 0) {
                XtUnmanageChildren(hide, hidecnt);
            }

            if (showcnt > 0) {
                XtManageChildren(show, showcnt);
            }
        }
    }
}

void build_joystick_status_widgets(app_shell_type *as, Widget parent)
{
    int i = 0;

    for (i = 0; i < JOYSTICK_NUM; i++)
    {
        /* skip port 1 for machines that have no internal joystick ports */
        if (((machine_class == VICE_MACHINE_CBM6x0) ||
             (machine_class == VICE_MACHINE_PET)
            ) && (i == 0)) {
            as->joystick_status[i] = None;
            continue;
        }

        /* skip port 2 for machines that only have one joystick port or no internal joystick ports */
        if (((machine_class == VICE_MACHINE_VIC20) ||
             (machine_class == VICE_MACHINE_CBM6x0) ||
             (machine_class == VICE_MACHINE_PET)
            ) && (i == 1)) {
            as->joystick_status[i] = None;
            continue;
        }

        /* skip port 3 for machines with no user port and no other joystick adapter type */
        if (((machine_class == VICE_MACHINE_CBM5x0)
            ) && (i == 2)) {
            as->joystick_status[i] = None;
            continue;
        }

        /* skip port 4 for machines with no user port, or not enough userport lines for 2 port userport adapters */
        if (((machine_class == VICE_MACHINE_CBM5x0) ||
             (machine_class == VICE_MACHINE_C64DTV)
            ) && (i == 3)) {
            as->joystick_status[i] = None;
            continue;
        }

        /* skip port 5 for machines with no 5th control port */
        if (((machine_class != VICE_MACHINE_PLUS4)) && (i == 4)) {
            as->joystick_status[i] = None;
            continue;
        }

        as->joystick_status[i] =
            XtVaCreateManagedWidget(joynames[i],
                    joystickWidgetClass, parent,
                    XtNwidth, 21,
                    XtNheight, 21,
                    XtNenableBits, 0,
                    XtNoffColor, drive_led_off_pixel,
                    XtNdirectionColor, drive_led_on_green_pixel,
                    XtNfireColor, drive_led_on_red_pixel,
                    XtNborderWidth, 0,
                    NULL);
    }
}

