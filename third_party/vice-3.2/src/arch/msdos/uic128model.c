/*
 * uic128model.c - C128 model selection UI for MS-DOS.
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

#include "c128model.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiapi.h"
#include "uic128model.h"

static TUI_MENU_CALLBACK(set_model_callback)
{
    if (been_activated) {
        c128model_set((int)param);
        ui_update_menus();
    }

    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;
    return NULL;
}

static tui_menu_item_def_t c128_model_items[] = {
    { "_Not Really!",
      "Keep the current settings",
      NULL, NULL, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "_A: C128 PAL",
      "Configure the emulator to emulate a C128 PAL machine and do a soft RESET",
      set_model_callback, (void *)C128MODEL_C128_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_B: C128DCR PAL",
      "Configure the emulator to emulate a C128DCR PAL machine and do a soft RESET",
      set_model_callback, (void *)C128MODEL_C128DCR_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_C: C128 NTSC",
      "Configure the emulator to emulate a C128 NTSC machine and do a soft RESET",
      set_model_callback, (void *)C128MODEL_C128_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_D: C128DCR NTSC",
      "Configure the emulator to emulate a C128DCR NTSC machine and do a soft RESET",
      set_model_callback, (void *)C128MODEL_C128DCR_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uic128model_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_c128model_submenu;

    ui_c128model_submenu = tui_menu_create("C128 model settings", 1);

    tui_menu_add(ui_c128model_submenu, c128_model_items);

    tui_menu_add_submenu(parent_submenu, "C128 _model settings...",
                         "C128 model settings",
                         ui_c128model_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
