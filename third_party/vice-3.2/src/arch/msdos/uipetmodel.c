/*
 * uipetmodel.c - PET model selection UI for MS-DOS.
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

#include "petmodel.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiapi.h"
#include "uipetmodel.h"

static TUI_MENU_CALLBACK(set_model_callback)
{
    if (been_activated) {
        petmodel_set((int)param);
        ui_update_menus();
    }

    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;
    return NULL;
}

static tui_menu_item_def_t pet_model_items[] = {
    { "_Not Really!",
      "Keep the current settings",
      NULL, NULL, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "_A: PET 2001",
      "Configure the emulator to emulate a PET 2001 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_2001, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_B: PET 3008",
      "Configure the emulator to emulate a PET 3008 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_3008, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_C: PET 3016",
      "Configure the emulator to emulate a PET 3016 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_3016, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_D: PET 3032",
      "Configure the emulator to emulate a PET 3032 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_3032, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_E: PET 3032B",
      "Configure the emulator to emulate a PET 3032B machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_3032B, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_F: PET 4016",
      "Configure the emulator to emulate a PET 4016 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_4016, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_G: PET 4032",
      "Configure the emulator to emulate a PET 4032 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_4032, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_H: PET 4032B",
      "Configure the emulator to emulate a PET 4032B machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_4032B, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_I: PET 8032",
      "Configure the emulator to emulate a PET 8032 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_8032, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_J: PET 8096",
      "Configure the emulator to emulate a PET 8096 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_8096, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_K: PET 8296",
      "Configure the emulator to emulate a PET 8296 machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_8296, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_L: SuperPET",
      "Configure the emulator to emulate a SuperPET machine and do a soft RESET",
      set_model_callback, (void *)PETMODEL_SUPERPET, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uipetmodel_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_petmodel_submenu;

    ui_petmodel_submenu = tui_menu_create("PET model settings", 1);

    tui_menu_add(ui_petmodel_submenu, pet_model_items);

    tui_menu_add_submenu(parent_submenu, "PET _model settings...",
                         "PET model settings",
                         ui_petmodel_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
