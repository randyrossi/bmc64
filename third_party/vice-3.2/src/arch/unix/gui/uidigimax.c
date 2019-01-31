/*
 * uidigimax.c
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

#include "uilib.h"
#include "uimenu.h"
#include "uidigimax.h"

UI_MENU_DEFINE_TOGGLE(DIGIMAX)
UI_MENU_DEFINE_RADIO(DIGIMAXbase)

static ui_menu_entry_t digimax_c64_base_submenu[] = {
    { "$DE00", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xde00, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DE20", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xde20, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DE40", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xde40, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DE60", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xde60, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DE80", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xde80, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DEA0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdea0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DEC0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdec0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DEE0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdee0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DF00", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdf00, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DF20", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdf20, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DF40", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdf40, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DF60", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdf60, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DF80", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdf80, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DFA0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdfa0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DFC0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdfc0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DFE0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0xdfe0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t digimax_vic20_base_submenu[] = {
    { "$9800", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9800, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9820", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9820, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9840", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9840, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9860", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9860, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9880", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9880, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$98A0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x98a0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$98C0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x98c0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$98E0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x98e0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9C00", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9c00, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9C20", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9c20, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9C40", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9c40, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9C60", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9c60, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9C80", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9c80, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9CA0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9ca0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9CC0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9cc0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9CE0", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_DIGIMAXbase, (ui_callback_data_t)0x9ce0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t digimax_c64_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DIGIMAX, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Base address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, digimax_c64_base_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t digimax_vic20_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DIGIMAX, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Base address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, digimax_vic20_base_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
