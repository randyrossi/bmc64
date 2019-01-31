/*
 * uikeymap.c
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

#include "keyboard.h"
#include "uikeymap.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *keymap_canvas;

static int ui_keymap_active_translate[] = {
    IDS_SYMBOLIC,
    IDS_POSITIONAL,
    IDS_SYMBOLIC_USER,
    IDS_POSITIONAL_USER,
    0
};

static char *ui_keymap_active[countof(ui_keymap_active_translate)];

static const int ui_keymap_active_values[] = {
    KBD_INDEX_SYM,
    KBD_INDEX_POS,
    KBD_INDEX_USERSYM,
    KBD_INDEX_USERPOS,
    -1
};

static char *ui_keymap_mapping[KBD_MAPPING_NUM + 1];

static int ui_keymap_mapping_values[KBD_MAPPING_NUM + 1];

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "KeymapIndex", ui_keymap_active, ui_keymap_active_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KeyboardMapping", ui_keymap_mapping, ui_keymap_mapping_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "KeymapUserSymFile", NULL, NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "KeymapUserPosFile", NULL, NULL, NULL },
    UI_END /* mandatory */
};

static ULONG BrowseSym( struct Hook *hook, Object *obj, APTR arg )
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT), "#?.vkm", keymap_canvas);

    if (fname != NULL) {
        set(ui_to_from[2].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static ULONG BrowsePos( struct Hook *hook, Object *obj, APTR arg )
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_POSITIONAL_KEYMAP_FILENAME_SELECT), "#?.vkm", keymap_canvas);

    if (fname != NULL) {
        set(ui_to_from[3].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static APTR build_gui(void)
{
    APTR app, ui, ok, browse_sym_button, browse_pos_button, cancel;

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseSymFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)BrowseSym, NULL };
    static const struct Hook BrowsePosFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)BrowsePos, NULL };
#else
    static const struct Hook BrowseSymFileHook = { { NULL, NULL }, (VOID *)BrowseSym, NULL, NULL };
    static const struct Hook BrowsePosFileHook = { { NULL, NULL }, (VOID *)BrowsePos, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_ACTIVE_KEYMAP), ui_keymap_active)
           CYCLE(ui_to_from[1].object, translate_text(IDS_KEYBOARD_MAPPING), ui_keymap_mapping)
           FILENAME(ui_to_from[2].object, translate_text(IDS_USER_SYMBOLIC_KEYMAP_FILENAME), browse_sym_button)
           FILENAME(ui_to_from[3].object, translate_text(IDS_USER_POSITIONAL_KEYMAP_FILENAME), browse_pos_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_sym_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseSymFileHook);

        DoMethod(browse_pos_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowsePosFileHook);
    }

    return ui;
}

void ui_keymap_settings_dialog(video_canvas_t *canvas)
{
    APTR window;
    mapping_info_t *kbdlist = keyboard_get_info_list();
    int num = keyboard_get_num_mappings();
    int i = 0;

    keymap_canvas = canvas;
    intl_convert_mui_table(ui_keymap_active_translate, ui_keymap_active);

    while (num) {
        ui_keymap_mapping[i] = kbdlist->name;
        ui_keymap_mapping_values[i] = kbdlist->mapping;
        i++;
        kbdlist++;
        num--;
    }
    ui_keymap_mapping[i] = NULL;
    ui_keymap_mapping_values[i] = -1;

    window = mui_make_simple_window(build_gui(), translate_text(IDS_KEYMAP_SETTINGS));

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
