/*
 * uiperipherial.c
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
#include <string.h>
#include <stdlib.h>

#include "fsdevice.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiperipherial.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(FileSystemDevice8)
TUI_MENU_DEFINE_TOGGLE(FileSystemDevice9)
TUI_MENU_DEFINE_TOGGLE(FileSystemDevice10)
TUI_MENU_DEFINE_TOGGLE(FileSystemDevice11)

TUI_MENU_DEFINE_TOGGLE(FSDevice8ConvertP00)
TUI_MENU_DEFINE_TOGGLE(FSDevice9ConvertP00)
TUI_MENU_DEFINE_TOGGLE(FSDevice10ConvertP00)
TUI_MENU_DEFINE_TOGGLE(FSDevice11ConvertP00)

TUI_MENU_DEFINE_TOGGLE(FSDevice8SaveP00)
TUI_MENU_DEFINE_TOGGLE(FSDevice9SaveP00)
TUI_MENU_DEFINE_TOGGLE(FSDevice10SaveP00)
TUI_MENU_DEFINE_TOGGLE(FSDevice11SaveP00)

TUI_MENU_DEFINE_TOGGLE(FSDevice8HideCBMFiles)
TUI_MENU_DEFINE_TOGGLE(FSDevice9HideCBMFiles)
TUI_MENU_DEFINE_TOGGLE(FSDevice10HideCBMFiles)
TUI_MENU_DEFINE_TOGGLE(FSDevice11HideCBMFiles)

static TUI_MENU_CALLBACK(set_fsdevice_directory_callback)
{
    int unit = (int)param;
    const char *v;
    char *rname;

    rname = lib_msprintf("FSDevice%dDir", unit);

    if (been_activated) {
        char *path;
        unsigned int len = 255;

        resources_get_string(rname, &v);
        if (len < strlen(v) * 2) {
            len = strlen(v) * 2;
        }
        path = alloca(len + 1);
        strcpy(path, v);
        if (tui_input_string("Insert path", "Path:", path, len) != -1) {
            util_remove_spaces(path);
            fsdevice_set_directory(path, unit);
        }
    }

    resources_get_string(rname, &v);
    lib_free(rname);
    return v;
}

#define DEFINE_FSDEVICE_SUBMENU(num)                           \
static tui_menu_item_def_t fsdevice##num##_submenu[] = {       \
    { "_Directory:",                                           \
      "Specify access directory for device" #num,              \
      set_fsdevice_directory_callback, (void *)(num), 40,      \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                     \
    { "--" },                                                  \
    { "_Allow access:",                                        \
      "Allow device" #num " to access the MS-DOS file system", \
      toggle_FileSystemDevice##num##_callback, NULL, 3,        \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                     \
    { "_Convert P00 names:",                                   \
      "Handle P00 names on device " #num,                      \
      toggle_FSDevice##num##ConvertP00_callback, NULL, 3,      \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                     \
    { "_Save P00 files:",                                      \
      "Create P00 files on device " #num,                      \
      toggle_FSDevice##num##SaveP00_callback, NULL, 3,         \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                     \
    { "_Hide non-P00 files: ",                                 \
      "Display only P00 files on device " #num,                \
      toggle_FSDevice##num##HideCBMFiles_callback, NULL, 3,    \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                     \
    TUI_MENU_ITEM_DEF_LIST_END                                 \
};

DEFINE_FSDEVICE_SUBMENU(8)
DEFINE_FSDEVICE_SUBMENU(9)
DEFINE_FSDEVICE_SUBMENU(10)
DEFINE_FSDEVICE_SUBMENU(11)

static tui_menu_item_def_t peripherial_submenu[] = {
    { "Drive _8...", "Settings for drive #8", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, fsdevice8_submenu, "Drive 8 directory access" },
    { "Drive _9...", "Settings for drive #9", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, fsdevice9_submenu, "Drive 9 directory access" },
    { "Drive 1_0...", "Settings for drive #10", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, fsdevice10_submenu, "Drive 10 directory access" },
    { "Drive 1_1...", "Settings for drive #11", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, fsdevice11_submenu, "Drive 11 directory access" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiperipherial_init(struct tui_menu *parent_submenu)
{
    tui_menu_t tmp = tui_menu_create("Peripherial settings", 1);

    tui_menu_add(tmp, peripherial_submenu);
    tui_menu_add_submenu(parent_submenu,
                         "_Peripherial settings...",
                         "Options to access MS-DOS directories from within the emulator",
                         tmp, NULL, NULL, 0);
}
