/*
 * menu_plus4cart.c - Implementation of the plus4 cartridge settings menu for the SDL UI.
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
#include "menu_common.h"
#include "menu_plus4cart.h"
#include "plus4cart.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

static UI_MENU_CALLBACK(attach_cart_callback)
{
    char *name = NULL;
    int type = vice_ptr_to_int(param);

    if (activated) {
        name = sdl_ui_file_selection_dialog("Select cartridge image", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (cartridge_attach_image(type, name) < 0) {
                ui_error("Cannot load cartridge image.");
            }
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(detach_cart_callback)
{
    if (activated) {
        cartridge_detach_image(-1);
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

UI_MENU_DEFINE_TOGGLE(CartridgeReset)

const ui_menu_entry_t plus4cart_menu[] = {
    { "Smart attach cartridge image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_PLUS4_DETECT },
    SDL_MENU_ITEM_SEPARATOR,
    { "Attach C0 low image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_PLUS4_16KB_C0LO },
    { "Attach C0 high image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_PLUS4_16KB_C0HI },
    { "Attach C1 low image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_PLUS4_16KB_C1LO },
    { "Attach C1 high image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_PLUS4_16KB_C1HI },
    { "Attach C2 low image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_PLUS4_16KB_C2LO },
    { "Attach C2 high image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_PLUS4_16KB_C2HI },
    SDL_MENU_ITEM_SEPARATOR,
    { "Detach cartridge image",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      NULL },
    { "I/O collision handling ($FD00-$FEFF)",
      MENU_ENTRY_SUBMENU,
      iocollision_show_type_callback,
      (ui_callback_data_t)iocollision_menu },
    { "Reset on cartridge change",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CartridgeReset_callback,
      NULL },
    SDL_MENU_LIST_END
};
