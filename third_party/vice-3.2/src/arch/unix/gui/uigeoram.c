/*
 * uigeoram.c
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
#include "uicartridge.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "uigeoram.h"

UI_MENU_DEFINE_TOGGLE(GEORAM)
UI_MENU_DEFINE_RADIO(GEORAMsize)
UI_MENU_DEFINE_TOGGLE(GEORAMImageWrite) /* FIXME */
UI_MENU_DEFINE_TOGGLE(GEORAMIOSwap)

UI_CALLBACK(set_georam_image_name);
static UI_CALLBACK(georam_flush_callback);
static UI_CALLBACK(georam_save_callback);

static ui_menu_entry_t georam_size_submenu[] = {
    { "64kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GEORAMsize, (ui_callback_data_t)64, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "128kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GEORAMsize, (ui_callback_data_t)128, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "256kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GEORAMsize, (ui_callback_data_t)256, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "512kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GEORAMsize, (ui_callback_data_t)512, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1024kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GEORAMsize, (ui_callback_data_t)1024, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "2048kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GEORAMsize, (ui_callback_data_t)2048, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "4096kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GEORAMsize, (ui_callback_data_t)4096, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t georam_c64_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_GEORAM, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, georam_size_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Image name"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_georam_image_name, (ui_callback_data_t)"GEORAMfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_GEORAMImageWrite, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image now"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)georam_flush_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image as"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)georam_save_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t georam_vic20_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_GEORAM, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("MasC=uerade I/O swap"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_GEORAMIOSwap, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, georam_size_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Image name"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_georam_image_name, (ui_callback_data_t)"GEORAMfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_GEORAMImageWrite, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_CALLBACK(set_georam_image_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("GEO-RAM file"), UILIB_FILTER_ALL);
}

static UI_CALLBACK(georam_save_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_GEORAM));
    } else {
        ui_cartridge_save_dialog(CARTRIDGE_GEORAM);
    }
}

static UI_CALLBACK(georam_flush_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_GEORAM));
    } else {
        if (cartridge_flush_image(CARTRIDGE_GEORAM) < 0) {
            ui_error(_("Can not save cartridge"));
        }
    }
}
