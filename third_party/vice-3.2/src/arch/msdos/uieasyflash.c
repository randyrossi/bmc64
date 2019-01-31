/*
 * uieasyflash.c - EasyFlash UI interface for MS-DOS.
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

#include "cartridge.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiapi.h"
#include "uieasyflash.h"


TUI_MENU_DEFINE_TOGGLE(EasyFlashJumper)
TUI_MENU_DEFINE_TOGGLE(EasyFlashWriteCRT)
TUI_MENU_DEFINE_TOGGLE(EasyFlashOptimizeCRT)

static TUI_MENU_CALLBACK(EasyFlash_save_now_callback)
{
    if (been_activated) {

        if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
            ui_error("Can not save to EasyFlash .crt file");
        }
    }
    return NULL;
}

static tui_menu_item_def_t easyflash_menu_items[] = {
    { "Enable EasyFlash _jumper:", "EasyFlash jumper",
      toggle_EasyFlashJumper_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable saving to EasyFlash CRT on _detach:", "Save to EasyFlash crt on detach",
      toggle_EasyFlashWriteCRT_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable EasyFlash CRT optimization:", "EasyFlash CRT optimize",
      toggle_EasyFlashOptimizeCRT_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save to EasyFlash CRT now:", "Save to EasyFlash CRT now",
      EasyFlash_save_now_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uieasyflash_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_easyflash_submenu;

    ui_easyflash_submenu = tui_menu_create("EasyFlash settings", 1);

    tui_menu_add(ui_easyflash_submenu, easyflash_menu_items);

    tui_menu_add_submenu(parent_submenu, "_EasyFlash settings...",
                         "EasyFlash settings",
                         ui_easyflash_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
