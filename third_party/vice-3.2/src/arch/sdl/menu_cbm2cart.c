/*
 * menu_cbm2cart.c - Implementation of the cbm2 cartridge settings menu for the SDL UI.
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
#include "cartridge.h"
#include "lib.h"
#include "menu_cbm2cart.h"
#include "menu_common.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

UI_MENU_DEFINE_TOGGLE(CartridgeReset)

static UI_MENU_CALLBACK(attach_cart_callback)
{
    char *name;
    if (activated) {
        name = sdl_ui_file_selection_dialog("Select Cart image", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (cartridge_attach_image(vice_ptr_to_int(param), name) < 0) {
                ui_error("Cannot load cartridge image.");
            }
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(detach_cart_callback)
{
    int m;
    m = vice_ptr_to_int(param);
    if (activated) {
        cartridge_detach_image(m);
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

const ui_menu_entry_t cbm2cart_menu[] = {
    { "Load new Cart $1***", MENU_ENTRY_OTHER, attach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_1000 },
    { "Unload Cart $1***", MENU_ENTRY_OTHER, detach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_1000 },
    { "Load new Cart $2-3***", MENU_ENTRY_OTHER, attach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_2000 },
    { "Unload Cart $2-3***", MENU_ENTRY_OTHER, detach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_2000 },
    { "Load new Cart $4-5***", MENU_ENTRY_OTHER, attach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_4000 },
    { "Unload Cart $4-5***", MENU_ENTRY_OTHER, detach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_4000 },
    { "Load new Cart $6-7***", MENU_ENTRY_OTHER, attach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_6000 },
    { "Unload Cart $6-7***", MENU_ENTRY_OTHER, detach_cart_callback, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_6000 },
    SDL_MENU_ITEM_SEPARATOR,
    { "I/O collision handling ($D800-$DFFF)", MENU_ENTRY_SUBMENU, iocollision_show_type_callback, (ui_callback_data_t)iocollision_menu },
    { "Reset on cartridge change", MENU_ENTRY_RESOURCE_TOGGLE, toggle_CartridgeReset_callback, NULL },
    SDL_MENU_LIST_END
};
