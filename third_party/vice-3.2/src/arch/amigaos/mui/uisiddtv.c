/*
 * uisid.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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
#include "uisiddtv.h"
#include "intl.h"
#include "translate.h"

static char *ui_sid_pages[] =
{
    "General",
#ifdef HAVE_RESID
    "ReSID-DTV",
#endif
#ifdef HAVE_CATWEASELMKIII
    "Catweasel MK3",
#endif
#ifdef HAVE_HARDSID
    "HardSID",
#endif
    NULL
};

static char **ui_siddtv_engine_model = NULL;

static int *ui_siddtv_engine_model_values = NULL;

static int ui_sid_samplemethod_translate[] =
{
    IDS_FAST,
    IDS_INTERPOLATING,
    IDS_RESAMPLING,
    IDS_FAST_RESAMPLING,
    0
};

static char *ui_sid_samplemethod[countof(ui_sid_samplemethod_translate)];

static const int ui_sid_samplemethod_values[] =
{
    0,
    1,
    2,
    3,
    -1
};

static int ui_band_range[] = {
    0,
    90
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE_SID, "dummy", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CHECK, "SidFilters", NULL, NULL, NULL },
#ifdef HAVE_RESID
    { NULL, MUI_TYPE_CYCLE, "SidResidSampling", ui_sid_samplemethod, ui_sid_samplemethod_values, NULL },
    { NULL, MUI_TYPE_INTEGER, "SidResidPassband", NULL, ui_band_range, NULL },
#endif
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    return RegisterObject,
             MUIA_Register_Titles, ui_sid_pages,
             Child, GroupObject,
               CYCLE(ui_to_from[0].object, translate_text(IDS_SID_ENGINE_MODEL), ui_siddtv_engine_model)
               CHECK(ui_to_from[1].object, translate_text(IDS_SID_FILTERS))
             End,
#if defined(HAVE_RESID)
             Child, GroupObject,
               CYCLE(ui_to_from[2].object, translate_text(IDS_SAMPLE_METHOD), ui_sid_samplemethod)
               Child, ui_to_from[3].object = StringObject,
                 MUIA_Frame, MUIV_Frame_String,
                 MUIA_FrameTitle, translate_text(IDS_PASSBAND_0_90),
                 MUIA_String_Accept, "0123456789",
                 MUIA_String_MaxLen, 3,
               End,
             End,
#endif
#ifdef HAVE_CATWEASELMKIII
             Child, GroupObject,
               Child, CLabel(translate_text(IDS_NOT_IMPLEMENTED_YET)),
             End,
#endif
#ifdef HAVE_HARDSID
             Child, GroupObject,
               Child, CLabel(translate_text(IDS_NOT_IMPLEMENTED_YET)),
             End,
#endif
           End;
}

static void set_sid_engines(void)
{
    int count = 0;
    sid_engine_model_t **list = sid_get_engine_model_list();

    for (count = 0; list[count]; ++count) {}

    ui_siddtv_engine_model = lib_malloc((count + 1) * sizeof(char *));
    ui_siddtv_engine_model_values = lib_malloc((count + 1) * sizeof(int));

    for (count = 0; list[count]; ++count) {
        ui_siddtv_engine_model[count] = list[count]->name;
        ui_siddtv_engine_model_values[count] = list[count]->value;
    }
    ui_siddtv_engine_model[count] = NULL;
    ui_siddtv_engine_model_values[count] = -1;

    ui_to_from[0].strings = ui_siddtv_engine_model;
    ui_to_from[0].values = ui_siddtv_engine_model_values;
}

static void free_sid_engines(void)
{
    lib_free(ui_siddtv_engine_model);
    lib_free(ui_siddtv_engine_model_values);
    ui_siddtv_engine_model = NULL;
    ui_siddtv_engine_model_values = NULL;
}

void ui_siddtv_settings_dialog(void)
{
    intl_convert_mui_table(ui_sid_samplemethod_translate, ui_sid_samplemethod);

    set_sid_engines();

    mui_show_dialog(build_gui(), translate_text(IDS_SID_SETTINGS), ui_to_from);

    free_sid_engines();
}
