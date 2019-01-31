/*
 * tuiview.c - A (very) simple text viewer.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <conio.h>
#include <keys.h>
#include <pc.h>
#include <stdio.h>
#include <string.h>

#include "tui.h"
#include "tui_backend.h"
#include "util.h"

void tui_view_text(int width, int height, const char *title, const char *text)
{
    const char *p;
    int x, y, i;
    int need_update = 1;
    tui_area_t backing_store = NULL;

    x = CENTER_X(width);
    y = CENTER_Y(height);

    tui_display_window(x, y, width, height, MESSAGE_BORDER, MESSAGE_BACK, title, &backing_store);
    
    tui_set_attr(MESSAGE_BORDER, MESSAGE_BACK, 0);
    tui_put_char(x + width - 1, y + 1, 0x1e); /* Up arrow */
    tui_put_char(x + width - 1, y + height - 2, 0x1f); /* Down arrow */
    for (i = y + 2; i < y + height - 2; i++) {
        tui_put_char(x + width - 1, i, 0xb1);
    }

    tui_set_attr(MESSAGE_FORE, MESSAGE_BACK, 0);
    p = text;
    while (1) {
        int key;

        if (need_update) {
            tui_display_text(x + 2, y + 1, width - 4, height - 2, p);
            need_update = 0;
        }
	
        key = getkey();

        switch (key) {
            case K_Escape:
            case K_Return:
                tui_area_put(backing_store, x, y);
                tui_area_free(backing_store);
                return;
            case K_Up:
            case K_Left:
                p = util_find_prev_line(text, p);
                need_update = 1;
                break;
            case K_Down:
            case K_Right:
                p = util_find_next_line(p);
                need_update = 1;
                break;
            case K_PageDown:
                for (i = 0; i < height - 3; i++) {
                    p = util_find_next_line(p);
                }
                need_update = 1;
                break;
            case K_PageUp:
                for (i = 0; i < height - 3; i++) {
                    p = util_find_prev_line(text, p);	
                }
                need_update = 1;
                break;
        }
    }
}
