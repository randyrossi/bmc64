/*
 * uijoyport.c - Joyport UI interface for MS-DOS.
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

#include "joyport.h"
#include "lib.h"
#include "mouse.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uijoyport.h"
#include "uijoystick.h"

TUI_MENU_DEFINE_TOGGLE(Mouse)
TUI_MENU_DEFINE_TOGGLE(SmartMouseRTCSave)
TUI_MENU_DEFINE_TOGGLE(BBRTCSave)
TUI_MENU_DEFINE_RADIO(JoyPort1Device)
TUI_MENU_DEFINE_RADIO(JoyPort2Device)
TUI_MENU_DEFINE_RADIO(JoyPort3Device)
TUI_MENU_DEFINE_RADIO(JoyPort4Device)
TUI_MENU_DEFINE_RADIO(JoyPort5Device)

static TUI_MENU_CALLBACK(joyport1_submenu_callback)
{
    int value;
    char *s;
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_1);

    resources_get_int("JoyPort1Device", &value);
    s = devices[value].name;
    lib_free(devices);
    return s;
}

static TUI_MENU_CALLBACK(joyport2_submenu_callback)
{
    int value;
    char *s;
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_2);

    resources_get_int("JoyPort2Device", &value);
    s = devices[value].name;
    lib_free(devices);
    return s;
}

static TUI_MENU_CALLBACK(joyport3_submenu_callback)
{
    int value;
    char *s;
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_3);

    resources_get_int("JoyPort3Device", &value);
    s = devices[value].name;
    lib_free(devices);
    return s;
}

static TUI_MENU_CALLBACK(joyport4_submenu_callback)
{
    int value;
    char *s;
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_4);

    resources_get_int("JoyPort4Device", &value);
    s = devices[value].name;
    lib_free(devices);
    return s;
}

static TUI_MENU_CALLBACK(joyport5_submenu_callback)
{
    int value;
    char *s;
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_5);

    resources_get_int("JoyPort5Device", &value);
    s = devices[value].name;
    lib_free(devices);
    return s;
}

static tui_menu_item_def_t joyport1_submenu[JOYPORT_MAX_DEVICES + 1];
static tui_menu_item_def_t joyport2_submenu[JOYPORT_MAX_DEVICES + 1];
static tui_menu_item_def_t joyport3_submenu[JOYPORT_MAX_DEVICES + 1];
static tui_menu_item_def_t joyport4_submenu[JOYPORT_MAX_DEVICES + 1];
static tui_menu_item_def_t joyport5_submenu[JOYPORT_MAX_DEVICES + 1];

static int joyport_ports[JOYPORT_MAX_PORTS];
static tui_menu_item_def_t joyport_menu_items[JOYPORT_MAX_PORTS + 4];

void uijoyport_init(struct tui_menu *parent_submenu, int port1, int port2, int port3, int port4, int port5)
{
    tui_menu_t ui_joyport_submenu;
    joyport_desc_t *devices_port_1 = NULL;
    joyport_desc_t *devices_port_2 = NULL;
    joyport_desc_t *devices_port_3 = NULL;
    joyport_desc_t *devices_port_4 = NULL;
    joyport_desc_t *devices_port_5 = NULL;
    int i;
    int j = 0;

    joyport_ports[JOYPORT_1] = port1;
    joyport_ports[JOYPORT_2] = port2;
    joyport_ports[JOYPORT_3] = port3;
    joyport_ports[JOYPORT_4] = port4;
    joyport_ports[JOYPORT_5] = port5;

    ui_joyport_submenu = tui_menu_create("Control port settings", 1);

    if (joyport_ports[JOYPORT_1]) {
        devices_port_1 = joyport_get_valid_devices(JOYPORT_1);
        for (i = 0; devices_port_1[i].name; ++i) {
            joyport1_submenu[i].label = devices_port_1[i].name;
            joyport1_submenu[i].help_string = NULL;
            joyport1_submenu[i].callback = radio_JoyPort1Device_callback;
            joyport1_submenu[i].callback_param = (void *)devices_port_1[i].id;
            joyport1_submenu[i].par_string_max_len = 20;
            joyport1_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
            joyport1_submenu[i].submenu = NULL;
            joyport1_submenu[i].submenu_title = NULL;
        }
        joyport1_submenu[i].label = NULL;
        joyport1_submenu[i].help_string = NULL;
        joyport1_submenu[i].callback = NULL;
        joyport1_submenu[i].callback_param = NULL;
        joyport1_submenu[i].par_string_max_len = 0;
        joyport1_submenu[i].behavior = 0;
        joyport1_submenu[i].submenu = NULL;
        joyport1_submenu[i].submenu_title = NULL;
        joyport_menu_items[j].label = joyport_get_port_name(JOYPORT_1);
        joyport_menu_items[j].help_string = "Select the device for this control port";
        joyport_menu_items[j].callback = joyport1_submenu_callback;
        joyport_menu_items[j].callback_param = NULL;
        joyport_menu_items[j].par_string_max_len = 25;
        joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
        joyport_menu_items[j].submenu = joyport1_submenu;
        joyport_menu_items[j].submenu_title = joyport_get_port_name(JOYPORT_1);
        ++j;
    }

    if (joyport_ports[JOYPORT_2]) {
        devices_port_2 = joyport_get_valid_devices(JOYPORT_2);
        for (i = 0; devices_port_2[i].name; ++i) {
            joyport2_submenu[i].label = devices_port_2[i].name;
            joyport2_submenu[i].help_string = NULL;
            joyport2_submenu[i].callback = radio_JoyPort2Device_callback;
            joyport2_submenu[i].callback_param = (void *)devices_port_2[i].id;
            joyport2_submenu[i].par_string_max_len = 20;
            joyport2_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
            joyport2_submenu[i].submenu = NULL;
            joyport2_submenu[i].submenu_title = NULL;
        }
        joyport2_submenu[i].label = NULL;
        joyport2_submenu[i].help_string = NULL;
        joyport2_submenu[i].callback = NULL;
        joyport2_submenu[i].callback_param = NULL;
        joyport2_submenu[i].par_string_max_len = 0;
        joyport2_submenu[i].behavior = 0;
        joyport2_submenu[i].submenu = NULL;
        joyport2_submenu[i].submenu_title = NULL;
        joyport_menu_items[j].label = joyport_get_port_name(JOYPORT_2);
        joyport_menu_items[j].help_string = "Select the device for this control port";
        joyport_menu_items[j].callback = joyport2_submenu_callback;
        joyport_menu_items[j].callback_param = NULL;
        joyport_menu_items[j].par_string_max_len = 25;
        joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
        joyport_menu_items[j].submenu = joyport2_submenu;
        joyport_menu_items[j].submenu_title = joyport_get_port_name(JOYPORT_2);
        ++j;
    }

    if (joyport_ports[JOYPORT_3]) {
        devices_port_3 = joyport_get_valid_devices(JOYPORT_3);
        for (i = 0; devices_port_3[i].name; ++i) {
            joyport3_submenu[i].label = devices_port_3[i].name;
            joyport3_submenu[i].help_string = NULL;
            joyport3_submenu[i].callback = radio_JoyPort3Device_callback;
            joyport3_submenu[i].callback_param = (void *)devices_port_3[i].id;
            joyport3_submenu[i].par_string_max_len = 20;
            joyport3_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
            joyport3_submenu[i].submenu = NULL;
            joyport3_submenu[i].submenu_title = NULL;
        }
        joyport3_submenu[i].label = NULL;
        joyport3_submenu[i].help_string = NULL;
        joyport3_submenu[i].callback = NULL;
        joyport3_submenu[i].callback_param = NULL;
        joyport3_submenu[i].par_string_max_len = 0;
        joyport3_submenu[i].behavior = 0;
        joyport3_submenu[i].submenu = NULL;
        joyport3_submenu[i].submenu_title = NULL;
        joyport_menu_items[j].label = joyport_get_port_name(JOYPORT_3);
        joyport_menu_items[j].help_string = "Select the device for this control port";
        joyport_menu_items[j].callback = joyport3_submenu_callback;
        joyport_menu_items[j].callback_param = NULL;
        joyport_menu_items[j].par_string_max_len = 25;
        joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
        joyport_menu_items[j].submenu = joyport3_submenu;
        joyport_menu_items[j].submenu_title = joyport_get_port_name(JOYPORT_3);
        ++j;
    }

    if (joyport_ports[JOYPORT_4]) {
        devices_port_4 = joyport_get_valid_devices(JOYPORT_4);
        for (i = 0; devices_port_4[i].name; ++i) {
            joyport4_submenu[i].label = devices_port_4[i].name;
            joyport4_submenu[i].help_string = NULL;
            joyport4_submenu[i].callback = radio_JoyPort4Device_callback;
            joyport4_submenu[i].callback_param = (void *)devices_port_4[i].id;
            joyport4_submenu[i].par_string_max_len = 20;
            joyport4_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
            joyport4_submenu[i].submenu = NULL;
            joyport4_submenu[i].submenu_title = NULL;
        }
        joyport4_submenu[i].label = NULL;
        joyport4_submenu[i].help_string = NULL;
        joyport4_submenu[i].callback = NULL;
        joyport4_submenu[i].callback_param = NULL;
        joyport4_submenu[i].par_string_max_len = 0;
        joyport4_submenu[i].behavior = 0;
        joyport4_submenu[i].submenu = NULL;
        joyport4_submenu[i].submenu_title = NULL;
        joyport_menu_items[j].label = joyport_get_port_name(JOYPORT_4);
        joyport_menu_items[j].help_string = "Select the device for this control port";
        joyport_menu_items[j].callback = joyport4_submenu_callback;
        joyport_menu_items[j].callback_param = NULL;
        joyport_menu_items[j].par_string_max_len = 25;
        joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
        joyport_menu_items[j].submenu = joyport4_submenu;
        joyport_menu_items[j].submenu_title = joyport_get_port_name(JOYPORT_4);
        ++j;
    }

    if (joyport_ports[JOYPORT_5]) {
        devices_port_5 = joyport_get_valid_devices(JOYPORT_5);
        for (i = 0; devices_port_5[i].name; ++i) {
            joyport5_submenu[i].label = devices_port_5[i].name;
            joyport5_submenu[i].help_string = NULL;
            joyport5_submenu[i].callback = radio_JoyPort5Device_callback;
            joyport5_submenu[i].callback_param = (void *)devices_port_5[i].id;
            joyport5_submenu[i].par_string_max_len = 20;
            joyport5_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
            joyport5_submenu[i].submenu = NULL;
            joyport5_submenu[i].submenu_title = NULL;
        }
        joyport5_submenu[i].label = NULL;
        joyport5_submenu[i].help_string = NULL;
        joyport5_submenu[i].callback = NULL;
        joyport5_submenu[i].callback_param = NULL;
        joyport5_submenu[i].par_string_max_len = 0;
        joyport5_submenu[i].behavior = 0;
        joyport5_submenu[i].submenu = NULL;
        joyport5_submenu[i].submenu_title = NULL;
        joyport_menu_items[j].label = joyport_get_port_name(JOYPORT_5);
        joyport_menu_items[j].help_string = "Select the device for this control port";
        joyport_menu_items[j].callback = joyport5_submenu_callback;
        joyport_menu_items[j].callback_param = NULL;
        joyport_menu_items[j].par_string_max_len = 25;
        joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
        joyport_menu_items[j].submenu = joyport5_submenu;
        joyport_menu_items[j].submenu_title = joyport_get_port_name(JOYPORT_5);
        ++j;
    }

    joyport_menu_items[j].label = "Save BBRTC data when changed";
    joyport_menu_items[j].help_string = "Save BBRTC data when changed";
    joyport_menu_items[j].callback = toggle_BBRTCSave_callback;
    joyport_menu_items[j].callback_param = NULL;
    joyport_menu_items[j].par_string_max_len = 3;
    joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
    joyport_menu_items[j].submenu = NULL;
    joyport_menu_items[j].submenu_title = NULL;
    ++j;

    if (joyport_ports[JOYPORT_1] == 2 || joyport_ports[JOYPORT_2] == 2 || joyport_ports[JOYPORT_3] == 2 || joyport_ports[JOYPORT_4] == 2 || joyport_ports[JOYPORT_5] == 2) {
        joyport_menu_items[j].label = "Save Smart Mouse RTC data when changed";
        joyport_menu_items[j].help_string = "Save Smart Mouse RTC data when changed";
        joyport_menu_items[j].callback = toggle_SmartMouseRTCSave_callback;
        joyport_menu_items[j].callback_param = NULL;
        joyport_menu_items[j].par_string_max_len = 3;
        joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
        joyport_menu_items[j].submenu = NULL;
        joyport_menu_items[j].submenu_title = NULL;
        ++j;
    }

    joyport_menu_items[j].label = "Grab mouse events:";
    joyport_menu_items[j].help_string = "Emulate a mouse";
    joyport_menu_items[j].callback = toggle_Mouse_callback;
    joyport_menu_items[j].callback_param = NULL;
    joyport_menu_items[j].par_string_max_len = 3;
    joyport_menu_items[j].behavior = TUI_MENU_BEH_CONTINUE;
    joyport_menu_items[j].submenu = NULL;
    joyport_menu_items[j].submenu_title = NULL;
    ++j;

    joyport_menu_items[j].label = NULL;
    joyport_menu_items[j].help_string = NULL;
    joyport_menu_items[j].callback = NULL;
    joyport_menu_items[j].callback_param = NULL;
    joyport_menu_items[j].par_string_max_len = 0;
    joyport_menu_items[j].behavior = 0;
    joyport_menu_items[j].submenu = NULL;
    joyport_menu_items[j].submenu_title = NULL;

    tui_menu_add(ui_joyport_submenu, joyport_menu_items);

    if (devices_port_1) {
        lib_free(devices_port_1);
    }
    if (devices_port_2) {
        lib_free(devices_port_2);
    }
    if (devices_port_3) {
        lib_free(devices_port_3);
    }
    if (devices_port_4) {
        lib_free(devices_port_4);
    }
    if (devices_port_5) {
        lib_free(devices_port_5);
    }

    tui_menu_add_submenu(parent_submenu, "_Control port settings...",
                         "Control port settings",
                         ui_joyport_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    uijoystick_init(ui_joyport_submenu);
}
