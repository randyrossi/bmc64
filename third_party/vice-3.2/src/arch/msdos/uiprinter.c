/*
 * uiprinter.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdio.h>

#include "machine.h"
#include "printer.h"
#include "resources.h"
#include "tuimenu.h"
#include "uiprinter.h"

static TUI_MENU_CALLBACK(form_feed_callback)
{
    if (been_activated) {
        printer_formfeed((unsigned int)param);
    }

    *become_default = 0;

    return NULL;
}

static TUI_MENU_CALLBACK(printer_driver_submenu_callback)
{
    const char *s;

    resources_get_string((char *)param, &s);

    return s;
}

static TUI_MENU_CALLBACK(printer_text_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int((char *)param, &value);

    sprintf(s, "%d", value + 1);

    return s;
}

TUI_MENU_DEFINE_RADIO(PrinterUserportDriver)

static tui_menu_item_def_t printerusdriver_submenu[] = {
    { "_ASCII", NULL, radio_PrinterUserportDriver_callback,
      (void *)"ascii", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_NL10", NULL, radio_PrinterUserportDriver_callback,
      (void *)"nl10", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_RAW", NULL, radio_PrinterUserportDriver_callback,
      (void *)"raw", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(PrinterUserportTextDevice)

static tui_menu_item_def_t printerus_text_submenu[] = {
    { "_1", NULL, radio_PrinterUserportTextDevice_callback,
      (void *)0, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2", NULL, radio_PrinterUserportTextDevice_callback,
      (void *)1, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3", NULL, radio_PrinterUserportTextDevice_callback,
      (void *)2, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(PrinterUserport)

static tui_menu_item_def_t printerus_submenu[] = {
    { "_Userport printer:", "Enable userport printer emulation",
      toggle_PrinterUserport_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Printer _driver:", "Select the printer driver",
      printer_driver_submenu_callback, "PrinterUserportDriver", 7,
      TUI_MENU_BEH_CONTINUE, printerusdriver_submenu,
      "Userport printer driver" },
    { "_Text output file/device:",
      "Select the number of the text output file/device",
      printer_text_submenu_callback, "PrinterUserportTextDevice", 2,
      TUI_MENU_BEH_CONTINUE, printerus_text_submenu,
      "Text output file/device" },
    { "--" },
    { "Send _form feed", "Send a form feed to the printer",
      form_feed_callback, (void *)3, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(printer_device_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int((char *)param, &value);

    switch (value) {
        case PRINTER_DEVICE_NONE:
            sprintf(s, "None");
            break;
        case PRINTER_DEVICE_FS:
            sprintf(s, "File system access");
            break;
    }

    return s;
}

TUI_MENU_DEFINE_RADIO(Printer4)

static tui_menu_item_def_t printer4dev_submenu[] = {
    { "_None", NULL, radio_Printer4_callback,
      (void *)PRINTER_DEVICE_NONE, 19, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_File system access", NULL, radio_Printer4_callback,
      (void *)PRINTER_DEVICE_FS, 19, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer4Driver)

static tui_menu_item_def_t printer4driver_submenu[] = {
    { "_ASCII", NULL, radio_Printer4Driver_callback,
      (void *)"ascii", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_MPS803", NULL, radio_Printer4Driver_callback,
      (void *)"mps803", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_NL10", NULL, radio_Printer4Driver_callback,
      (void *)"nl10", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_RAW", NULL, radio_Printer4Driver_callback,
      (void *)"raw", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer4TextDevice)

static tui_menu_item_def_t printer4_text_submenu[] = {
    { "_1", NULL, radio_Printer4TextDevice_callback,
      (void *)0, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2", NULL, radio_Printer4TextDevice_callback,
      (void *)1, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3", NULL, radio_Printer4TextDevice_callback,
      (void *)2, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(IECDevice4)

static tui_menu_item_def_t printer4_submenu[] = {
    { "_Printer device:", "Select the printer device",
      printer_device_submenu_callback, "Printer4", 19,
      TUI_MENU_BEH_CONTINUE, printer4dev_submenu,
      "Printer #4 device" },
    { "_IEC device:", "Enable IEC device emulation for printer #4",
      toggle_IECDevice4_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Printer _driver:", "Select the printer driver",
      printer_driver_submenu_callback, "Printer4Driver", 7,
      TUI_MENU_BEH_CONTINUE, printer4driver_submenu,
      "Printer #4 driver" },
    { "_Text output file/device:",
      "Select the number of the text output file/device",
      printer_text_submenu_callback, "Printer4TextDevice", 2,
      TUI_MENU_BEH_CONTINUE, printer4_text_submenu,
      "Text output file/device" },
    { "--" },
    { "Send _form feed", "Send a form feed to the printer",
      form_feed_callback, (void *)0, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer5)

static tui_menu_item_def_t printer5dev_submenu[] = {
    { "_None", NULL, radio_Printer5_callback,
      (void *)PRINTER_DEVICE_NONE, 19, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_File system access", NULL, radio_Printer5_callback,
      (void *)PRINTER_DEVICE_FS, 19, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer5Driver)

static tui_menu_item_def_t printer5driver_submenu[] = {
    { "_ASCII", NULL, radio_Printer5Driver_callback,
      (void *)"ascii", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_MPS803", NULL, radio_Printer5Driver_callback,
      (void *)"mps803", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_NL10", NULL, radio_Printer5Driver_callback,
      (void *)"nl10", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_RAW", NULL, radio_Printer5Driver_callback,
      (void *)"raw", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer5TextDevice)

static tui_menu_item_def_t printer5_text_submenu[] = {
    { "_1", NULL, radio_Printer5TextDevice_callback,
      (void *)0, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2", NULL, radio_Printer5TextDevice_callback,
      (void *)1, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3", NULL, radio_Printer5TextDevice_callback,
      (void *)2, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(IECDevice5)

static tui_menu_item_def_t printer5_submenu[] = {
    { "_Printer device:", "Select the printer device",
      printer_device_submenu_callback, "Printer5", 19,
      TUI_MENU_BEH_CONTINUE, printer5dev_submenu,
      "Printer #5 device" },
    { "_IEC device:", "Enable IEC device emulation for printer #5",
      toggle_IECDevice5_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Printer _driver:", "Select the printer driver",
      printer_driver_submenu_callback, "Printer5Driver", 7,
      TUI_MENU_BEH_CONTINUE, printer5driver_submenu,
      "Printer #5 driver" },
    { "_Text output file/device:",
      "Select the number of the text output file/device",
      printer_text_submenu_callback, "Printer5TextDevice", 2,
      TUI_MENU_BEH_CONTINUE, printer5_text_submenu,
      "Text output file/device" },
    { "--" },
    { "Send _form feed", "Send a form feed to the printer",
      form_feed_callback, (void *)1, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer6)

static tui_menu_item_def_t printer6dev_submenu[] = {
    { "_None", NULL, radio_Printer6_callback,
      (void *)PRINTER_DEVICE_NONE, 19, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_File system access", NULL, radio_Printer6_callback,
      (void *)PRINTER_DEVICE_FS, 19, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer6Driver)

static tui_menu_item_def_t printer6driver_submenu[] = {
    { "_1520", NULL, radio_Printer6Driver_callback,
      (void *)"1520", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_RAW", NULL, radio_Printer6Driver_callback,
      (void *)"raw", 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(Printer6TextDevice)

static tui_menu_item_def_t printer6_text_submenu[] = {
    { "_1", NULL, radio_Printer6TextDevice_callback,
      (void *)0, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2", NULL, radio_Printer6TextDevice_callback,
      (void *)1, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3", NULL, radio_Printer6TextDevice_callback,
      (void *)2, 2, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(IECDevice6)

static tui_menu_item_def_t printer6_submenu[] = {
    { "_Printer device:", "Select the printer device",
      printer_device_submenu_callback, "Printer6", 19,
      TUI_MENU_BEH_CONTINUE, printer6dev_submenu,
      "Printer #6 device" },
    { "_IEC device:", "Enable IEC device emulation for printer #6",
      toggle_IECDevice6_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Printer _driver:", "Select the printer driver",
      printer_driver_submenu_callback, "Printer6Driver", 7,
      TUI_MENU_BEH_CONTINUE, printer6driver_submenu,
      "Printer #6 driver" },
    { "_Text output file/device:",
      "Select the number of the text output file/device",
      printer_text_submenu_callback, "Printer6TextDevice", 2,
      TUI_MENU_BEH_CONTINUE, printer6_text_submenu,
      "Text output file/device" },
    { "--" },
    { "Send _form feed", "Send a form feed to the printer",
      form_feed_callback, (void *)2, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(text_output_file_callback)
{
    const char *s;

    resources_get_string((char *)param, &s);

    return s;
}

static tui_menu_item_def_t printer_with_userport_submenu[] = {
    { "_Userport printer...", "Settings for userport printer", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, printerus_submenu, "Userport printer settings" },
    { "Printer #_4...", "Settings for printer #4", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, printer4_submenu, "Printer #4 settings" },
    { "Printer #_5...", "Settings for printer #5", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, printer5_submenu, "Printer #5 settings" },
    { "Printer #_6...", "Settings for printer #6", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, printer6_submenu, "Printer #6 settings" },
    { "--" },
    { "Text output file/device _1:",
      "Select the text output file or device",
      text_output_file_callback, (void *)"PrinterTextDevice1", 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Text output file/device _2:",
      "Select the text output file or device",
      text_output_file_callback, (void *)"PrinterTextDevice2", 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Text output file/device _3:",
      "Select the text output file or device",
      text_output_file_callback, (void *)"PrinterTextDevice3", 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t printer_submenu[] = {
    { "Printer #_4...", "Settings for printer #4", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, printer4_submenu, "Printer #4 settings" },
    { "Printer #_5...", "Settings for printer #5", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, printer5_submenu, "Printer #5 settings" },
    { "Printer #_6...", "Settings for printer #6", NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, printer6_submenu, "Printer #6 settings" },
    { "--" },
    { "Text output file/device _1:",
      "Select the text output file or device",
      text_output_file_callback, (void *)"PrinterTextDevice1", 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Text output file/device _2:",
      "Select the text output file or device",
      text_output_file_callback, (void *)"PrinterTextDevice2", 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Text output file/device _3:",
      "Select the text output file or device",
      text_output_file_callback, (void *)"PrinterTextDevice3", 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiprinter_init(struct tui_menu *parent_submenu)
{
    tui_menu_t tmp = tui_menu_create("Printer settings", 1);

    if (machine_class != VICE_MACHINE_VSID &&
        machine_class != VICE_MACHINE_C64DTV &&
        machine_class != VICE_MACHINE_PLUS4) {
        tui_menu_add(tmp, printer_with_userport_submenu);
    } else {
        tui_menu_add(tmp, printer_submenu);
    }

    tui_menu_add_submenu(parent_submenu,
                         "P_rinter settings...",
                         "Various printer settings",
                         tmp, NULL, NULL, 0);
}
