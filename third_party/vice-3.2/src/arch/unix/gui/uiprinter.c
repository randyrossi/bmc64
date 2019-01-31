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
#include <string.h>

#include "lib.h"
#include "printer.h"
#include "resources.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

#include "uiprinter.h"


UI_MENU_DEFINE_STRING_RADIO(Printer4Driver)
UI_MENU_DEFINE_STRING_RADIO(Printer5Driver)
UI_MENU_DEFINE_STRING_RADIO(Printer6Driver)
UI_MENU_DEFINE_STRING_RADIO(PrinterUserportDriver)

#define UIPRINTER_PR_DRIVER_MENU(x)                                                \
ui_menu_entry_t uiprinter_pr##x##_driver_submenu[] = {                             \
    { "ASCII", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Printer##x##Driver, (ui_callback_data_t)"ascii", NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "MPS803", UI_MENU_TYPE_TICK,                                                 \
      (ui_callback_t)radio_Printer##x##Driver, (ui_callback_data_t)"mps803", NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "NL10", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Printer##x##Driver, (ui_callback_data_t)"nl10", NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "RAW", UI_MENU_TYPE_TICK,                                                    \
      (ui_callback_t)radio_Printer##x##Driver, (ui_callback_data_t)"raw", NULL,    \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    UI_MENU_ENTRY_LIST_END                                                         \
}

UIPRINTER_PR_DRIVER_MENU(4);
UIPRINTER_PR_DRIVER_MENU(5);

ui_menu_entry_t uiprinter_pr6_driver_submenu[] = {
    { "1520", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_Printer6Driver, (ui_callback_data_t)"1520", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "RAW", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_Printer6Driver, (ui_callback_data_t)"raw", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiprinter_pruser_driver_submenu[] = {
    { "ASCII", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PrinterUserportDriver, (ui_callback_data_t)"ascii", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "NL10", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PrinterUserportDriver, (ui_callback_data_t)"nl10", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "RAW", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PrinterUserportDriver, (ui_callback_data_t)"raw", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_STRING_RADIO(Printer4Output)
UI_MENU_DEFINE_STRING_RADIO(Printer5Output)
UI_MENU_DEFINE_STRING_RADIO(Printer6Output)
UI_MENU_DEFINE_STRING_RADIO(PrinterUserportOutput)

#define UIPRINTER_PR_OUTPUT_MENU(x)                                                  \
ui_menu_entry_t uiprinter_pr##x##_output_submenu[] = {                               \
    { "Text", UI_MENU_TYPE_TICK,                                                     \
      (ui_callback_t)radio_Printer##x##Output, (ui_callback_data_t)"text", NULL,     \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                     \
    { "Graphics", UI_MENU_TYPE_TICK,                                                 \
      (ui_callback_t)radio_Printer##x##Output, (ui_callback_data_t)"graphics", NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                     \
    UI_MENU_ENTRY_LIST_END                                                           \
}

UIPRINTER_PR_OUTPUT_MENU(4);
UIPRINTER_PR_OUTPUT_MENU(5);
UIPRINTER_PR_OUTPUT_MENU(6);
UIPRINTER_PR_OUTPUT_MENU(Userport);

UI_MENU_DEFINE_RADIO(PrinterUserportTextDevice)
UI_MENU_DEFINE_RADIO(Printer4TextDevice)
UI_MENU_DEFINE_RADIO(Printer5TextDevice)
UI_MENU_DEFINE_RADIO(Printer6TextDevice)

#define UIPRINTER_PR_DEVICE_MENU(x, y)                                          \
ui_menu_entry_t uiprinter_pr##x##_device_submenu[] = {                          \
    { N_("Printer 1 (file dump)"), UI_MENU_TYPE_TICK,                           \
      (ui_callback_t)radio_Printer##y##TextDevice, (ui_callback_data_t)0, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { N_("Printer 2 (exec)"), UI_MENU_TYPE_TICK,                                \
      (ui_callback_t)radio_Printer##y##TextDevice, (ui_callback_data_t)1, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { N_("Printer 3 (exec)"), UI_MENU_TYPE_TICK,                                \
      (ui_callback_t)radio_Printer##y##TextDevice, (ui_callback_data_t)2, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    UI_MENU_ENTRY_LIST_END                                                      \
}

UIPRINTER_PR_DEVICE_MENU(user, Userport);
UIPRINTER_PR_DEVICE_MENU(4, 4);
UIPRINTER_PR_DEVICE_MENU(5, 5);
UIPRINTER_PR_DEVICE_MENU(6, 6);

UI_CALLBACK(uiprinter_set_printer_exec_file)
{
    char *command_text = util_concat(_("Command"), ":", NULL);

    uilib_select_string((char *)UI_MENU_CB_PARAM, _("Command to execute for printing (preceed with '|')"), command_text);
    lib_free(command_text);
}

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(Printer4)
UI_MENU_DEFINE_RADIO(Printer5)
UI_MENU_DEFINE_RADIO(Printer6)

#define UIPRINTER_SET_PRINTER_TYPE_COMMON(x)                                          \
    { N_("None"), UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Printer##x, (ui_callback_data_t)PRINTER_DEVICE_NONE, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { N_("File system access"), UI_MENU_TYPE_TICK,                                    \
      (ui_callback_t)radio_Printer##x, (ui_callback_data_t)PRINTER_DEVICE_FS, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }

#define UIPRINTER_SET_PRINTER_TYPE_OPENCBM(x)                                         \
    { N_("Real device access"), UI_MENU_TYPE_TICK,                                    \
      (ui_callback_t)radio_Printer##x, (ui_callback_data_t)PRINTER_DEVICE_REAL, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }

ui_menu_entry_t uiprinter_set_printer4_type_submenu[] = {
    UIPRINTER_SET_PRINTER_TYPE_COMMON(4),
#ifdef HAVE_OPENCBM
    UIPRINTER_SET_PRINTER_TYPE_OPENCBM(4),
#endif
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiprinter_set_printer5_type_submenu[] = {
    UIPRINTER_SET_PRINTER_TYPE_COMMON(5),
#ifdef HAVE_OPENCBM
    UIPRINTER_SET_PRINTER_TYPE_OPENCBM(5),
#endif
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiprinter_set_printer6_type_submenu[] = {
    UIPRINTER_SET_PRINTER_TYPE_COMMON(6),
#ifdef HAVE_OPENCBM
    UIPRINTER_SET_PRINTER_TYPE_OPENCBM(6),
#endif
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

UI_CALLBACK(uiprinter_formfeed)
{
    printer_formfeed(vice_ptr_to_uint(UI_MENU_CB_PARAM));
}
