/*
 * uidigimax.c - DigiMAX cartridge UI interface for MS-DOS.
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
#include "uidigimax.h"

TUI_MENU_DEFINE_TOGGLE(DIGIMAX)
TUI_MENU_DEFINE_RADIO(DIGIMAXbase)

static TUI_MENU_CALLBACK(digimax_base_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("DIGIMAXbase", &value);
    if (value == 0xdd00) {
        sprintf(s, "Userport");
    } else {
        sprintf(s, "$%X", value);
    }
    return s;
}

static tui_menu_item_def_t digimax_c64_base_submenu[] = {
    { "$DE00", NULL, radio_DIGIMAXbase_callback,
      (void *)0xde00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE20", NULL, radio_DIGIMAXbase_callback,
      (void *)0xde20, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE40", NULL, radio_DIGIMAXbase_callback,
      (void *)0xde40, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE60", NULL, radio_DIGIMAXbase_callback,
      (void *)0xde60, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE80", NULL, radio_DIGIMAXbase_callback,
      (void *)0xde80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEA0", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdea0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEC0", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdec0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEE0", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdee0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF00", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdf00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF20", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdf20, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF40", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdf40, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF60", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdf60, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF80", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdf80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFA0", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdfa0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFC0", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdfc0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFE0", NULL, radio_DIGIMAXbase_callback,
      (void *)0xdfe0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t digimax_vic20_base_submenu[] = {
    { "$9800", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9800, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9820", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9820, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9840", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9840, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9860", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9860, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9880", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9880, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98A0", NULL, radio_DIGIMAXbase_callback,
      (void *)0x98a0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98C0", NULL, radio_DIGIMAXbase_callback,
      (void *)0x98c0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98E0", NULL, radio_DIGIMAXbase_callback,
      (void *)0x98e0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C00", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9c00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C20", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9c20, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C40", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9c40, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C60", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9c60, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C80", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9c80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CA0", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9ca0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CC0", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9cc0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CE0", NULL, radio_DIGIMAXbase_callback,
      (void *)0x9ce0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t digimax_c64_menu_items[] = {
    { "_Enable DigiMAX:", "Emulate DigiMAX Cartridge",
      toggle_DIGIMAX_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "DigiMAX _base:", "Select the base of the DigiMAX Cartridge",
      digimax_base_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, digimax_c64_base_submenu,
      "DigiMAX base" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t digimax_vic20_menu_items[] = {
    { "_Enable DigiMAX:", "Emulate DigiMAX Cartridge",
      toggle_DIGIMAX_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "DigiMAX _base:", "Select the base of the DigiMAX Cartridge",
      digimax_base_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, digimax_vic20_base_submenu,
      "DigiMAX base" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uidigimax_c64_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_digimax_submenu;

    ui_digimax_submenu = tui_menu_create("DigiMAX settings", 1);

    tui_menu_add(ui_digimax_submenu, digimax_c64_menu_items);

    tui_menu_add_submenu(parent_submenu, "_DigiMAX settings...",
                         "DigiMAX settings",
                         ui_digimax_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uidigimax_vic20_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_digimax_submenu;

    ui_digimax_submenu = tui_menu_create("DigiMAX settings (MasC=uerade)", 1);

    tui_menu_add(ui_digimax_submenu, digimax_vic20_menu_items);

    tui_menu_add_submenu(parent_submenu, "_DigiMAX settings (MasC=uerade)...",
                         "DigiMAX settings",
                         ui_digimax_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
