/*
 * uiiocollisions.c - I/O collision settings UI interface for MS-DOS.
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

#include "cartio.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiiocollisions.h"

TUI_MENU_DEFINE_RADIO(IOCollisionHandling)

static TUI_MENU_CALLBACK(iocollisions_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("IOCollisionHandling", &value);
    switch (value) {
        case IO_COLLISION_METHOD_DETACH_ALL:
        default:
            s = "Detach all involved carts";
            break;
        case IO_COLLISION_METHOD_DETACH_LAST:
            s = "Detach last inserted cart";
            break;
        case IO_COLLISION_METHOD_AND_WIRES:
            s = "'AND' the wires";
            break;
    }
    return s;
}

static tui_menu_item_def_t iocollisions_submenu[] = {
    { "Detach all involved carts", NULL, radio_IOCollisionHandling_callback,
      (void *)IO_COLLISION_METHOD_DETACH_ALL, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Detach last inserted cart", NULL, radio_IOCollisionHandling_callback,
      (void *)IO_COLLISION_METHOD_DETACH_LAST, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "'AND' the wires", NULL, radio_IOCollisionHandling_callback,
      (void *)IO_COLLISION_METHOD_AND_WIRES, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t iocollisions_menu_items[] = {
    { "I/O collision handling:", "Select how to handle I/O collisions",
      iocollisions_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, iocollisions_submenu,
      "I/O collision handling" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiiocollisions_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_iocollisions_submenu;

    ui_iocollisions_submenu = tui_menu_create("I/O collision settings", 1);

    tui_menu_add(ui_iocollisions_submenu, iocollisions_menu_items);

    tui_menu_add_submenu(parent_submenu, "I/O collision settings...",
                         "I/O collisions settings",
                         ui_iocollisions_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
