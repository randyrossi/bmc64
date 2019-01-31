/*
 * menu_sampler.c - Sampler menu for SDL UI.
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

#include "menu_sampler.h"
#include "menu_common.h"
#include "sampler.h"
#include "uimenu.h"


/* DIGIMAX MENU */

UI_MENU_DEFINE_RADIO(SamplerDevice)
UI_MENU_DEFINE_INT(SamplerGain)
UI_MENU_DEFINE_FILE_STRING(SampleName)

static ui_menu_entry_t sampler_device_menu[SAMPLER_MAX_DEVICES + 1];

ui_menu_entry_t sampler_menu[] = {
    { "Sampler device",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sampler_device_menu },
    { "Sampler gain",
      MENU_ENTRY_RESOURCE_INT,
      int_SamplerGain_callback,
      (ui_callback_data_t)"Enter sampler gain (1-200)" },
    { "Sampler input media file",
      MENU_ENTRY_DIALOG,
      file_string_SampleName_callback,
      (ui_callback_data_t)"Select sampler input media file" },
    SDL_MENU_LIST_END
};

void uisampler_menu_create(void)
{
    sampler_device_t *devices = sampler_get_devices();
    int i;

    for (i = 0; devices[i].name; ++i) {
        sampler_device_menu[i].string = (char*)devices[i].name;
        sampler_device_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        sampler_device_menu[i].callback = radio_SamplerDevice_callback;
        sampler_device_menu[i].data = (ui_callback_data_t)int_to_void_ptr(i);
    }

    sampler_device_menu[i].string = NULL;
    sampler_device_menu[i].type = MENU_ENTRY_TEXT;
    sampler_device_menu[i].callback = NULL;
    sampler_device_menu[i].data = NULL;
}
