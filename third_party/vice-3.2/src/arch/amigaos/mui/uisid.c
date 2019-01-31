/*
 * uisid.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include "sid.h"
#include "machine.h"
#include "ui.h"
#include "uisid.h"
#include "intl.h"
#include "translate.h"

static int ui_sid_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_sid_enable[countof(ui_sid_enable_translate)];

static const int ui_sid_enable_values[] = {
    0,
    1,
    -1
};

static int ui_sid_samplemethod_translate[] = {
    IDS_FAST,
    IDS_INTERPOLATING,
    IDS_RESAMPLING,
    IDS_FAST_RESAMPLING,
    0
};

static char *ui_sid_samplemethod[countof(ui_sid_samplemethod_translate)];

static const int ui_sid_samplemethod_values[] = {
    0,
    1,
    2,
    3,
    -1
};

static char **ui_sid_engine_model = NULL;

static int *ui_sid_engine_model_values = NULL;

static char *ui_sid_address64[] = {
    "$D420",
    "$D440",
    "$D460",
    "$D480",
    "$D4A0",
    "$D4C0",
    "$D4E0",
    "$D500",
    "$D520",
    "$D540",
    "$D560",
    "$D580",
    "$D5A0",
    "$D5C0",
    "$D5E0",
    "$D600",
    "$D620",
    "$D640",
    "$D660",
    "$D680",
    "$D6A0",
    "$D6C0",
    "$D6E0",
    "$D700",
    "$D720",
    "$D740",
    "$D760",
    "$D780",
    "$D7A0",
    "$D7C0",
    "$D7E0",
    "$DE00",
    "$DE20",
    "$DE40",
    "$DE60",
    "$DE80",
    "$DEA0",
    "$DEC0",
    "$DEE0",
    "$DF00",
    "$DF20",
    "$DF40",
    "$DF60",
    "$DF80",
    "$DFA0",
    "$DFC0",
    "$DFE0",
    NULL
};

static const int ui_sid_address64_values[] = {
    0xd420,
    0xd440,
    0xd460,
    0xd480,
    0xd4a0,
    0xd4c0,
    0xd4e0,
    0xd500,
    0xd520,
    0xd540,
    0xd560,
    0xd580,
    0xd5a0,
    0xd5c0,
    0xd5e0,
    0xd600,
    0xd620,
    0xd640,
    0xd660,
    0xd680,
    0xd6a0,
    0xd6c0,
    0xd6e0,
    0xd700,
    0xd720,
    0xd740,
    0xd760,
    0xd780,
    0xd7a0,
    0xd7c0,
    0xd7e0,
    0xde00,
    0xde20,
    0xde40,
    0xde60,
    0xde80,
    0xdea0,
    0xdec0,
    0xdee0,
    0xdf00,
    0xdf20,
    0xdf40,
    0xdf60,
    0xdf80,
    0xdfa0,
    0xdfc0,
    0xdfe0,
    -1
};

static char *ui_sid_address128[] = {
    "$D420",
    "$D440",
    "$D460",
    "$D480",
    "$D4A0",
    "$D4C0",
    "$D4E0",
    "$D700",
    "$D720",
    "$D740",
    "$D760",
    "$D780",
    "$D7A0",
    "$D7C0",
    "$D7E0",
    "$DE00",
    "$DE20",
    "$DE40",
    "$DE60",
    "$DE80",
    "$DEA0",
    "$DEC0",
    "$DEE0",
    "$DF00",
    "$DF20",
    "$DF40",
    "$DF60",
    "$DF80",
    "$DFA0",
    "$DFC0",
    "$DFE0",
    NULL
};

static const int ui_sid_address128_values[] = {
    0xd420,
    0xd440,
    0xd460,
    0xd480,
    0xd4a0,
    0xd4c0,
    0xd4e0,
    0xd700,
    0xd720,
    0xd740,
    0xd760,
    0xd780,
    0xd7a0,
    0xd7c0,
    0xd7e0,
    0xde00,
    0xde20,
    0xde40,
    0xde60,
    0xde80,
    0xdea0,
    0xdec0,
    0xdee0,
    0xdf00,
    0xdf20,
    0xdf40,
    0xdf60,
    0xdf80,
    0xdfa0,
    0xdfc0,
    0xdfe0,
    -1
};

static char *ui_extra_sid_amount[] = {
    "0",
    "1",
    "2",
    "3",
    NULL
};

static const int ui_extra_sid_amount_values[] = {
    0,
    1,
    2,
    3,
    -1
};

#ifdef HAVE_RESID
static const int ui_band_range[] = {
    0,
    90
};

static const int ui_gain_range[] = {
    90,
    100
};

static const int ui_bias_range[] = {
    -5000,
    5000
};
#endif

static ui_to_from_t ui_to_from2[] = {
    { NULL, MUI_TYPE_CYCLE_SID, "dummy", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidFilters", ui_sid_enable, ui_sid_enable_values, NULL },
#ifdef HAVE_RESID
    { NULL, MUI_TYPE_CYCLE, "SidResidSampling", ui_sid_samplemethod, ui_sid_samplemethod_values, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidPassband", NULL, ui_band_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidGain", NULL, ui_gain_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidFilterBias", NULL, ui_bias_range, NULL },
#endif
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from64[] = {
    { NULL, MUI_TYPE_CYCLE_SID, "dummy", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidStereo", ui_extra_sid_amount, ui_extra_sid_amount_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidStereoAddressStart", ui_sid_address64, ui_sid_address64_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidTripleAddressStart", ui_sid_address64, ui_sid_address64_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidQuadAddressStart", ui_sid_address64, ui_sid_address64_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidFilters", ui_sid_enable, ui_sid_enable_values, NULL },
#ifdef HAVE_RESID
    { NULL, MUI_TYPE_CYCLE, "SidResidSampling", ui_sid_samplemethod, ui_sid_samplemethod_values, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidPassband", NULL, ui_band_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidGain", NULL, ui_gain_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidFilterBias", NULL, ui_bias_range, NULL },
#endif
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from128[] = {
    { NULL, MUI_TYPE_CYCLE_SID, "dummy", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidStereo", ui_extra_sid_amount, ui_extra_sid_amount_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidStereoAddressStart", ui_sid_address128, ui_sid_address128_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidTripleAddressStart", ui_sid_address128, ui_sid_address128_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidQuadAddressStart", ui_sid_address128, ui_sid_address128_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidFilters", ui_sid_enable, ui_sid_enable_values, NULL },
#ifdef HAVE_RESID
    { NULL, MUI_TYPE_CYCLE, "SidResidSampling", ui_sid_samplemethod, ui_sid_samplemethod_values, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidPassband", NULL, ui_band_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidGain", NULL, ui_gain_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidFilterBias", NULL, ui_bias_range, NULL },
#endif
    UI_END /* mandatory */
};

static APTR build_gui2(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from2[0].object, translate_text(IDS_SID_ENGINE_MODEL), ui_sid_engine_model)
           CYCLE(ui_to_from2[1].object, translate_text(IDS_SID_FILTERS), ui_sid_enable)
#ifdef HAVE_RESID
           CYCLE(ui_to_from2[2].object, translate_text(IDS_SAMPLE_METHOD), ui_sid_samplemethod)
           NSTRING(ui_to_from2[3].object, translate_text(IDS_PASSBAND_0_90), "0123456789", 5+1)
           NSTRING(ui_to_from2[4].object, translate_text(IDS_GAIN_90_100), "0123456789", 5+1)
           NSTRING(ui_to_from2[5].object, translate_text(IDS_BIAS_M5000_P5000), "0123456789", 5+1)
#endif
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

static APTR build_gui64(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from64[0].object, translate_text(IDS_SID_ENGINE_MODEL), ui_sid_engine_model)
           CYCLE(ui_to_from64[1].object, translate_text(IDS_AMOUNT_OF_EXTRA_SIDS), ui_extra_sid_amount)
           CYCLE(ui_to_from64[2].object, translate_text(IDS_STEREO_SID_AT), ui_sid_address64)
           CYCLE(ui_to_from64[3].object, translate_text(IDS_TRIPLE_SID_AT), ui_sid_address64)
           CYCLE(ui_to_from64[4].object, translate_text(IDS_QUAD_SID_AT), ui_sid_address64)
           CYCLE(ui_to_from64[5].object, translate_text(IDS_SID_FILTERS), ui_sid_enable)
#ifdef HAVE_RESID
           CYCLE(ui_to_from64[6].object, translate_text(IDS_SAMPLE_METHOD), ui_sid_samplemethod)
           NSTRING(ui_to_from64[7].object, translate_text(IDS_PASSBAND_0_90), "0123456789", 5+1)
           NSTRING(ui_to_from64[8].object, translate_text(IDS_GAIN_90_100), "0123456789", 5+1)
           NSTRING(ui_to_from64[9].object, translate_text(IDS_BIAS_M5000_P5000), "0123456789", 5+1)
#endif
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

static APTR build_gui128(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from128[0].object, translate_text(IDS_SID_ENGINE_MODEL), ui_sid_engine_model)
           CYCLE(ui_to_from128[1].object, translate_text(IDS_AMOUNT_OF_EXTRA_SIDS), ui_extra_sid_amount)
           CYCLE(ui_to_from128[2].object, translate_text(IDS_STEREO_SID_AT), ui_sid_address128)
           CYCLE(ui_to_from128[3].object, translate_text(IDS_TRIPLE_SID_AT), ui_sid_address128)
           CYCLE(ui_to_from128[4].object, translate_text(IDS_QUAD_SID_AT), ui_sid_address128)
           CYCLE(ui_to_from128[5].object, translate_text(IDS_SID_FILTERS), ui_sid_enable)
#ifdef HAVE_RESID
           CYCLE(ui_to_from128[6].object, translate_text(IDS_SAMPLE_METHOD), ui_sid_samplemethod)
           NSTRING(ui_to_from128[7].object, translate_text(IDS_PASSBAND_0_90), "0123456789", 5+1)
           NSTRING(ui_to_from128[8].object, translate_text(IDS_GAIN_90_100), "0123456789", 5+1)
           NSTRING(ui_to_from128[9].object, translate_text(IDS_BIAS_M5000_P5000), "0123456789", 5+1)
#endif
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

static void set_sid_engines(void)
{
    int count = 0;
    sid_engine_model_t **list = sid_get_engine_model_list();

    for (count = 0; list[count]; ++count) {}

    ui_sid_engine_model = lib_malloc((count + 1) * sizeof(char *));
    ui_sid_engine_model_values = lib_malloc((count + 1) * sizeof(int));

    for (count = 0; list[count]; ++count) {
        ui_sid_engine_model[count] = list[count]->name;
        ui_sid_engine_model_values[count] = list[count]->value;
    }
    ui_sid_engine_model[count] = NULL;
    ui_sid_engine_model_values[count] = -1;
}

static void free_sid_engines(void)
{
    lib_free(ui_sid_engine_model);
    lib_free(ui_sid_engine_model_values);
    ui_sid_engine_model = NULL;
    ui_sid_engine_model_values = NULL;
}

void ui_sid_settings2_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_sid_samplemethod_translate, ui_sid_samplemethod);
    intl_convert_mui_table(ui_sid_enable_translate, ui_sid_enable);

    set_sid_engines();

    ui_to_from2[0].strings = ui_sid_engine_model;
    ui_to_from2[0].values = ui_sid_engine_model_values;

    window = mui_make_simple_window(build_gui2(), translate_text(IDS_SID_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from2);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from2);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
    free_sid_engines();
}

void ui_sid_settings64_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_sid_samplemethod_translate, ui_sid_samplemethod);
    intl_convert_mui_table(ui_sid_enable_translate, ui_sid_enable);

    set_sid_engines();

    ui_to_from64[0].strings = ui_sid_engine_model;
    ui_to_from64[0].values = ui_sid_engine_model_values;

    window = mui_make_simple_window(build_gui64(), translate_text(IDS_SID_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from64);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from64);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
    free_sid_engines();
}

void ui_sid_settings128_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_sid_samplemethod_translate, ui_sid_samplemethod);
    intl_convert_mui_table(ui_sid_enable_translate, ui_sid_enable);

    set_sid_engines();

    ui_to_from128[0].strings = ui_sid_engine_model;
    ui_to_from128[0].values = ui_sid_engine_model_values;

    window = mui_make_simple_window(build_gui128(), translate_text(IDS_SID_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from128);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from128);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
    free_sid_engines();
}
