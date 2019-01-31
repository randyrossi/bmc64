/*
 * menu_petcart.c - Implementation of the pet cartridge settings menu for the SDL UI.
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
#include <stdlib.h>
#include <string.h>

#include "cartio.h"
#include "menu_petcart.h"
#include "menu_common.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

UI_MENU_DEFINE_FILE_STRING(RomModule9Name)
UI_MENU_DEFINE_FILE_STRING(RomModuleAName)
UI_MENU_DEFINE_FILE_STRING(RomModuleBName)

static UI_MENU_CALLBACK(detach_cart_callback)
{
    if (activated) {
        resources_set_string((char *)param, "");
    }
    return NULL;
}

UI_MENU_DEFINE_RADIO(IOCollisionHandling)

static const ui_menu_entry_t iocollision_menu[] = {
    { "Detach all",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IOCollisionHandling_callback,
      (ui_callback_data_t)IO_COLLISION_METHOD_DETACH_ALL },
    { "Detach last",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IOCollisionHandling_callback,
      (ui_callback_data_t)IO_COLLISION_METHOD_DETACH_LAST },
    { "AND values",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IOCollisionHandling_callback,
      (ui_callback_data_t)IO_COLLISION_METHOD_AND_WIRES },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(iocollision_show_type_callback)
{
    int type;

    resources_get_int("IOCollisionHandling", &type);
    switch (type) {
        case IO_COLLISION_METHOD_DETACH_ALL:
            return "-> detach all";
            break;
        case IO_COLLISION_METHOD_DETACH_LAST:
            return "-> detach last";
            break;
        case IO_COLLISION_METHOD_AND_WIRES:
            return "-> AND values";
            break;
    }
    return "n/a";
}

const ui_menu_entry_t petcart_menu[] = {
    { "Attach ROM 9",
      MENU_ENTRY_DIALOG,
      file_string_RomModule9Name_callback,
      (ui_callback_data_t)"Select ROM 9 image" },
    { "Detach ROM 9",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      (ui_callback_data_t)"RomModule9Name" },
    { "Attach ROM A",
      MENU_ENTRY_DIALOG,
      file_string_RomModuleAName_callback,
      (ui_callback_data_t)"Select ROM A image" },
    { "Detach ROM A",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      (ui_callback_data_t)"RomModuleAName" },
    { "Attach ROM B",
      MENU_ENTRY_DIALOG,
      file_string_RomModuleBName_callback,
      (ui_callback_data_t)"Select ROM B image" },
    { "Detach ROM B",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      (ui_callback_data_t)"RomModuleBName" },
    SDL_MENU_ITEM_SEPARATOR,
    { "I/O collision handling ($8800-$8FFF / $E900-$EEFF)",
      MENU_ENTRY_SUBMENU,
      iocollision_show_type_callback,
      (ui_callback_data_t)iocollision_menu },
    SDL_MENU_LIST_END
};
