/*
 * uiromset.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include "machine.h"
#include "resources.h"
#include "romset.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "uiromset.h"
#include "util.h"
#include "vsync.h"

UI_CALLBACK(ui_set_romset)
{
    machine_romset_file_load(UI_MENU_CB_PARAM);
    ui_update_menus();
}

UI_CALLBACK(ui_load_rom_file)
{
    char *filename;
    ui_button_t button;
    static char *last_dir;
    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;

    vsync_suspend_speed_eval();

    filename = ui_select_file(_("Load ROM file"), NULL, 0, last_dir, &filter, 1, &button, 0, NULL, UI_FC_LOAD);

    switch (button) {
        case UI_BUTTON_OK:
            if (resources_set_string(UI_MENU_CB_PARAM, filename) < 0) {
                ui_error(_("Could not load ROM file\n'%s'"), filename);
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    ui_update_menus();
    lib_free(filename);
}

UI_CALLBACK(ui_unload_rom_file)
{
    resources_set_string((char *)UI_MENU_CB_PARAM, NULL);
}

static UI_CALLBACK(uiromset_archive_load)
{
    char *filename;
    ui_button_t button;
    static char *last_dir;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_ROMSET_ARCHIVE, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();

    filename = ui_select_file(_("Load custom ROM set archive"), NULL, 0, last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);

    switch (button) {
        case UI_BUTTON_OK:
            if (romset_archive_load(filename, 0) < 0) {
                ui_error(_("Could not load ROM set archive\n'%s'"), filename);
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }

    ui_update_menus();

    lib_free(filename);
}

static UI_CALLBACK(uiromset_archive_save)
{
    char *new_value;
    char *rom_set_archive = util_concat(_("ROM set archive"), ":", NULL);
    ui_button_t button;
    int len = 512;

    vsync_suspend_speed_eval();

    new_value = lib_malloc(len + 1);
    strcpy(new_value, "");

    button = uilib_input_file(_("File to save ROM set archive to"), rom_set_archive, new_value, len);
    lib_free(rom_set_archive);

    if (button == UI_BUTTON_OK) {
        romset_archive_save(new_value);
    }

    lib_free(new_value);
}

static UI_CALLBACK(uiromset_archive_list)
{
    char *list;

    list = romset_archive_list();

    ui_show_text(_("Current ROM set archive"), list, 300, 400);

    lib_free(list);
}

static UI_CALLBACK(uiromset_archive_item_create)
{
    char *new_value;
    ui_button_t button;
    int len = 512;

    vsync_suspend_speed_eval();

    new_value = lib_malloc(len + 1);
    strcpy(new_value, "");

    button = ui_input_string(_("ROM set item name to create"), _("ROM set name:"), new_value, len);

    if (button == UI_BUTTON_OK) {
        machine_romset_archive_item_create(new_value);
    }

    lib_free(new_value);
}

static UI_CALLBACK(uiromset_archive_item_delete)
{
    static char input_string[32];
    ui_button_t button;

    if (!CHECK_MENUS) {
        vsync_suspend_speed_eval();
        button = ui_input_string(_("Delete configuration"), _("Enter name"), input_string, 32);
        if (button == UI_BUTTON_OK) {
            romset_archive_item_delete(input_string);
            ui_update_menus();
        }
    }
}

static UI_CALLBACK(uiromset_archive_item_select)
{
    static char input_string[32 + 4]; /* FIXME +4 ??? */
    ui_button_t button;

    if (!CHECK_MENUS) {
        vsync_suspend_speed_eval();
        button = ui_input_string(_("Active configuration"), _("Enter name"), input_string, 32);
        if (button == UI_BUTTON_OK) {
            romset_archive_item_select(input_string);
            ui_update_menus();
        }
    }
}

ui_menu_entry_t uiromset_archive_submenu[] = {
    { N_("Load ROM set archive"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_archive_load, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save ROM set archive"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_archive_save, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("List current ROM set archive"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_archive_list, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Create ROM set item"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_archive_item_create, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Delete ROM set item"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_archive_item_delete, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Select ROM set item"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_archive_item_select, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

void uiromset_menu_init(void)
{
}

static UI_CALLBACK(uiromset_file_load)
{
    char *filename;
    ui_button_t button;
    static char *last_dir;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_ROMSET_FILE, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();

    filename = ui_select_file(_("Load custom ROM set file"), NULL, 0, last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);

    switch (button) {
        case UI_BUTTON_OK:
            if (machine_romset_file_load(filename) < 0) {
                ui_error(_("Could not load ROM set file\n'%s'"), filename);
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }

    ui_update_menus();

    lib_free(filename);
}

static UI_CALLBACK(uiromset_file_save)
{
    char *new_value;
    char *rom_set_file = util_concat(_("ROM set file"), ":", NULL);
    ui_button_t button;
    int len = 512;

    vsync_suspend_speed_eval();

    new_value = lib_malloc(len + 1);
    strcpy(new_value, "");

    button = uilib_input_file(_("File to save ROM set definition to"), rom_set_file, new_value, len);
    lib_free(rom_set_file);

    if (button == UI_BUTTON_OK) {
        machine_romset_file_save(new_value);
    }

    lib_free(new_value);
}

static UI_CALLBACK(uiromset_file_list)
{
    char *list;

    list = machine_romset_file_list();

    ui_show_text(_("Current ROM set"), list, 300, 400);

    lib_free(list);
}

ui_menu_entry_t uiromset_file_submenu[] = {
    { N_("Load custom ROM set from file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_file_load, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Dump ROM set definition to file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_file_save, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("List current ROM set"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiromset_file_list, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
