/*
 * uifliplist.c - Implementation of the fliplist dialogs.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#include "archdep.h"
#include "fliplist.h"
#include "intl.h"
#include "lib.h"
#include "res.h"
#include "system.h"
#include "translate.h"
#include "uiapi.h"
#include "uilib.h"
#include "util.h"

#include "uifliplist.h"


static void uifliplist_load_dialog(HWND hwnd)
{
    TCHAR *st_name;

    if ((st_name = uilib_select_file(hwnd, translate_text(IDS_LOAD_FLIP_LIST_FILE), UILIB_FILTER_FLIPLIST, UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DEFAULT)) != NULL) {
        char *name;

        name = system_wcstombs_alloc(st_name);
        if (fliplist_load_list((unsigned int)-1, name, 0) != 0) {
            ui_error(translate_text(IDS_CANNOT_READ_FLIP_LIST));
        }
        system_wcstombs_free(name);
        lib_free(st_name);
    }
}

static void uifliplist_save_dialog(HWND hwnd)
{
    TCHAR *st_name;

    if ((st_name = uilib_select_file(hwnd, translate_text(IDS_SAVE_FLIP_LIST_FILE), UILIB_FILTER_FLIPLIST, UILIB_SELECTOR_TYPE_FILE_SAVE, UILIB_SELECTOR_STYLE_DEFAULT)) != NULL) {
        char *name;

        name = system_wcstombs_alloc(st_name);

        util_add_extension(&name, "vfl");

        if (fliplist_save_list((unsigned int)-1, name) != 0) {
            ui_error(translate_text(IDS_CANNOT_WRITE_FLIP_LIST));
        }
        system_wcstombs_free(name);
        lib_free(st_name);
    }
}

void uifliplist_save_settings(void)
{
    char *fname = archdep_default_fliplist_file_name();

    fliplist_save_list((unsigned int)-1, fname);
    lib_free(fname);
}

void uifliplist_command(HWND hwnd, WPARAM wparam)
{
    switch (wparam & 0xffff) {
        case IDM_FLIP_ADD:
            fliplist_add_image(8);
            break;
        case IDM_FLIP_REMOVE:
            fliplist_remove(8, NULL);
            break;
        case IDM_FLIP_NEXT:
            fliplist_attach_head(8, 1);
            break;
        case IDM_FLIP_PREVIOUS:
            fliplist_attach_head(8, 0);
            break;
        case IDM_FLIP_LOAD:
            uifliplist_load_dialog(hwnd);
            break;
        case IDM_FLIP_SAVE:
            uifliplist_save_dialog(hwnd);
            break;
    }
}
