/*
 * uiscpu64.c - SCPU64 UI interface for MS-DOS.
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
#include "uiscpu64.h"

TUI_MENU_DEFINE_TOGGLE(JiffySwitch)
TUI_MENU_DEFINE_TOGGLE(SpeedSwitch)
TUI_MENU_DEFINE_RADIO(SIMMSize)

static TUI_MENU_CALLBACK(scpu64_simm_size_submenu_callback)
{
    char *retval;
    int value;

    resources_get_int("SIMMSize", &value);
    switch (value) {
        case 0:
        default:
            retval = "0 MB";
            break;
        case 1:
            retval = "1 MB";
            break;
        case 4:
            retval = "4 MB";
            break;
        case 8:
            retval = "8 MB";
            break;
        case 16:
            retval = "16 MB";
            break;
    }
    return retval;
}

static tui_menu_item_def_t scpu64_simm_size_submenu[] = {
    { "0 MB", NULL, radio_SIMMSize_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "1 MB", NULL, radio_SIMMSize_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "4 MB", NULL, radio_SIMMSize_callback,
      (void *)4, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "8 MB", NULL, radio_SIMMSize_callback,
      (void *)8, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "16 MB", NULL, radio_SIMMSize_callback,
      (void *)16, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t scpu64_menu_items[] = {
    { "SuperCPU64 SIMM size:", "Select the size of the SIMM",
      scpu64_simm_size_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, scpu64_simm_size_submenu,
      "SuperCPU SIMM size" },
    { "_Enable SuperCPU64 jiffy switch:", "Enable the SuperCPU64 jiffy switch",
      toggle_JiffySwitch_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Enable SuperCPU64 speed switch:", "Enable the SuperCPU64 speed switch",
      toggle_SpeedSwitch_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiscpu64_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_scpu64_submenu;

    ui_scpu64_submenu = tui_menu_create("SuperCPU64 settings", 1);

    tui_menu_add(ui_scpu64_submenu, scpu64_menu_items);

    tui_menu_add_submenu(parent_submenu, "SuperCPU64 settings...",
                         "SuperCPU64 settings",
                         ui_scpu64_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
