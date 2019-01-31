/*
 * mousedrv.cc - Mouse handling for BeOS
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "vice.h"

#include <Application.h>
#include <stdio.h>

#include "vicewindow.h"

extern "C" {
#include "keyboard.h"
#include "log.h"
#include "mouse.h"
#include "mousedrv.h"
#include "types.h"
#include "ui.h"
#include "vsyncapi.h"
}

static mouse_func_t mouse_funcs;

int _mouse_x, _mouse_y;
static unsigned long mouse_timestamp = 0;
/* ------------------------------------------------------------------------- */

void mousedrv_mouse_changed(void)
{
}

int mousedrv_resources_init(mouse_func_t *funcs)
{
    mouse_funcs.mbl = funcs->mbl;
    mouse_funcs.mbr = funcs->mbr;
    mouse_funcs.mbm = funcs->mbm;
    mouse_funcs.mbu = funcs->mbu;
    mouse_funcs.mbd = funcs->mbd;
    return 0;
}

/* ------------------------------------------------------------------------- */

int mousedrv_cmdline_options_init(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */

extern ViceWindow *windowlist[];
extern int window_count;

void mouse_update_mouse(void)
{
    /* this implementation is realy ugly, but the MouseMoved event
       doesn't work as expected
     */
    static BPoint last_point;

    BPoint point;
    uint32 buttons;

    if (!_mouse_enabled || window_count == 0) {
        return;
    }

    windowlist[0]->Lock();
    windowlist[0]->view->GetMouse(&point, &buttons);
    windowlist[0]->Unlock();

    if (buttons & B_SECONDARY_MOUSE_BUTTON) {
        last_point = point;
        return;
    }

    if (point.x != last_point.x || point.y != last_point.y) {
        _mouse_x += (int)(point.x - last_point.x);
        _mouse_y -= (int)(point.y - last_point.y);
        mouse_timestamp = vsyncarch_gettime();
    }

    last_point = point;
}

void mousedrv_init(void)
{
}

int mousedrv_get_x(void)
{
    mouse_update_mouse();
    return _mouse_x;
}

int mousedrv_get_y(void)
{
    mouse_update_mouse();
    return _mouse_y;
}

unsigned long mousedrv_get_timestamp(void)
{
    return mouse_timestamp;
}

void mousedrv_button_left(int pressed)
{
    mouse_funcs.mbl(pressed);
}

void mousedrv_button_right(int pressed)
{
    mouse_funcs.mbr(pressed);
}

void mousedrv_button_middle(int pressed)
{
    mouse_funcs.mbm(pressed);
}
