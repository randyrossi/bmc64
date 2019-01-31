/*
 * uiuserport.c
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

#include "uilib.h"
#include "uimenu.h"
#include "uiuserport.h"

UI_MENU_DEFINE_TOGGLE(UserportDAC)
UI_MENU_DEFINE_TOGGLE(UserportDIGIMAX)
UI_MENU_DEFINE_TOGGLE(UserportRTCDS1307)
UI_MENU_DEFINE_TOGGLE(UserportRTCDS1307Save)
UI_MENU_DEFINE_TOGGLE(UserportRTC58321a)
UI_MENU_DEFINE_TOGGLE(UserportRTC58321aSave)
UI_MENU_DEFINE_TOGGLE(Userport4bitSampler)
UI_MENU_DEFINE_TOGGLE(Userport8BSS)

ui_menu_entry_t userport_c64_cbm2_submenu[] = {
    { N_("Enable 4 bit sampler"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Userport4bitSampler, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable 8 bit stereo sampler"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Userport8BSS, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable 8 bit DAC"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportDAC, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable DigiMAX"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportDIGIMAX, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (58321a)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTC58321a, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (58321a) saving"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTC58321aSave, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (DS1307)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTCDS1307, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (DS1307) saving"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTCDS1307Save, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t userport_pet_vic20_submenu[] = {
    { N_("Enable 8 bit DAC"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportDAC, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (58321a)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTC58321a, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (58321a) saving"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTC58321aSave, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (DS1307)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTCDS1307, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable RTC (DS1307) saving"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportRTCDS1307Save, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t userport_plus4_submenu[] = {
    { N_("Enable 8 bit DAC"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UserportDAC, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
