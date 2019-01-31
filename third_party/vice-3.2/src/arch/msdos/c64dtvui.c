/*
 * c64dtvui.c - Definition of the C64DTV-specific part of the UI.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c64ui.h"
#include "lib.h"
#include "machine.h"
#include "menudefs.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "types.h"
#include "ui.h"
#include "uic64dtv.h"
#include "uic64dtvmodel.h"
#include "uidrive.h"
#include "uisiddtv.h"
#include "uivideo.h"

TUI_MENU_DEFINE_TOGGLE(ps2mouse)

static tui_menu_item_def_t ioextenstions_menu_items[] = {
    { "PS/2 mouse emulation",
      "Emulate a PS/2 mouse",
      toggle_ps2mouse_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

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
    { "Load new 1540 ROM...",
      "Load new 1540 ROM",
      load_rom_file_callback, "DosName1540", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1541 ROM...",
      "Load new 1541 ROM",
      load_rom_file_callback, "DosName1541", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1541-II ROM...",
      "Load new 1541-II ROM",
      load_rom_file_callback, "DosName1541ii", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1570 ROM...",
      "Load new 1570 ROM",
      load_rom_file_callback, "DosName1570", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1571 ROM...",
      "Load new 1571 ROM",
      load_rom_file_callback, "DosName1571", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1581 ROM...",
      "Load new 1581 ROM",
      load_rom_file_callback, "DosName1581", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2000 ROM...",
      "Load new 2000 ROM",
      load_rom_file_callback, "DosName2000", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 4000 ROM...",
      "Load new 4000 ROM",
      load_rom_file_callback, "DosName4000", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

int c64dtvui_init(void)
{
    tui_menu_t ui_ioextensions_submenu;

    ui_create_main_menu(1, 1, 1, 0x1c, 1, drivec64dtv_settings_submenu);

    tui_menu_add_separator(ui_special_submenu);

    uic64dtvmodel_init(ui_special_submenu);

    uic64dtv_init(ui_special_submenu);
    tui_menu_add_separator(ui_special_submenu);

    ui_ioextensions_submenu = tui_menu_create("I/O extensions", 1);
    tui_menu_add(ui_ioextensions_submenu, ioextenstions_menu_items);
    tui_menu_add_submenu(ui_special_submenu, "_I/O extensions...",
                         "Configure I/O extensions",
                         ui_ioextensions_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    tui_menu_add_separator(ui_video_submenu);

    uivideo_init(ui_video_submenu, VID_VICII, VID_NONE);

    siddtv_build_menu();

    tui_menu_add(ui_sound_submenu, siddtv_ui_menu_items);
    tui_menu_add(ui_rom_submenu, rom_menu_items);

    return 0;
}

void c64dtvui_shutdown(void)
{
}
