/*
 * uipetreu.c - PET RAM and Expansion Unit UI interface for MS-DOS.
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
#include "uipetreu.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(PETREU)
TUI_MENU_DEFINE_RADIO(PETREUsize)
TUI_MENU_DEFINE_FILENAME(PETREUfilename, "PET REU")

static TUI_MENU_CALLBACK(petreu_size_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("PETREUsize", &value);
    sprintf(s, "%dKB", value);
    return s;
}

static tui_menu_item_def_t petreu_size_submenu[] = {
    { "_128KB", NULL, radio_PETREUsize_callback,
      (void *)128, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_512KB", NULL, radio_PETREUsize_callback,
      (void *)512, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "1_024KB", NULL, radio_PETREUsize_callback,
      (void *)1024, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2048KB", NULL, radio_PETREUsize_callback,
      (void *)2048, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t petreu_menu_items[] = {
    { "_Enable PET REU:", "Emulate PET RAM and Expansion Unit",
      toggle_PETREU_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "PET REU _size:", "Select the size of the PET REU",
      petreu_size_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, petreu_size_submenu,
      "PET REU size" },
    { "PET REU _image file:", "Select the PET REU image file",
      filename_PETREUfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uipetreu_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_petreu_submenu;

    ui_petreu_submenu = tui_menu_create("PET REU settings", 1);

    tui_menu_add(ui_petreu_submenu, petreu_menu_items);

    tui_menu_add_submenu(parent_submenu, "PET _REU settings...",
                         "PET REU settings",
                         ui_petreu_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
