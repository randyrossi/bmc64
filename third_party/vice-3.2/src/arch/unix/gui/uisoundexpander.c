/*
 * uisoundexpander.c
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
#include "uisoundexpander.h"

UI_MENU_DEFINE_TOGGLE(SFXSoundExpander)
UI_MENU_DEFINE_TOGGLE(SFXSoundExpanderIOSwap)
UI_MENU_DEFINE_RADIO(SFXSoundExpanderChip)

static ui_menu_entry_t soundexpander_chip_submenu[] = {
    { "3526", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SFXSoundExpanderChip, (ui_callback_data_t)3526, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "3812", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SFXSoundExpanderChip, (ui_callback_data_t)3812, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t soundexpander_c64_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SFXSoundExpander, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("YM chip type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, soundexpander_chip_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t soundexpander_vic20_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SFXSoundExpander, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("MasC=uerade I/O swap"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SFXSoundExpanderIOSwap, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("YM chip type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, soundexpander_chip_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
