/*
 * uidqbb.c - Double Quick Brown Box UI interface for MS-DOS.
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
#include "uidqbb.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(DQBB)
TUI_MENU_DEFINE_TOGGLE(DQBBImageWrite)
TUI_MENU_DEFINE_FILENAME(DQBBfilename, "DQBB")

static tui_menu_item_def_t dqbb_menu_items[] = {
    { "_Enable DQBB:", "Emulate Double Quick Brown Box",
      toggle_DQBB_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Write to DQBB image when changed:", "Write to DQBB image when the data has been changed",
      toggle_DQBBImageWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "DQBB _image file:", "Select the DQBB image file",
      filename_DQBBfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uidqbb_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_dqbb_submenu;

    ui_dqbb_submenu = tui_menu_create("Double Quick Brown Box settings", 1);

    tui_menu_add(ui_dqbb_submenu, dqbb_menu_items);

    tui_menu_add_submenu(parent_submenu, "_Double Quick Brown Box settings...",
                         "Double Quick Brown Box settings",
                         ui_dqbb_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
