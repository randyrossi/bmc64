/*
 * uikeymap.c - Keymap UI interface for MS-DOS.
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

#include "keyboard.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uikeymap.h"
#include "util.h"

TUI_MENU_DEFINE_RADIO(KeymapIndex)
TUI_MENU_DEFINE_RADIO(KeyboardMapping)
TUI_MENU_DEFINE_FILENAME(KeymapUserSymFile, "User symbolic keymap")
TUI_MENU_DEFINE_FILENAME(KeymapUserPosFile, "User positional keymap")

static TUI_MENU_CALLBACK(active_keymap_submenu_callback)
{
    int value;
    static char *s;

    resources_get_int("KeymapIndex", &value);
    switch (value) {
        case KBD_INDEX_SYM:
            s = "Symbolic";
            break;
        case KBD_INDEX_POS:
            s = "Positional";
            break;
        case KBD_INDEX_USERSYM:
            s = "Symbolic (user)";
            break;
        case KBD_INDEX_USERPOS:
            s = "Positional (user)";
            break;
        default:
            s = "Unknown";
            break;
    }
    return s;
}

static tui_menu_item_def_t keymap_active_submenu[] = {
    { "Symbolic", NULL, radio_KeymapIndex_callback,
      (void *)KBD_INDEX_SYM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Positional", NULL, radio_KeymapIndex_callback,
      (void *)KBD_INDEX_POS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Symbolic (user)", NULL, radio_KeymapIndex_callback,
      (void *)KBD_INDEX_USERSYM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Positional(user)", NULL, radio_KeymapIndex_callback,
      (void *)KBD_INDEX_USERPOS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(keymap_mapping_submenu_callback)
{
    int value;
    int i;
    static char *s = NULL;
    mapping_info_t *kbdlist = keyboard_get_info_list();
    int num = keyboard_get_num_mappings();

    resources_get_int("KeyboardMapping", &value);

    for (i = 0; i < num; i++) {
        if (value == kbdlist->mapping) {
            s = kbdlist->name;
        }
        kbdlist++;
    }
    if (!s) {
        s = "Unknown";
    }
    return s;
}

static tui_menu_item_def_t keymap_mapping_submenu[KBD_MAPPING_NUM + 1];

static tui_menu_item_def_t keymap_menu_items[] = {
    { "Active keymap:", "Select the active keymap",
      active_keymap_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, keymap_active_submenu,
      "Active keymap" },
    { "Keymap mapping:", "Select the keymap mapping",
      keymap_mapping_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, keymap_mapping_submenu,
      "Keymap mapping" },
    { "User symbolic keymap file:", "Select the user symbolic keymap file",
      filename_KeymapUserSymFile_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "User positional keymap file:", "Select the user positional keymap file",
      filename_KeymapUserPosFile_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uikeymap_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_keymap_submenu;
    int i = 0;
    mapping_info_t *kbdlist = keyboard_get_info_list();
    int num = keyboard_get_num_mappings();

    while (num) {
        keymap_mapping_submenu[i].label = kbdlist->name;
        keymap_mapping_submenu[i].help_string = NULL;
        keymap_mapping_submenu[i].callback = radio_KeyboardMapping_callback;
        keymap_mapping_submenu[i].callback_param = (void *)kbdlist->mapping;
        keymap_mapping_submenu[i].par_string_max_len = 20;
        keymap_mapping_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
        keymap_mapping_submenu[i].submenu = NULL;
        keymap_mapping_submenu[i].submenu_title = NULL;
        kbdlist++;
        num--;
    }
    keymap_mapping_submenu[i].label = NULL;
    keymap_mapping_submenu[i].help_string = NULL;
    keymap_mapping_submenu[i].callback = NULL;
    keymap_mapping_submenu[i].callback_param = NULL;
    keymap_mapping_submenu[i].par_string_max_len = 0;
    keymap_mapping_submenu[i].behavior = 0;
    keymap_mapping_submenu[i].submenu = NULL;
    keymap_mapping_submenu[i].submenu_title = NULL;

    ui_keymap_submenu = tui_menu_create("Keymap settings", 1);

    tui_menu_add(ui_keymap_submenu, keymap_menu_items);

    tui_menu_add_submenu(parent_submenu, "Keymap settings...",
                         "Keymap settings",
                         ui_keymap_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
