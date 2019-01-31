/*
 * uinetplay.c - Netplay dialog for the MS-DOS version of VICE.
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

#ifdef HAVE_NETWORK
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "machine.h"
#include "network.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "tuifs.h"
#include "ui.h"
#include "uinetplay.h"
#include "util.h"
#include "videoarch.h"

static TUI_MENU_CALLBACK(ui_netplay_set_port_callback);
static TUI_MENU_CALLBACK(ui_netplay_set_bind_callback);
static TUI_MENU_CALLBACK(ui_netplay_start_server_callback);
static TUI_MENU_CALLBACK(ui_netplay_set_host_callback);
static TUI_MENU_CALLBACK(ui_netplay_connect_to_server_callback);
static TUI_MENU_CALLBACK(ui_netplay_disconnect_callback);

tui_menu_item_def_t ui_netplay_menu_def[] = {
    { "TCP Port",
      "Set the TCP port to use",
      ui_netplay_set_port_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Server Bind",
      "Bind to this host",
      ui_netplay_set_bind_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Start server",
      "Start the netplay server",
      ui_netplay_start_server_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Hostname",
      "Name of the server to connect to",
      ui_netplay_set_host_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Connect to server",
      "Connect to the netplay server",
      ui_netplay_connect_to_server_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Disconnect",
      "Disconnect netplay server/client",
      ui_netplay_disconnect_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(ui_netplay_set_host_callback)
{
    if (been_activated) {
      const char *current_host;
      char buf[44];

      resources_get_string("NetworkServerName", &current_host);
    
      strncpy(buf, current_host, 40);

      if (tui_input_string("Hostname", "Enter the name of the netplay server:", buf, 40) == 0) {
          resources_set_string("NetworkServerName", buf);
          tui_message("Hostname set to : %s",buf);
      } else {
          return NULL;
      }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_netplay_set_bind_callback)
{
    if (been_activated) {
        const char *current_bind;
        char buf[44];

        resources_get_string("NetworkServerBindAddress", &current_bind);
    
        strncpy(buf, current_bind, 40);

        if (tui_input_string("Bind address", "Enter the address to bind to:", buf, 40) == 0) {
            resources_set_string("NetworkServerBindAddress", buf);
            tui_message("Bind address set to : %s",buf);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_netplay_set_port_callback)
{
    if (been_activated) {
        int current_port, value;
        char buf[10];

        resources_get_int("NetworkServerPort", &current_port);
        sprintf(buf, "%d", current_port);

        if (tui_input_string("TCP Port", "Enter TCP Port to use:", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 65535) {
                value = 65535;
            } else if (value < 1) {
                value = 1;
            }
            resources_set_int("NetworkServerPort", value);
            tui_message("TPC Port set to : %d",value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_netplay_start_server_callback)
{
    if (been_activated) {
        if (network_start_server() < 0) {
            tui_error("Error starting the netplay server.");
        } else {
            tui_message("Netplay server started succesfully.");
        }
        return NULL;
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_netplay_connect_to_server_callback)
{
    if (been_activated) {
        if (network_connect_client() < 0) {
            tui_error("Error connecting to server.");
        } else {
            tui_message("Connected to server.");
        }
        return NULL;
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_netplay_disconnect_callback)
{
    if (been_activated) {
        network_disconnect();
        tui_message("Disconnected.");
        return NULL;
    }
    return NULL;
}
#endif
