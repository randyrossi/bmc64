/*
 * uiscpu64.c
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

#include "uiscpu64.h"
#include "intl.h"
#include "translate.h"

static int ui_scpu64_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_scpu64_enable[countof(ui_scpu64_enable_translate)];

static const int ui_scpu64_enable_values[] = {
    0,
    1,
    -1
};

static char *ui_scpu64_simm_size[] = {
    "0 MB",
    "1 MB",
    "4 MB",
    "8 MB",
    "16 MB",
    NULL
};

static const int ui_scpu64_simm_size_values[] = {
    0,
    1,
    4,
    8,
    16,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "SIMMSize", ui_scpu64_simm_size, ui_scpu64_simm_size_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "JiffySwitch", ui_scpu64_enable, ui_scpu64_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SpeedSwitch", ui_scpu64_enable, ui_scpu64_enable_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_SCPU64_SIMM_SIZE), ui_scpu64_simm_size)
           CYCLE(ui_to_from[1].object, translate_text(IDS_SCPU64_JIFFY_ENABLED), ui_scpu64_enable)
           CYCLE(ui_to_from[2].object, translate_text(IDS_SCPU64_SPEED_ENABLED), ui_scpu64_enable)
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

void ui_scpu64_settings_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_scpu64_enable_translate, ui_scpu64_enable);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_SCPU64_SETTINGS));

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
