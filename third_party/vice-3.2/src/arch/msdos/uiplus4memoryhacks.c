/*
 * uiplus4memoryhacks.c - Plus4 CSORY 256K / HANNES 256K/1024K/4096K memory hacks
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

#include "plus4memhacks.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uic64memoryhacks.h"

TUI_MENU_DEFINE_RADIO(MemoryHack)

static TUI_MENU_CALLBACK(memory_hack_device_submenu_callback)
{
    char *s;
    int value;

    resources_get_int("MemoryHack", &value);
    switch (value) {
        default:
        case MEMORY_HACK_NONE:
            s = "None";
            break;
        case MEMORY_HACK_C256K:
            s = "CSORY 256K";
            break;
        case MEMORY_HACK_H256K:
            s = "HANNES 256K";
            break;
        case MEMORY_HACK_H1024K:
            s = "HANNES 1024K";
            break;
        case MEMORY_HACK_H4096K:
            s = "HANNES 4096K";
            break;
    }
    return s;
}

tui_menu_item_def_t memory_hacks_menu_items[] = {
    { "None", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_NONE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "CSORY 256K", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_C256K, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "HANNES 256K", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_H256K, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "HANNES 1024K", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_H1024K, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "HANNES 4096K", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_H4096K, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiplus4_memory_hacks_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_memory_hacks_submenu;

    ui_memory_hacks_submenu = tui_menu_create("Memory Expansion Hacks settings", 1);

    tui_menu_add(ui_memory_hacks_submenu, memory_hacks_menu_items);

    tui_menu_add_submenu(parent_submenu, "Memory Expansion Hacks settings...",
                         "Memory Expansion Hacks settings",
                         ui_memory_hacks_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
