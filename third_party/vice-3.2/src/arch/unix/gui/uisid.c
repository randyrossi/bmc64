/*
 * uisid.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "lib.h"
#include "resources.h"
#include "sid.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "vsync.h"

#include "uisid.h"


static UI_CALLBACK(radio_SidModel)
{
    int engine, model, selected;
    selected = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (!CHECK_MENUS) {
        engine = selected >> 8;
        model = selected & 0xff;
        sid_set_engine_model(engine, model);
        ui_update_menus();
    } else {
        resources_get_int("SidEngine", &engine);
        resources_get_int("SidModel", &model);

        if (selected == ((engine << 8) | model)) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
    }
}

#ifdef HAVE_RESID
UI_MENU_DEFINE_RADIO(SidResidSampling)

ui_menu_entry_t sid_resid_sampling_submenu[] = {
    { N_("Fast"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidResidSampling, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Interpolating"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidResidSampling, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Resampling"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidResidSampling, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Fast resampling"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidResidSampling, (ui_callback_data_t)3, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

#if !defined(USE_GNOMEUI)
UI_CALLBACK(set_sid_resid_passband)
{
    static char input_string[32];
    char *msg_string;
    ui_button_t button;
    int i;

    vsync_suspend_speed_eval();

    resources_get_int("SidResidPassband", &i);

    sprintf(input_string, "%d", i);
    msg_string = lib_stralloc(_("Enter passband in percentage of total bandwidth\n(0 - 90, lower is faster, higher is better)"));
    button = ui_input_string(_("Passband percentage"), msg_string, input_string, 32);
    lib_free(msg_string);
    if (button == UI_BUTTON_OK) {
        i = atoi(input_string);
        if (i < 0) {
            i = 0;
        } else {
            if (i > 90) {
                i = 90;
            }
        }
        resources_set_int("SidResidPassband", i);
        ui_update_menus();
    }
}
#endif

#endif

UI_MENU_DEFINE_RADIO(SidStereo)

ui_menu_entry_t sid_extra_sids_submenu[] = {
    { N_("None"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidStereo, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("One"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidStereo, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Two"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidStereo, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Three"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidStereo, (ui_callback_data_t)3, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t sid_model_submenu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t *attach_sid_model_submenu = NULL;


void uisid_model_menu_create(void)
{
    int i;
    sid_engine_model_t **list = sid_get_engine_model_list();

    for (i = 0; list[i]; ++i) {}
    attach_sid_model_submenu = lib_calloc((size_t)(i + 1), sizeof(ui_menu_entry_t));

    for (i = 0; list[i]; ++i) {
        attach_sid_model_submenu[i].string = (ui_callback_data_t)lib_msprintf("%s", list[i]->name);
        attach_sid_model_submenu[i].type = UI_MENU_TYPE_TICK;
        attach_sid_model_submenu[i].callback = (ui_callback_t)radio_SidModel;
        attach_sid_model_submenu[i].callback_data = (ui_callback_data_t)(unsigned long)list[i]->value;
    }
    sid_model_submenu[0].sub_menu = attach_sid_model_submenu;
}


void uisid_model_menu_shutdown(void)
{
    int i = 0;

    if (attach_sid_model_submenu) {
        while (attach_sid_model_submenu[i].string != NULL) {
            lib_free(attach_sid_model_submenu[i].string);
            i++;
        }

        lib_free(attach_sid_model_submenu);
        attach_sid_model_submenu = NULL;
    }
}
