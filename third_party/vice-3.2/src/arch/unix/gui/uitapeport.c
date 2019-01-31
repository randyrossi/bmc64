/** \file   uitapeport.c
 * \brief   GTk2/Xaw tapeport submenu
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
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

#include "tapecart.h"

#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "uitapeport.h"


UI_MENU_DEFINE_TOGGLE(Datasette)
UI_MENU_DEFINE_TOGGLE(TapeLog)
UI_MENU_DEFINE_TOGGLE(TapeLogDestination)
UI_MENU_DEFINE_TOGGLE(CPClockF83)
UI_MENU_DEFINE_TOGGLE(CPClockF83Save)
UI_MENU_DEFINE_TOGGLE(TapeSenseDongle)
UI_MENU_DEFINE_TOGGLE(DTLBasicDongle)

static UI_CALLBACK(set_tapelog_filename)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("Tape log file"), UILIB_FILTER_ALL);
}


/*
 * Tapecart menu handlers
 */
UI_MENU_DEFINE_TOGGLE(TapecartEnabled)
UI_MENU_DEFINE_TOGGLE(TapecartUpdateTCRT)
UI_MENU_DEFINE_TOGGLE(TapecartOptimizeTCRT)
UI_MENU_DEFINE_RADIO(TapecartLogLevel)


/** \brief  Select a tapecart image file
 */
static UI_CALLBACK(set_tapecart_filename)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM,
            _("Tapecart image file"),
            UILIB_FILTER_ALL);
}


/** \brief  Force a flush of the tapecart image to disk
 */
static UI_CALLBACK(flush_tapecart_image)
{
    if (tapecart_flush_tcrt() == 0) {
        ui_message("Succesfully flushed tapecart image to disk");
    } else {
        ui_error("Failed to flush tapecart image to disk");
    }
}


/** \brief  Tapecart log level submenu
 */
static ui_menu_entry_t tapecart_loglevel_submenu[] = {
    { N_("0 (errors only)"), UI_MENU_TYPE_TICK,
        (ui_callback_t)radio_TapecartLogLevel, (ui_callback_data_t)0, NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("1 (0 plus mode changes and command bytes)"), UI_MENU_TYPE_TICK,
        (ui_callback_t)radio_TapecartLogLevel, (ui_callback_data_t)1, NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("2 (1 plus command parameter details)"), UI_MENU_TYPE_TICK,
        (ui_callback_t)radio_TapecartLogLevel, (ui_callback_data_t)2, NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_LIST_END
};


/** \brief  Tape port devices submenu (for non-C64 emus)
 */
ui_menu_entry_t tapeport_submenu[] = {
    { N_("Enable datasette"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Datasette, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable tape sense dongle"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TapeSenseDongle, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable DTL basic dongle"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DTLBasicDongle, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_SEPERATOR,

    { N_("Enable tape log device"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TapeLog, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable tape log save to user specified file"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TapeLogDestination, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("User specified tape log filename"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_tapelog_filename, (ui_callback_data_t)"TapeLogfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_SEPERATOR,

    { N_("Enable CP CLock F83"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CPClockF83, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save RTC data when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CPClockF83Save, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_LIST_END
};


/** \brief  Tape port devices submenu (x64, x64sc and x128)
 *
 * Contains tapecart submenu, the tapecart device doesn't yet support anything
 * other than the C128/C128.
 */
ui_menu_entry_t tapeport_submenu_c64[] = {
    { N_("Enable datasette"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Datasette, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable tape sense dongle"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TapeSenseDongle, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable DTL basic dongle"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DTLBasicDongle, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_SEPERATOR,

    { N_("Enable tape log device"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TapeLog, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable tape log save to user specified file"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TapeLogDestination, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("User specified tape log filename"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_tapelog_filename, (ui_callback_data_t)"TapeLogfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_SEPERATOR,

    { N_("Enable CP CLock F83"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CPClockF83, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save RTC data when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CPClockF83Save, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_SEPERATOR,

    { N_("Enable tapecart"), UI_MENU_TYPE_TICK,
        (ui_callback_t)toggle_TapecartEnabled, NULL, NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save tapecart data when changed"), UI_MENU_TYPE_TICK,
        (ui_callback_t)toggle_TapecartUpdateTCRT, NULL, NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Optimize tapecart data when changed"), UI_MENU_TYPE_TICK,
        (ui_callback_t)toggle_TapecartOptimizeTCRT, NULL, NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Select tapecart TCRT image"), UI_MENU_TYPE_DOTS,
        (ui_callback_t)set_tapecart_filename,
        (ui_callback_data_t)"TapecartTCRTFilename", NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Tapecart log level"), UI_MENU_TYPE_NORMAL,
        NULL, NULL, tapecart_loglevel_submenu,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Flush tapecart image to disk"), UI_MENU_TYPE_NORMAL,
        (ui_callback_t)flush_tapecart_image, NULL, NULL,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

    UI_MENU_ENTRY_LIST_END
};
