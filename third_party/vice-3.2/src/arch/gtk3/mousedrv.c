/** \file   mousedrv.c
 * \brief   Native GTK3 UI mouse driver stuff.
 *
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

#include <stdio.h>

#include "not_implemented.h"

#include "vsyncapi.h"
#include "mouse.h"
#include "mousedrv.h"


/** \brief The callbacks registered for mouse buttons being pressed or
 *         released. 
 *  \sa mousedrv_resources_init which sets these values properly
 *  \sa mouse_button which uses them
 */
static mouse_func_t mouse_funcs = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/** \brief Current mouse X value.
 *
 *  This is a dead-reckoning sum of left and right motions and does
 *  not necessarily bear any connection to any actual X coordinates.
 *
 *  \sa mousedrv_get_x
 */
static float mouse_x = 0.0;

/** \brief Current mouse Y value.
 *
 *  This is a dead-reckoning sum of left and right motions and does
 *  not necessarily bear any connection to any actual X coordinates.
 *
 *  \sa mousedrv_get_y
 */
static float mouse_y = 0.0;

/** \brief Last time the mouse was moved.
 *
 *  \sa mousedrv_get_timestamp
 */
static unsigned long mouse_timestamp = 0;

int mousedrv_cmdline_options_init(void)
{
    return 0;
}

unsigned long mousedrv_get_timestamp(void)
{
    return mouse_timestamp;
}

int mousedrv_get_x(void)
{
    return (int)mouse_x;
}

int mousedrv_get_y(void)
{
    return (int)mouse_y;
}


void mouse_move(float dx, float dy)
{
    mouse_x += dx;
    mouse_y -= dy;  /* why ? */

    /* can't this be done with int modulo ? */
    while (mouse_x < 0.0) {
        mouse_x += 65536.0;
    }
    while (mouse_x >= 65536.0) {
        mouse_x -= 65536.0;
    }
    while (mouse_y < 0.0) {
        mouse_y += 65536.0;
    }
    while (mouse_y >= 65536.0) {
        mouse_y -= 65536.0;
    }

    mouse_timestamp = vsyncarch_gettime();
}

void mouse_button(int bnumber, int state)
{
    switch(bnumber) {
    case 0:
        if (mouse_funcs.mbl) {
            mouse_funcs.mbl(state);
        }
        break;
    case 1:
        if (mouse_funcs.mbm) {
            mouse_funcs.mbm(state);
        }
        break;
    case 2:
        if (mouse_funcs.mbr) {
            mouse_funcs.mbr(state);
        }
        break;
    case 3:
        if (mouse_funcs.mbu) {
            mouse_funcs.mbu(state);
        }
        break;
    case 4:
        if (mouse_funcs.mbd) {
            mouse_funcs.mbd(state);
        }
        break;
    default:
        fprintf(stderr, "GTK3MOUSE: Warning: Strange mouse button %d\n", bnumber);
    }
}

void mousedrv_init(void)
{
    /* This does not require anything special to be done */
}

void mousedrv_mouse_changed(void)
{
    /** \todo Tell UI level to capture mouse cursor if necessary and
     *        permitted */
    fprintf(stderr, "GTK3MOUSE: Status changed\n");
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


