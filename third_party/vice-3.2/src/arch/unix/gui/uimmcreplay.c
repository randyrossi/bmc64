/*
 * uimmcreplay.c
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
#include "uiclockport-device.h"
#include "uimmcreplay.h"

UI_MENU_DEFINE_TOGGLE(MMCRCardRW)
UI_MENU_DEFINE_TOGGLE(MMCREEPROMRW)
UI_MENU_DEFINE_TOGGLE(MMCRRescueMode)
UI_MENU_DEFINE_RADIO(MMCRSDType)
UI_MENU_DEFINE_TOGGLE(MMCRImageWrite) /* FIXME */
UI_MENU_DEFINE_RADIO(MMCRClockPort)

static UI_CALLBACK(mmcreplay_flush_callback);
static UI_CALLBACK(mmcreplay_save_callback);

UI_CALLBACK(set_mmcreplay_card_filename);

UI_CALLBACK(set_mmcreplay_eeprom_filename);

static ui_menu_entry_t mmcreplay_sd_type_submenu[] = {
    { N_("Auto"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MMCRSDType, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "MMC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MMCRSDType, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "SD", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MMCRSDType, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "SDHC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MMCRSDType, (ui_callback_data_t)3, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t mmcreplay_submenu[] = {
    { N_("Enable rescue mode"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMCRRescueMode, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    /* the dynamically generated clockport menu goes here */
    { N_("Clockport device"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Save image when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMCRImageWrite, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image now"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)mmcreplay_flush_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save image as"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)mmcreplay_save_callback, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    /* Translators: this means card as in SD/MMC card, not a cartridge! */
    { N_("Card image file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_mmcreplay_card_filename, (ui_callback_data_t)"MMCRCardImage", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    /* Translators: this means card as in SD/MMC card, not a cartridge! */
    { N_("Enable writes to card image"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMCRCardRW, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    /* Translators: this means card as in SD/MMC card, not a cartridge! */
    { N_("Card type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mmcreplay_sd_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("EEPROM image file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_mmcreplay_eeprom_filename, (ui_callback_data_t)"MMCREEPROMImage", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable writes to MMC Replay EEPROM image"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMCREEPROMRW, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(mmcreplay_save_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_MMC_REPLAY));
    } else {
        ui_cartridge_save_dialog(CARTRIDGE_MMC_REPLAY);
    }
}

static UI_CALLBACK(mmcreplay_flush_callback)
{
    if (CHECK_MENUS) {
        ui_menu_set_sensitive(w, cartridge_type_enabled(CARTRIDGE_MMC_REPLAY));
    } else {
        if (cartridge_flush_image(CARTRIDGE_MMC_REPLAY) < 0) {
            ui_error(_("Can not save cartridge"));
        }
    }
}

UI_CALLBACK(set_mmcreplay_card_filename)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("MMC Replay card image filename"),
                        UILIB_FILTER_ALL);
}

UI_CALLBACK(set_mmcreplay_eeprom_filename)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("MMC Replay EEPROM image filename"),
                        UILIB_FILTER_ALL);
}


/** \brief  Generate dynamic menu for clockport device selection
 */
void uimmcr_menu_create(void)
{
    ui_menu_entry_t *cpdev_menu = uiclockport_device_menu_create(
            (ui_callback_t)radio_MMCRClockPort);
    mmcreplay_submenu[1].sub_menu = cpdev_menu;
}

/** \brief  Clean up memory used by clockport device selection menu
 */
void uimmcr_menu_shutdown(void)
{
    uiclockport_device_menu_shutdown(mmcreplay_submenu[1].sub_menu);
}
