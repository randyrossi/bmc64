/*
 * menu_ethernet.c - Ethernet settings menu for SDL UI.
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

#ifdef HAVE_RAWNET

#include <stdio.h>

#include "types.h"

#include "lib.h"
#include "menu_common.h"
#include "menu_ethernet.h"
#include "rawnet.h"
#include "resources.h"
#include "uimenu.h"

/* *nix Ethernet settings */
#ifdef UNIX_COMPILE

void sdl_menu_ethernet_interface_free(void)
{
}

UI_MENU_DEFINE_STRING(ETHERNET_INTERFACE)

#define VICE_SDL_ETHERNET_ARCHDEP_ITEMS   \
    { "Interface",                        \
      MENU_ENTRY_RESOURCE_STRING,         \
      string_ETHERNET_INTERFACE_callback, \
      (ui_callback_data_t)"Ethernet interface" },

#endif /* defined(UNIX_COMPILE) */

/* win32 TFE settings */
#ifdef WIN32_COMPILE

UI_MENU_DEFINE_RADIO(ETHERNET_INTERFACE)

static ui_menu_entry_t ethernet_interface_dyn_menu[21];
static int ethernet_interface_dyn_menu_init = 0;

void sdl_menu_ethernet_interface_free(void)
{
    int i;

    for (i = 0; ethernet_interface_dyn_menu[i].string != NULL; i++) {
        lib_free(ethernet_interface_dyn_menu[i].string);
        lib_free(ethernet_interface_dyn_menu[i].data);
    }
}

UI_MENU_CALLBACK(ETHERNET_INTERFACE_dynmenu_callback)
{
    char *pname;
    char *pdescription;
    int i;

    if (ethernet_interface_dyn_menu_init != 0) {
        sdl_menu_ethernet_interface_free();
    } else {
        ethernet_interface_dyn_menu_init = 1;
    }

    if (!rawnet_enumadapter_open()) {
        ethernet_interface_dyn_menu[0].string = (char *)lib_stralloc("No Devices Present");
        ethernet_interface_dyn_menu[0].type = MENU_ENTRY_TEXT;
        ethernet_interface_dyn_menu[0].callback = seperator_callback;
        ethernet_interface_dyn_menu[0].data = NULL;
        ethernet_interface_dyn_menu[1].string = NULL;
        ethernet_interface_dyn_menu[1].type = 0;
        ethernet_interface_dyn_menu[1].callback = NULL;
        ethernet_interface_dyn_menu[1].data = NULL;
    } else {
        for (i = 0; (rawnet_enumadapter(&pname, &pdescription)) && (i < 20); i++) {
            ethernet_interface_dyn_menu[i].string = (char *)lib_stralloc(pdescription);
            ethernet_interface_dyn_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
            ethernet_interface_dyn_menu[i].callback = radio_ETHERNET_INTERFACE_callback;
            ethernet_interface_dyn_menu[i].data = (ui_callback_data_t)(char *)lib_stralloc(pname);
        }
        ethernet_interface_dyn_menu[i].string = NULL;
        ethernet_interface_dyn_menu[i].type = 0;
        ethernet_interface_dyn_menu[i].callback = NULL;
        ethernet_interface_dyn_menu[i].data = NULL;
    }
    return "->";
}

#define VICE_SDL_ETHERNET_ARCHDEP_ITEMS    \
    { "Interface",                         \
      MENU_ENTRY_DYNAMIC_SUBMENU,          \
      ETHERNET_INTERFACE_dynmenu_callback, \
      (ui_callback_data_t)ethernet_interface_dyn_menu },

#endif /* defined(WIN32_COMPILE) */

/* Common menus */

static UI_MENU_CALLBACK(show_ETHERNET_DISABLED_callback)
{
    int value;

    resources_get_int("ETHERNET_DISABLED", &value);

    return value ? "(disabled)" : NULL;
}

const ui_menu_entry_t ethernet_menu[] = {
    { "Ethernet support",
      MENU_ENTRY_TEXT,
      show_ETHERNET_DISABLED_callback,
      (ui_callback_data_t)1 },
    VICE_SDL_ETHERNET_ARCHDEP_ITEMS
    SDL_MENU_LIST_END
};

#endif /* HAVE_RAWNET */
