/*
 * uiramcart.c - RamCart UI interface for MS-DOS.
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
#include "uiramcart.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(RAMCART)
TUI_MENU_DEFINE_TOGGLE(RAMCART_RO)
TUI_MENU_DEFINE_TOGGLE(RAMCARTImageWrite)
TUI_MENU_DEFINE_RADIO(RAMCARTsize)
TUI_MENU_DEFINE_FILENAME(RAMCARTfilename, "RamCart")

static TUI_MENU_CALLBACK(ramcart_size_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("RAMCARTsize", &value);
    sprintf(s, "%dKB",value);
    return s;
}

static tui_menu_item_def_t ramcart_size_submenu[] = {
    { "_64KB", NULL, radio_RAMCARTsize_callback,
      (void *)64, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_128KB", NULL, radio_RAMCARTsize_callback,
      (void *)128, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ramcart_menu_items[] = {
    { "_Enable RamCart:", "Emulate RamCart Expansion",
      toggle_RAMCART_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Readonly:", "RamCart Readonly",
      toggle_RAMCART_RO_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "RamCart _size:", "Select the size of the RamCart",
      ramcart_size_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, ramcart_size_submenu,
      "RamCart size" },
    { "_Write to RamCart image when changed:", "Write to RamCart image when the data has been changed",
      toggle_RAMCARTImageWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "RamCart _image file:", "Select the RamCart image file",
      filename_RAMCARTfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiramcart_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ramcart_submenu;

    ui_ramcart_submenu = tui_menu_create("RamCart settings", 1);

    tui_menu_add(ui_ramcart_submenu, ramcart_menu_items);

    tui_menu_add_submenu(parent_submenu, "_RamCart settings...",
                         "RamCart settings",
                         ui_ramcart_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
