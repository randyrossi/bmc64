/*
 * uiplus4cart.c
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

#include "cartridge.h"
#include "keyboard.h"
#include "lib.h"
#include "menudefs.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "ui.h"
#include "uiplus4cart.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(CartridgeReset)

static TUI_MENU_CALLBACK(attach_cartridge_callback)
{
    if (been_activated) {
        char *name;
        int type = (int)param;

        name = tui_file_selector("Attach cartridge image", NULL, "*", NULL, NULL, NULL, NULL);
        if (name != NULL && cartridge_attach_image(type, name) < 0) {
            tui_error("Invalid cartridge image.");
        }
        ui_update_menus();
        lib_free(name);
    }

    return NULL;
}

static tui_menu_item_def_t attach_cartridge_submenu_items[] = {
    { "Smart attach cartridge image...",
      "Smart attach a cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_PLUS4_DETECT, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "Attach C0 low image...",
      "Attach C0 low image",
      attach_cartridge_callback, (void *)CARTRIDGE_PLUS4_16KB_C0LO, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach C0 high image...",
      "Attach C0 high image",
      attach_cartridge_callback, (void *)CARTRIDGE_PLUS4_16KB_C0HI, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach C1 low image...",
      "Attach C1 low image",
      attach_cartridge_callback, (void *)CARTRIDGE_PLUS4_16KB_C1LO, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach C1 high image...",
      "Attach C1 high image",
      attach_cartridge_callback, (void *)CARTRIDGE_PLUS4_16KB_C1HI, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach C2 low image...",
      "Attach C2 low image",
      attach_cartridge_callback, (void *)CARTRIDGE_PLUS4_16KB_C2LO, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach C2 high image...",
      "Attach C2 high image",
      attach_cartridge_callback, (void *)CARTRIDGE_PLUS4_16KB_C2HI, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "Reset on cart change",
      "Reset the machine when a cart is inserted or detached",
      toggle_CartridgeReset_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t attach_cartridge_menu_items[] = {
    { "--" },
    { "_Cartridge:",
      "Attach a cartridge image",
      NULL, NULL, 30,
      TUI_MENU_BEH_CONTINUE, attach_cartridge_submenu_items,
      "Attach cartridge" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(detach_cartridge_callback)
{
    const char *s;

    if (been_activated) {
        cartridge_detach_image(-1);
    }
    return "";
}

static tui_menu_item_def_t detach_cartridge_menu_items[] = {
    { "--" },
    { "_Cartridge:",
      "Detach attached cartridge image",
      detach_cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiplus4cart_init(struct tui_menu *parent_submenu)
{
    tui_menu_add(ui_attach_submenu, attach_cartridge_menu_items);
    tui_menu_add(ui_detach_submenu, detach_cartridge_menu_items);
}
