/*
 * uivic20model.c - VIC20 model selection UI for MS-DOS.
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
#include "uiapi.h"
#include "uivic20model.h"
#include "vic20model.h"

static TUI_MENU_CALLBACK(set_model_callback)
{
    if (been_activated) {
        vic20model_set((int)param);
        ui_update_menus();
    }

    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;
    return NULL;
}


static tui_menu_item_def_t vic20_model_items[] = {
    { "_Not Really!",
      "Keep the current settings",
      NULL, NULL, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "_A: VIC20 PAL",
      "Configure the emulator to emulate a VIC20 PAL machine and do a soft RESET",
      set_model_callback, (void *)VIC20MODEL_VIC20_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_B: VIC20 NTSC",
      "Configure the emulator to emulate a VIC20 NTSC machine and do a soft RESET",
      set_model_callback, (void *)VIC20MODEL_VIC20_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_C: VIC21",
      "Configure the emulator to emulate a VIC21 machine and do a soft RESET",
      set_model_callback, (void *)VIC20MODEL_VIC21, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uivic20model_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_vic20model_submenu;

    ui_vic20model_submenu = tui_menu_create("VIC20 model settings", 1);

    tui_menu_add(ui_vic20model_submenu, vic20_model_items);

    tui_menu_add_submenu(parent_submenu, "VIC20 _model settings...",
                         "VIC20 model settings",
                         ui_vic20model_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
