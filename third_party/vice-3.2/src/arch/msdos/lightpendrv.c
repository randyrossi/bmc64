/*
 * lightpendrv.c - Lightpen driver for WIN32 UI.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
#include "types.h"

#include <allegro.h>

#include "lightpen.h"
#include "lightpendrv.h"
#include "machine.h"

static int mouse_install_done = 0;

/* ------------------------------------------------------------------ */
/* External interface */

void msdos_lightpen_update(void)
{
    int x, y;
    int buttons;

    if (!lightpen_enabled) {
        return;
    }

    if (mouse_install_done == 0) {
        if (install_mouse() != -1) {
            set_mouse_range(0, 0, ~0, ~0);
            set_mouse_speed(1, 1);
            mouse_install_done = 1;
        } else {
            mouse_install_done = -1;
        }
    }

    if (mouse_install_done == -1) {
        return;
    }

    poll_mouse();
    x = mouse_pos >> 16;
    y = mouse_pos & 0xffff;

    buttons = (mouse_b & 1) ? LP_HOST_BUTTON_1 : 0;
    buttons |= (mouse_b & 2) ? LP_HOST_BUTTON_2 : 0;

    lightpen_update((machine_class == VICE_MACHINE_C128) ? 1 : 0, x, y, buttons);
}
