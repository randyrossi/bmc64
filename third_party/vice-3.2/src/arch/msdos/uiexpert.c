/*
 * uiexpert.c - Exper cartridge UI interface for MS-DOS.
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
#include <string.h>

#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uiexpert.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(ExpertCartridgeEnabled)
TUI_MENU_DEFINE_TOGGLE(ExpertImageWrite)
TUI_MENU_DEFINE_RADIO(ExpertCartridgeMode)
TUI_MENU_DEFINE_FILENAME(Expertfilename, "Expert Cartridge")

static TUI_MENU_CALLBACK(expert_mode_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("ExpertCartridgeMode", &value);
    switch (value) {
        default:
        case 0:
            s = "Off";
            break;
        case 1:
            s = "Prg";
            break;
        case 2:
            s = "On";
            break;
    }
    return s;
}

static tui_menu_item_def_t expert_mode_submenu[] = {
    { "O_ff", NULL, radio_ExpertCartridgeMode_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Prg", NULL, radio_ExpertCartridgeMode_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "O_n", NULL, radio_ExpertCartridgeMode_callback,
      (void *)2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t expert_menu_items[] = {
    { "_Enable Expert Cartridge:", "Emulate the Expert Cartridge",
      toggle_ExpertCartridgeEnabled_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Expert Cartridge _mode:", "Select the mode of the expert cartridge",
      expert_mode_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, expert_mode_submenu,
      "Expert Cartridge mode" },
    { "_Write to Expert Cartridge image when changed:", "Write to Expert Cartridge image when the data has been changed",
      toggle_ExpertImageWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Expert Cartridge _image file:", "Select the Expert Cartridge image file",
      filename_Expertfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiexpert_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_expert_submenu;

    ui_expert_submenu = tui_menu_create("Expert Cartridge settings", 1);

    tui_menu_add(ui_expert_submenu, expert_menu_items);

    tui_menu_add_submenu(parent_submenu, "_Expert Cartridge settings...",
                         "Expert Cartridge settings",
                         ui_expert_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
