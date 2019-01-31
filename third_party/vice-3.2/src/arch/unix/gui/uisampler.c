/*
 * uisampler.c
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

#include "cartridge.h"
#include "lib.h"
#include "resources.h"
#include "sampler.h"
#include "uicartridge.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "uisampler.h"
#include "vsync.h"

UI_MENU_DEFINE_RADIO(SamplerDevice)


static UI_CALLBACK(set_sampler_gain)
{
    static char input_string[32];
    char *msg_string;
    ui_button_t button;
    int i;

    vsync_suspend_speed_eval();

    resources_get_int("SamplerGain", &i);

    sprintf(input_string, "%d", i);
    msg_string = lib_stralloc(_("Enter gain in percentage\n(1 - 200)"));
    button = ui_input_string(_("Gain percentage"), msg_string, input_string, 32);
    lib_free(msg_string);
    if (button == UI_BUTTON_OK) {
        i = atoi(input_string);
        if (i < 1) {
            i = 1;
        } else {
            if (i > 200) {
                i = 200;
            }
        }
        resources_set_int("SamplerGain", i);
        ui_update_menus();
    }
}


static UI_CALLBACK(set_sampler_image_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("Sampler input media file"), UILIB_FILTER_ALL);
}


ui_menu_entry_t sampler_settings_submenu[] = {
    { N_("Sampler device"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Sampler gain"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_sampler_gain, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Sampler input media file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_sampler_image_name, (ui_callback_data_t)"SampleName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_sampler_settings_menu[] = {
    { N_("Sampler settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sampler_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

void uisampler_menu_create(void)
{
    unsigned int i, num = 0;
    ui_menu_entry_t *devices_submenu;
    sampler_device_t *devices = sampler_get_devices();

    for (i = 0; devices[i].name; ++i) {
        ++num;
    }

    devices_submenu = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));

    for (i = 0; i < num ; i++) {
        devices_submenu[i].string = (ui_callback_data_t)lib_msprintf("%s", devices[i].name);
        devices_submenu[i].type = UI_MENU_TYPE_TICK;
        devices_submenu[i].callback = (ui_callback_t)radio_SamplerDevice;
        devices_submenu[i].callback_data = (ui_callback_data_t)lib_stralloc(devices[i].name);
    }

    sampler_settings_submenu[0].sub_menu = devices_submenu;
}

void uisampler_menu_shutdown(void)
{
    unsigned int i;
    ui_menu_entry_t *devices_submenu = NULL;

    devices_submenu = sampler_settings_submenu[0].sub_menu;

    if (devices_submenu == NULL) {
        return;
    }

    sampler_settings_submenu[0].sub_menu = NULL;

    i = 0;

    while (devices_submenu[i].string != NULL) {
        lib_free(devices_submenu[i].string);
        lib_free(devices_submenu[i].callback_data);
        i++;
    }

    lib_free(devices_submenu);
}
