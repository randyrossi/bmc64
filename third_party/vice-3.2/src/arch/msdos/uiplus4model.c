/*
 * uiplus4model.c - PLUS4 model selection UI for MS-DOS.
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

#include "plus4model.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiapi.h"
#include "uiplus4model.h"

static TUI_MENU_CALLBACK(set_model_callback)
{
    if (been_activated) {
        plus4model_set((int)param);
        ui_update_menus();
    }

    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;
    return NULL;
}


static tui_menu_item_def_t plus4_model_items[] = {
    { "_Not Really!",
      "Keep the current settings",
      NULL, NULL, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "_A: C16 PAL",
      "Configure the emulator to emulate a C16 PAL machine and do a soft RESET",
      set_model_callback, (void *)PLUS4MODEL_C16_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_B: C16 NTSC",
      "Configure the emulator to emulate a C16 NTSC machine and do a soft RESET",
      set_model_callback, (void *)PLUS4MODEL_C16_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_C: PLUS4 PAL",
      "Configure the emulator to emulate a Plus4 PAL machine and do a soft RESET",
      set_model_callback, (void *)PLUS4MODEL_PLUS4_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_D: PLUS4 NTSC",
      "Configure the emulator to emulate a Plus4 NTSC machine and do a soft RESET",
      set_model_callback, (void *)PLUS4MODEL_PLUS4_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_E: V364 NTSC",
      "Configure the emulator to emulate a V364 NTSC machine and do a soft RESET",
      set_model_callback, (void *)PLUS4MODEL_V364_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_F: C232 NTSC",
      "Configure the emulator to emulate a C232 NTSC machine and do a soft RESET",
      set_model_callback, (void *)PLUS4MODEL_232_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiplus4model_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_plus4model_submenu;

    ui_plus4model_submenu = tui_menu_create("Plus4 model settings", 1);

    tui_menu_add(ui_plus4model_submenu, plus4_model_items);

    tui_menu_add_submenu(parent_submenu, "Plus4 _model settings...",
                         "Plus4 model settings",
                         ui_plus4model_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
