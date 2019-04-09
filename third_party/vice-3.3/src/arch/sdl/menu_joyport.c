/*
 * menu_joyport.c - Joyport menu for SDL UI.
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
#include "joyport.h"
#include "uimenu.h"
#include "lib.h"

#include "menu_joyport.h"

UI_MENU_DEFINE_RADIO(JoyPort1Device)
UI_MENU_DEFINE_RADIO(JoyPort2Device)
UI_MENU_DEFINE_RADIO(JoyPort3Device)
UI_MENU_DEFINE_RADIO(JoyPort4Device)
UI_MENU_DEFINE_RADIO(JoyPort5Device)

static ui_menu_entry_t joyport1_dyn_menu[JOYPORT_MAX_DEVICES + 1];
static ui_menu_entry_t joyport2_dyn_menu[JOYPORT_MAX_DEVICES + 1];
static ui_menu_entry_t joyport3_dyn_menu[JOYPORT_MAX_DEVICES + 1];
static ui_menu_entry_t joyport4_dyn_menu[JOYPORT_MAX_DEVICES + 1];
static ui_menu_entry_t joyport5_dyn_menu[JOYPORT_MAX_DEVICES + 1];

static int joyport1_dyn_menu_init = 0;
static int joyport2_dyn_menu_init = 0;
static int joyport3_dyn_menu_init = 0;
static int joyport4_dyn_menu_init = 0;
static int joyport5_dyn_menu_init = 0;

static void sdl_menu_joyport1_free(void)
{
    int i;

    for (i = 0; joyport1_dyn_menu[i].string != NULL; i++) {
        lib_free(joyport1_dyn_menu[i].string);
    }
}

static void sdl_menu_joyport2_free(void)
{
    int i;

    for (i = 0; joyport2_dyn_menu[i].string != NULL; i++) {
        lib_free(joyport2_dyn_menu[i].string);
    }
}

static void sdl_menu_joyport3_free(void)
{
    int i;

    for (i = 0; joyport3_dyn_menu[i].string != NULL; i++) {
        lib_free(joyport3_dyn_menu[i].string);
    }
}

static void sdl_menu_joyport4_free(void)
{
    int i;

    for (i = 0; joyport4_dyn_menu[i].string != NULL; i++) {
        lib_free(joyport4_dyn_menu[i].string);
    }
}

static void sdl_menu_joyport5_free(void)
{
    int i;

    for (i = 0; joyport5_dyn_menu[i].string != NULL; i++) {
        lib_free(joyport5_dyn_menu[i].string);
    }
}

static UI_MENU_CALLBACK(JoyPort1Device_dynmenu_callback)
{
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_1);
    int i;

    /* rebuild menu if it already exists. */
    if (joyport1_dyn_menu_init != 0) {
        sdl_menu_joyport1_free();
    } else {
        joyport1_dyn_menu_init = 1;
    }

    for (i = 0; devices[i].name; ++i) {
        joyport1_dyn_menu[i].string = (char *)lib_stralloc(devices[i].name);
        joyport1_dyn_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        joyport1_dyn_menu[i].callback = radio_JoyPort1Device_callback;
        joyport1_dyn_menu[i].data = (ui_callback_data_t)int_to_void_ptr(devices[i].id);
    }

    joyport1_dyn_menu[i].string = NULL;
    joyport1_dyn_menu[i].type = 0;
    joyport1_dyn_menu[i].callback = NULL;
    joyport1_dyn_menu[i].data = NULL;

    lib_free(devices);

    return MENU_SUBMENU_STRING;
}

static UI_MENU_CALLBACK(JoyPort2Device_dynmenu_callback)
{
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_2);
    int i;

    /* rebuild menu if it already exists. */
    if (joyport2_dyn_menu_init != 0) {
        sdl_menu_joyport2_free();
    } else {
        joyport2_dyn_menu_init = 1;
    }

    for (i = 0; devices[i].name; ++i) {
        joyport2_dyn_menu[i].string = (char *)lib_stralloc(devices[i].name);
        joyport2_dyn_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        joyport2_dyn_menu[i].callback = radio_JoyPort2Device_callback;
        joyport2_dyn_menu[i].data = (ui_callback_data_t)int_to_void_ptr(devices[i].id);
    }

    joyport2_dyn_menu[i].string = NULL;
    joyport2_dyn_menu[i].type = 0;
    joyport2_dyn_menu[i].callback = NULL;
    joyport2_dyn_menu[i].data = NULL;

    lib_free(devices);

    return MENU_SUBMENU_STRING;
}

static UI_MENU_CALLBACK(JoyPort3Device_dynmenu_callback)
{
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_3);
    int i;

    /* rebuild menu if it already exists. */
    if (joyport3_dyn_menu_init != 0) {
        sdl_menu_joyport3_free();
    } else {
        joyport3_dyn_menu_init = 1;
    }

    for (i = 0; devices[i].name; ++i) {
        joyport3_dyn_menu[i].string = (char *)lib_stralloc(devices[i].name);
        joyport3_dyn_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        joyport3_dyn_menu[i].callback = radio_JoyPort3Device_callback;
        joyport3_dyn_menu[i].data = (ui_callback_data_t)int_to_void_ptr(devices[i].id);
    }

    joyport3_dyn_menu[i].string = NULL;
    joyport3_dyn_menu[i].type = 0;
    joyport3_dyn_menu[i].callback = NULL;
    joyport3_dyn_menu[i].data = NULL;

    lib_free(devices);

    return MENU_SUBMENU_STRING;
}

static UI_MENU_CALLBACK(JoyPort4Device_dynmenu_callback)
{
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_4);
    int i;

    /* rebuild menu if it already exists. */
    if (joyport4_dyn_menu_init != 0) {
        sdl_menu_joyport4_free();
    } else {
        joyport4_dyn_menu_init = 1;
    }

    for (i = 0; devices[i].name; ++i) {
        joyport4_dyn_menu[i].string = (char *)lib_stralloc(devices[i].name);
        joyport4_dyn_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        joyport4_dyn_menu[i].callback = radio_JoyPort4Device_callback;
        joyport4_dyn_menu[i].data = (ui_callback_data_t)int_to_void_ptr(devices[i].id);
    }

    joyport4_dyn_menu[i].string = NULL;
    joyport4_dyn_menu[i].type = 0;
    joyport4_dyn_menu[i].callback = NULL;
    joyport4_dyn_menu[i].data = NULL;

    lib_free(devices);

    return MENU_SUBMENU_STRING;
}


static UI_MENU_CALLBACK(JoyPort5Device_dynmenu_callback)
{
    joyport_desc_t *devices = joyport_get_valid_devices(JOYPORT_5);
    int i;

    /* rebuild menu if it already exists. */
    if (joyport5_dyn_menu_init != 0) {
        sdl_menu_joyport5_free();
    } else {
        joyport5_dyn_menu_init = 1;
    }

    for (i = 0; devices[i].name; ++i) {
        joyport5_dyn_menu[i].string = (char *)lib_stralloc(devices[i].name);
        joyport5_dyn_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        joyport5_dyn_menu[i].callback = radio_JoyPort5Device_callback;
        joyport5_dyn_menu[i].data = (ui_callback_data_t)int_to_void_ptr(devices[i].id);
    }

    joyport5_dyn_menu[i].string = NULL;
    joyport5_dyn_menu[i].type = 0;
    joyport5_dyn_menu[i].callback = NULL;
    joyport5_dyn_menu[i].data = NULL;

    lib_free(devices);

    return MENU_SUBMENU_STRING;
}

ui_menu_entry_t joyport_menu[JOYPORT_MAX_PORTS + 2];

UI_MENU_DEFINE_TOGGLE(BBRTCSave)

void uijoyport_menu_create(int port1, int port2, int port3, int port4, int port5)
{
    int j = 0;

    joyport_menu[j].string = "Save BBRTC data when changed";
    joyport_menu[j].type = MENU_ENTRY_RESOURCE_TOGGLE;
    joyport_menu[j].callback = toggle_BBRTCSave_callback;
    joyport_menu[j].data = NULL;
    ++j;

    if (port1) {
        joyport_menu[j].string = joyport_get_port_name(JOYPORT_1);
        joyport_menu[j].type = MENU_ENTRY_DYNAMIC_SUBMENU;
        joyport_menu[j].callback = JoyPort1Device_dynmenu_callback;
        joyport_menu[j].data = (ui_callback_data_t)joyport1_dyn_menu;
        ++j;
    }

    if (port2) {
        joyport_menu[j].string = joyport_get_port_name(JOYPORT_2);
        joyport_menu[j].type = MENU_ENTRY_DYNAMIC_SUBMENU;
        joyport_menu[j].callback = JoyPort2Device_dynmenu_callback;
        joyport_menu[j].data = (ui_callback_data_t)joyport2_dyn_menu;
        ++j;
    }

    if (port3) {
        joyport_menu[j].string = joyport_get_port_name(JOYPORT_3);
        joyport_menu[j].type = MENU_ENTRY_DYNAMIC_SUBMENU;
        joyport_menu[j].callback = JoyPort3Device_dynmenu_callback;
        joyport_menu[j].data = (ui_callback_data_t)joyport3_dyn_menu;
        ++j;
    }

    if (port4) {
        joyport_menu[j].string = joyport_get_port_name(JOYPORT_4);
        joyport_menu[j].type = MENU_ENTRY_DYNAMIC_SUBMENU;
        joyport_menu[j].callback = JoyPort4Device_dynmenu_callback;
        joyport_menu[j].data = (ui_callback_data_t)joyport4_dyn_menu;
        ++j;
    }

    if (port5) {
        joyport_menu[j].string = joyport_get_port_name(JOYPORT_5);
        joyport_menu[j].type = MENU_ENTRY_DYNAMIC_SUBMENU;
        joyport_menu[j].callback = JoyPort5Device_dynmenu_callback;
        joyport_menu[j].data = (ui_callback_data_t)joyport5_dyn_menu;
        ++j;
    }
    joyport_menu[j].string = NULL;
    joyport_menu[j].type = MENU_ENTRY_TEXT;
    joyport_menu[j].callback = NULL;
    joyport_menu[j].data = NULL;
}


/** \brief  Clean up memory used by the dynamically created joyport menus
 */
void uijoyport_menu_shutdown(void)
{
    if (joyport1_dyn_menu_init) {
        sdl_menu_joyport1_free();
    }
    if (joyport2_dyn_menu_init) {
        sdl_menu_joyport2_free();
    }
    if (joyport3_dyn_menu_init) {
        sdl_menu_joyport3_free();
    }
    if (joyport4_dyn_menu_init) {
        sdl_menu_joyport4_free();
    }
    if (joyport5_dyn_menu_init) {
        sdl_menu_joyport5_free();
    }
}

