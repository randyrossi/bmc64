/*
 * uisidcbm2.c - CBM2 SID UI interface for MS-DOS.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "lib.h"
#include "resources.h"
#include "sid.h"
#include "tui.h"
#include "tuimenu.h"
#include "uisidcbm2.h"

static TUI_MENU_CALLBACK(sid_engine_model_submenu_callback)
{
    char *s;
    int temp;
    int value;

    resources_get_int("SidModel", &temp);
    resources_get_int("SidEngine", &value);
    value <<= 8;
    value |= temp;
    switch (value) {
        case SID_FASTSID_6581:
            s = "6581 (Fast SID)";
            break;
        case SID_FASTSID_8580:
            s = "8580 (Fast SID)";
            break;
#ifdef HAVE_RESID
        case SID_RESID_6581:
            s = "6581 (ReSID)";
            break;
        case SID_RESID_8580:
            s = "8580 (ReSID)";
            break;
        case SID_RESID_8580D:
            s = "8580 + digi boost (ReSID)";
            break;
#endif
#ifdef HAVE_PARSID
        case SID_PARSID:
            s = "ParSID";
            break;
#endif
#ifdef HAVE_CATWEASELMKIII
        case SID_CATWEASELMKIII:
            s = "Catweasel";
            break;
#endif
#ifdef HAVE_HARDSID
        case SID_HARDSID:
            s = "HardSID";
            break;
#endif
#ifdef HAVE_SSI2001
        case SID_SSI2001:
            s = "SSI2001";
            break;
#endif
    }
    return s;
}

static TUI_MENU_CALLBACK(sid_radio_engine_model_callback)
{
    int engine;
    int model;

    if (been_activated) {
        engine = (int)param;
        engine >>= 8;
        model = (int)param;
        model &= 0xff;
        sid_set_engine_model(engine, model);
        *become_default = 1;
    } else {
        resource_value_t v;
        resources_get_int("SidEngine", &engine);
        resources_get_int("SidModel", &model);
        engine <<= 8;
        engine |= model;
        if (engine == (int)param) {
            *become_default = 1;
        }
    }
    return NULL;
}

static tui_menu_item_def_t *sid_engine_model_submenu = NULL;

#ifdef HAVE_RESID
static TUI_MENU_CALLBACK(toggle_ResidSampling_callback)
{
    int value;

    resources_get_int("SidResidSampling", &value);
    if (been_activated) {
        value = (value + 1) % 4;
        resources_set_int("SidResidSampling", value);
    }

    return (value == 0) ? "fast" : ((value == 1) ? "interpolate" : "resample");
}

static TUI_MENU_CALLBACK(ui_set_ResidPassBand_callback)
{
    if (been_activated) {
        int passband, value;
        char buf[10];

        resources_get_int("SidResidPassband", &passband);
        sprintf(buf, "%d", passband);

        if (tui_input_string("ReSID passband", "Enter ReSID passband to use:", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 90) {
                value = 90;
            } else if (value < 0) {
                value = 0;
            }
            resources_set_int("SidResidPassband", value);
            tui_message("ReSID passband set to : %d",value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_set_ResidGain_callback)
{
    if (been_activated) {
        int gain, value;
        char buf[10];

        resources_get_int("SidResidGain", &gain);
        sprintf(buf, "%d", gain);

        if (tui_input_string("ReSID gain", "Enter ReSID gain to use:", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 100) {
                value = 100;
            } else if (value < 90) {
                value = 90;
            }
            resources_set_int("SidResidGain", value);
            tui_message("ReSID gain set to : %d",value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_set_ResidBias_callback)
{
    if (been_activated) {
        int bias, value;
        char buf[10];

        resources_get_int("SidResidFilterBias", &bias);
        sprintf(buf, "%d", bias);

        if (tui_input_string("ReSID filter bias", "Enter ReSID bia to use:", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 5000) {
                value = 5000;
            } else if (value < -5000) {
                value = -5000;
            }
            resources_set_int("SidResidFilterBias", value);
            tui_message("ReSID filter bias set to : %d",value);
        } else {
            return NULL;
        }
    }
    return NULL;
}
#endif

TUI_MENU_DEFINE_TOGGLE(SidFilters)

tui_menu_item_def_t sid_cbm2_ui_menu_items[] = {
    { "--" },
    { "SID _Engine/Model:",
      "Select the SID engine and model to emulate",
      sid_engine_model_submenu_callback, NULL, 16,
      TUI_MENU_BEH_CONTINUE, NULL, "SID engine/model" },
    { "SID _Filters:",
      "Enable/disable emulation of the SID built-in programmable filters",
      toggle_SidFilters_callback, NULL, 4,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
#ifdef HAVE_RESID
    { "--"},
    { "ReSID s_ampling method:",
      "How the reSID engine generates the samples",
      toggle_ResidSampling_callback, NULL, 12,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "ReSID passband",
      "Set the ReSID passband to use",
      ui_set_ResidPassBand_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "ReSID gain",
      "Set the ReSID gain to use",
      ui_set_ResidGain_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "ReSID filter bias",
      "Set the ReSID filter bias to use",
      ui_set_ResidBias_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
#endif
    TUI_MENU_ITEM_DEF_LIST_END
};

void sid_cbm2_build_menu(void)
{
    sid_engine_model_t **list = sid_get_engine_model_list();
    int count;

    for (count = 0; list[count]; ++count) {}

    sid_engine_model_submenu = lib_malloc((count + 1) * sizeof(tui_menu_item_def_t));

    for (count = 0; list[count]; ++count) {
        sid_engine_model_submenu[count].label = list[count]->name;
        sid_engine_model_submenu[count].help_string = list[count]->name;
        sid_engine_model_submenu[count].callback = sid_radio_engine_model_callback;
        sid_engine_model_submenu[count].callback_param = (void *)list[count]->value;
        sid_engine_model_submenu[count].par_string_max_len = 0;
        sid_engine_model_submenu[count].behavior = TUI_MENU_BEH_CLOSE;
        sid_engine_model_submenu[count].submenu = NULL;
        sid_engine_model_submenu[count].submenu_title = NULL;
    }
    sid_engine_model_submenu[count].label = NULL;
    sid_engine_model_submenu[count].help_string = NULL;
    sid_engine_model_submenu[count].callback = NULL;
    sid_engine_model_submenu[count].callback_param = NULL;
    sid_engine_model_submenu[count].par_string_max_len = 0;
    sid_engine_model_submenu[count].behavior = 0;
    sid_engine_model_submenu[count].submenu = NULL;
    sid_engine_model_submenu[count].submenu_title = NULL;

    sid_cbm2_ui_menu_items[1].submenu = sid_engine_model_submenu;
}
