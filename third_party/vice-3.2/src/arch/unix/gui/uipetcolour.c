/*
 * uipetcolour.c
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

#include "lib.h"
#include "pet.h"
#include "resources.h"
#include "uilib.h"
#include "uimenu.h"
#include "uipetcolour.h"
#include "vsync.h"

UI_MENU_DEFINE_RADIO(PETColour)

static ui_menu_entry_t petcolour_type_submenu[] = {
    { "Off", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PETColour, (ui_callback_data_t)PET_COLOUR_TYPE_OFF, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "RGBI", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PETColour, (ui_callback_data_t)PET_COLOUR_TYPE_RGBI, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Analog", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PETColour, (ui_callback_data_t)PET_COLOUR_TYPE_ANALOG, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};


static UI_CALLBACK(set_petcolour_bg)
{
    static char input_string[32];
    char *msg_string;
    ui_button_t button;
    int i;

    vsync_suspend_speed_eval();

    resources_get_int("PETColourBG", &i);

    sprintf(input_string, "%d", i);
    msg_string = lib_stralloc(_("Enter PET Colour background\n(range: 0-255)"));
    button = ui_input_string(_("PET Colour background"), msg_string, input_string, 32);
    lib_free(msg_string);
    if (button == UI_BUTTON_OK) {
        i = atoi(input_string);
        if (i < 0) {
            i = 0;
        } else {
            if (i > 255) {
                i = 255;
            }
        }
        resources_set_int("PETColourBG", i);
        ui_update_menus();
    }
}

ui_menu_entry_t petcolour_submenu[] = {
    { N_("PET Colour type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petcolour_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("PET Colour background"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_petcolour_bg, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
