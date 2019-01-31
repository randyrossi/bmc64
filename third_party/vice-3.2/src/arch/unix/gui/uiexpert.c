/*
 * uiexpert.c
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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
#include "c64cart.h"
#include "ui.h"
#include "uiapi.h"
#include "uicartridge.h"
#include "uilib.h"
#include "uimenu.h"
#include "uiexpert.h"
#include "vsync.h"

UI_MENU_DEFINE_RADIO(ExpertCartridgeMode)
UI_MENU_DEFINE_TOGGLE(ExpertCartridgeEnabled)
UI_MENU_DEFINE_TOGGLE(ExpertImageWrite) /* FIXME */

static UI_CALLBACK(expert_set_image_name); /* FIXME */
static UI_CALLBACK(expert_flush_cartridge);
static UI_CALLBACK(expert_save_cartridge);

ui_menu_entry_t expert_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_ExpertCartridgeEnabled, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Prg"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ExpertCartridgeMode, (ui_callback_data_t)EXPERT_MODE_PRG, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Off"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ExpertCartridgeMode, (ui_callback_data_t)EXPERT_MODE_OFF, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("On"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ExpertCartridgeMode, (ui_callback_data_t)EXPERT_MODE_ON, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Image name"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)expert_set_image_name, (ui_callback_data_t)"Expertfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_ExpertImageWrite, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image now"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)expert_flush_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image as"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)expert_save_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(expert_set_image_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("Expert Cartridge file"), UILIB_FILTER_ALL);
}

static UI_CALLBACK(expert_flush_cartridge)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_EXPERT));
    } else {
        if (cartridge_flush_image(CARTRIDGE_EXPERT) < 0) {
            ui_error(_("Can not save cartridge"));
        }
    }
}

static UI_CALLBACK(expert_save_cartridge)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_EXPERT));
    } else {
        ui_cartridge_save_dialog(CARTRIDGE_EXPERT);
    }
}

