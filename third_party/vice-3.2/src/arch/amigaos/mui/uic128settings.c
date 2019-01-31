/*
 * uic128settings.c
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

#include "c128.h"
#include "uic128settings.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *c128_canvas;

static int ui_c128_machine_type_translate[] = {
    IDMS_LANGUAGE_INTERNATIONAL,
    IDMS_LANGUAGE_FINNISH,
    IDMS_LANGUAGE_FRENCH,
    IDMS_LANGUAGE_GERMAN,
    IDMS_LANGUAGE_ITALIAN,
    IDMS_LANGUAGE_NORWEGIAN,
    IDMS_LANGUAGE_SWEDISH,
    IDMS_LANGUAGE_SWISS,
    0
};

static char *ui_c128_machine_type[countof(ui_c128_machine_type_translate)];

static const int ui_c128_machine_type_values[] = {
    C128_MACHINE_INT,
    C128_MACHINE_FINNISH,
    C128_MACHINE_FRENCH,
    C128_MACHINE_GERMAN,
    C128_MACHINE_ITALIAN,
    C128_MACHINE_NORWEGIAN,
    C128_MACHINE_SWEDISH,
    C128_MACHINE_SWISS,
    -1
};

static int ui_c128_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_c128_enable[countof(ui_c128_enable_translate)];

static const int ui_c128_enable_values[] = {
    0,
    1,
    -1
};

static char *ui_function_rom[] = {
    NULL,	/* place holder for 'none' */
    "ROM",
    "RAM",
    "RTC",
    NULL
};

static const int ui_function_rom_values[] = {
    0,
    1,
    2,
    3,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "MachineType", ui_c128_machine_type, ui_c128_machine_type_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "InternalFunctionROM", ui_function_rom, ui_function_rom_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "InternalFunctionName", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "InternalFunctionROMRTCSave", ui_c128_enable, ui_c128_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "ExternalFunctionROM", ui_function_rom, ui_function_rom_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "ExternalFunctionName", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "ExternalFunctionROMRTCSave", ui_c128_enable, ui_c128_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "C128FullBanks", ui_c128_enable, ui_c128_enable_values, NULL },
    UI_END /* mandatory */
};

static ULONG BrowseInternal(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME), "#?", c128_canvas);

    if (fname != NULL) {
        set(ui_to_from[2].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static ULONG BrowseExternal(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME), "#?", c128_canvas);

    if (fname != NULL) {
        set(ui_to_from[5].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel, browse_button1, browse_button2;

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseInternalHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)BrowseInternal, NULL };
    static const struct Hook BrowseExternalHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)BrowseExternal, NULL };
#else
    static const struct Hook BrowseInternalHook = { { NULL, NULL }, (VOID *)BrowseInternal, NULL, NULL };
    static const struct Hook BrowseExternalHook = { { NULL, NULL }, (VOID *)BrowseExternal, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_MACHINE_TYPE), ui_c128_machine_type)
           CYCLE(ui_to_from[1].object, translate_text(IDS_INTERNAL_FUNCTION_ROM), ui_function_rom)
           FILENAME(ui_to_from[2].object, translate_text(IDS_INTERNAL_FUNCTION_ROM_FILENAME), browse_button1)
           CYCLE(ui_to_from[3].object, translate_text(IDS_INTERNAL_FUNCTION_RTC_SAVE), ui_c128_enable)
           CYCLE(ui_to_from[4].object, translate_text(IDS_EXTERNAL_FUNCTION_ROM), ui_function_rom)
           FILENAME(ui_to_from[5].object, translate_text(IDS_EXTERNAL_FUNCTION_ROM_FILENAME), browse_button2)
           CYCLE(ui_to_from[6].object, translate_text(IDS_EXTERNAL_FUNCTION_RTC_SAVE), ui_c128_enable)
           CYCLE(ui_to_from[7].object, translate_text(IDS_RAM_BANKS_2_AND_3), ui_c128_enable)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button1, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseInternalHook);

        DoMethod(browse_button2, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseExternalHook);
    }

    return ui;
}

void ui_c128_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    c128_canvas = canvas;

    intl_convert_mui_table(ui_c128_machine_type_translate, ui_c128_machine_type);
    intl_convert_mui_table(ui_c128_enable_translate, ui_c128_enable);
    ui_function_rom[0] = translate_text(IDS_NONE);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_C128_SETTINGS));

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
