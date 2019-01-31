/*
 * uiscpu64.c
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
#include "uilib.h"
#include "uimenu.h"
#include "uiscpu64.h"

UI_MENU_DEFINE_TOGGLE(JiffySwitch)
UI_MENU_DEFINE_TOGGLE(SpeedSwitch)
UI_MENU_DEFINE_RADIO(SIMMSize)

static ui_menu_entry_t scpu64_simm_size_submenu[] = {
    { "0 MB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SIMMSize, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1 MB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SIMMSize, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "4 MB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SIMMSize, (ui_callback_data_t)4, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "8 MB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SIMMSize, (ui_callback_data_t)8, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "16 MB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SIMMSize, (ui_callback_data_t)16, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t scpu64_submenu[] = {
    { N_("SIMM size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, scpu64_simm_size_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Jiffy switch enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_JiffySwitch, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Speed switch enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SpeedSwitch, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
