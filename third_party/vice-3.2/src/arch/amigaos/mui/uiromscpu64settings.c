/*
 * uiromscpu64settings.c
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

#include "uiromscpu64settings.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *rom_canvas;

static ui_to_from_t ui_to_from_computer[] = {
    {NULL, MUI_TYPE_FILENAME, "SCPU64Name", NULL, NULL},
    {NULL, MUI_TYPE_FILENAME, "ChargenName", NULL, NULL},
    UI_END /* mandatory */
};

static APTR hook_object_computer[countof(ui_to_from_computer)];

static ui_to_from_t ui_to_from_drive[] = {
    { NULL, MUI_TYPE_FILENAME, "DosName1540", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName1541", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName1541ii", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName1570", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName1571", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName1581", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName2000", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName4000", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName2031", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName2040", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName3040", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName4040", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DosName1001", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DriveProfDOS1571Name", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "DriveSuperCardName", NULL, NULL },
    UI_END /* mandatory */
};

static APTR hook_object_drive[countof(ui_to_from_drive)];

    BROWSE(BrowseComputer0, ComputerHook0, ui_to_from_computer[0].object);
    BROWSE(BrowseComputer1, ComputerHook1, ui_to_from_computer[1].object);

    BROWSE(BrowseDrive0, DriveHook0, ui_to_from_drive[0].object);
    BROWSE(BrowseDrive1, DriveHook1, ui_to_from_drive[1].object);
    BROWSE(BrowseDrive2, DriveHook2, ui_to_from_drive[2].object);
    BROWSE(BrowseDrive3, DriveHook3, ui_to_from_drive[3].object);
    BROWSE(BrowseDrive4, DriveHook4, ui_to_from_drive[4].object);
    BROWSE(BrowseDrive5, DriveHook5, ui_to_from_drive[5].object);
    BROWSE(BrowseDrive6, DriveHook6, ui_to_from_drive[6].object);
    BROWSE(BrowseDrive7, DriveHook7, ui_to_from_drive[7].object);
    BROWSE(BrowseDrive8, DriveHook8, ui_to_from_drive[8].object);
    BROWSE(BrowseDrive9, DriveHook9, ui_to_from_drive[9].object);
    BROWSE(BrowseDrive10, DriveHook10, ui_to_from_drive[10].object);
    BROWSE(BrowseDrive11, DriveHook11, ui_to_from_drive[11].object);
    BROWSE(BrowseDrive12, DriveHook12, ui_to_from_drive[12].object);
    BROWSE(BrowseDrive13, DriveHook13, ui_to_from_drive[13].object);
    BROWSE(BrowseDrive14, DriveHook14, ui_to_from_drive[14].object);

static APTR build_computer_gui(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           FILENAME(ui_to_from_computer[0].object, "SCPU64 ROM", hook_object_computer[0])
           FILENAME(ui_to_from_computer[1].object, "Character", hook_object_computer[2])
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(hook_object_computer[0], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseComputer0);

        DoMethod(hook_object_computer[1], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseComputer1);
    }

    return ui;
}

static APTR build_drive_gui(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           FILENAME(ui_to_from_drive[0].object, "1540", hook_object_drive[0])
           FILENAME(ui_to_from_drive[1].object, "1541", hook_object_drive[1])
           FILENAME(ui_to_from_drive[2].object, "1541-II", hook_object_drive[2])
           FILENAME(ui_to_from_drive[3].object, "1570", hook_object_drive[3])
           FILENAME(ui_to_from_drive[4].object, "1571", hook_object_drive[4])
           FILENAME(ui_to_from_drive[5].object, "1581", hook_object_drive[5])
           FILENAME(ui_to_from_drive[6].object, "2000", hook_object_drive[6])
           FILENAME(ui_to_from_drive[7].object, "4000", hook_object_drive[7])
           FILENAME(ui_to_from_drive[8].object, "2031", hook_object_drive[8])
           FILENAME(ui_to_from_drive[9].object, "2030", hook_object_drive[9])
           FILENAME(ui_to_from_drive[10].object, "3040", hook_object_drive[10])
           FILENAME(ui_to_from_drive[11].object, "4040", hook_object_drive[11])
           FILENAME(ui_to_from_drive[12].object, "1001", hook_object_drive[12])
           FILENAME(ui_to_from_drive[13].object, "ProfDOS", hook_object_drive[13])
           FILENAME(ui_to_from_drive[14].object, "SC+", hook_object_drive[14])
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(hook_object_drive[0], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive0);

        DoMethod(hook_object_drive[1], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive1);

        DoMethod(hook_object_drive[2], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive2);

        DoMethod(hook_object_drive[3], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive3);

        DoMethod(hook_object_drive[4], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive4);

        DoMethod(hook_object_drive[5], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive5);

        DoMethod(hook_object_drive[6], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive6);

        DoMethod(hook_object_drive[7], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive7);

        DoMethod(hook_object_drive[8], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive8);

        DoMethod(hook_object_drive[9], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive9);

        DoMethod(hook_object_drive[10], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive10);

        DoMethod(hook_object_drive[11], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive11);

        DoMethod(hook_object_drive[12], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive12);

        DoMethod(hook_object_drive[13], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive13);

        DoMethod(hook_object_drive[14], MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseDrive14);
    }

    return ui;
}

void ui_scpu64_computer_rom_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    rom_canvas = canvas;

    window = mui_make_simple_window(build_computer_gui(), translate_text(IDS_COMPUTER_ROM_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from_computer);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from_computer);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_scpu64_drive_rom_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    rom_canvas = canvas;

    window = mui_make_simple_window(build_drive_gui(), translate_text(IDS_DRIVE_ROM_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from_drive);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from_drive);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
