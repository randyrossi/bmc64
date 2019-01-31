/*
 * uieasyflash.c
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

#include "cartridge.h"
#include "intl.h"
#include "translate.h"

static int ui_easyflash_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_easyflash_enable[countof(ui_easyflash_enable_translate)];

static const int ui_easyflash_enable_values[] = {
    0,
    1,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "EasyFlashJumper", ui_easyflash_enable, ui_easyflash_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "EasyFlashWriteCRT", ui_easyflash_enable, ui_easyflash_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "EasyFlashOptimizeCRT", ui_easyflash_enable, ui_easyflash_enable_values, NULL },
    UI_END /* mandatory */
};

static ULONG SaveEasyFlashCRT(struct Hook *hook, Object *obj, APTR arg)
{
    if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
        ui_error(translate_text(IDS_ERROR_SAVING_EASYFLASH_CRT));
    }

    return 0;
}

static APTR build_gui(void)
{
    APTR app, ui, ok, action_button, cancel;

#ifdef AMIGA_MORPHOS
    static const struct Hook SaveEasyFlashCRTHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)SaveEasyFlashCRT, NULL };
#else
    static const struct Hook SaveEasyFlashCRTHook = { { NULL, NULL }, (VOID *)SaveEasyFlashCRT, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_EASYFLASH_JUMPER), ui_easyflash_enable)
           CYCLE(ui_to_from[1].object, translate_text(IDS_SAVE_EASYFLASH_CRT_ON_DETACH), ui_easyflash_enable)
           CYCLE(ui_to_from[2].object, translate_text(IDS_OPTIMIZE_EASYFLASH_CRT_SAVE), ui_easyflash_enable)
           BUTTON(action_button, translate_text(IDS_SAVE_EASYFLASH_CRT_NOW))
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(action_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &SaveEasyFlashCRTHook);
    }

    return ui;
}

void ui_easyflash_settings_dialog(void)
{
    APTR window;

    intl_convert_mui_table(ui_easyflash_enable_translate, ui_easyflash_enable);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_EASYFLASH_SETTINGS));

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
