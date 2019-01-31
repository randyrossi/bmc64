/*
 * uitapestatus.c - Xaw(3d) only, UI controls for Datasette emulation
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

/* #define DEBUG_X11UI */

#include <stdarg.h>
#include "vice.h"

#include "datasette.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "ui.h"
#include "uiarch.h"
#include "uiapi.h"
#include "uicontents.h"
#include "uimenu.h"
#include "util.h"
#include "video.h"
#include "resources.h"
#include "videoarch.h"

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/Simple.h>
#include <X11/Xaw3d/Tip.h>
#else
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/Tip.h>
#endif

#ifndef XawRDisplayList
#define XawRDisplayList         "XawDisplayList"
#endif

#ifndef XawNdisplayList
#define XawNdisplayList         "displayList"
#endif

/* #define DEBUG_X11UI */
#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/******************************************************************************/

static int tape_motor_status = -1;        /* tape motor on/off */
static int tape_control_status = -1;      /* tape button/operation status */
static int tape_image_status = -1;        /* tape image present */
static XtTranslations tape_menu_translations;

char *last_attached_tape; /* FIXME: We want this to be static.  */
Widget left_tape_menu, right_tape_menu;

static void invalidate_tape_menu(void);

/*
 * displayLists aren't documented in the Xaw paper, "Athena Widget Set -
 * C Language Interface", since they are a late addition (like tips).
 * They are in the Xaw(3) manual page instead.
 */
static XawDisplayList *create_display_list(Widget w, char *fmt, ...)
{
    /* va_list args; */
    char *buf;
    XawDisplayList *xdl;
    XrmValue from, to;
#if 0
    va_start(args, fmt);
    buf = lib_mvsprintf(fmt, args);
    va_end(args);
#endif
    buf = fmt;

    DBG(("converting displayList: [%s]", buf));
    from.size = strlen(buf) + 1;
    from.addr = buf;
    to.size = sizeof(xdl);
    to.addr = (void *)&xdl;

    if (!XtConvertAndStore(w, XtRString, &from,
                              XawRDisplayList, &to)) {
        xdl = NULL;
    }

    /* lib_free(buf); */

    return xdl;
}

void build_tape_status_widget(tape_widgets_t *ts, Widget parent, int width, int height)
{
    Widget w = ts->button_status;

    /* PLAY */
    ts->motor_on [0] = create_display_list(w,
                "foreground rgb:0/0/0;"
                "line-width 3;"
                "draw-lines 4,4, -6,1/2, 4,-5, 4,4");
    ts->motor_off[0] = create_display_list(w,
                "foreground rgb:80/80/80;"
                "line-width 3;"
                "draw-lines 4,4, -6,1/2, 4,-5, 4,4");

    /* Fast Forward */
    ts->motor_on [1] = create_display_list(w,
                "foreground rgb:0/0/0;"
                "line-width 3;"
                "draw-lines 4,4, 1/2,1/2, 4,-5;"
                "draw-lines 1/2,4, -5,1/2, 1/2,-5");
    ts->motor_off[1] = create_display_list(w,
                "foreground rgb:80/80/80;"
                "line-width 3;"
                "draw-lines 4,4, 1/2,1/2, 4,-5;"
                "draw-lines 1/2,4, -5,1/2, 1/2,-5");

    /* Fast Rewind */
    ts->motor_on [2] = create_display_list(w,
                "foreground rgb:0/0/0;"
                "line-width 3;"
                "draw-lines -5,4, 1/2,1/2, -5,-5;"
                "draw-lines 1/2,4, 4,1/2, 1/2,-5;");
    ts->motor_off[2] = create_display_list(w,
                "foreground rgb:80/80/80;"
                "line-width 3;"
                "draw-lines -5,4, 1/2,1/2, -5,-5;"
                "draw-lines 1/2,4, 4,1/2, 1/2,-5;");

    /* Record */
                /*
                 * The RECORDING button is red. To make sure it is visible even
                 * on the red background, draw some  extra black lines. However
                 * this should not be necessary since the UI tends to prevent
                 * recording without a tape inserted.
                 */
    ts->motor_on [3] = create_display_list(w,
                "foreground rgb:f/0/0;"         
                "line-width 5;"                 
                "draw-arc 4,4, -5,-5;"          
                "foreground rgb:0/0/0;"         
                "line-width 1;"                 
                "draw-arc 1,1, -2,-2;"          
                "draw-arc 6,6, -7,-7;");        
    ts->motor_off[3] = create_display_list(w,
                "foreground rgb:80/80/80;"
                "line-width 5;"
                "draw-arc 4,4, -5,-5;");

    /* STOP */
    ts->motor_on [4] = create_display_list(w,
                "foreground rgb:0/0/0;"
                "line-width 5;"
                "draw-rectangle 5,5,-6,-6;");
    ts->motor_off[4] = create_display_list(w,
                "foreground rgb:80/80/80;"
                "line-width 5;"
                "draw-rectangle 5,5,-6,-6;");

    if (!tape_menu_translations) {
        char *translation_table;

        translation_table =
                "<Btn1Down>: "
                        "XawPositionSimpleMenu(leftTapeMenu) "
                        "XtMenuPopup(leftTapeMenu)\n"
                "Meta Shift <KeyDown>z: "
                        "FakeButton(1) "
                        "XawPositionSimpleMenu(leftTapeMenu) "
                        "XtMenuPopup(leftTapeMenu)\n"
                "<Btn3Down>: "
                        "RebuildTapeMenu(1) "
                        "XawPositionSimpleMenu(rightTapeMenu) "
                        "XtMenuPopup(rightTapeMenu)\n"
                "Meta Shift <KeyDown>x: "
                        "RebuildTapeMenu(1) "
                        "FakeButton(3) "
                        "XawPositionSimpleMenu(rightTapeMenu) "
                        "XtMenuPopup(rightTapeMenu)\n";
        tape_menu_translations = XtParseTranslationTable(translation_table);
    }

    XtOverrideTranslations(ts->counter_label, tape_menu_translations);
}

/* tape-related ui */
void ui_set_tape_status(int tape_status)
{
    DBG(("ui_set_tape_status (%d)", tape_status));
    if (tape_status == DATASETTE_CONTROL_RECORD) {
        invalidate_tape_menu();
    }
    if (tape_image_status == tape_status) {
        return;
    }
    tape_image_status = tape_status;
    ui_display_tape_control_status(-1);
}

void ui_display_tape_motor_status(int motor)
{
    DBG(("ui_display_tape_motor_status (%d)", motor));
    if (tape_motor_status == motor) {
        return;
    }
    tape_motor_status = motor;
    ui_display_tape_control_status(-1);
}

const char *tip_motor_on[5] = {
    "playing",
    "ffwding",
    "rewinding",
    "recording",
    "motor on",
};

const char *tip_motor_off[5] = {
    "play (motor off)",
    "ffwd (motor off)",
    "rewind (motor off)",
    "record (motor off)",
    "motor off",
};

void ui_display_tape_control_status(int control)
{
    int num_app_shells = get_num_shells();
    int i;
    int displayList;
    const char *tip;
    Pixel colour;

    DBG(("ui_display_tape_control_status (%d) motor:%d image:%d", control, tape_motor_status, tape_image_status));

    if (control < 0) {
        control = tape_control_status;
    } else {
        tape_control_status = control;
    }

    /* Pick a background color */
    colour = (tape_image_status > 0) ? drive_led_on_green_pixel : drive_led_on_red_pixel;

    /* Pick a pre-compiled display list */
    switch (control) {
        case DATASETTE_CONTROL_START:
            displayList = 0;
            break;
        case DATASETTE_CONTROL_FORWARD:
            displayList = 1;
            break;
        case DATASETTE_CONTROL_REWIND:
            displayList = 2;
            break;
        case DATASETTE_CONTROL_RECORD:
            displayList = 3;
            invalidate_tape_menu();
            break;
        case DATASETTE_CONTROL_RESET:
            displayList = 4;
            invalidate_tape_menu();
            break;
        default:
            displayList = 4;
            break;
    }

    tip = tape_motor_status > 0 ? tip_motor_on[displayList]
                                : tip_motor_off[displayList];

    for (i = 0; i < num_app_shells; i++) {
        tape_widgets_t *tw = &app_shells[i].tape_widgets[0];
        Widget w = tw->button_status;
        XawDisplayList *xdl;

        xdl = tape_motor_status > 0 ? tw->motor_on[displayList]
                                    : tw->motor_off[displayList];
        if (w) {
            DBG(("XtVaSetValues: tip [%s] xdl %p", tip, xdl));
            XtVaSetValues(w,
                          XtNbackground, colour,
                          XtNtip, tip,
                          XawNdisplayList, xdl,
                          NULL);
        }
    }
}

void ui_set_tape_menu(ui_menu_entry_t *menu)
{
    DBG(("ui_set_tape_menu"));
    if (left_tape_menu) {
        ui_menu_delete(left_tape_menu);
    }
    left_tape_menu = ui_menu_create("leftTapeMenu", menu, NULL);
}

void ui_display_tape_counter(int counter)
{
    if (counter != app_shells[0].tape_widgets[0].counter_value) {
        int num_app_shells = get_num_shells();
        char label[100];
        int i;

        /* Translators: the string is fixed with 100 byte! Don't exceed this
        limit !*/
        sprintf(label, _("Tape #1: %03d"), counter % 1000);
        DBG(("ui_display_tape_counter: (%s)", label));
        for (i = 0; i < num_app_shells; i++) {
            XtVaSetValues(app_shells[i].tape_widgets[0].counter_label,
                        XtNlabel, label,
                        NULL);
        }
        app_shells[0].tape_widgets[0].counter_value = counter;
    }
}

void ui_display_tape_current_image(const char *image)
{
    char *name;
    int i;
    int num_app_shells = get_num_shells();

    lib_free(last_attached_tape);
    last_attached_tape = lib_stralloc(image);
    util_fname_split(image, NULL, &name);

    for (i = 0; i < num_app_shells; i++) {
        XtVaSetValues(app_shells[i].tape_widgets[0].counter_label,
                    XtNtip, name,
                    NULL);
    }
    lib_free(name);
    invalidate_tape_menu();
}

static void invalidate_tape_menu()
{
    DBG(("invalidate_tape_menu"));
    if (right_tape_menu) {
        ui_menu_delete(right_tape_menu);
        right_tape_menu = NULL;
    }
}

void rebuild_tape_menu_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    DBG(("rebuild_tape_menu_action"));
    if (right_tape_menu == NULL) {
        if (last_attached_tape == NULL || last_attached_tape[0] == 0) {
            return;
        }

        right_tape_menu = rebuild_contents_menu("rightTapeMenu", 1, last_attached_tape);
    }
}

void tape_play_stop_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    int action;

    if (tape_control_status == DATASETTE_CONTROL_STOP) {
        action = DATASETTE_CONTROL_START;

        if (*num_params >= 1) {
            switch (params[0][0]) {
                case 'p':       /* play */
                    action = DATASETTE_CONTROL_START;
                    break;
                case 'r':       /* rew (Ctrl-click) */
                    if (params[0][2] == 'c') {
                        action = DATASETTE_CONTROL_RECORD;
                    } else {    /* rec (Shift-doubleclick) */
                        action = DATASETTE_CONTROL_REWIND;
                    }
                    break;
                case 'f':       /* ffwd (Meta-click) */
                    action = DATASETTE_CONTROL_FORWARD;
                    break;
                case 's':       /* stop */
                    action = DATASETTE_CONTROL_STOP;
                    break;
            }
        }
    } else {
        action = DATASETTE_CONTROL_STOP;
    }
    DBG(("tape_play_stop_action %d", action));
    datasette_control(action);
}
