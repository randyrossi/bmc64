/*
 * uic64model.c
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

#ifdef AMIGA_OS4
#define ASL_PRE_V38_NAMES
#endif

#include "private.h"

#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

#include "c64-resources.h"
#include "uic64model.h"
#include "vicii.h"
#include "intl.h"
#include "translate.h"

static int ui_iec_reset_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_iec_reset_enable[countof(ui_iec_reset_enable_translate)];

static const int ui_iec_reset_enable_values[] = {
    0,
    1,
    -1
};

static int ui_cia_models_translate[] = {
    IDS_6526_OLD,
    IDS_6526A_NEW,
    0
};

static char *ui_cia_models[countof(ui_cia_models_translate)];

static const int ui_cia_models_values[] = {
    0,
    1,
    -1
};

static char *ui_kernal_rev[] = {
    "Rev 1",
    "Rev 2",
    "Rev 3",
    "SX-64",
    "4064",
    NULL
};

static int ui_kernal_rev_values[] = {
    C64_KERNAL_REV1,
    C64_KERNAL_REV2,
    C64_KERNAL_REV3,
    C64_KERNAL_SX64,
    C64_KERNAL_4064,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "CIA1Model", ui_cia_models, ui_cia_models_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "CIA2Model", ui_cia_models, ui_cia_models_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "IECReset", ui_iec_reset_enable, ui_iec_reset_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KernalRev", ui_kernal_rev, ui_kernal_rev_values, NULL },
    UI_END /* mandatory */
};

#define VICMODEL_UNKNOWN -1
#define VICMODEL_NUM 5

struct vicmodel_s {
    int video;
    int luma;
};

static struct vicmodel_s vicmodels[] = {
    { MACHINE_SYNC_PAL,     1 }, /* VICII_MODEL_PALG */
    { MACHINE_SYNC_PAL,     0 }, /* VICII_MODEL_PALG_OLD */
    { MACHINE_SYNC_NTSC,    1 }, /* VICII_MODEL_NTSCM */
    { MACHINE_SYNC_NTSCOLD, 0 }, /* VICII_MODEL_NTSCM_OLD */
    { MACHINE_SYNC_PALN,    1 }  /* VICII_MODEL_PALN */
};

static int vicmodel_get_temp(int video)
{
    int i;

    for (i = 0; i < VICMODEL_NUM; ++i) {
        if (vicmodels[i].video == video) {
            return i;
        }
    }

    return VICMODEL_UNKNOWN;
}

static int vicmodel_get(void)
{
    int video;

    if (resources_get_int("MachineVideoStandard", &video) < 0) {
        return -1;
    }

    return vicmodel_get_temp(video);
}

static void vicmodel_set(int model)
{
    int old_model;

    old_model = vicmodel_get();

    if ((model == old_model) || (model == VICMODEL_UNKNOWN)) {
        return;
    }

    resources_set_int("MachineVideoStandard", vicmodels[model].video);
}

enum {
    REQ_VICII_MODEL_PALN,
    REQ_VICII_MODEL_PALG,
    REQ_VICII_MODEL_OLDPALG,
    REQ_VICII_MODEL_NTSCM,
    REQ_VICII_MODEL_OLDNTSCM
};

static ULONG vicii_model(struct Hook *hook, Object *obj, APTR arg)
{
    int vicii = ui_requester(translate_text(IDMES_VICE_MESSAGE), translate_text(IDS_CHOOSE_VICII_MODEL), "PAL-G|Old PAL-G|NTSC-M|Old NTSC-M|PAL-N", 0);
    int viciiset = -1;

    switch (vicii) {
        case REQ_VICII_MODEL_PALN:
            viciiset = VICII_MODEL_PALN;
            break;
        case REQ_VICII_MODEL_PALG:
            viciiset = VICII_MODEL_PALG;
            break;
        case REQ_VICII_MODEL_OLDPALG:
            viciiset = VICII_MODEL_PALG_OLD;
            break;
        case REQ_VICII_MODEL_NTSCM:
            viciiset = VICII_MODEL_NTSCM;
            break;
        case REQ_VICII_MODEL_OLDNTSCM:
            viciiset = VICII_MODEL_NTSCM_OLD;
    }

    if (viciiset != -1) {
        vicmodel_set(viciiset);
    }

    return 0;
}

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel;
    APTR vicii_model_button;

#ifdef AMIGA_MORPHOS
    static const struct Hook VICIImodelHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)vicii_model, NULL };
#else
    static const struct Hook VICIImodelHook = { { NULL, NULL }, (VOID *)vicii_model, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(vicii_model_button, translate_text(IDS_VICII_MODEL_SELECTION))
           CYCLE(ui_to_from[0].object, translate_text(IDS_CIA1_MODEL), ui_cia_models)
           CYCLE(ui_to_from[1].object, translate_text(IDS_CIA2_MODEL), ui_cia_models)
           CYCLE(ui_to_from[2].object, translate_text(IDS_IEC_RESET), ui_iec_reset_enable)
           CYCLE(ui_to_from[3].object, translate_text(IDS_KERNAL_REVISION), ui_kernal_rev)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(vicii_model_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &VICIImodelHook);
    }

    return ui;
}

void ui_c64_model_custom_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_iec_reset_enable_translate, ui_iec_reset_enable);
    intl_convert_mui_table(ui_cia_models_translate, ui_cia_models);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_C64_MODEL_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
