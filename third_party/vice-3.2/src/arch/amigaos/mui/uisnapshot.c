/*
 * uisnapshot.c
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

#include "machine.h"
#include "uisnapshot.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *snapshot_canvas;

static APTR save_disks_check, save_roms_check, filename_check;

static ULONG Browse(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SELECT_SNAPSHOT_FILE), "#?.vsf", snapshot_canvas);

    if (fname != NULL && *fname != '\0') {
        if (strcasecmp(fname+(strlen(fname)-4),".vsf")) {
            strcat(fname, ".vsf");
        }
        set(filename_check, MUIA_String_Contents, fname);
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
           CHECK(save_disks_check, translate_text(IDS_SAVE_CURRENTLY_ATTACHED_DISKS))
           CHECK(save_roms_check, translate_text(IDS_SAVE_CURRENTLY_ATTACHED_ROMS))
           FILENAME(filename_check, translate_text(IDS_SNAPSHOT_FILE), browse_button)
           Child, HGroup,
             Child, ok = TextObject,
               ButtonFrame,
               MUIA_Background, MUII_ButtonBack,
               MUIA_Text_Contents, translate_text(IDS_SAVE),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode, MUIV_InputMode_RelVerify,
             End,
             Child, cancel = TextObject,
               ButtonFrame,
               MUIA_Background, MUII_ButtonBack,
               MUIA_Text_Contents, translate_text(IDS_CANCEL),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode, MUIV_InputMode_RelVerify,
             End,
           End,
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

void uisnapshot_save_dialog(video_canvas_t *canvas)
{
    APTR window;
    char *str;
    int save_roms = 0;
    int save_disks = 0;

    snapshot_canvas = canvas;

    window = mui_make_simple_window(build_gui(), translate_text(IDS_SAVE_SNAPSHOT_IMAGE));

    if (window != NULL) {
        mui_add_window(window);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            get(save_disks_check, MUIA_String_Contents, (APTR)&str);
            save_disks=atoi(str);
            get(save_roms_check, MUIA_String_Contents, (APTR)&str);
            save_roms = atoi(str);
            get(filename_check, MUIA_String_Contents, (APTR)&str);
            if (str != NULL && *str != '\0') {
                if (machine_write_snapshot(str, save_roms, save_disks, 0) < 0) {
                    snapshot_display_error();
                }
            }
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void uisnapshot_load_dialog(video_canvas_t *canvas)
{
    char *fname;

    snapshot_canvas = canvas;

    fname = BrowseFile(translate_text(IDS_LOAD_SNAPSHOT_IMAGE), "#?.vsf", snapshot_canvas);

    if (fname != NULL && *fname != '\0')
    {
        if (machine_read_snapshot(fname, 0) < 0) {
            snapshot_display_error();
        }
    }
}
