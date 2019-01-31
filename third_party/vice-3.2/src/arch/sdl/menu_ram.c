/*
 * menu_ram.c - RAM pattern menu for SDL UI.
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

#include "types.h"

#include "menu_common.h"
#include "menu_ram.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(RAMInitStartValue)
UI_MENU_DEFINE_RADIO(RAMInitValueInvert)
UI_MENU_DEFINE_RADIO(RAMInitPatternInvert)

static const ui_menu_entry_t constant_values_menu[] = {
    { "0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)0 },
    { "1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)1 },
    { "2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)2 },
    { "4",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)4 },
    { "8",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)8 },
    { "16",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)16 },
    { "32",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)32 },
    { "64",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)64 },
    { "128",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)128 },
    { "256",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)256 },
    { "512",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)512 },
    { "1024",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitValueInvert_callback,
      (ui_callback_data_t)1024 },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t constant_pattern_menu[] = {
    { "0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)0 },
    { "1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)1 },
    { "2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)2 },
    { "4",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)4 },
    { "8",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)8 },
    { "16",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)16 },
    { "32",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)32 },
    { "64",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)64 },
    { "128",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)128 },
    { "256",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)256 },
    { "512",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)512 },
    { "1024",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitPatternInvert_callback,
      (ui_callback_data_t)1024 },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t ram_menu[] = {
    SDL_MENU_ITEM_TITLE("Value of first byte"),
    { "0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitStartValue_callback,
      (ui_callback_data_t)0 },
    { "255",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMInitStartValue_callback,
      (ui_callback_data_t)255 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Length of constant values",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)constant_values_menu },
    { "Length of constant pattern",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)constant_pattern_menu },
    SDL_MENU_LIST_END
};
