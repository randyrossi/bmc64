/*
 * lightpendrv.c - Lightpen driver for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "vice_sdl.h"

#include "machine.h"
#include "lightpen.h"
#include "lightpendrv.h"
#include "videoarch.h"

/* ------------------------------------------------------------------ */
/* External interface */

void sdl_lightpen_update(void)
{
    int x, y, screen_num;
    unsigned int buttons;

    if (!lightpen_enabled) {
        return;
    }

    if (!sdl_ui_get_mouse_state(&x, &y, &buttons)) {
        x = y = -1;
        buttons = 0;
    }

#ifdef SDL_DEBUG
    fprintf(stderr, "%s : x = %i, y = %i, buttons = %02x\n", __func__, x, y, buttons);
#endif

    screen_num = sdl_active_canvas_num;

    /* HACK: In x128, the VDC window is 0, but sdl/video.c uses the canvas
       init order (which for some reason is the reverse) for enumeration. */
    if (machine_class == VICE_MACHINE_C128) {
        screen_num ^= 1;
    }

    lightpen_update(screen_num, x, y, (int)buttons);
}
