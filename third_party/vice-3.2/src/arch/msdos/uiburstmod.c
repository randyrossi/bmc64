/*
 * uiburstmod.c - C64 Burst Modification UI interface for MS-DOS.
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
#include "uiburstmod.h"

TUI_MENU_DEFINE_RADIO(BurstMod)

static TUI_MENU_CALLBACK(burstmod_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("BurstMod", &value);
    switch (value) {
        case 0:
        default:
            s = "None";
            break;
        case 1:
            s = "CIA-1";
            break;
        case 2:
            s = "CIA-2";
            break;
    }
    return s;
}

static tui_menu_item_def_t burstmod_submenu[] = {
    { "None", NULL, radio_BurstMod_callback,
      (void *)0, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "CIA-1", NULL, radio_BurstMod_callback,
      (void *)1, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "CIA-2", NULL, radio_BurstMod_callback,
      (void *)2, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t burstmod_menu_items[] = {
    { "Burst Modification:", "Select the burst modification",
      burstmod_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, burstmod_submenu,
      "Burst Modification" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiburstmod_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_burstmod_submenu;

    ui_burstmod_submenu = tui_menu_create("Burst Modification settings", 1);

    tui_menu_add(ui_burstmod_submenu, burstmod_menu_items);

    tui_menu_add_submenu(parent_submenu, "Burst Modification settings...",
                         "Burst Modification settings",
                         ui_burstmod_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
