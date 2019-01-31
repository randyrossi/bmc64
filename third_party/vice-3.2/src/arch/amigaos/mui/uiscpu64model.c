/*
 * uiscpu64model.c
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

#include "uiscpu64model.h"
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

static int ui_vicii_models_translate[] = {
    IDS_6569_PAL,
    IDS_8565_PAL,
    IDS_6569R1_OLD_PAL,
    IDS_6567_NTSC,
    IDS_8562_NTSC,
    IDS_6567R56A_OLD_NTSC,
    IDS_6572_PAL_N,
    0
};

static char *ui_vicii_models[countof(ui_vicii_models_translate)];

static const int ui_vicii_models_values[] = {
    VICII_MODEL_6569,
    VICII_MODEL_8565,
    VICII_MODEL_6569R1,
    VICII_MODEL_6567,
    VICII_MODEL_8562,
    VICII_MODEL_6567R56A,
    VICII_MODEL_6572,
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

static int ui_glue_logic_translate[] = {
    IDS_DISCRETE,
    IDS_CUSTOM_IC,
    0
};

static char *ui_glue_logic[countof(ui_glue_logic_translate)];

static const int ui_glue_logic_values[] = {
    0,
    1,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "VICIIModel", ui_vicii_models, ui_vicii_models_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "CIA1Model", ui_cia_models, ui_cia_models_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "CIA2Model", ui_cia_models, ui_cia_models_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "GlueLogic", ui_glue_logic, ui_glue_logic_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "IECReset", ui_iec_reset_enable, ui_iec_reset_enable_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_VICII_MODEL), ui_vicii_models)
           CYCLE(ui_to_from[1].object, translate_text(IDS_CIA1_MODEL), ui_cia_models)
           CYCLE(ui_to_from[2].object, translate_text(IDS_CIA2_MODEL), ui_cia_models)
           CYCLE(ui_to_from[3].object, translate_text(IDS_GLUE_LOGIC), ui_glue_logic)
           CYCLE(ui_to_from[4].object, translate_text(IDS_IEC_RESET), ui_iec_reset_enable)
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

void ui_scpu64_model_custom_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_vicii_models_translate, ui_vicii_models);
    intl_convert_mui_table(ui_iec_reset_enable_translate, ui_iec_reset_enable);
    intl_convert_mui_table(ui_cia_models_translate, ui_cia_models);
    intl_convert_mui_table(ui_glue_logic_translate, ui_glue_logic);

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
