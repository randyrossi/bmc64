/*
 * cbm2ui.c - Definition of the CBM2-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbm2ui.h"
#include "machine.h"
#include "menudefs.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "ui.h"
#include "uicbm2model.h"
#include "uiciamodel.h"
#include "uidrive.h"
#include "uiiocollisions.h"
#include "uisidcbm2.h"
#include "uitapeport.h"
#include "uiuserport.h"
#include "uivideo.h"

static TUI_MENU_CALLBACK(load_rom_file_callback)
{
    if (been_activated) {
        char *name;

        name = tui_file_selector("Load ROM file", NULL, "*", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            if (resources_set_string(param, name) < 0) {
                ui_error("Could not load ROM file '%s'", name);
            }
            lib_free(name);
        }
    }
    return NULL;
}

static tui_menu_item_def_t rom_menu_items[] = {
    { "--" },
    { "Load new Kernal ROM...",
      "Load new Kernal ROM",
      load_rom_file_callback, "KernalName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new BASIC ROM...",
      "Load new BASIC ROM",
      load_rom_file_callback, "BasicName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Character ROM...",
      "Load new Character ROM",
      load_rom_file_callback, "ChargenName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "Load new 2031 ROM...",
      "Load new 2031 ROM",
      load_rom_file_callback, "DosName2031", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2040 ROM...",
      "Load new 2040 ROM",
      load_rom_file_callback, "DosName2040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 3040 ROM...",
      "Load new 3040 ROM",
      load_rom_file_callback, "DosName3040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 4040 ROM...",
      "Load new 4040 ROM",
      load_rom_file_callback, "DosName4040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1001 ROM...",
      "Load new 1001 ROM",
      load_rom_file_callback, "DosName1001", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

int cbm2ui_init(void)
{
    tui_menu_t ui_ioextensions_submenu;

    ui_create_main_menu(0, 1, 0, 6, 1, driveieee_settings_submenu);

    tui_menu_add_separator(ui_video_submenu);

    sid_cbm2_build_menu();

    tui_menu_add(ui_sound_submenu, sid_cbm2_ui_menu_items);

    uivideo_init(ui_video_submenu, VID_CRTC, VID_NONE);

    uicbm2model_init(ui_special_submenu);
    uiciamodel_single_init(ui_special_submenu);

    ui_ioextensions_submenu = tui_menu_create("I/O extensions", 1);
    tui_menu_add_submenu(ui_special_submenu, "_I/O extensions",
                         "Configure I/O extensions",
                         ui_ioextensions_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    uiuserport_c64_cbm2_init(ui_ioextensions_submenu);

    uitapeport_init(ui_ioextensions_submenu);

    uiiocollisions_init(ui_ioextensions_submenu);

    tui_menu_add(ui_rom_submenu, rom_menu_items);

    return 0;
}

void cbm2ui_shutdown(void)
{
}
