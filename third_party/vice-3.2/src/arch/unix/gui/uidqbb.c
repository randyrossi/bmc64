/*
 * uidqbb.c
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
#include "uiapi.h"
#include "uicartridge.h"
#include "uilib.h"
#include "uimenu.h"
#include "uidqbb.h"

UI_MENU_DEFINE_TOGGLE(DQBB)
UI_MENU_DEFINE_TOGGLE(DQBBImageWrite) /* FIXME */

static UI_CALLBACK(dqbb_set_image_name);
static UI_CALLBACK(dqbb_flush_callback);
static UI_CALLBACK(dqbb_save_callback);

ui_menu_entry_t dqbb_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DQBB, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Image name"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)dqbb_set_image_name, (ui_callback_data_t)"DQBBfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DQBBImageWrite, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image now"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)dqbb_flush_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image as"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)dqbb_save_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(dqbb_set_image_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("DQBB file"), UILIB_FILTER_ALL);
}

static UI_CALLBACK(dqbb_flush_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_DQBB));
    } else {
        if (cartridge_flush_image(CARTRIDGE_DQBB) < 0) {
            ui_error(_("Can not save cartridge"));
        }
    }
}

static UI_CALLBACK(dqbb_save_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_DQBB));
    } else {
        ui_cartridge_save_dialog(CARTRIDGE_DQBB);
    }
}
