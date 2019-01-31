/*
 * uigmod2.c - GMod2 Replay UI interface for MS-DOS.
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
#include "uigmod2.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(GMOD2EEPROMRW)
TUI_MENU_DEFINE_TOGGLE(GMod2FlashWrite)
TUI_MENU_DEFINE_FILENAME(GMod2EEPROMImage, "GMod2 EEPROM")

static tui_menu_item_def_t gmod2_menu_items[] = {
    { "GMod2 EEPROM read/_write:", "Enable GMod2 EEPROM read/write",
      toggle_GMOD2EEPROMRW_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save GMod2 _EEPROM when changed:", "Enable GMod2 EEPROM save when changed",
      toggle_GMod2FlashWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "GMod2 EE_PROM file:", "Select the GMod2 EEPROM file",
      filename_GMod2EEPROMImage_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uigmod2_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_gmod2_submenu;

    ui_gmod2_submenu = tui_menu_create("GMod2 settings", 1);

    tui_menu_add(ui_gmod2_submenu, gmod2_menu_items);

    tui_menu_add_submenu(parent_submenu, "GMod2 settings...",
                         "GMod2 settings",
                         ui_gmod2_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
