/*
 * menu_sid.c - Implementation of the SID settings menu for the SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include <stdlib.h>

#include "lib.h"
#include "menu_common.h"
#include "menu_sid.h"
#include "resources.h"
#include "sid.h"
#include "uimenu.h"

static UI_MENU_CALLBACK(custom_SidModel_callback)
{
    int engine, model, selected;

    selected = vice_ptr_to_int(param);

    if (activated) {
        engine = selected >> 8;
        model = selected & 0xff;
        sid_set_engine_model(engine, model);
    } else {
        resources_get_int("SidEngine", &engine);
        resources_get_int("SidModel", &model);

        if (selected == ((engine << 8) | model)) {
            return sdl_menu_text_tick;
        }
    }

    return NULL;
}

static ui_menu_entry_t *sid_model_menu = NULL;

#ifdef HAVE_RESID
UI_MENU_DEFINE_RADIO(SidResidSampling)

static const ui_menu_entry_t sid_sampling_menu[] = {
    { "Fast",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidResidSampling_callback,
      (ui_callback_data_t)0 },
    { "Interpolating",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidResidSampling_callback,
      (ui_callback_data_t)1 },
    { "Resampling",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidResidSampling_callback,
      (ui_callback_data_t)2 },
    { "Fast Resampling",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidResidSampling_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_SLIDER(SidResidPassband, 0, 90)
UI_MENU_DEFINE_SLIDER(SidResidGain, 90, 100)
UI_MENU_DEFINE_SLIDER(SidResidFilterBias, -5000, 5000)
UI_MENU_DEFINE_SLIDER(SidResid8580Passband, 0, 90)
UI_MENU_DEFINE_SLIDER(SidResid8580Gain, 90, 100)
UI_MENU_DEFINE_SLIDER(SidResid8580FilterBias, -5000, 5000)

#define VICE_SDL_RESID_OPTIONS                                                                                     \
    { "reSID sampling method",                                                                                     \
      MENU_ENTRY_SUBMENU,                                                                                          \
      submenu_radio_callback,                                                                                      \
      (ui_callback_data_t)sid_sampling_menu },                                                                     \
    { "reSID resampling passband",                                                                                 \
      MENU_ENTRY_RESOURCE_INT,                                                                                     \
      slider_SidResidPassband_callback,                                                                            \
      (ui_callback_data_t)"Enter passband in percentage of total bandwidth (lower is faster, higher is better)" }, \
    { "reSID filter gain",                                                                                         \
      MENU_ENTRY_RESOURCE_INT,                                                                                     \
      slider_SidResidGain_callback,                                                                                \
      (ui_callback_data_t)"Set filter gain in percent" },                                                          \
    { "reSID filter bias",                                                                                         \
      MENU_ENTRY_RESOURCE_INT,                                                                                     \
      slider_SidResidFilterBias_callback,                                                                          \
      (ui_callback_data_t)"Set filter bias in mV" },                                                               \
    { "reSID 8580 resampling passband",                                                                                 \
      MENU_ENTRY_RESOURCE_INT,                                                                                     \
      slider_SidResid8580Passband_callback,                                                                        \
      (ui_callback_data_t)"Enter passband in percentage of total bandwidth (lower is faster, higher is better)" }, \
    { "reSID 8580 filter gain",                                                                                         \
      MENU_ENTRY_RESOURCE_INT,                                                                                     \
      slider_SidResid8580Gain_callback,                                                                            \
      (ui_callback_data_t)"Set filter gain in percent" },                                                          \
    { "reSID 8580 filter bias",                                                                                         \
      MENU_ENTRY_RESOURCE_INT,                                                                                     \
      slider_SidResid8580FilterBias_callback,                                                                      \
      (ui_callback_data_t)"Set filter bias in mV" },

#endif /* HAVE_RESID */

UI_MENU_DEFINE_TOGGLE(SidFilters)
UI_MENU_DEFINE_RADIO(SidStereo)
UI_MENU_DEFINE_RADIO(SidStereoAddressStart)
UI_MENU_DEFINE_RADIO(SidTripleAddressStart)
UI_MENU_DEFINE_RADIO(SidQuadAddressStart)

static UI_MENU_CALLBACK(show_SidStereoAddressStart_callback)
{
    static char buf[20];
    int value;

    resources_get_int("SidStereoAddressStart", &value);

    sprintf(buf, "$%04x", value);
    return buf;
}

#define SID_D4XX_MENU(menu, txt, showcb, cb) \
static const ui_menu_entry_t menu[] = {      \
    { txt, MENU_ENTRY_TEXT, showcb, NULL },  \
    { "$D420", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd420 },      \
    { "$D440", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd440 },      \
    { "$D460", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd460 },      \
    { "$D480", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd480 },      \
    { "$D4A0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd4a0 },      \
    { "$D4C0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd4c0 },      \
    { "$D4E0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd4e0 },      \
    SDL_MENU_LIST_END                        \
};

#define SID_D5XX_MENU(menu, txt, showcb, cb) \
static const ui_menu_entry_t menu[] = {      \
    { txt, MENU_ENTRY_TEXT, showcb, NULL },  \
    { "$D500", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd500 },      \
    { "$D520", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd520 },      \
    { "$D540", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd540 },      \
    { "$D560", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd560 },      \
    { "$D580", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd580 },      \
    { "$D5A0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd5a0 },      \
    { "$D5C0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd5c0 },      \
    { "$D5E0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd5e0 },      \
    SDL_MENU_LIST_END                        \
};

#define SID_D6XX_MENU(menu, txt, showcb, cb) \
static const ui_menu_entry_t menu[] = {      \
    { txt, MENU_ENTRY_TEXT, showcb, NULL },  \
    { "$D600", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd600 },      \
    { "$D620", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd620 },      \
    { "$D640", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd640 },      \
    { "$D660", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd660 },      \
    { "$D680", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd680 },      \
    { "$D6A0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd6a0 },      \
    { "$D6C0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd6c0 },      \
    { "$D6E0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd6e0 },      \
    SDL_MENU_LIST_END                        \
};

#define SID_D7XX_MENU(menu, txt, showcb, cb) \
static const ui_menu_entry_t menu[] = {      \
    { txt, MENU_ENTRY_TEXT, showcb, NULL },  \
    { "$D700", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd700 },      \
    { "$D720", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd720 },      \
    { "$D740", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd740 },      \
    { "$D760", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd760 },      \
    { "$D780", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd780 },      \
    { "$D7A0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd7a0 },      \
    { "$D7C0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd7c0 },      \
    { "$D7E0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xd7e0 },      \
    SDL_MENU_LIST_END                        \
};

#define SID_DEXX_MENU(menu, txt, showcb, cb) \
static const ui_menu_entry_t menu[] = {      \
    { txt, MENU_ENTRY_TEXT, showcb, NULL },  \
    { "$DE00", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xde00 },      \
    { "$DE20", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xde20 },      \
    { "$DE40", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xde40 },      \
    { "$DE60", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xde60 },      \
    { "$DE80", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xde80 },      \
    { "$DEA0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdea0 },      \
    { "$DEC0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdec0 },      \
    { "$DEE0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdee0 },      \
    SDL_MENU_LIST_END                        \
};

#define SID_DFXX_MENU(menu, txt, showcb, cb) \
static const ui_menu_entry_t menu[] = {      \
    { txt, MENU_ENTRY_TEXT, showcb, NULL },  \
    { "$DF00", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdf00 },      \
    { "$DF20", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdf20 },      \
    { "$DF40", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdf40 },      \
    { "$DF60", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdf60 },      \
    { "$DF80", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdf80 },      \
    { "$DFA0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdfa0 },      \
    { "$DFC0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdfc0 },      \
    { "$DFE0", MENU_ENTRY_RESOURCE_RADIO,    \
      cb, (ui_callback_data_t)0xdfe0 },      \
    SDL_MENU_LIST_END                        \
};

SID_D4XX_MENU(sid_d4x0_menu, "Second SID base address", show_SidStereoAddressStart_callback, radio_SidStereoAddressStart_callback)
SID_D5XX_MENU(sid_d5x0_menu, "Second SID base address", show_SidStereoAddressStart_callback, radio_SidStereoAddressStart_callback)
SID_D6XX_MENU(sid_d6x0_menu, "Second SID base address", show_SidStereoAddressStart_callback, radio_SidStereoAddressStart_callback)
SID_D7XX_MENU(sid_d7x0_menu, "Second SID base address", show_SidStereoAddressStart_callback, radio_SidStereoAddressStart_callback)
SID_DEXX_MENU(sid_dex0_menu, "Second SID base address", show_SidStereoAddressStart_callback, radio_SidStereoAddressStart_callback)
SID_DFXX_MENU(sid_dfx0_menu, "Second SID base address", show_SidStereoAddressStart_callback, radio_SidStereoAddressStart_callback)

static const ui_menu_entry_t c128_stereo_sid_base_menu[] = {
    { "Second SID base address",
      MENU_ENTRY_TEXT,
      show_SidStereoAddressStart_callback,
      NULL},
    { "$D4x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_d4x0_menu },
    { "$D7x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_d7x0_menu },
    { "$DEx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_dex0_menu },
    { "$DFx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_dfx0_menu },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t c64_stereo_sid_base_menu[] = {
    { "Second SID base address",
      MENU_ENTRY_TEXT,
      show_SidStereoAddressStart_callback,
      NULL },
    { "$D4x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_d4x0_menu },
    { "$D5x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_d5x0_menu },
    { "$D6x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_d6x0_menu },
    { "$D7x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_d7x0_menu },
    { "$DEx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_dex0_menu },
    { "$DFx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_dfx0_menu },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(show_SidTripleAddressStart_callback)
{
    static char buf[20];
    int value;

    resources_get_int("SidTripleAddressStart", &value);

    sprintf(buf, "$%04x", value);
    return buf;
}

SID_D4XX_MENU(sid3_d4x0_menu, "Third SID base address", show_SidTripleAddressStart_callback, radio_SidTripleAddressStart_callback)
SID_D5XX_MENU(sid3_d5x0_menu, "Third SID base address", show_SidTripleAddressStart_callback, radio_SidTripleAddressStart_callback)
SID_D6XX_MENU(sid3_d6x0_menu, "Third SID base address", show_SidTripleAddressStart_callback, radio_SidTripleAddressStart_callback)
SID_D7XX_MENU(sid3_d7x0_menu, "Third SID base address", show_SidTripleAddressStart_callback, radio_SidTripleAddressStart_callback)
SID_DEXX_MENU(sid3_dex0_menu, "Third SID base address", show_SidTripleAddressStart_callback, radio_SidTripleAddressStart_callback)
SID_DFXX_MENU(sid3_dfx0_menu, "Third SID base address", show_SidTripleAddressStart_callback, radio_SidTripleAddressStart_callback)

static const ui_menu_entry_t c128_triple_sid_base_menu[] = {
    { "Third SID base address",
      MENU_ENTRY_TEXT,
      show_SidTripleAddressStart_callback,
      NULL},
    { "$D4x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_d4x0_menu },
    { "$D7x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_d7x0_menu },
    { "$DEx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_dex0_menu },
    { "$DFx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_dfx0_menu },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t c64_triple_sid_base_menu[] = {
    { "Third SID base address",
      MENU_ENTRY_TEXT,
      show_SidTripleAddressStart_callback,
      NULL },
    { "$D4x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_d4x0_menu },
    { "$D5x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_d5x0_menu },
    { "$D6x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_d6x0_menu },
    { "$D7x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_d7x0_menu },
    { "$DEx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_dex0_menu },
    { "$DFx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid3_dfx0_menu },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(show_SidQuadAddressStart_callback)
{
    static char buf[20];
    int value;

    resources_get_int("SidQuadAddressStart", &value);

    sprintf(buf, "$%04x", value);
    return buf;
}

SID_D4XX_MENU(sid4_d4x0_menu, "Fourth SID base address", show_SidQuadAddressStart_callback, radio_SidQuadAddressStart_callback)
SID_D5XX_MENU(sid4_d5x0_menu, "Fourth SID base address", show_SidQuadAddressStart_callback, radio_SidQuadAddressStart_callback)
SID_D6XX_MENU(sid4_d6x0_menu, "Fourth SID base address", show_SidQuadAddressStart_callback, radio_SidQuadAddressStart_callback)
SID_D7XX_MENU(sid4_d7x0_menu, "Fourth SID base address", show_SidQuadAddressStart_callback, radio_SidQuadAddressStart_callback)
SID_DEXX_MENU(sid4_dex0_menu, "Fourth SID base address", show_SidQuadAddressStart_callback, radio_SidQuadAddressStart_callback)
SID_DFXX_MENU(sid4_dfx0_menu, "Fourth SID base address", show_SidQuadAddressStart_callback, radio_SidQuadAddressStart_callback)

static const ui_menu_entry_t c128_quad_sid_base_menu[] = {
    { "Fourth SID base address",
      MENU_ENTRY_TEXT,
      show_SidQuadAddressStart_callback,
      NULL},
    { "$D4x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_d4x0_menu },
    { "$D7x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_d7x0_menu },
    { "$DEx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_dex0_menu },
    { "$DFx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_dfx0_menu },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t c64_quad_sid_base_menu[] = {
    { "Fourth SID base address",
      MENU_ENTRY_TEXT,
      show_SidQuadAddressStart_callback,
      NULL },
    { "$D4x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_d4x0_menu },
    { "$D5x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_d5x0_menu },
    { "$D6x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_d6x0_menu },
    { "$D7x0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_d7x0_menu },
    { "$DEx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_dex0_menu },
    { "$DFx0",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid4_dfx0_menu },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(show_SidStereo_callback)
{
    int value;

    resources_get_int("SidStereo", &value);
    switch (value) {
        case 1:
            return "One";
        case 2:
            return "Two";
        case 3:
            return "Three";
    }
    return "None";
}

static const ui_menu_entry_t c64_stereo_sid_menu[] = {
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidStereo_callback,
      (ui_callback_data_t)0 },
    { "One",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidStereo_callback,
      (ui_callback_data_t)1 },
    { "Two",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidStereo_callback,
      (ui_callback_data_t)2 },
    { "Three",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidStereo_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_LIST_END
};

ui_menu_entry_t sid_c64_menu[] = {
    { "SID Model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      NULL },
    { "Extra SIDs",
      MENU_ENTRY_SUBMENU,
      show_SidStereo_callback,
      (ui_callback_data_t)c64_stereo_sid_menu },
    { "Second SID base address",
      MENU_ENTRY_SUBMENU,
      show_SidStereoAddressStart_callback,
      (ui_callback_data_t)c64_stereo_sid_base_menu },
    { "Third SID base address",
      MENU_ENTRY_SUBMENU,
      show_SidTripleAddressStart_callback,
      (ui_callback_data_t)c64_triple_sid_base_menu },
    { "Fourth SID base address",
      MENU_ENTRY_SUBMENU,
      show_SidQuadAddressStart_callback,
      (ui_callback_data_t)c64_quad_sid_base_menu },
    { "Emulate filters",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidFilters_callback,
      NULL },
#ifdef HAVE_RESID
    VICE_SDL_RESID_OPTIONS
#endif
    SDL_MENU_LIST_END
};

ui_menu_entry_t sid_c128_menu[] = {
    { "SID Model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      NULL },
    { "Extra SIDs",
      MENU_ENTRY_SUBMENU,
      show_SidStereo_callback,
      (ui_callback_data_t)c64_stereo_sid_menu },
    { "Second SID base address",
      MENU_ENTRY_SUBMENU,
      show_SidStereoAddressStart_callback,
      (ui_callback_data_t)c128_stereo_sid_base_menu },
    { "Third SID base address",
      MENU_ENTRY_SUBMENU,
      show_SidTripleAddressStart_callback,
      (ui_callback_data_t)c128_triple_sid_base_menu },
    { "Fourth SID base address",
      MENU_ENTRY_SUBMENU,
      show_SidQuadAddressStart_callback,
      (ui_callback_data_t)c128_quad_sid_base_menu },
    { "Emulate filters",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidFilters_callback,
      NULL },
#ifdef HAVE_RESID
    VICE_SDL_RESID_OPTIONS
#endif
    SDL_MENU_LIST_END
};

ui_menu_entry_t sid_cbm2_menu[] = {
    { "SID Model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      NULL },
    { "Emulate filters",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidFilters_callback,
      NULL },
#ifdef HAVE_RESID
    VICE_SDL_RESID_OPTIONS
#endif
    SDL_MENU_LIST_END
};

ui_menu_entry_t sid_dtv_menu[] = {
    { "SID Model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      NULL },
    { "Emulate filters",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidFilters_callback,
      NULL },
#ifdef HAVE_RESID
    VICE_SDL_RESID_OPTIONS
#endif
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(SidCart)
UI_MENU_DEFINE_RADIO(SidAddress)
UI_MENU_DEFINE_RADIO(SidClock)

ui_menu_entry_t sid_vic_menu[] = {
    { "Enable SID cart emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidCart_callback,
      NULL },
    { "SID Model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      NULL },
    { "Emulate filters",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidFilters_callback,
      NULL },
#ifdef HAVE_RESID
    VICE_SDL_RESID_OPTIONS
#endif
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("SID address"),
    { "$9800",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidAddress_callback,
      (ui_callback_data_t)0x9800 },
    { "$9C00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidAddress_callback,
      (ui_callback_data_t)0x9c00 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("SID clock"),
    { "C64",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidClock_callback,
      (ui_callback_data_t)0 },
    { "VIC20",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidClock_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_LIST_END
};

ui_menu_entry_t sid_pet_menu[] = {
    { "Enable SID cart emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidCart_callback,
      NULL },
    { "SID Model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      NULL },
    { "Emulate filters",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidFilters_callback,
      NULL },
#ifdef HAVE_RESID
    VICE_SDL_RESID_OPTIONS
#endif
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("SID address"),
    { "$8F00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidAddress_callback,
      (ui_callback_data_t)0x8f00 },
    { "$E900",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidAddress_callback,
      (ui_callback_data_t)0xe900 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("SID clock"),
    { "C64",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidClock_callback,
      (ui_callback_data_t)0 },
    { "PET",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidClock_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(DIGIBLASTER)

ui_menu_entry_t sid_plus4_menu[] = {
    { "Enable SID cart emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidCart_callback,
      NULL },
    { "SID Model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      NULL },
    { "Emulate filters",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SidFilters_callback,
      NULL },
#ifdef HAVE_RESID
    VICE_SDL_RESID_OPTIONS
#endif
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("SID address"),
    { "$FD40",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidAddress_callback,
      (ui_callback_data_t)0xfd40 },
    { "$FE80",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidAddress_callback,
      (ui_callback_data_t)0xfe80 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("SID clock"),
    { "C64",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidClock_callback,
      (ui_callback_data_t)0 },
    { "PLUS4",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidClock_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Enable SID cart digiblaster add-on",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DIGIBLASTER_callback,
      NULL },
    SDL_MENU_LIST_END
};


void uisid_menu_create(void)
{
    sid_engine_model_t **list = sid_get_engine_model_list();
    int i;

    for (i = 0; list[i]; ++i) {}

    sid_model_menu = lib_malloc((i + 1) * sizeof(ui_menu_entry_t));

    for (i = 0; list[i]; ++i) {
        sid_model_menu[i].string = (char*)list[i]->name;
        sid_model_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        sid_model_menu[i].callback = custom_SidModel_callback;
        sid_model_menu[i].data = (ui_callback_data_t)int_to_void_ptr(list[i]->value);
    }

    sid_model_menu[i].string = NULL;
    sid_model_menu[i].type = MENU_ENTRY_TEXT;
    sid_model_menu[i].callback = NULL;
    sid_model_menu[i].data = NULL;

    sid_c64_menu[0].data = (ui_callback_data_t)sid_model_menu;
    sid_c128_menu[0].data = (ui_callback_data_t)sid_model_menu;
    sid_cbm2_menu[0].data = (ui_callback_data_t)sid_model_menu;
    sid_dtv_menu[0].data = (ui_callback_data_t)sid_model_menu;
    sid_vic_menu[1].data = (ui_callback_data_t)sid_model_menu;
    sid_pet_menu[1].data = (ui_callback_data_t)sid_model_menu;
    sid_plus4_menu[1].data = (ui_callback_data_t)sid_model_menu;
}

/** \brief  Clean up memory used by the SID model menu
 */
void uisid_menu_shutdown(void)
{
    if (sid_model_menu != NULL) {
        lib_free(sid_model_menu);
    }
}

