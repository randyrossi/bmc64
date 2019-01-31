/*
 * uipalette.c
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
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "resources.h"
#include "ui.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

#include "uipalette.h"


UI_CALLBACK(ui_load_palette)
{
    char r1[0x20], r2[0x20];
    char *filename, *title;
    ui_button_t button;
    static char *last_dir;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_PALETTE, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();
    title = lib_stralloc(_("Load custom palette"));
    filename = ui_select_file(title, NULL, 0, last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);

    lib_free(title);
    switch (button) {
        case UI_BUTTON_OK:
            strcpy(r1, UI_MENU_CB_PARAM);
            strcpy(r2, UI_MENU_CB_PARAM);
            strcat(r1, "PaletteFile");
            strcat(r2, "ExternalPalette");
            if (resources_set_string(r1, filename) < 0) {
                ui_error(_("Could not load palette file\n'%s'"), filename);
            } else {
                resources_set_int(r2, 1);
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    ui_update_menus();
    lib_free(filename);
}

void ui_select_palette(ui_window_t w, int check, char *palette, char *chip)
{
    char r1[0x20], r2[0x20];
    int val;
    const char *s;

    strcpy(r1, chip);
    strcpy(r2, chip);
    strcat(r1, "PaletteFile");
    strcat(r2, "ExternalPalette");

    resources_get_int(r2, &val);
    resources_get_string(r1, &s);
    if (!check) {
        if (palette == NULL) {
            if (val) {
                val = 0;
            } else {
                val = 1;
            }
        } else {
            val = 1;
            resources_set_string(r1, palette);
        }
        resources_set_int(r2, val);
        ui_update_menus();
    } else {
        if (palette == NULL) {
            if (val) {
                ui_menu_set_tick(w, 0);
            } else {
                ui_menu_set_tick(w, 1);
            }
        } else {
            if ((val != 0) && (!strcmp(s, palette))) {
                ui_menu_set_tick(w, 1);
            } else {
                ui_menu_set_tick(w, 0);
            }
        }
    }
}
