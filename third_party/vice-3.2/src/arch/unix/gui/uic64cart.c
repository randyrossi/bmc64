/*
 * uic64cart.c - Implementation of the C64/C128 cart-specific part of the UI.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Nathan Huizinga <nathan.huizinga@chess.nl>
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
#include <stdlib.h>
#include <string.h>

#include "c64cart.h"
#include "cartridge.h"
#include "lib.h"
#include "ui.h"
#include "uiapi.h"
#include "uic64cart.h"
#include "uicartridge.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"


static char *last_dir = NULL;

static UI_CALLBACK(attach_cartridge)
{
    int type = vice_ptr_to_int(UI_MENU_CB_PARAM);
    char *filename;
    ui_button_t button;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_CARTRIDGE, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();

    filename = ui_select_file(_("Attach cartridge image"), NULL, 0, last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);
    switch (button) {
        case UI_BUTTON_OK:
            if (cartridge_attach_image(type, filename) < 0) {
                ui_error(_("Cannot attach cartridge"));
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special. */
            break;
    }
    lib_free(filename);

    ui_update_menus();
}

static UI_CALLBACK(detach_cartridge)
{
    cartridge_detach_image(-1);
    ui_update_menus();
}

static UI_CALLBACK(default_cartridge)
{
    cartridge_set_default();
}

static UI_CALLBACK(freeze_cartridge)
{
    cartridge_trigger_freeze();
}

static ui_menu_entry_t *attach_cartridge_image_freezer_submenu;
static ui_menu_entry_t *attach_cartridge_image_game_submenu;
static ui_menu_entry_t *attach_cartridge_image_util_submenu;

static ui_menu_entry_t attach_cartridge_image_submenu[] = {
    { N_("Smart attach CRT image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_CRT, NULL,
      KEYSYM_c, UI_HOTMOD_META },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Attach raw 8KB cartridge image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_GENERIC_8KB, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach raw 16KB cartridge image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_GENERIC_16KB, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach raw ultimax cartridge image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_ULTIMAX, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Freezer"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Games"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Utilities"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Set cartridge as default"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)default_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_c64cart_commands_menu[] = {
    { N_("Attach cartridge image"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, attach_cartridge_image_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Detach cartridge image(s)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Cartridge freeze"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)freeze_cartridge, NULL, NULL,
      KEYSYM_z, UI_HOTMOD_META },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_c64scpucart_commands_menu[] = {
    { N_("Attach cartridge image"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, attach_cartridge_image_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Detach cartridge image(s)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_c64cart_entry = {
    NULL, UI_MENU_TYPE_DOTS,
    (ui_callback_t)attach_cartridge, (ui_callback_data_t)0, NULL,
    (ui_keysym_t)0, (ui_hotkey_modifier_t)0
};

static int countgroup(cartridge_info_t *cartlist, int flags)
{
    int num = 0;
    while(cartlist->name) {
        if ((cartlist->flags & flags) == flags) {
            num++;
        }
        cartlist++;
    }
    return num;
}

static void makegroup(cartridge_info_t *cartlist, ui_menu_entry_t *entry, int flags)
{
    while(cartlist->name) {
        if ((cartlist->flags & flags) == flags) {
            ui_c64cart_entry.string = cartlist->name;
            ui_c64cart_entry.callback_data = (ui_callback_data_t)(unsigned long)cartlist->crtid;
            memcpy(entry, &ui_c64cart_entry, sizeof(ui_menu_entry_t));
            entry++;
        }
        cartlist++;
    }
    memset(entry, 0, sizeof(ui_menu_entry_t));
}

void uicart_menu_create(void)
{
    int num;
    cartridge_info_t *cartlist = cartridge_get_info_list();

    num = countgroup(cartlist, CARTRIDGE_GROUP_FREEZER);
    attach_cartridge_image_freezer_submenu = lib_malloc(sizeof(ui_menu_entry_t) * (num + 1));
    makegroup(cartlist, attach_cartridge_image_freezer_submenu, CARTRIDGE_GROUP_FREEZER);
    attach_cartridge_image_submenu[6 + 0].sub_menu = attach_cartridge_image_freezer_submenu;

    num = countgroup(cartlist, CARTRIDGE_GROUP_GAME);
    attach_cartridge_image_game_submenu = lib_malloc(sizeof(ui_menu_entry_t) * (num + 1));
    makegroup(cartlist, attach_cartridge_image_game_submenu, CARTRIDGE_GROUP_GAME);
    attach_cartridge_image_submenu[6 + 1].sub_menu = attach_cartridge_image_game_submenu;

    num = countgroup(cartlist, CARTRIDGE_GROUP_UTIL);
    attach_cartridge_image_util_submenu = lib_malloc(sizeof(ui_menu_entry_t) * (num + 1));
    makegroup(cartlist, attach_cartridge_image_util_submenu, CARTRIDGE_GROUP_UTIL);
    attach_cartridge_image_submenu[6 + 2].sub_menu = attach_cartridge_image_util_submenu;

}

void uicart_menu_shutdown(void)
{
    if (attach_cartridge_image_freezer_submenu) {
        lib_free(attach_cartridge_image_freezer_submenu);
        attach_cartridge_image_freezer_submenu = NULL;
    }
    if (attach_cartridge_image_game_submenu) {
        lib_free(attach_cartridge_image_game_submenu);
        attach_cartridge_image_game_submenu = NULL;
    }
    if (attach_cartridge_image_util_submenu) {
        lib_free(attach_cartridge_image_util_submenu);
        attach_cartridge_image_util_submenu = NULL;
    }
    if (last_dir != NULL) {
        lib_free(last_dir);
    }
}

