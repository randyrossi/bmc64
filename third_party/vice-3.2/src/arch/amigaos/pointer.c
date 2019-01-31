/*
 * pointer.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include "private.h"

#include <stdio.h>
#include "pointer.h"

static unsigned short empty_pointer[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int pointer_default = POINTER_SHOW, pointer_hidden = 0;

void pointer_set_default(int def)
{
    pointer_default = def;
}

void pointer_hide(void)
{
    video_canvas_t *canvas;

    if (pointer_hidden == 1) {
        return;
    }

    for (canvas = canvaslist; canvas; canvas = canvas->next) {
        SetPointer(canvas->os->window, empty_pointer, 2, 16, 0, 0);
    }

    pointer_hidden = 1;
}

void pointer_to_default(void)
{
    if (pointer_default == POINTER_HIDE) {
        pointer_hide();
    } else {
        pointer_show();
    }
}

void pointer_show(void)
{
    video_canvas_t *canvas;

    if (pointer_hidden == 0) {
        return;
    }

    for (canvas = canvaslist; canvas; canvas = canvas->next) {
        ClearPointer(canvas->os->window);
    }

    pointer_hidden = 0;
}
