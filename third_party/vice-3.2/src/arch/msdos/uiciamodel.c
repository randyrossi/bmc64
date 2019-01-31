/*
 * uiciamodel.c - CIA model selection UI for MS-DOS.
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
#include "uiciamodel.h"

TUI_MENU_DEFINE_RADIO(CIA1Model)
TUI_MENU_DEFINE_RADIO(CIA2Model)

static char *get_cia_model(int value)
{
    char *retval;

    switch (value) {
        default:
        case 0:
            retval = "6526 (old)";
            break;
        case 1:
            retval = "8521 (new)";
            break;
    }
    return retval;
}

static TUI_MENU_CALLBACK(cia1_model_submenu_callback)
{
    int value;

    resources_get_int("CIA1Model", &value);

    return get_cia_model(value);
}

static tui_menu_item_def_t cia1_model_submenu[] = {
    { "6526 (old)", NULL, radio_CIA1Model_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "8521 (new)", NULL, radio_CIA1Model_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(cia2_model_submenu_callback)
{
    int value;

    resources_get_int("CIA2Model", &value);

    return get_cia_model(value);
}

static tui_menu_item_def_t cia2_model_submenu[] = {
    { "6526 (old)", NULL, radio_CIA2Model_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "8521 (new)", NULL, radio_CIA2Model_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t cia_double_model_menu_items[] = {
    { "CIA _1 model:", "Select the CIA #1 model",
      cia1_model_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, cia1_model_submenu,
      "CIA 1 model" },
    { "CIA _2 model:", "Select the CIA #2 model",
      cia2_model_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, cia2_model_submenu,
      "CIA 2 model" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t cia_single_model_menu_items[] = {
    { "CIA model:", "Select the CIA model",
      cia1_model_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, cia1_model_submenu,
      "CIA model" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiciamodel_double_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ciamodel_submenu;

    ui_ciamodel_submenu = tui_menu_create("CIA model settings", 1);

    tui_menu_add(ui_ciamodel_submenu, cia_double_model_menu_items);

    tui_menu_add_submenu(parent_submenu, "CIA _model settings...",
                         "CIA model settings",
                         ui_ciamodel_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uiciamodel_single_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ciamodel_submenu;

    ui_ciamodel_submenu = tui_menu_create("CIA model settings", 1);

    tui_menu_add(ui_ciamodel_submenu, cia_single_model_menu_items);

    tui_menu_add_submenu(parent_submenu, "CIA _model settings...",
                         "CIA model settings",
                         ui_ciamodel_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
