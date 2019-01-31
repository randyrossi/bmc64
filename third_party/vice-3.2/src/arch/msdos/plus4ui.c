/*
 * plus4ui.c - Definition of the PLUS4-specific part of the UI.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "lib.h"
#include "menudefs.h"
#include "plus4ui.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "ui.h"
#include "uidrive.h"
#include "uiiocollisions.h"
#include "uiplus4cart.h"
#include "uiplus4memoryhacks.h"
#include "uiplus4model.h"
#include "uisidcbm2.h"
#include "uisidcart.h"
#include "uitapeport.h"
#include "uiuserport.h"
#include "uiv364speech.h"
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
    { "Load new Function LO ROM...",
      "Load new Function LO ROM",
      load_rom_file_callback, "FunctionLowName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Function HI ROM...",
      "Load new Function HI ROM",
      load_rom_file_callback, "FunctionHighName", 0,
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
    { "Load new 1551 ROM...",
      "Load new 1551 ROM",
      load_rom_file_callback, "DosName1551", 0,
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

int plus4ui_init(void)
{
    ui_create_main_menu(0, 1, 0, 0x1f, 1, driveplus4_settings_submenu);

    tui_menu_add_separator(ui_special_submenu);

    uiplus4model_init(ui_special_submenu);

    uiplus4_memory_hacks_init(ui_special_submenu);

    uiuserport_plus4_init(ui_special_submenu);

    uitapeport_init(ui_special_submenu);

    uiiocollisions_init(ui_special_submenu);

    tui_menu_add_separator(ui_video_submenu);

    uivideo_init(ui_video_submenu, VID_TED, VID_NONE);

    uisidcart_plus4_init(ui_sound_submenu, "$FD40", "$FE80", "PLUS4", 0xfd40, 0xfe80);
    uiv364speech_init(ui_sound_submenu);

    tui_menu_add(ui_rom_submenu, rom_menu_items);

    uiplus4cart_init(NULL);

    return 0;
}

void plus4ui_shutdown(void)
{
}
