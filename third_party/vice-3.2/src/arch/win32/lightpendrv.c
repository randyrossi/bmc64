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
#include <windows.h>

#include "types.h"

#include "lightpen.h"
#include "lightpendrv.h"
#include "statusbar.h"
#include "ui.h"
#include "video.h"
#include "videoarch.h"

/* ------------------------------------------------------------------ */
/* External interface */

void win32_lightpen_update(void)
{
    POINT mouse_pos;
    int x, y, on_screen;
    int buttons;
    RECT rcClient;
    video_canvas_t *lp_canvas;
    int dx, dy, cx, cy, dx9;

    if (!lightpen_enabled) {
        return;
    }

    GetCursorPos(&mouse_pos);
    on_screen = ScreenToClient(ui_active_window, &mouse_pos);

    if (on_screen) {
        buttons = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? LP_HOST_BUTTON_1 : 0;
        buttons |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? LP_HOST_BUTTON_2 : 0;
        x = mouse_pos.x;
        y = mouse_pos.y;
    } else {
        x = y = -1;
        buttons = 0;
    }

    lp_canvas = video_canvas_for_hwnd(ui_active_window);
    GetClientRect(ui_active_window, &rcClient);
    dx = lp_canvas->draw_buffer->canvas_physical_width;
    dy = lp_canvas->draw_buffer->canvas_physical_height;
    cx = rcClient.right;
    cy = rcClient.bottom;
    dx9 = video_dx9_enabled();

    /* check if coordinates are off-window/screen */
    if (x > cx || y > (cy - statusbar_get_status_height())) {
        on_screen = 0;
    }

#ifdef LP_DEBUG
    fprintf(stderr,"%s pre : x = %i, y = %i, buttons = %02x, on_screen = %i\n", __func__, x, y, buttons, on_screen);
#endif

    if (on_screen) {
        if (!dx9) {
            /* no dx9 */
            x -= (int)((cx - dx) / 2);
            y -= (int)((cy - (dy + statusbar_get_status_height())) / 2);
        } else {
            /* dx9 */
            x = (int)(x * dx / cx);
            y = (int)(y * dy / (cy - statusbar_get_status_height()));
        }

        /* scale x size */
        x /= lp_canvas->videoconfig->scalex;

        /* scale y size */
        y /= lp_canvas->videoconfig->scaley;
    } else {
        x = y = -1;
    }

#ifdef LP_DEBUG
    fprintf(stderr,"%s post: x = %i, y = %i\n", __func__, x, y);
#endif

    lightpen_update(video_canvas_nr_for_hwnd(ui_active_window), x, y, (int)buttons);
}
