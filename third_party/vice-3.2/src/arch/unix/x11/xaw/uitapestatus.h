/*
 * uitapestatus.h - Xaw only, UI controls for Datasette emulation
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

#ifndef UITAPESTATUS_H_
#define UITAPESTATUS_H_

#include "uiarch.h"

typedef struct _XawDisplayList XawDisplayList;

struct tape_widgets {
    int counter_value;
    Widget counter_label;
    Widget button_status;
    XawDisplayList *motor_on[5];
    XawDisplayList *motor_off[5];
};

typedef struct tape_widgets tape_widgets_t;

extern void build_tape_status_widget(tape_widgets_t *ts, Widget parent, int width, int height);
extern void rebuild_tape_menu_action(Widget w, XEvent *event, String *params, Cardinal *num_params);
extern void tape_play_stop_action(Widget w, XEvent *event, String *params, Cardinal *num_params);

#endif
