/*
 * uiprinter.c
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

#include "printer.h"
#include "uiprinter.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *printer_canvas;

static int ui_printer_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_printer_enable[countof(ui_printer_enable_translate)];

static const int ui_printer_enable_values[] = {
    0,
    1,
    -1
};

static int ui_printer_emulation_translate[] = {
    IDS_NONE,
    IDS_FILESYSTEM,
    0
};

static char *ui_printer_emulation[countof(ui_printer_emulation_translate)];

static const int ui_printer_emulation_values[] = {
    PRINTER_DEVICE_NONE,
    PRINTER_DEVICE_FS,
    -1
};

static char *ui_userprinter_driver[] = {
    "ASCII",
    "NL10",
    NULL,               /* placeholder for 'raw' */
    NULL
};

static char *ui_userprinter_driver_strings[] = {
    "ascii",
    "nl10",
    "raw",
    NULL
};

static char *ui_printer_driver[] = {
    "ASCII",
    "MPS803",
    "NL10",
    NULL,               /* placeholder for 'raw' */
    NULL
};

static char *ui_printer_driver_strings[] = {
    "ascii",
    "mps803",
    "nl10",
    "raw",
    NULL
};

static char *ui_plotter_driver[] = {
    "1520",
    NULL,               /* placeholder for 'raw' */
    NULL
};

static char *ui_plotter_driver_strings[] = {
    "1520",
    "raw",
    NULL
};

static int ui_printer_output_type_translate[] = {
    IDS_TEXT,
    IDS_GRAPHICS,
    0
};

static char *ui_printer_output_type[countof(ui_printer_output_type_translate)];

static char *ui_printer_output_type_strings[] = {
    "text",
    "graphics",
    NULL
};

static char *ui_printer_output_device[] = {
    "1",
    "2",
    "3",
    NULL
};

static int ui_printer_output_device_values[] = {
    0,
    1,
    2,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "Printer4", ui_printer_emulation, ui_printer_emulation_values, NULL },
    { NULL, MUI_TYPE_CYCLE_STR, "Printer4Driver", ui_printer_driver, NULL, ui_printer_driver_strings },
    { NULL, MUI_TYPE_CYCLE_STR, "Printer4Output", ui_printer_output_type, NULL, ui_printer_output_type_strings },
    { NULL, MUI_TYPE_CYCLE, "Printer4TextDevice", ui_printer_output_device, ui_printer_output_device_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "IECDevice4", ui_printer_enable, ui_printer_enable_values, NULL },

    { NULL, MUI_TYPE_CYCLE, "Printer5", ui_printer_emulation, ui_printer_emulation_values, NULL },
    { NULL, MUI_TYPE_CYCLE_STR, "Printer5Driver", ui_printer_driver, NULL, ui_printer_driver_strings },
    { NULL, MUI_TYPE_CYCLE_STR, "Printer5Output", ui_printer_output_type, NULL, ui_printer_output_type_strings },
    { NULL, MUI_TYPE_CYCLE, "Printer5TextDevice", ui_printer_output_device, ui_printer_output_device_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "IECDevice5", ui_printer_enable, ui_printer_enable_values, NULL },

    { NULL, MUI_TYPE_CYCLE, "Printer6", ui_printer_emulation, ui_printer_emulation_values, NULL },
    { NULL, MUI_TYPE_CYCLE_STR, "Printer6Driver", ui_plotter_driver, NULL, ui_plotter_driver_strings },
    { NULL, MUI_TYPE_CYCLE_STR, "Printer6Output", ui_printer_output_type, NULL, ui_printer_output_type_strings },
    { NULL, MUI_TYPE_CYCLE, "Printer6TextDevice", ui_printer_output_device, ui_printer_output_device_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "IECDevice6", ui_printer_enable, ui_printer_enable_values, NULL },

    { NULL, MUI_TYPE_CYCLE, "IECDevice7", ui_printer_enable, ui_printer_enable_values, NULL },

    { NULL, MUI_TYPE_CYCLE, "PrinterUserport", ui_printer_enable, ui_printer_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE_STR, "PrinterUserportDriver", ui_userprinter_driver, NULL, ui_userprinter_driver_strings },
    { NULL, MUI_TYPE_CYCLE_STR, "PrinterUserportOutput", ui_printer_output_type, NULL, ui_printer_output_type_strings },
    { NULL, MUI_TYPE_CYCLE, "PrinterUserportTextDevice", ui_printer_output_device, ui_printer_output_device_values, NULL },
    { NULL, MUI_TYPE_FILENAME, "PrinterTextDevice1", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "PrinterTextDevice2", NULL, NULL },
    { NULL, MUI_TYPE_FILENAME, "PrinterTextDevice3", NULL, NULL },
    UI_END /* mandatory */
};

static ULONG formfeed_4(struct Hook *hook, Object *obj, APTR arg)
{
    printer_formfeed(0);
    return 0;
}

static ULONG formfeed_5(struct Hook *hook, Object *obj, APTR arg)
{
    printer_formfeed(1);
    return 0;
}

static ULONG formfeed_6(struct Hook *hook, Object *obj, APTR arg)
{
    printer_formfeed(2);
    return 0;
}

static ULONG formfeed_userport(struct Hook *hook, Object *obj, APTR arg)
{
    printer_formfeed(3);
    return 0;
}

static ULONG Browse1(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_PRINTER_OUTPUT_FILENAME_SELECT), "#?", printer_canvas);

    if (fname != NULL) {
        set(ui_to_from[14].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static ULONG Browse2(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_PRINTER_OUTPUT_FILENAME_SELECT), "#?", printer_canvas);

    if (fname != NULL) {
        set(ui_to_from[15].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static ULONG Browse3(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_PRINTER_OUTPUT_FILENAME_SELECT), "#?", printer_canvas);

    if (fname != NULL) {
        set(ui_to_from[16].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static APTR build_gui(int ieee, int userport)
{
    APTR app, ui, ok, cancel;
    APTR browse_button1, browse_button2, browse_button3;
    APTR ff_button1, ff_button2, ff_button3, ff_button4;

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook1 = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse1, NULL };
    static const struct Hook BrowseFileHook2 = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse2, NULL };
    static const struct Hook BrowseFileHook3 = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse3, NULL };
    static const struct Hook FFHook4 = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)formfeed_4, NULL };
    static const struct Hook FFHook5 = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)formfeed_5, NULL };
    static const struct Hook FFHook6 = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)formfeed_6, NULL };
    static const struct Hook FFHookUserport = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)formfeed_userport, NULL };
#else
    static const struct Hook BrowseFileHook1 = { { NULL, NULL }, (VOID *)Browse1, NULL, NULL };
    static const struct Hook BrowseFileHook2 = { { NULL, NULL }, (VOID *)Browse2, NULL, NULL };
    static const struct Hook BrowseFileHook3 = { { NULL, NULL }, (VOID *)Browse3, NULL, NULL };
    static const struct Hook FFHook4 = { { NULL, NULL }, (VOID *)formfeed_4, NULL, NULL };
    static const struct Hook FFHook5 = { { NULL, NULL }, (VOID *)formfeed_5, NULL, NULL };
    static const struct Hook FFHook6 = { { NULL, NULL }, (VOID *)formfeed_6, NULL, NULL };
    static const struct Hook FFHookUserport = { { NULL, NULL }, (VOID *)formfeed_userport, NULL, NULL };
#endif

    app = mui_get_app();

    if (ieee) {
        ui = GroupObject,
               CYCLE(ui_to_from[0].object, translate_text(IDS_PRINTER_4_EMULATION), ui_printer_emulation)
               CYCLE(ui_to_from[1].object, translate_text(IDS_PRINTER_4_DRIVER), ui_printer_driver)
               CYCLE(ui_to_from[2].object, translate_text(IDS_PRINTER_4_OUTPUT_TYPE), ui_printer_output_type)
               CYCLE(ui_to_from[3].object, translate_text(IDS_PRINTER_4_OUTPUT_DEVICE), ui_printer_output_device)
               BUTTON(ff_button1, translate_text(IDS_PRINTER_4_SEND_FORMFEED))
               CYCLE(ui_to_from[5].object, translate_text(IDS_PRINTER_5_EMULATION), ui_printer_emulation)
               CYCLE(ui_to_from[6].object, translate_text(IDS_PRINTER_5_DRIVER), ui_printer_driver)
               CYCLE(ui_to_from[7].object, translate_text(IDS_PRINTER_5_OUTPUT_TYPE), ui_printer_output_type)
               CYCLE(ui_to_from[8].object, translate_text(IDS_PRINTER_5_OUTPUT_DEVICE), ui_printer_output_device)
               BUTTON(ff_button2, translate_text(IDS_PRINTER_5_SEND_FORMFEED))
               CYCLE(ui_to_from[10].object, translate_text(IDS_PRINTER_6_EMULATION), ui_printer_emulation)
               CYCLE(ui_to_from[11].object, translate_text(IDS_PRINTER_6_DRIVER), ui_plotter_driver)
               CYCLE(ui_to_from[12].object, translate_text(IDS_PRINTER_6_OUTPUT_TYPE), ui_printer_output_type)
               CYCLE(ui_to_from[13].object, translate_text(IDS_PRINTER_6_OUTPUT_DEVICE), ui_printer_output_device)
               BUTTON(ff_button3, translate_text(IDS_PRINTER_6_SEND_FORMFEED))
               CYCLE(ui_to_from[16].object, translate_text(IDS_USERPORT_PRINTER_EMULATION), ui_printer_enable)
               CYCLE(ui_to_from[17].object, translate_text(IDS_USERPORT_PRINTER_DRIVER), ui_userprinter_driver)
               CYCLE(ui_to_from[18].object, translate_text(IDS_USERPORT_PRINTER_OUTPUT_TYPE), ui_printer_output_type)
               CYCLE(ui_to_from[19].object, translate_text(IDS_USERPORT_PRINTER_OUTPUT_DEVICE), ui_printer_output_device)
               BUTTON(ff_button4, translate_text(IDS_USERPORT_PRINTER_SEND_FORMFEED))
               FILENAME(ui_to_from[20].object, translate_text(IDS_PRINTER_DEVICE_1_FILENAME), browse_button1)
               FILENAME(ui_to_from[21].object, translate_text(IDS_PRINTER_DEVICE_2_FILENAME), browse_button2)
               FILENAME(ui_to_from[22].object, translate_text(IDS_PRINTER_DEVICE_3_FILENAME), browse_button3)
               OK_CANCEL_BUTTON
             End;
    } else {
        if (userport) {
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, translate_text(IDS_PRINTER_4_EMULATION), ui_printer_emulation)
                   CYCLE(ui_to_from[1].object, translate_text(IDS_PRINTER_4_DRIVER), ui_printer_driver)
                   CYCLE(ui_to_from[2].object, translate_text(IDS_PRINTER_4_OUTPUT_TYPE), ui_printer_output_type)
                   CYCLE(ui_to_from[3].object, translate_text(IDS_PRINTER_4_OUTPUT_DEVICE), ui_printer_output_device)
                   BUTTON(ff_button1, translate_text(IDS_PRINTER_4_SEND_FORMFEED))
                   CYCLE(ui_to_from[4].object, translate_text(IDS_ENABLE_IEC_PRINTER_4), ui_printer_enable)
                   CYCLE(ui_to_from[5].object, translate_text(IDS_PRINTER_5_EMULATION), ui_printer_emulation)
                   CYCLE(ui_to_from[6].object, translate_text(IDS_PRINTER_5_DRIVER), ui_printer_driver)
                   CYCLE(ui_to_from[7].object, translate_text(IDS_PRINTER_5_OUTPUT_TYPE), ui_printer_output_type)
                   CYCLE(ui_to_from[8].object, translate_text(IDS_PRINTER_5_OUTPUT_DEVICE), ui_printer_output_device)
                   BUTTON(ff_button2, translate_text(IDS_PRINTER_5_SEND_FORMFEED))
                   CYCLE(ui_to_from[9].object, translate_text(IDS_ENABLE_IEC_PRINTER_5), ui_printer_enable)
                   CYCLE(ui_to_from[10].object, translate_text(IDS_PRINTER_6_EMULATION), ui_printer_emulation)
                   CYCLE(ui_to_from[11].object, translate_text(IDS_PRINTER_6_DRIVER), ui_plotter_driver)
                   CYCLE(ui_to_from[12].object, translate_text(IDS_PRINTER_6_OUTPUT_TYPE), ui_printer_output_type)
                   CYCLE(ui_to_from[13].object, translate_text(IDS_PRINTER_6_OUTPUT_DEVICE), ui_printer_output_device)
                   BUTTON(ff_button3, translate_text(IDS_PRINTER_6_SEND_FORMFEED))
                   CYCLE(ui_to_from[14].object, translate_text(IDS_ENABLE_IEC_PRINTER_6), ui_printer_enable)
                   CYCLE(ui_to_from[15].object, translate_text(IDS_ENABLE_IEC_DEVICE_7), ui_printer_enable)
                   CYCLE(ui_to_from[16].object, translate_text(IDS_USERPORT_PRINTER_EMULATION), ui_printer_enable)
                   CYCLE(ui_to_from[17].object, translate_text(IDS_USERPORT_PRINTER_DRIVER), ui_userprinter_driver)
                   CYCLE(ui_to_from[18].object, translate_text(IDS_USERPORT_PRINTER_OUTPUT_TYPE), ui_printer_output_type)
                   CYCLE(ui_to_from[19].object, translate_text(IDS_USERPORT_PRINTER_OUTPUT_DEVICE), ui_printer_output_device)
                   BUTTON(ff_button4, translate_text(IDS_USERPORT_PRINTER_SEND_FORMFEED))
                   FILENAME(ui_to_from[20].object, translate_text(IDS_PRINTER_DEVICE_1_FILENAME), browse_button1)
                   FILENAME(ui_to_from[21].object, translate_text(IDS_PRINTER_DEVICE_2_FILENAME), browse_button2)
                   FILENAME(ui_to_from[22].object, translate_text(IDS_PRINTER_DEVICE_3_FILENAME), browse_button3)
                   OK_CANCEL_BUTTON
                 End;
        } else {
            ui = GroupObject,
                   CYCLE(ui_to_from[0].object, translate_text(IDS_PRINTER_4_EMULATION), ui_printer_emulation)
                   CYCLE(ui_to_from[1].object, translate_text(IDS_PRINTER_4_DRIVER), ui_printer_driver)
                   CYCLE(ui_to_from[2].object, translate_text(IDS_PRINTER_4_OUTPUT_TYPE), ui_printer_output_type)
                   CYCLE(ui_to_from[3].object, translate_text(IDS_PRINTER_4_OUTPUT_DEVICE), ui_printer_output_device)
                   BUTTON(ff_button1, translate_text(IDS_PRINTER_4_SEND_FORMFEED))
                   CYCLE(ui_to_from[4].object, translate_text(IDS_ENABLE_IEC_PRINTER_4), ui_printer_enable)
                   CYCLE(ui_to_from[5].object, translate_text(IDS_PRINTER_5_EMULATION), ui_printer_emulation)
                   CYCLE(ui_to_from[6].object, translate_text(IDS_PRINTER_5_DRIVER), ui_printer_driver)
                   CYCLE(ui_to_from[7].object, translate_text(IDS_PRINTER_5_OUTPUT_TYPE), ui_printer_output_type)
                   CYCLE(ui_to_from[8].object, translate_text(IDS_PRINTER_5_OUTPUT_DEVICE), ui_printer_output_device)
                   BUTTON(ff_button2, translate_text(IDS_PRINTER_5_SEND_FORMFEED))
                   CYCLE(ui_to_from[9].object, translate_text(IDS_ENABLE_IEC_PRINTER_5), ui_printer_enable)
                   CYCLE(ui_to_from[10].object, translate_text(IDS_PRINTER_6_EMULATION), ui_printer_emulation)
                   CYCLE(ui_to_from[11].object, translate_text(IDS_PRINTER_6_DRIVER), ui_plotter_driver)
                   CYCLE(ui_to_from[12].object, translate_text(IDS_PRINTER_6_OUTPUT_TYPE), ui_printer_output_type)
                   CYCLE(ui_to_from[13].object, translate_text(IDS_PRINTER_6_OUTPUT_DEVICE), ui_printer_output_device)
                   BUTTON(ff_button3, translate_text(IDS_PRINTER_6_SEND_FORMFEED))
                   CYCLE(ui_to_from[14].object, translate_text(IDS_ENABLE_IEC_PRINTER_6), ui_printer_enable)
                   CYCLE(ui_to_from[15].object, translate_text(IDS_ENABLE_IEC_DEVICE_7), ui_printer_enable)
                   FILENAME(ui_to_from[20].object, translate_text(IDS_PRINTER_DEVICE_1_FILENAME), browse_button1)
                   FILENAME(ui_to_from[21].object, translate_text(IDS_PRINTER_DEVICE_2_FILENAME), browse_button2)
                   FILENAME(ui_to_from[22].object, translate_text(IDS_PRINTER_DEVICE_3_FILENAME), browse_button3)
                   OK_CANCEL_BUTTON
                 End;
        }
    }

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button1, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook1);

        DoMethod(browse_button2, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook2);

        DoMethod(browse_button3, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook3);

        DoMethod(ff_button1, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &FFHook4);

        DoMethod(ff_button2, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &FFHook5);

        DoMethod(ff_button3, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &FFHook6);

        DoMethod(ff_button4, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &FFHookUserport);
    }

    return ui;
}

void ui_printer_settings_dialog(video_canvas_t *canvas, int ieee, int userport)
{
    APTR window;

    printer_canvas = canvas;
    intl_convert_mui_table(ui_printer_enable_translate, ui_printer_enable);
    intl_convert_mui_table(ui_printer_emulation_translate, ui_printer_emulation);
    ui_printer_driver[3] = translate_text(IDS_RAW);
    ui_userprinter_driver[2] = ui_printer_driver[3];
    ui_plotter_driver[1] = ui_printer_driver[3];
    intl_convert_mui_table(ui_printer_output_type_translate, ui_printer_output_type);
 
    window = mui_make_simple_window(build_gui(ieee, userport), translate_text(IDS_PRINTER_SETTINGS));

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
