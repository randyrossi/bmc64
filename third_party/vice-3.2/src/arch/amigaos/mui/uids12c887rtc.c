/*
 * uids12c887rtc.c
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

#include "uids12c887rtc.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *ds12c887rtc_canvas;

static int ui_ds12c887rtc_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_ds12c887rtc_enable[countof(ui_ds12c887rtc_enable_translate)];

static const int ui_ds12c887rtc_enable_values[] = {
    0,
    1,
    -1
};

static char *ui_ds12c887rtc_c64_base[] = {
    "$D500",
    "$D600",
    "$D700",
    "$DE00",
    "$DF00",
    NULL
};

static const int ui_ds12c887rtc_c64_base_values[] = {
    0xd500,
    0xd600,
    0xd700,
    0xde00,
    0xdf00,
    -1
};

static char *ui_ds12c887rtc_c128_base[] = {
    "$D700",
    "$DE00",
    "$DF00",
    NULL
};

static const int ui_ds12c887rtc_c128_base_values[] = {
    0xd700,
    0xde00,
    0xdf00,
    -1
};

static char *ui_ds12c887rtc_vic20_base[] = {
    "$9800",
    "$9C00",
    NULL
};

static const int ui_ds12c887rtc_vic20_base_values[] = {
    0x9800,
    0x9c00,
    -1
};

static int ui_ds12c887rtc_run_mode_translate[] = {
    IDS_HALTED,
    IDS_RUNNING,
    0
};

static char *ui_ds12c887rtc_run_mode[countof(ui_ds12c887rtc_run_mode_translate)];

static const int ui_ds12c887rtc_run_mode_values[] = {
    0,
    1,
    -1
};

static ui_to_from_t ui_to_from64[] = {
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTC", ui_ds12c887rtc_enable, ui_ds12c887rtc_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCbase", ui_ds12c887rtc_c64_base, ui_ds12c887rtc_c64_base_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCRunMode", ui_ds12c887rtc_run_mode, ui_ds12c887rtc_run_mode_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCSave", ui_ds12c887rtc_enable, ui_ds12c887rtc_enable_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from128[] = {
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTC", ui_ds12c887rtc_enable, ui_ds12c887rtc_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCbase", ui_ds12c887rtc_c128_base, ui_ds12c887rtc_c128_base_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCRunMode", ui_ds12c887rtc_run_mode, ui_ds12c887rtc_run_mode_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCSave", ui_ds12c887rtc_enable, ui_ds12c887rtc_enable_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from20[] = {
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTC", ui_ds12c887rtc_enable, ui_ds12c887rtc_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCbase", ui_ds12c887rtc_vic20_base, ui_ds12c887rtc_vic20_base_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCRunMode", ui_ds12c887rtc_run_mode, ui_ds12c887rtc_run_mode_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DS12C887RTCSave", ui_ds12c887rtc_enable, ui_ds12c887rtc_enable_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui64(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from64[0].object, translate_text(IDS_DS12C887RTC_ENABLED), ui_ds12c887rtc_enable)
           CYCLE(ui_to_from64[1].object, translate_text(IDS_DS12C887RTC_BASE), ui_ds12c887rtc_c64_base)
           CYCLE(ui_to_from64[2].object, translate_text(IDS_DS12C887RTC_RUN_MODE), ui_ds12c887rtc_run_mode)
           CYCLE(ui_to_from64[3].object, translate_text(IDS_DS12C887RTC_SAVE), ui_ds12c887rtc_enable)
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
           CYCLE(ui_to_from128[0].object, translate_text(IDS_DS12C887RTC_ENABLED), ui_ds12c887rtc_enable)
           CYCLE(ui_to_from128[1].object, translate_text(IDS_DS12C887RTC_BASE), ui_ds12c887rtc_c128_base)
           CYCLE(ui_to_from128[2].object, translate_text(IDS_DS12C887RTC_RUN_MODE), ui_ds12c887rtc_run_mode)
           CYCLE(ui_to_from128[3].object, translate_text(IDS_DS12C887RTC_SAVE), ui_ds12c887rtc_enable)
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
           CYCLE(ui_to_from20[0].object, translate_text(IDS_DS12C887RTC_ENABLED), ui_ds12c887rtc_enable)
           CYCLE(ui_to_from20[1].object, translate_text(IDS_DS12C887RTC_BASE), ui_ds12c887rtc_vic20_base)
           CYCLE(ui_to_from20[2].object, translate_text(IDS_DS12C887RTC_RUN_MODE), ui_ds12c887rtc_run_mode)
           CYCLE(ui_to_from20[3].object, translate_text(IDS_DS12C887RTC_SAVE), ui_ds12c887rtc_enable)
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

void ui_ds12c887rtc_c64_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    ds12c887rtc_canvas = canvas;
    intl_convert_mui_table(ui_ds12c887rtc_enable_translate, ui_ds12c887rtc_enable);
    intl_convert_mui_table(ui_ds12c887rtc_run_mode_translate, ui_ds12c887rtc_run_mode);

    window = mui_make_simple_window(build_gui64(), translate_text(IDS_DS12C887RTC_SETTINGS));

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

void ui_ds12c887rtc_c128_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    ds12c887rtc_canvas = canvas;
    intl_convert_mui_table(ui_ds12c887rtc_enable_translate, ui_ds12c887rtc_enable);
    intl_convert_mui_table(ui_ds12c887rtc_run_mode_translate, ui_ds12c887rtc_run_mode);

    window = mui_make_simple_window(build_gui128(), translate_text(IDS_DS12C887RTC_SETTINGS));

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
}

void ui_ds12c887rtc_vic20_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    ds12c887rtc_canvas = canvas;
    intl_convert_mui_table(ui_ds12c887rtc_enable_translate, ui_ds12c887rtc_enable);
    intl_convert_mui_table(ui_ds12c887rtc_run_mode_translate, ui_ds12c887rtc_run_mode);

    window = mui_make_simple_window(build_gui20(), translate_text(IDS_DS12C887RTC_SETTINGS));

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
