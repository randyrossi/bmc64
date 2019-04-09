/*
 * menu_c64_expansions.c - C64 expansions menu for SDL UI.
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

#include "c64-memory-hacks.h"
#include "menu_c64_expansions.h"
#include "menu_common.h"
#include "uimenu.h"

/* MEMORY EXPANSION HACK MENU */

UI_MENU_DEFINE_RADIO(MemoryHack)

UI_MENU_DEFINE_RADIO(C64_256Kbase)
UI_MENU_DEFINE_FILE_STRING(C64_256Kfilename)

UI_MENU_DEFINE_RADIO(PLUS60Kbase)
UI_MENU_DEFINE_FILE_STRING(PLUS60Kfilename)

UI_MENU_DEFINE_FILE_STRING(PLUS256Kfilename)

const ui_menu_entry_t c64_memory_hacks_menu[] = {
    SDL_MENU_ITEM_TITLE("Memory Expansion Hack Device"),
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_NONE },
    { "C64 256K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_C64_256K },
    { "+60K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_PLUS60K },
    { "+256K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_PLUS256K },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("C64 256K Base address"),
    { "$DE00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_C64_256Kbase_callback,
      (ui_callback_data_t)0xde00 },
    { "$DE80",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_C64_256Kbase_callback,
      (ui_callback_data_t)0xde80 },
    { "$DF00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_C64_256Kbase_callback,
      (ui_callback_data_t)0xdf00 },
    { "$DF80",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_C64_256Kbase_callback,
      (ui_callback_data_t)0xdf80 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("C64 256K RAM image"),
    { "C64 256K image file",
      MENU_ENTRY_DIALOG,
      file_string_C64_256Kfilename_callback,
      (ui_callback_data_t)"Select C64 256K image" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("+60K Base address"),
    { "$D040",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PLUS60Kbase_callback,
      (ui_callback_data_t)0xd040 },
    { "$D100",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PLUS60Kbase_callback,
      (ui_callback_data_t)0xd100 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("+60K RAM image"),
    { "+60K image file",
      MENU_ENTRY_DIALOG,
      file_string_PLUS60Kfilename_callback,
      (ui_callback_data_t)"Select +60K image" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("+256K RAM image"),
    { "+256K image file",
      MENU_ENTRY_DIALOG,
      file_string_PLUS256Kfilename_callback,
      (ui_callback_data_t)"Select +256K image" },
    SDL_MENU_LIST_END
};
