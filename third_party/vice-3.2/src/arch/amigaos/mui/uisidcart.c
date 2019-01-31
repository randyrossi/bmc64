/*
 * uisidcart.c
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

#include "sid.h"
#include "uisidcart.h"
#include "intl.h"
#include "translate.h"

static int ui_sidcart_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_sidcart_enable[countof(ui_sidcart_enable_translate)];

static const int ui_sidcart_enable_values[] = {
    0,
    1,
    -1
};

static char **ui_sidcart_engine_model = NULL;
static int *ui_sidcart_engine_model_values = NULL;

static char *ui_sidcart_address[] =
{
    NULL, /* placeholder for primary address */
    NULL, /* placeholder for secondary address */
    NULL
};

static int ui_sidcart_address_values[] =
{
    0, /* placeholder for primary address */
    1, /* placeholder for secondary address */
    -1
};

static char *ui_sidcart_clock[] =
{
    "C64",
    NULL, /* placeholder for native clock */
    NULL
};

static const int ui_sidcart_clock_values[] =
{
    0,
    1,
    -1
};

#ifdef HAVE_RESID
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

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "SidCart", ui_sidcart_enable, ui_sidcart_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE_SID, "dummy", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidFilters", ui_sidcart_enable, ui_sidcart_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidAddress", ui_sidcart_address, ui_sidcart_address_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SidClock", ui_sidcart_clock, ui_sidcart_clock_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DIGIBLASTER", ui_sidcart_enable, ui_sidcart_enable_values, NULL },
#ifdef HAVE_RESID
    { NULL, MUI_TYPE_CYCLE, "SidResidSampling", ui_sid_samplemethod, ui_sid_samplemethod_values, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidPassband", NULL, ui_band_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidGain", NULL, ui_gain_range, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidFilterBias", NULL, ui_bias_range, NULL },
#endif
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    return GroupObject,
             CYCLE(ui_to_from[0].object, translate_text(IDS_SID_CART), ui_sidcart_enable)
             CYCLE(ui_to_from[1].object, translate_text(IDS_SID_ENGINE_MODEL), ui_sidcart_engine_model)
             CYCLE(ui_to_from[2].object, translate_text(IDS_SID_FILTERS), ui_sidcart_enable)
             CYCLE(ui_to_from[3].object, translate_text(IDS_SID_ADDRESS), ui_sidcart_address)
             CYCLE(ui_to_from[4].object, translate_text(IDS_SID_CLOCK), ui_sidcart_clock)
#ifdef HAVE_RESID
           CYCLE(ui_to_from[6].object, translate_text(IDS_SAMPLE_METHOD), ui_sid_samplemethod)
           NSTRING(ui_to_from[7].object, translate_text(IDS_PASSBAND_0_90), "0123456789", 5+1)
           NSTRING(ui_to_from[8].object, translate_text(IDS_GAIN_90_100), "0123456789", 5+1)
           NSTRING(ui_to_from[9].object, translate_text(IDS_BIAS_M5000_P5000), "0123456789", 5+1)
#endif
           End;
}

static APTR build_gui_plus4(void)
{
  return GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_SID_CART), ui_sidcart_enable)
           CYCLE(ui_to_from[1].object, translate_text(IDS_SID_ENGINE_MODEL), ui_sidcart_engine_model)
           CYCLE(ui_to_from[2].object, translate_text(IDS_SID_FILTERS), ui_sidcart_enable)
           CYCLE(ui_to_from[3].object, translate_text(IDS_SID_ADDRESS), ui_sidcart_address)
           CYCLE(ui_to_from[4].object, translate_text(IDS_SID_CLOCK), ui_sidcart_clock)
           CYCLE(ui_to_from[5].object, translate_text(IDS_SID_DIGIBLASTER), ui_sidcart_enable)
#ifdef HAVE_RESID
           CYCLE(ui_to_from[6].object, translate_text(IDS_SAMPLE_METHOD), ui_sid_samplemethod)
           NSTRING(ui_to_from[7].object, translate_text(IDS_PASSBAND_0_90), "0123456789", 5+1)
           NSTRING(ui_to_from[8].object, translate_text(IDS_GAIN_90_100), "0123456789", 5+1)
           NSTRING(ui_to_from[9].object, translate_text(IDS_BIAS_M5000_P5000), "0123456789", 5+1)
#endif
         End;
}

static void set_sid_engines(void)
{
    int count = 0;
    sid_engine_model_t **list = sid_get_engine_model_list();

    for (count = 0; list[count]; ++count) {}

    ui_sidcart_engine_model = lib_malloc((count + 1) * sizeof(char *));
    ui_sidcart_engine_model_values = lib_malloc((count + 1) * sizeof(int));

    for (count = 0; list[count]; ++count) {
        ui_sidcart_engine_model[count] = list[count]->name;
        ui_sidcart_engine_model_values[count] = list[count]->value;
    }
    ui_sidcart_engine_model[count] = NULL;
    ui_sidcart_engine_model_values[count] = -1;

    ui_to_from[1].strings = ui_sidcart_engine_model;
    ui_to_from[1].values = ui_sidcart_engine_model_values;
}

static void free_sid_engines(void)
{
    lib_free(ui_sidcart_engine_model);
    lib_free(ui_sidcart_engine_model_values);
    ui_sidcart_engine_model = NULL;
    ui_sidcart_engine_model_values = NULL;
}

void ui_sidcart_settings_dialog(char *addr_txt_1, char *addr_txt_2, char *clock, int addr_int_1, int addr_int_2)
{
    intl_convert_mui_table(ui_sidcart_enable_translate, ui_sidcart_enable);

    set_sid_engines();

    ui_sidcart_address[0] = addr_txt_1;
    ui_sidcart_address[1] = addr_txt_2;
    ui_sidcart_clock[1] = clock;
    ui_sidcart_address_values[0] = addr_int_1;
    ui_sidcart_address_values[1] = addr_int_2;
    mui_show_dialog(build_gui(), translate_text(IDS_SIDCART_SETTINGS), ui_to_from);

    free_sid_engines();
}

void ui_sidcart_plus4_settings_dialog(char *addr_txt_1, char *addr_txt_2, char *clock, int addr_int_1, int addr_int_2)
{
    intl_convert_mui_table(ui_sidcart_enable_translate, ui_sidcart_enable);

    set_sid_engines();

    ui_sidcart_address[0] = addr_txt_1;
    ui_sidcart_address[1] = addr_txt_2;
    ui_sidcart_clock[1] = clock;
    ui_sidcart_address_values[0] = addr_int_1;
    ui_sidcart_address_values[1] = addr_int_2;
    mui_show_dialog(build_gui_plus4(), translate_text(IDS_SIDCART_SETTINGS), ui_to_from);

    free_sid_engines();
}
