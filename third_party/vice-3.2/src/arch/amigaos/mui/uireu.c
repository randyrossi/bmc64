/*
 * uireu.c
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

#include "uireu.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *reu_canvas;

static int ui_reu_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_reu_enable[countof(ui_reu_enable_translate)];

static const int ui_reu_enable_values[] = {
    0,
    1,
    -1
};

static char *ui_reu_size[] = {
    "128K",
    "256K",
    "512K",
    "1024K",
    "2048K",
    "4096K",
    "8192K",
    "16384K",
    NULL
};

static const int ui_reu_size_values[] = {
    128,
    256,
    512,
    1024,
    2048,
    4096,
    8192,
    16384,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "REU", ui_reu_enable, ui_reu_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "REUsize", ui_reu_size, ui_reu_size_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "REUfilename", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "REUImageWrite", ui_reu_enable, ui_reu_enable_values, NULL },
    UI_END /* mandatory */
};

static ULONG Browse(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_REU_FILENAME_SELECT), "#?", reu_canvas);

    if (fname != NULL) {
        set(ui_to_from[2].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static APTR build_gui(void)
{
    APTR app, ui, ok, browse_button, cancel;

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse, NULL };
#else
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, "REU", ui_reu_enable)
           CYCLE(ui_to_from[1].object, translate_text(IDS_REU_SIZE), ui_reu_size)
           FILENAME(ui_to_from[2].object, translate_text(IDS_REU_FILENAME), browse_button)
           CYCLE(ui_to_from[3].object, translate_text(IDS_SAVE_REU_IMAGE_WHEN_CHANGED), ui_reu_enable)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);
    }

    return ui;
}

void ui_reu_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    reu_canvas = canvas;
    intl_convert_mui_table(ui_reu_enable_translate, ui_reu_enable);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_REU_SETTINGS));

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
