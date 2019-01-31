/*
 * uifliplist.c
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
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

#include "fliplist.h"
#include "uifliplist.h"
#include "intl.h"
#include "translate.h"

void uifliplist_load_dialog(video_canvas_t *canvas)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_LOAD_FLIP_LIST_FILE), "#?.vfl", canvas);

    if (fname != NULL && *fname != '\0') {
        if (fliplist_load_list((unsigned int)-1, fname, 0) != 0) {
            ui_error(translate_text(IDS_CANNOT_READ_FLIP_LIST));
        }
    }
}

void uifliplist_save_dialog(video_canvas_t *canvas)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SAVE_FLIP_LIST_FILE), "#?.vfl", canvas);

    if (fname != NULL && *fname != '\0') {
        if (strcasecmp(fname+(strlen(fname)-4),".vfl")) {
            strcat(fname, ".vfl");
        }
        if (fliplist_save_list((unsigned int)-1, fname) != 0) {
            ui_error(translate_text(IDS_CANNOT_WRITE_FLIP_LIST));
        }
    }
}
