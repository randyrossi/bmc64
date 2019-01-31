/*
 * uijoyport.c - Implementation of joyport UI settings.
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
#include <string.h>

#include "lib.h"
#include "joyport.h"
#include "translate.h"
#include "uilib.h"
#include "uimenu.h"
#include "uijoyport.h"

static int ports[JOYPORT_MAX_PORTS];

UI_MENU_DEFINE_RADIO(JoyPort1Device)
UI_MENU_DEFINE_RADIO(JoyPort2Device)
UI_MENU_DEFINE_RADIO(JoyPort3Device)
UI_MENU_DEFINE_RADIO(JoyPort4Device)
UI_MENU_DEFINE_RADIO(JoyPort5Device)

UI_MENU_DEFINE_TOGGLE(BBRTCSave)

ui_menu_entry_t joyport_settings_submenu[JOYPORT_MAX_PORTS + 1];

ui_menu_entry_t ui_joyport_settings_menu[] = {
    { N_("Control port settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, joyport_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save BBRTC data when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_BBRTCSave, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

void uijoyport_menu_create(int port1, int port2, int port3, int port4, int port5)
{
    unsigned int i, num;
    int j = 0;
    ui_menu_entry_t *devices_submenu1;
    ui_menu_entry_t *devices_submenu2;
    ui_menu_entry_t *devices_submenu3;
    ui_menu_entry_t *devices_submenu4;
    ui_menu_entry_t *devices_submenu5;
    joyport_desc_t *devices_port_1 = NULL;
    joyport_desc_t *devices_port_2 = NULL;
    joyport_desc_t *devices_port_3 = NULL;
    joyport_desc_t *devices_port_4 = NULL;
    joyport_desc_t *devices_port_5 = NULL;

    ports[JOYPORT_1] = port1;
    ports[JOYPORT_2] = port2;
    ports[JOYPORT_3] = port3;
    ports[JOYPORT_4] = port4;
    ports[JOYPORT_5] = port5;

    memset(joyport_settings_submenu, 0, sizeof(joyport_settings_submenu));

    if (port1) {
        devices_port_1 = joyport_get_valid_devices(JOYPORT_1);
        for (i = 0; devices_port_1[i].name; ++i) {}
        num = i;

        devices_submenu1 = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));

        for (i = 0; i < num ; i++) {
            devices_submenu1[i].string = (ui_callback_data_t)lib_msprintf("%s", translate_text(devices_port_1[i].trans_name));
            devices_submenu1[i].type = UI_MENU_TYPE_TICK;
            devices_submenu1[i].callback = (ui_callback_t)radio_JoyPort1Device;
            devices_submenu1[i].callback_data = (ui_callback_data_t)(unsigned long)devices_port_1[i].id;
        }
        joyport_settings_submenu[j].string = translate_text(joyport_get_port_trans_name(JOYPORT_1));
        joyport_settings_submenu[j].type = UI_MENU_TYPE_NORMAL;
        joyport_settings_submenu[j].sub_menu = devices_submenu1;
        ++j;
    }

    if (port2) {
        devices_port_2 = joyport_get_valid_devices(JOYPORT_2);
        for (i = 0; devices_port_2[i].name; ++i) {}
        num = i;

        devices_submenu2 = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));

        for (i = 0; i < num ; i++) {
            devices_submenu2[i].string = (ui_callback_data_t)lib_msprintf("%s", translate_text(devices_port_2[i].trans_name));
            devices_submenu2[i].type = UI_MENU_TYPE_TICK;
            devices_submenu2[i].callback = (ui_callback_t)radio_JoyPort2Device;
            devices_submenu2[i].callback_data = (ui_callback_data_t)(unsigned long)devices_port_2[i].id;
        }
        joyport_settings_submenu[j].string = translate_text(joyport_get_port_trans_name(JOYPORT_2));
        joyport_settings_submenu[j].type = UI_MENU_TYPE_NORMAL;
        joyport_settings_submenu[j].sub_menu = devices_submenu2;
        ++j;
    }

    if (port3) {
        devices_port_3 = joyport_get_valid_devices(JOYPORT_3);
        for (i = 0; devices_port_3[i].name; ++i) {}
        num = i;

        devices_submenu3 = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));

        for (i = 0; i < num ; i++) {
            devices_submenu3[i].string = (ui_callback_data_t)lib_msprintf("%s", translate_text(devices_port_3[i].trans_name));
            devices_submenu3[i].type = UI_MENU_TYPE_TICK;
            devices_submenu3[i].callback = (ui_callback_t)radio_JoyPort3Device;
            devices_submenu3[i].callback_data = (ui_callback_data_t)(unsigned long)devices_port_3[i].id;
        }
        joyport_settings_submenu[j].string = translate_text(joyport_get_port_trans_name(JOYPORT_3));
        joyport_settings_submenu[j].type = UI_MENU_TYPE_NORMAL;
        joyport_settings_submenu[j].sub_menu = devices_submenu3;
        ++j;
    }

    if (port4) {
        devices_port_4 = joyport_get_valid_devices(JOYPORT_4);
        for (i = 0; devices_port_4[i].name; ++i) {}
        num = i;

        devices_submenu4 = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));

        for (i = 0; i < num ; i++) {
            devices_submenu4[i].string = (ui_callback_data_t)lib_msprintf("%s", translate_text(devices_port_4[i].trans_name));
            devices_submenu4[i].type = UI_MENU_TYPE_TICK;
            devices_submenu4[i].callback = (ui_callback_t)radio_JoyPort4Device;
            devices_submenu4[i].callback_data = (ui_callback_data_t)(unsigned long)devices_port_4[i].id;
        }
        joyport_settings_submenu[j].string = translate_text(joyport_get_port_trans_name(JOYPORT_4));
        joyport_settings_submenu[j].type = UI_MENU_TYPE_NORMAL;
        joyport_settings_submenu[j].sub_menu = devices_submenu4;
        ++j;
    }

    if (port5) {
        devices_port_5 = joyport_get_valid_devices(JOYPORT_5);
        for (i = 0; devices_port_5[i].name; ++i) {}
        num = i;

        devices_submenu5 = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));

        for (i = 0; i < num ; i++) {
            devices_submenu5[i].string = (ui_callback_data_t)lib_msprintf("%s", translate_text(devices_port_5[i].trans_name));
            devices_submenu5[i].type = UI_MENU_TYPE_TICK;
            devices_submenu5[i].callback = (ui_callback_t)radio_JoyPort5Device;
            devices_submenu5[i].callback_data = (ui_callback_data_t)(unsigned long)devices_port_5[i].id;
        }
        joyport_settings_submenu[j].string = translate_text(joyport_get_port_trans_name(JOYPORT_5));
        joyport_settings_submenu[j].type = UI_MENU_TYPE_NORMAL;
        joyport_settings_submenu[j].sub_menu = devices_submenu5;
        ++j;
    }

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
}

void uijoyport_menu_shutdown(void)
{
    unsigned int i;
    ui_menu_entry_t *devices_submenu1 = NULL;
    ui_menu_entry_t *devices_submenu2 = NULL;
    ui_menu_entry_t *devices_submenu3 = NULL;
    ui_menu_entry_t *devices_submenu4 = NULL;
    ui_menu_entry_t *devices_submenu5 = NULL;
    int j = 0;

    if (ports[JOYPORT_1]) {
        devices_submenu1 = joyport_settings_submenu[j].sub_menu;
        joyport_settings_submenu[j].sub_menu = NULL;
        for (i = 0; devices_submenu1[i].string; ++i) {
            lib_free(devices_submenu1[i].string);
        }
        lib_free(devices_submenu1);
        ++j;
    }

    if (ports[JOYPORT_2]) {
        devices_submenu2 = joyport_settings_submenu[j].sub_menu;
        joyport_settings_submenu[j].sub_menu = NULL;
        for (i = 0; devices_submenu2[i].string; ++i) {
            lib_free(devices_submenu2[i].string);
        }
        lib_free(devices_submenu2);
        ++j;
    }

    if (ports[JOYPORT_3]) {
        devices_submenu3 = joyport_settings_submenu[j].sub_menu;
        joyport_settings_submenu[j].sub_menu = NULL;
        for (i = 0; devices_submenu3[i].string; ++i) {
            lib_free(devices_submenu3[i].string);
        }
        lib_free(devices_submenu3);
        ++j;
    }

    if (ports[JOYPORT_4]) {
        devices_submenu4 = joyport_settings_submenu[j].sub_menu;
        joyport_settings_submenu[j].sub_menu = NULL;
        for (i = 0; devices_submenu4[i].string; ++i) {
            lib_free(devices_submenu4[i].string);
        }
        lib_free(devices_submenu4);
        ++j;
    }

    if (ports[JOYPORT_5]) {
        devices_submenu5 = joyport_settings_submenu[j].sub_menu;
        joyport_settings_submenu[j].sub_menu = NULL;
        for (i = 0; devices_submenu5[i].string; ++i) {
            lib_free(devices_submenu5[i].string);
        }
        lib_free(devices_submenu5);
        ++j;
    }
}
