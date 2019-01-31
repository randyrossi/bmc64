/*
 * menu_network.c - Network menu for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#ifdef HAVE_NETWORK

#include <stdio.h>

#include "types.h"

#include "menu_common.h"
#include "menu_network.h"
#include "network.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

UI_MENU_DEFINE_STRING(NetworkServerName)
UI_MENU_DEFINE_STRING(NetworkServerBindAddress)
UI_MENU_DEFINE_INT(NetworkServerPort)

static UI_MENU_CALLBACK(custom_network_control_callback)
{
    int value;
    int flag = vice_ptr_to_int(param);

    resources_get_int("NetworkControl", &value);

    if (activated) {
        value ^= flag;
        resources_set_int("NetworkControl", value);
    } else {
        if (value & flag) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static const ui_menu_entry_t network_control_menu[] = {
    { "Server keyboard",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)NETWORK_CONTROL_KEYB },
    { "Server joystick 1",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)NETWORK_CONTROL_JOY1 },
    { "Server joystick 2",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)NETWORK_CONTROL_JOY2 },
    { "Server devices",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)NETWORK_CONTROL_DEVC },
    { "Server settings",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)NETWORK_CONTROL_RSRC },
    { "Client keyboard",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)(NETWORK_CONTROL_KEYB << NETWORK_CONTROL_CLIENTOFFSET) },
    { "Client joystick 1",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)(NETWORK_CONTROL_JOY1 << NETWORK_CONTROL_CLIENTOFFSET) },
    { "Client joystick 2",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)(NETWORK_CONTROL_JOY2 << NETWORK_CONTROL_CLIENTOFFSET) },
    { "Client devices",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)(NETWORK_CONTROL_DEVC << NETWORK_CONTROL_CLIENTOFFSET) },
    { "Client settings",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_network_control_callback,
      (ui_callback_data_t)(NETWORK_CONTROL_RSRC << NETWORK_CONTROL_CLIENTOFFSET) },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(custom_connect_client_callback)
{
    if (activated) {
        if (network_connect_client() < 0) {
            ui_error("Couldn't connect client.");
        } else {
            return sdl_menu_text_exit_ui;
        }
    }

    return NULL;
}

static UI_MENU_CALLBACK(custom_start_server_callback)
{
    if (activated) {
        if (network_start_server() < 0) {
            ui_error("Couldn't start netplay server.");
        } else {
            return sdl_menu_text_exit_ui;
        }
    }

    return NULL;
}

static UI_MENU_CALLBACK(custom_disconnect_callback)
{
    if (activated) {
        network_disconnect();
    }

    return NULL;
}

const ui_menu_entry_t network_menu[] = {
    SDL_MENU_ITEM_TITLE("Netplay"),
    { "Server name",
      MENU_ENTRY_RESOURCE_STRING,
      string_NetworkServerName_callback,
      (ui_callback_data_t)"Set network server name" },
    { "Server port",
      MENU_ENTRY_RESOURCE_INT,
      int_NetworkServerPort_callback,
      (ui_callback_data_t)"Set network server port" },
    { "Bind address",
      MENU_ENTRY_RESOURCE_STRING,
      string_NetworkServerBindAddress_callback,
      (ui_callback_data_t)"Set network server bind address" },
    { "Control",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)network_control_menu },
    { "Start server",
      MENU_ENTRY_OTHER,
      custom_start_server_callback,
      NULL },
    { "Connect",
      MENU_ENTRY_OTHER,
      custom_connect_client_callback,
      NULL },
    { "Disconnect",
      MENU_ENTRY_OTHER,
      custom_disconnect_callback,
      NULL },
    SDL_MENU_LIST_END
};

#endif
