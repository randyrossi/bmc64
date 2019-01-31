/*
 * uic64memoryhacks.c
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

#include "c64-memory-hacks.h"
#include "uic64memoryhacks.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *memory_hacks_canvas;

static char *ui_c64_256k_base[] = {
    "$DE00",
    "$DE80",
    "$DF00",
    "$DF80",
    NULL
};

static const int ui_c64_256k_base_values[] = {
    0xde00,
    0xde80,
    0xdf00,
    0xdf80,
    -1
};

static char *ui_plus60k_base[] = {
    "$D040",
    "$D100",
    NULL
};

static const int ui_plus60k_base_values[] = {
    0xd040,
    0xd100,
    -1
};

static char *ui_memory_hack_device[] = {
    NULL,	/* place holder for 'None' */
    "C64 256K",
    "+60K",
    "+256K",
    NULL
};

static const int ui_memory_hack_device_values[] = {
    MEMORY_HACK_NONE,
    MEMORY_HACK_C64_256K,
    MEMORY_HACK_PLUS60K,
    MEMORY_HACK_PLUS256K,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "MemoryHack", ui_memory_hack_device, ui_memory_hack_device_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "C64_256Kbase", ui_c64_256k_base, ui_c64_256k_base_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "C64_256Kfilename", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "PLUS60Kbase", ui_plus60k_base, ui_plus60k_base_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "PLUS60Kfilename", NULL, NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "PLUS256Kfilename", NULL, NULL, NULL },
    UI_END /* mandatory */
};

static ULONG Browse_256K(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_256K_FILENAME_SELECT), "#?", memory_hacks_canvas);

    if (fname != NULL) {
        set(ui_to_from[2].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static ULONG Browse_PLUS60K(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_PLUS60K_FILENAME_SELECT), "#?", memory_hacks_canvas);

    if (fname != NULL) {
        set(ui_to_from[4].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static ULONG Browse_PLUS256K(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_PLUS256K_FILENAME_SELECT), "#?", memory_hacks_canvas);

    if (fname != NULL) {
        set(ui_to_from[5].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static APTR build_gui(void)
{
    APTR app, ui, ok, browse_button_256K, browse_button_PLUS60K, browse_button_PLUS256K, cancel;

#ifdef AMIGA_MORPHOS
    static const struct Hook Browse256KFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_256K, NULL };
    static const struct Hook BrowsePLUS60KFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_PLUS60K, NULL };
    static const struct Hook BrowsePLUS256KFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_PLUS256K, NULL };
#else
    static const struct Hook Browse256KFileHook = { { NULL, NULL }, (VOID *)Browse_256K, NULL, NULL };
    static const struct Hook BrowsePLUS60KFileHook = { { NULL, NULL }, (VOID *)Browse_PLUS60K, NULL, NULL };
    static const struct Hook BrowsePLUS256KFileHook = { { NULL, NULL }, (VOID *)Browse_PLUS256K, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_MEMORY_HACK_DEVICE), ui_memory_hack_device)
           CYCLE(ui_to_from[1].object, translate_text(IDS_256K_BASE), ui_c64_256k_base)
           FILENAME(ui_to_from[2].object, translate_text(IDS_256K_FILENAME), browse_button_256K)
           CYCLE(ui_to_from[3].object, translate_text(IDS_PLUS60K_BASE), ui_plus60k_base)
           FILENAME(ui_to_from[4].object, translate_text(IDS_PLUS60K_FILENAME), browse_button_PLUS60K)
           FILENAME(ui_to_from[5].object, translate_text(IDS_PLUS256K_FILENAME), browse_button_PLUS256K)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button_256K, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &Browse256KFileHook);

        DoMethod(browse_button_PLUS60K, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowsePLUS60KFileHook);

        DoMethod(browse_button_PLUS256K, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowsePLUS256KFileHook);
    }

    return ui;
}

void ui_c64_memory_hacks_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    memory_hacks_canvas = canvas;
    ui_memory_hack_device[0] = translate_text(IDS_NONE);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_C64_MEMORY_HACKS_SETTINGS));

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
