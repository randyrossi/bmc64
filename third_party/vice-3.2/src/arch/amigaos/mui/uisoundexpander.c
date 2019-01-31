/*
 * uisoundexpander.c
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

#include "uisoundexpander.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *soundexpander_canvas;

static int ui_soundexpander_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_soundexpander_enable[countof(ui_soundexpander_enable_translate)];

static const int ui_soundexpander_enable_values[] = {
    0,
    1,
    -1
};

static char *ui_soundexpander_chip[] = {
    "3526",
    "3812",
    NULL
};

static const int ui_soundexpander_chip_values[] = {
    3526,
    3812,
    -1
};

static ui_to_from_t ui_to_from64[] = {
    { NULL, MUI_TYPE_CYCLE, "SFXSoundExpander", ui_soundexpander_enable, ui_soundexpander_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SFXSoundExpanderChip", ui_soundexpander_chip, ui_soundexpander_chip_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from20[] = {
    { NULL, MUI_TYPE_CYCLE, "SFXSoundExpander", ui_soundexpander_enable, ui_soundexpander_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SFXSoundExpanderIOSwap", ui_soundexpander_enable, ui_soundexpander_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SFXSoundExpanderChip", ui_soundexpander_chip, ui_soundexpander_chip_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui64(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from64[0].object, translate_text(IDS_SFX_SE_ENABLED), ui_soundexpander_enable)
           CYCLE(ui_to_from64[1].object, translate_text(IDS_SFX_SE_CHIP), ui_soundexpander_chip)
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

static APTR build_gui20(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from20[0].object, translate_text(IDS_SFX_SE_ENABLED), ui_soundexpander_enable)
           CYCLE(ui_to_from20[1].object, translate_text(IDS_MASCUERADE_IO_SWAP), ui_soundexpander_enable)
           CYCLE(ui_to_from20[2].object, translate_text(IDS_SFX_SE_CHIP), ui_soundexpander_chip)
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

void ui_soundexpander_c64_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    soundexpander_canvas = canvas;
    intl_convert_mui_table(ui_soundexpander_enable_translate, ui_soundexpander_enable);

    window = mui_make_simple_window(build_gui64(), translate_text(IDS_SFX_SE_SETTINGS));

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
}

void ui_soundexpander_vic20_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    soundexpander_canvas = canvas;
    intl_convert_mui_table(ui_soundexpander_enable_translate, ui_soundexpander_enable);

    window = mui_make_simple_window(build_gui20(), translate_text(IDS_SFX_SE_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from20);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from20);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
