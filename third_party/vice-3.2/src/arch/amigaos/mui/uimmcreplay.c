/*
 * uimmcreplay.c
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

#include "uimmcreplay.h"
#include "clockport.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *mmcreplay_canvas;

static int ui_mmcreplay_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_mmcreplay_enable[countof(ui_mmcreplay_enable_translate)];

static const int ui_mmcreplay_enable_values[] = {
    0,
    1,
    -1
};

static char *ui_mmcreplay_card_type[] = {
    "Auto",
    "MMC",
    "SD",
    "SDHC",
    NULL
};

static const int ui_mmcreplay_card_type_values[] = {
    0,
    1,
    2,
    3,
    -1
};

static char *ui_mmcreplay_clockport_devices[CLOCKPORT_MAX_ENTRIES + 1];
static int ui_mmcreplay_clockport_devices_values[CLOCKPORT_MAX_ENTRIES + 1];

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_FILENAME, "MMCRCardImage", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "MMCRCardRW", ui_mmcreplay_enable, ui_mmcreplay_enable_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "MMCREEPROMImage", NULL, NULL, NULL },
    { NULL, MUI_TYPE_CYCLE, "MMCRImageWrite", ui_mmcreplay_enable, ui_mmcreplay_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "MMCREEPROMRW", ui_mmcreplay_enable, ui_mmcreplay_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "MMCRRescueMode", ui_mmcreplay_enable, ui_mmcreplay_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "MMCRSDType", ui_mmcreplay_card_type, ui_mmcreplay_card_type_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "MMCRClockPort", ui_mmcreplay_clockport_devices, ui_mmcreplay_clockport_devices_values, NULL },
    UI_END /* mandatory */
};

static ULONG Browse_BIOS(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SELECT_EEPROM_FILE), "#?", mmcreplay_canvas);

    if (fname != NULL) {
        set(ui_to_from[2].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static ULONG Browse_Image(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SELECT_CARD_IMAGE_FILE), "#?", mmcreplay_canvas);

    if (fname != NULL) {
        set(ui_to_from[0].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static APTR build_gui(void)
{
    APTR app, ui, ok, browse_button_bios, browse_button_image, cancel;

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseBIOSHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_BIOS, NULL };
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_Image, NULL };
#else
    static const struct Hook BrowseBIOSHook = { { NULL, NULL }, (VOID *)Browse_BIOS, NULL, NULL };
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse_Image, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           FILENAME(ui_to_from[0].object, translate_text(IDS_CARD_FILE), browse_button_image)
           CYCLE(ui_to_from[1].object, translate_text(IDS_CARD_READ_WRITE), ui_mmcreplay_enable)
           FILENAME(ui_to_from[2].object, translate_text(IDS_EEPROM_FILE), browse_button_bios)
           CYCLE(ui_to_from[3].object, translate_text(IDS_WRITE_TO_EEPROM_WHEN_CHANGED), ui_mmcreplay_enable)
           CYCLE(ui_to_from[4].object, translate_text(IDS_EEPROM_READ_WRITE), ui_mmcreplay_enable)
           CYCLE(ui_to_from[5].object, translate_text(IDS_RESCUE_MODE), ui_mmcreplay_enable)
           CYCLE(ui_to_from[6].object, translate_text(IDS_SD_TYPE), ui_mmcreplay_card_type)
           CYCLE(ui_to_from[7].object, translate_text(IDS_CLOCKPORT_DEVICE), ui_mmcreplay_clockport_devices)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button_bios, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseBIOSHook);

        DoMethod(browse_button_image, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);
    }

    return ui;
}

void ui_mmcreplay_settings_dialog(video_canvas_t *canvas)
{
    APTR window;
    int i;

    for (i = 0; clockport_supported_devices[i].name; ++i) {
        ui_mmcreplay_clockport_devices[i] = clockport_supported_devices[i].name;
        ui_mmcreplay_clockport_devices_values[i] = clockport_supported_devices[i].id;
    }
    ui_mmcreplay_clockport_devices[i] = NULL;
    ui_mmcreplay_clockport_devices_values[i] = -1;

    mmcreplay_canvas = canvas;
    intl_convert_mui_table(ui_mmcreplay_enable_translate, ui_mmcreplay_enable);

    window = mui_make_simple_window(build_gui(), translate_text(IDS_MMCREPLAY_SETTINGS));

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
