/*
 * uigmod2.c
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
#include "uiapi.h"
#include "uicartridge.h"
#include "uilib.h"
#include "uimenu.h"
#include "uigmod2.h"

UI_MENU_DEFINE_TOGGLE(GMOD2EEPROMRW)
UI_MENU_DEFINE_TOGGLE(GMod2FlashWrite) /* FIXME */

static UI_CALLBACK(gmod2_flush_callback);
static UI_CALLBACK(gmod2_save_callback);

UI_CALLBACK(set_gmod2_eeprom_filename);

ui_menu_entry_t gmod2_submenu[] = {
    { N_("Save image when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_GMod2FlashWrite, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image now"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)gmod2_flush_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image as"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)gmod2_save_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("EEPROM image file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_gmod2_eeprom_filename, (ui_callback_data_t)"GMOD2EEPROMImage", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable writes to GMod2 EEPROM image"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_GMOD2EEPROMRW, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(gmod2_save_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_GMOD2));
    } else {
        ui_cartridge_save_dialog(CARTRIDGE_GMOD2);
    }
}

static UI_CALLBACK(gmod2_flush_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_GMOD2));
    } else {
        if (cartridge_flush_image(CARTRIDGE_GMOD2) < 0) {
            ui_error(_("Can not save cartridge"));
        }
    }
}

#if 0
static UI_CALLBACK(set_gmod2_card_filename)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("GMod2 card image filename"),
                        UILIB_FILTER_ALL);
}
#endif


UI_CALLBACK(set_gmod2_eeprom_filename)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("GMod2 EEPROM image filename"),
                        UILIB_FILTER_ALL);
}
