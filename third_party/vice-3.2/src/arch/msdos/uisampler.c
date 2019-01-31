/*
 * uisampler.c - Sampler UI interface for MS-DOS.
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
#include "mouse.h"
#include "resources.h"
#include "sampler.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uisampler.h"
#include "util.h"

TUI_MENU_DEFINE_RADIO(SamplerDevice)
TUI_MENU_DEFINE_FILENAME(SampleName, "Sampler input media file")

static TUI_MENU_CALLBACK(ui_set_gain_callback)
{
    if (been_activated) {
        int current_gain, value;
        char buf[10];

        resources_get_int("SamplerGain", &current_gain);
        sprintf(buf, "%d", current_gain);

        if (tui_input_string("Sampler gain", "Enter the sampler gain in % (1-200):", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 200) {
                value = 200;
            } else if (value < 1) {
                value = 1;
            }
            resources_set_int("SamplerGain", value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(sampler_device_submenu_callback)
{
    int value;
    char *s;
    sampler_device_t *devices = sampler_get_devices();

    resources_get_int("SamplerDevice", &value);
    s = devices[value].name;
    return s;
}

static tui_menu_item_def_t sampler_device_submenu[SAMPLER_MAX_DEVICES + 1];

static tui_menu_item_def_t sampler_menu_items[] = {
    { "Sampler device:", "Select the sampler device",
      sampler_device_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, sampler_device_submenu,
      "Sampler device" },
    { "Sampler gain",
      "Set the sampler gain",
      ui_set_gain_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Sampler input media file:", "Select the sampler input media file",
      filename_SampleName_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uisampler_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_sampler_submenu;
    sampler_device_t *devices = sampler_get_devices();
    int i;

    ui_sampler_submenu = tui_menu_create("Sampler settings", 1);

    for (i = 0; devices[i].name; ++i) {
        sampler_device_submenu[i].label = devices[i].name;
        sampler_device_submenu[i].help_string = NULL;
        sampler_device_submenu[i].callback = radio_SamplerDevice_callback;
        sampler_device_submenu[i].callback_param = (void *)i;
        sampler_device_submenu[i].par_string_max_len = 20;
        sampler_device_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
        sampler_device_submenu[i].submenu = NULL;
        sampler_device_submenu[i].submenu_title = NULL;
    }
    sampler_device_submenu[i].label = NULL;
    sampler_device_submenu[i].help_string = NULL;
    sampler_device_submenu[i].callback = NULL;
    sampler_device_submenu[i].callback_param = NULL;
    sampler_device_submenu[i].par_string_max_len = 0;
    sampler_device_submenu[i].behavior = 0;
    sampler_device_submenu[i].submenu = NULL;
    sampler_device_submenu[i].submenu_title = NULL;

    tui_menu_add(ui_sampler_submenu, sampler_menu_items);

    tui_menu_add_submenu(parent_submenu, "Sampler settings...",
                         "Sampler settings",
                         ui_sampler_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
