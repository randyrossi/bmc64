/*
 * uirrnetmk3.c - RRNET MK3 UI interface for MS-DOS.
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

#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uirrnetmk3.h"

TUI_MENU_DEFINE_TOGGLE(RRNETMK3_flashjumper)
TUI_MENU_DEFINE_TOGGLE(RRNETMK3_bios_write)

static tui_menu_item_def_t rrnetmk3_menu_items[] = {
    { "RRNET MK3 flash jumper:", "Enable RRNET MK3 flash jumper",
      toggle_RRNETMK3_flashjumper_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save RRNET MK3 BIOS when changed:", "Enable RRNET MK3 BIOS save when changed",
      toggle_RRNETMK3_bios_write_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uirrnetmk3_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_rrnetmk3_submenu;

    ui_rrnetmk3_submenu = tui_menu_create("RRNET MK3 settings", 1);

    tui_menu_add(ui_rrnetmk3_submenu, rrnetmk3_menu_items);

    tui_menu_add_submenu(parent_submenu, "RRNET MK3 settings...",
                         "RRNET MK3 settings",
                         ui_rrnetmk3_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
