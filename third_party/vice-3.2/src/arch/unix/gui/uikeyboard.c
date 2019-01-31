/*
 * uikeyboard.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ioutil.h"
#include "keyboard.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "uiapi.h"
#include "uilib.h"
#include "uikeyboard.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

static UI_CALLBACK(set_keymap_type)
{
    int kindex, newindex = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (resources_get_int("KeymapIndex", &kindex) < 0) {
        return;
    }

    if (!CHECK_MENUS) {
        if (kindex != newindex) {
            resources_set_int("KeymapIndex", newindex);
            ui_update_menus();
        }
    } else {
        ui_menu_set_tick(w, (kindex == newindex));
    }
}

static ui_menu_entry_t keyboard_maptype_submenu[] = {
    { N_("Symbolic mapping"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_keymap_type, (ui_callback_data_t)KBD_INDEX_SYM, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Positional mapping"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_keymap_type, (ui_callback_data_t)KBD_INDEX_POS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Symbolic mapping (User)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_keymap_type, (ui_callback_data_t)KBD_INDEX_USERSYM, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Positional mapping (User)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_keymap_type, (ui_callback_data_t)KBD_INDEX_USERPOS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(set_layout_type)
{
    int kindex, newindex = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (resources_get_int("KeyboardMapping", &kindex) < 0) {
        return;
    }

    if (!CHECK_MENUS) {
        if (kindex != newindex) {
            resources_set_int("KeyboardMapping", newindex);
            ui_update_menus();
        }
    } else {
        ui_menu_set_tick(w, (kindex == newindex));
    }
}

static UI_CALLBACK(select_user_keymap)
{
    char *filename;
    const char *resname;
    ui_button_t button;
    int kindex;
    static char *last_dir;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_KEYMAP, UILIB_FILTER_ALL };

    resources_get_int("KeymapIndex", &kindex);
    kindex = (kindex & ~1) + vice_ptr_to_int(UI_MENU_CB_PARAM);
    resname = machine_get_keymap_res_name(kindex);

    vsync_suspend_speed_eval();
    filename = ui_select_file(_("Read Keymap File"), NULL, 0, last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);

    switch (button) {
        case UI_BUTTON_OK:
            resources_set_string(resname, filename);
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    lib_free(filename);
}

static UI_CALLBACK(dump_keymap)
{
    char *wd;
    int len;

    len = ioutil_maxpathlen();
    wd = lib_malloc(len);

    ioutil_getcwd(wd, len);
    if (uilib_input_file(_("VICE setting"), _("Write to Keymap File:"), wd, len) == UI_BUTTON_OK) {
        if (keyboard_keymap_dump(wd) < 0) {
            ui_error(strerror(errno));
        }
    }
    lib_free(wd);
}

/* weird, but GCC tells me this isn't used */
#if 0
static void ui_select_keymap(ui_window_t w, int check, char *name, int sympos)
{
    const char *resname;
    int kindex;
    const char *wd;

    resources_get_int("KeymapIndex", &kindex);
    kindex = (kindex & ~1) + sympos;
    resname = machine_get_keymap_res_name(kindex);

    if (name) {
        if (!check) {
            resources_set_string(resname, name);
            ui_update_menus();
        } else {
            resources_get_string(resname, &wd);
            if (!strcmp(wd, name)) {
                ui_menu_set_tick(w, 1);
            } else {
                ui_menu_set_tick(w, 0);
            }
        }
    }
}
#endif

static ui_menu_entry_t keyboard_settings_submenu[] = {
    { N_("Keyboard mapping type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, keyboard_maptype_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
      /* Do not change position as position 1 is hard coded. */
    { N_("Keyboard layout type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Set symbolic user keymap file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)select_user_keymap, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Set positional user keymap file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)select_user_keymap, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Dump keymap to file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t) dump_keymap, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uikeyboard_settings_menu[] = {
    { N_("Keyboard settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, keyboard_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

void uikeyboard_menu_create(void)
{
    unsigned int i, num;
    ui_menu_entry_t *keyboard_layouttype_submenu;
    mapping_info_t *list;

    num = keyboard_get_num_mappings();

    if (num == 0) {
        return;
    }

    keyboard_layouttype_submenu = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));
    list = keyboard_get_info_list();

    for (i = 0; i < num ; i++) {
        keyboard_layouttype_submenu[i].string = (ui_callback_data_t)lib_msprintf("%s", list->name);
        keyboard_layouttype_submenu[i].type = UI_MENU_TYPE_TICK;
        keyboard_layouttype_submenu[i].callback = (ui_callback_t)set_layout_type;
        keyboard_layouttype_submenu[i].callback_data = (ui_callback_data_t)uint_to_void_ptr(list->mapping);
        ++list;
    }

    keyboard_settings_submenu[1].sub_menu = keyboard_layouttype_submenu;
}

void uikeyboard_menu_shutdown(void)
{
    unsigned int i;
    ui_menu_entry_t *keyboard_layouttype_submenu = NULL;

    keyboard_layouttype_submenu = keyboard_settings_submenu[1].sub_menu;

    if (keyboard_layouttype_submenu == NULL) {
        return;
    }

    keyboard_settings_submenu[1].sub_menu = NULL;

    i = 0;

    while (keyboard_layouttype_submenu[i].string != NULL) {
        lib_free(keyboard_layouttype_submenu[i].string);
        i++;
    }

    lib_free(keyboard_layouttype_submenu);
}
