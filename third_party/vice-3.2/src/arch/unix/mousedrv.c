/*
 * mousedrv.c - Mouse handling for Unix-Systems.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Oliver Schaertel <orschaer@forwiss.uni-erlangen.de>
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

/* This is a first rough implementation of mouse emulation for MS-DOS.
   A smarter and less buggy emulation is of course possible. */

/* #define DEBUG_MOUSE */

#ifdef DEBUG_MOUSE
#define DBG(x)  log_debug x
#else
#define DBG(x)
#endif

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mouse.h"
#include "mousedrv.h"
#include "log.h"
#include "ui.h"
#include "vsyncapi.h"

#ifndef MACOSX_COCOA

static mouse_func_t mouse_funcs;

static float mouse_x = 0.0, mouse_y = 0.0;
static unsigned long mouse_timestamp = 0;

void mousedrv_mouse_changed(void)
{
    ui_check_mouse_cursor();
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

void mousedrv_init(void)
{
}

/* ------------------------------------------------------------------------- */

void mouse_button(int bnumber, int state)
{
    switch (bnumber) {
    case 0:
        mouse_funcs.mbl(state);
        break;
    case 1:
        mouse_funcs.mbm(state);
        break;
    case 2:
        mouse_funcs.mbr(state);
        break;
    case 3:
        mouse_funcs.mbu(state);
        break;
    case 4:
        mouse_funcs.mbd(state);
        break;
    default:
        break;
    }
}

/* ------------------------------------------------------------------------- */

int mousedrv_get_x(void)
{
    return (int)mouse_x;
}

int mousedrv_get_y(void)
{
    return (int)mouse_y;
}

/* ------------------------------------------------------------------------- */

void mouse_move(float dx, float dy)
{
    mouse_x += dx;
    mouse_y -= dy;
    while (mouse_x < 0.0) mouse_x += 65536.0;
    while (mouse_x >= 65536.0) mouse_x -= 65536.0;
    while (mouse_y < 0.0) mouse_y += 65536.0;
    while (mouse_y >= 65536.0) mouse_y -= 65536.0;

    mouse_timestamp = vsyncarch_gettime();
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
#endif
