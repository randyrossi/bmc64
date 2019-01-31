/*
 * uidrivec64.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include "resources.h"
#include "uidrive.h"
#include "uidrivec64.h"
#include "uidrivec64c128.h"
#include "uidrivec64vic20.h"
#include "uidriveiec.h"
#include "uimenu.h"
#include "uiperipheral.h"
#include "uiperipheraliec.h"

UI_MENU_DEFINE_TOGGLE(DriveTrueEmulation)
UI_MENU_DEFINE_TOGGLE(DriveSoundEmulation)

static ui_menu_entry_t drivec64dtv_settings_submenu[] = {
    { N_("True drive emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DriveTrueEmulation, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive sound emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DriveSoundEmulation, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #8 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uidrivec64vic20_set_drive0_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_settings_drive8_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #8 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)0, uidrivec64c128_drive0_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #8 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)0, set_drive0_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #8 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)0, set_drive0_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #9 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uidrivec64vic20_set_drive1_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_settings_drive9_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #9 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)1, uidrivec64c128_drive1_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #9 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)1, set_drive1_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #9 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)1, set_drive1_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #10 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uidrivec64vic20_set_drive2_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_settings_drive10_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #10 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)2, uidrivec64c128_drive2_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #10 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)2, set_drive2_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #10 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)2, set_drive2_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #11 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uidrivec64vic20_set_drive3_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_settings_drive11_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #11 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)3, uidrivec64c128_drive3_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #11 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)3, set_drive3_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #11 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)3, set_drive3_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_RAWDRIVE
    UI_MENU_ENTRY_SEPERATOR,
    { N_("RAW Block Device Name"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiperipheral_set_rawdevice_name, (ui_callback_data_t)"RawDriveDriver", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_drivec64dtv_settings_menu[] = {
    { N_("Drive settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, drivec64dtv_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
