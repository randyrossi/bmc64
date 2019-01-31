/*
 * uiretroreplay.c
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

#include "uiretroreplay.h"
#include "clockport.h"
#include "intl.h"
#include "translate.h"

static int ui_retroreplay_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_retroreplay_enable[countof(ui_retroreplay_enable_translate)];

static const int ui_retroreplay_enable_values[] = {
    0,
    1,
    -1
};

static char *ui_retroreplay_revision[] = {
    "Retro Replay",
    "Nordic Replay",
    NULL
};

static const int ui_retroreplay_revision_values[] = {
    0,
    1,
    -1
};

static char *ui_retroreplay_clockport_devices[CLOCKPORT_MAX_ENTRIES + 1];
static int ui_retroreplay_clockport_devices_values[CLOCKPORT_MAX_ENTRIES + 1];

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "RRFlashJumper", ui_retroreplay_enable, ui_retroreplay_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RRBankJumper", ui_retroreplay_enable, ui_retroreplay_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RRrevision", ui_retroreplay_revision, ui_retroreplay_revision_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RRBiosWrite", ui_retroreplay_enable, ui_retroreplay_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RRClockPort", ui_retroreplay_clockport_devices, ui_retroreplay_clockport_devices_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from[0].object, translate_text(IDS_BIOS_FLASH_JUMPER), ui_retroreplay_enable)
           CYCLE(ui_to_from[1].object, translate_text(IDS_BANK_JUMPER), ui_retroreplay_enable)
           CYCLE(ui_to_from[2].object, translate_text(IDS_BIOS_REVISION), ui_retroreplay_revision)
           CYCLE(ui_to_from[3].object, translate_text(IDS_SAVE_BIOS_WHEN_CHANGED), ui_retroreplay_enable)
           CYCLE(ui_to_from[4].object, translate_text(IDS_CLOCKPORT_DEVICE), ui_retroreplay_clockport_devices)
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

void ui_retroreplay_settings_dialog(void)
{
    APTR window;
    int i;

    for (i = 0; clockport_supported_devices[i].name; ++i) {
        ui_retroreplay_clockport_devices[i] = clockport_supported_devices[i].name;
        ui_retroreplay_clockport_devices_values[i] = clockport_supported_devices[i].id;
    }
    ui_retroreplay_clockport_devices[i] = NULL;
    ui_retroreplay_clockport_devices_values[i] = -1;

    intl_convert_mui_table(ui_retroreplay_enable_translate, ui_retroreplay_enable);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_RETROREPLAY_SETTINGS));

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
