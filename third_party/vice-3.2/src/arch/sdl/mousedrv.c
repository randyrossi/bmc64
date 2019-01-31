/*
 * mousedrv.c - Mouse handling for SDL.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
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

#include "vice.h"

#include "vice_sdl.h"

#include "mouse.h"
#include "mousedrv.h"
#include "ui.h"
#include "vsyncapi.h"

#ifdef ANDROID_COMPILE
int mouse_x, mouse_y;
#else
static int mouse_x, mouse_y;
#endif

static unsigned long mouse_timestamp = 0;
static mouse_func_t mouse_funcs;

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
        case SDL_BUTTON_LEFT:
            mouse_funcs.mbl(state);
            break;
        case SDL_BUTTON_MIDDLE:
            mouse_funcs.mbm(state);
            break;
        case SDL_BUTTON_RIGHT:
            mouse_funcs.mbr(state);
            break;
/* FIXME: fix for SDL2 */
#ifndef USE_SDLUI2
        case SDL_BUTTON_WHEELUP:
            mouse_funcs.mbu(state);
            break;
        case SDL_BUTTON_WHEELDOWN:
            mouse_funcs.mbd(state);
            break;
#endif
        default:
            break;
    }
}

int mousedrv_get_x(void)
{
    return mouse_x;
}

int mousedrv_get_y(void)
{
    return mouse_y;
}

void mouse_move(int x, int y)
{
    mouse_x += x;
    mouse_y -= y;
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
