/*
 * uijoyport.c
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
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

#include "joyport.h"
#include "uijoyport.h"
#include "intl.h"
#include "machine.h"
#include "translate.h"

static int ports[JOYPORT_MAX_PORTS];

static char *ui_joyport_1[JOYPORT_MAX_DEVICES + 1];
static int ui_joyport_1_values[JOYPORT_MAX_DEVICES + 1];

static char *ui_joyport_2[JOYPORT_MAX_DEVICES + 1];
static int ui_joyport_2_values[JOYPORT_MAX_DEVICES + 1];

static char *ui_joyport_3[JOYPORT_MAX_DEVICES + 1];
static int ui_joyport_3_values[JOYPORT_MAX_DEVICES + 1];

static char *ui_joyport_4[JOYPORT_MAX_DEVICES + 1];
static int ui_joyport_4_values[JOYPORT_MAX_DEVICES + 1];

static char *ui_joyport_5[JOYPORT_MAX_DEVICES + 1];
static int ui_joyport_5_values[JOYPORT_MAX_DEVICES + 1];

static int ui_joyport_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_joyport_enable[countof(ui_joyport_enable_translate)];

static const int ui_joyport_enable_values[] = {
    0,
    1,
    -1
};

static ui_to_from_t ui_to_from[JOYPORT_MAX_PORTS + 2];

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel;
    char *joyport1_device = NULL;
    char *joyport2_device = NULL;
    char *joyport3_device = NULL;
    char *joyport4_device = NULL;
    char *joyport5_device = NULL;
    int total = (ports[JOYPORT_1] << 4) | (ports[JOYPORT_2] << 3) | (ports[JOYPORT_3] << 2) | (ports[JOYPORT_4] << 1) | ports[JOYPORT_5];

    if (ports[JOYPORT_1]) {
        joyport1_device = lib_msprintf(translate_text(IDS_JOYPORT_S_DEVICE), translate_text(joyport_get_port_trans_name(JOYPORT_1)));
    }

    if (ports[JOYPORT_2]) {
        joyport2_device = lib_msprintf(translate_text(IDS_JOYPORT_S_DEVICE), translate_text(joyport_get_port_trans_name(JOYPORT_2)));
    }

    if (ports[JOYPORT_3]) {
        joyport3_device = lib_msprintf(translate_text(IDS_JOYPORT_S_DEVICE), translate_text(joyport_get_port_trans_name(JOYPORT_3)));
    }

    if (ports[JOYPORT_4]) {
        joyport4_device = lib_msprintf(translate_text(IDS_JOYPORT_S_DEVICE), translate_text(joyport_get_port_trans_name(JOYPORT_4)));
    }

    if (ports[JOYPORT_5]) {
        joyport5_device = lib_msprintf(translate_text(IDS_JOYPORT_S_DEVICE), translate_text(joyport_get_port_trans_name(JOYPORT_5)));
    }

    app = mui_get_app();

    switch (total) {
        case 31:
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, joyport1_device, ui_joyport_1)
                   CYCLE(ui_to_from[1].object, joyport2_device, ui_joyport_2)
                   CYCLE(ui_to_from[2].object, joyport3_device, ui_joyport_3)
                   CYCLE(ui_to_from[3].object, joyport4_device, ui_joyport_4)
                   CYCLE(ui_to_from[4].object, joyport5_device, ui_joyport_5)
                   CYCLE(ui_to_from[5].object, translate_text(IDS_SAVE_BBRTC_DATA_WHEN_CHANGED), ui_joyport_enable)
                   OK_CANCEL_BUTTON
                 End;
             break;
        case 30:
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, joyport1_device, ui_joyport_1)
                   CYCLE(ui_to_from[1].object, joyport2_device, ui_joyport_2)
                   CYCLE(ui_to_from[2].object, joyport3_device, ui_joyport_3)
                   CYCLE(ui_to_from[3].object, joyport4_device, ui_joyport_4)
                   CYCLE(ui_to_from[4].object, translate_text(IDS_SAVE_BBRTC_DATA_WHEN_CHANGED), ui_joyport_enable)
                   OK_CANCEL_BUTTON
                 End;
             break;
        case 22:
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, joyport1_device, ui_joyport_1)
                   CYCLE(ui_to_from[1].object, joyport3_device, ui_joyport_3)
                   CYCLE(ui_to_from[2].object, joyport4_device, ui_joyport_4)
                   CYCLE(ui_to_from[3].object, translate_text(IDS_SAVE_BBRTC_DATA_WHEN_CHANGED), ui_joyport_enable)
                   OK_CANCEL_BUTTON
                 End;
             break;
        case 28:
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, joyport1_device, ui_joyport_1)
                   CYCLE(ui_to_from[1].object, joyport2_device, ui_joyport_2)
                   CYCLE(ui_to_from[2].object, joyport3_device, ui_joyport_3)
                   CYCLE(ui_to_from[3].object, translate_text(IDS_SAVE_BBRTC_DATA_WHEN_CHANGED), ui_joyport_enable)
                   OK_CANCEL_BUTTON
                 End;
             break;
        case 24:
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, joyport1_device, ui_joyport_1)
                   CYCLE(ui_to_from[1].object, joyport2_device, ui_joyport_2)
                   CYCLE(ui_to_from[2].object, translate_text(IDS_SAVE_BBRTC_DATA_WHEN_CHANGED), ui_joyport_enable)
                   OK_CANCEL_BUTTON
                 End;
             break;
        case 6:
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, joyport3_device, ui_joyport_3)
                   CYCLE(ui_to_from[1].object, joyport4_device, ui_joyport_4)
                   CYCLE(ui_to_from[2].object, translate_text(IDS_SAVE_BBRTC_DATA_WHEN_CHANGED), ui_joyport_enable)
                   OK_CANCEL_BUTTON
                 End;
             break;

    }

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

void ui_joyport_settings_dialog(int port1, int port2, int port3, int port4, int port5)
{
    APTR window;
    int i;
    int j = 0;
    joyport_desc_t *devices_port_1;
    joyport_desc_t *devices_port_2;
    joyport_desc_t *devices_port_3;
    joyport_desc_t *devices_port_4;
    joyport_desc_t *devices_port_5;

    ports[JOYPORT_1] = port1;
    ports[JOYPORT_2] = port2;
    ports[JOYPORT_3] = port3;
    ports[JOYPORT_4] = port4;
    ports[JOYPORT_5] = port5;

    if (ports[JOYPORT_1]) {
        devices_port_1 = joyport_get_valid_devices(JOYPORT_1);
        for (i = 0; devices_port_1[i].name; ++i) {
            ui_joyport_1[i] = translate_text(devices_port_1[i].trans_name);
            ui_joyport_1_values[i] = devices_port_1[i].id;
        }
        ui_joyport_1[i] = NULL;
        ui_joyport_1_values[i] = -1;
        lib_free(devices_port_1);
        ui_to_from[j].object = NULL;
        ui_to_from[j].type = MUI_TYPE_CYCLE;
        ui_to_from[j].resource = "JoyPort1Device";
        ui_to_from[j].strings = ui_joyport_1;
        ui_to_from[j].values = ui_joyport_1_values;
        ui_to_from[j].string_choices = NULL;
        ++j;
    }

    if (ports[JOYPORT_2]) {
        devices_port_2 = joyport_get_valid_devices(JOYPORT_2);
        for (i = 0; devices_port_2[i].name; ++i) {
            ui_joyport_2[i] = translate_text(devices_port_2[i].trans_name);
            ui_joyport_2_values[i] = devices_port_2[i].id;
        }
        ui_joyport_2[i] = NULL;
        ui_joyport_2_values[i] = -1;
        lib_free(devices_port_2);
        ui_to_from[j].object = NULL;
        ui_to_from[j].type = MUI_TYPE_CYCLE;
        ui_to_from[j].resource = "JoyPort2Device";
        ui_to_from[j].strings = ui_joyport_2;
        ui_to_from[j].values = ui_joyport_2_values;
        ui_to_from[j].string_choices = NULL;
        ++j;
    }

    if (ports[JOYPORT_3]) {
        devices_port_3 = joyport_get_valid_devices(JOYPORT_3);
        for (i = 0; devices_port_3[i].name; ++i) {
            ui_joyport_3[i] = translate_text(devices_port_3[i].trans_name);
            ui_joyport_3_values[i] = devices_port_3[i].id;
        }
        ui_joyport_3[i] = NULL;
        ui_joyport_3_values[i] = -1;
        lib_free(devices_port_3);
        ui_to_from[j].object = NULL;
        ui_to_from[j].type = MUI_TYPE_CYCLE;
        ui_to_from[j].resource = "JoyPort3Device";
        ui_to_from[j].strings = ui_joyport_3;
        ui_to_from[j].values = ui_joyport_3_values;
        ui_to_from[j].string_choices = NULL;
        ++j;
    }

    if (ports[JOYPORT_4]) {
        devices_port_4 = joyport_get_valid_devices(JOYPORT_4);
        for (i = 0; devices_port_4[i].name; ++i) {
            ui_joyport_4[i] = translate_text(devices_port_4[i].trans_name);
            ui_joyport_4_values[i] = devices_port_4[i].id;
        }
        ui_joyport_4[i] = NULL;
        ui_joyport_4_values[i] = -1;
        lib_free(devices_port_4);
        ui_to_from[j].object = NULL;
        ui_to_from[j].type = MUI_TYPE_CYCLE;
        ui_to_from[j].resource = "JoyPort4Device";
        ui_to_from[j].strings = ui_joyport_4;
        ui_to_from[j].values = ui_joyport_4_values;
        ui_to_from[j].string_choices = NULL;
        ++j;
    }

    if (ports[JOYPORT_5]) {
        devices_port_5 = joyport_get_valid_devices(JOYPORT_5);
        for (i = 0; devices_port_5[i].name; ++i) {
            ui_joyport_5[i] = translate_text(devices_port_5[i].trans_name);
            ui_joyport_5_values[i] = devices_port_5[i].id;
        }
        ui_joyport_5[i] = NULL;
        ui_joyport_5_values[i] = -1;
        lib_free(devices_port_5);
        ui_to_from[j].object = NULL;
        ui_to_from[j].type = MUI_TYPE_CYCLE;
        ui_to_from[j].resource = "JoyPort5Device";
        ui_to_from[j].strings = ui_joyport_5;
        ui_to_from[j].values = ui_joyport_5_values;
        ui_to_from[j].string_choices = NULL;
        ++j;
    }

    ui_to_from[j].object = NULL;
    ui_to_from[j].type = MUI_TYPE_CYCLE;
    ui_to_from[j].resource = "BBRTCSave";
    ui_to_from[j].strings = ui_joyport_enable;
    ui_to_from[j].values = ui_joyport_enable_values;
    ui_to_from[j].string_choices = NULL;
    ++j;
    
    ui_to_from[j].object = NULL;
    ui_to_from[j].type = MUI_TYPE_NONE;
    ui_to_from[j].resource = NULL;
    ui_to_from[j].strings = NULL;
    ui_to_from[j].values = NULL;
    ui_to_from[j].string_choices = NULL;

    window = mui_make_simple_window(build_gui(), translate_text(IDS_JOYPORT_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
