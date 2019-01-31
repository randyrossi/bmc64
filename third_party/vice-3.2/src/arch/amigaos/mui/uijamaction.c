/*
 * uijamaction.c
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

#include "uijamaction.h"
#include "intl.h"
#include "machine.h"
#include "translate.h"

static int ui_jamaction_translate[] = {
    IDS_ASK,
    IDS_CONTINUE,
    IDS_START_MONITOR,
    IDMS_RESET,
    IDS_HARD_RESET,
    IDS_QUIT_EMULATOR,
    0
};

static char *ui_jamaction[countof(ui_jamaction_translate)];

static const int ui_jamaction_values[] = {
    MACHINE_JAM_ACTION_DIALOG,
    MACHINE_JAM_ACTION_CONTINUE,
    MACHINE_JAM_ACTION_MONITOR,
    MACHINE_JAM_ACTION_RESET,
    MACHINE_JAM_ACTION_HARD_RESET,
    MACHINE_JAM_ACTION_QUIT,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "JAMAction", ui_jamaction, ui_jamaction_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_DEFAULT_JAM_ACTION), ui_jamaction)
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

void ui_jamaction_settings_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_jamaction_translate, ui_jamaction);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_JAMACTION_SETTINGS));

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
