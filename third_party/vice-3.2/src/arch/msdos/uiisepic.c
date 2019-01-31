/*
 * uiisepic.c - ISEPIC UI interface for MS-DOS.
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
#include "uiisepic.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(IsepicCartridgeEnabled)
TUI_MENU_DEFINE_TOGGLE(IsepicSwitch)
TUI_MENU_DEFINE_TOGGLE(IsepicImageWrite)
TUI_MENU_DEFINE_FILENAME(Isepicfilename, "ISEPIC")

static tui_menu_item_def_t isepic_menu_items[] = {
    { "_Enable ISEPIC:", "Emulate ISEPIC cartridge",
      toggle_IsepicCartridgeEnabled_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable ISEPIC _Switch:", "ISEPIC switch on/off",
      toggle_IsepicSwitch_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Write to ISEPIC image when changed:", "Write to ISEPIC image when the data has been changed",
      toggle_IsepicImageWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "ISEPIC _image file:", "Select the ISEPIC image file",
      filename_Isepicfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiisepic_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_isepic_submenu;

    ui_isepic_submenu = tui_menu_create("ISEPIC settings", 1);

    tui_menu_add(ui_isepic_submenu, isepic_menu_items);

    tui_menu_add_submenu(parent_submenu, "_ISEPIC settings...",
                         "ISEPIC settings",
                         ui_isepic_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
