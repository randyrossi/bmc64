/*
 * menu_printer.h - Printer menu for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "types.h"

#include "menu_common.h"
#include "printer.h"
#include "resources.h"
#include "uimenu.h"

#define VICE_SDL_PRINTER_DRIVER_MENU(prn)                             \
    UI_MENU_DEFINE_RADIO(Printer##prn##Driver)                        \
    static const ui_menu_entry_t printer_##prn##_driver_submenu[] = { \
        { "ASCII",                                                    \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##Driver_callback,                        \
          (ui_callback_data_t)"ascii" },                              \
        { "MPS803",                                                   \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##Driver_callback,                        \
          (ui_callback_data_t)"mps803" },                             \
        { "NL10",                                                     \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##Driver_callback,                        \
          (ui_callback_data_t)"nl10" },                               \
        { "Raw",                                                      \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##Driver_callback,                        \
          (ui_callback_data_t)"raw" },                                \
        SDL_MENU_LIST_END                                             \
    };

VICE_SDL_PRINTER_DRIVER_MENU(4)
VICE_SDL_PRINTER_DRIVER_MENU(5)

UI_MENU_DEFINE_RADIO(PrinterUserportDriver)

static const ui_menu_entry_t printer_Userport_driver_submenu[] = {
    { "ASCII",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PrinterUserportDriver_callback,
      (ui_callback_data_t)"ascii" },
    { "NL10",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PrinterUserportDriver_callback,
      (ui_callback_data_t)"nl10" },
    { "Raw",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PrinterUserportDriver_callback,
      (ui_callback_data_t)"raw" },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(Printer6Driver)

static const ui_menu_entry_t printer_6_driver_submenu[] = {
    { "1520",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Printer6Driver_callback,
      (ui_callback_data_t)"1520" },
    { "Raw",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Printer6Driver_callback,
      (ui_callback_data_t)"raw" },
    SDL_MENU_LIST_END
};

#define VICE_SDL_PRINTER_DEVICE_MENU(prn)                             \
    UI_MENU_DEFINE_RADIO(Printer##prn##TextDevice)                    \
    static const ui_menu_entry_t printer_##prn##_device_submenu[] = { \
        { "1",                                                        \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##TextDevice_callback,                    \
          (ui_callback_data_t)0 },                                    \
        { "2",                                                        \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##TextDevice_callback,                    \
          (ui_callback_data_t)1 },                                    \
        { "3",                                                        \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##TextDevice_callback,                    \
          (ui_callback_data_t)2 },                                    \
        SDL_MENU_LIST_END                                             \
    };

VICE_SDL_PRINTER_DEVICE_MENU(4)
VICE_SDL_PRINTER_DEVICE_MENU(5)
VICE_SDL_PRINTER_DEVICE_MENU(6)
VICE_SDL_PRINTER_DEVICE_MENU(Userport)


#define VICE_SDL_PRINTER_OUTPUT_MENU(prn)                             \
    UI_MENU_DEFINE_RADIO(Printer##prn##Output)                        \
    static const ui_menu_entry_t printer_##prn##_output_submenu[] = { \
        { "Text",                                                     \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##Output_callback,                        \
          (ui_callback_data_t)"text" },                               \
        { "Graphics",                                                 \
          MENU_ENTRY_RESOURCE_RADIO,                                  \
          radio_Printer##prn##Output_callback,                        \
          (ui_callback_data_t)"graphics" },                           \
        SDL_MENU_LIST_END                                             \
    };

VICE_SDL_PRINTER_OUTPUT_MENU(4)
VICE_SDL_PRINTER_OUTPUT_MENU(5)
VICE_SDL_PRINTER_OUTPUT_MENU(6)
VICE_SDL_PRINTER_OUTPUT_MENU(Userport)

#ifdef HAVE_OPENCBM

#define VICE_SDL_PRINTER_TYPE_MENU(prn)                             \
    UI_MENU_DEFINE_RADIO(Printer##prn)                              \
    static const ui_menu_entry_t printer_##prn##_type_submenu[] = { \
        { "None",                                                   \
          MENU_ENTRY_RESOURCE_RADIO,                                \
          radio_Printer##prn##_callback,                            \
          (ui_callback_data_t)PRINTER_DEVICE_NONE },                \
        { "File system access",                                     \
          MENU_ENTRY_RESOURCE_RADIO,                                \
          radio_Printer##prn##_callback,                            \
          (ui_callback_data_t)PRINTER_DEVICE_FS },                  \
        { "Real device access",                                     \
          MENU_ENTRY_RESOURCE_RADIO,                                \
          radio_Printer##prn##_callback,                            \
          (ui_callback_data_t)PRINTER_DEVICE_REAL },                \
        SDL_MENU_LIST_END                                           \
    };

#define VICE_SDL_DEVICE_TYPE_MENU(prn)                              \
    UI_MENU_DEFINE_RADIO(Printer##prn)                              \
    static const ui_menu_entry_t device_##prn##_type_submenu[] = {  \
        { "None",                                                   \
          MENU_ENTRY_RESOURCE_RADIO,                                \
          radio_Printer##prn##_callback,                            \
          (ui_callback_data_t)PRINTER_DEVICE_NONE },                \
        { "Real device access",                                     \
          MENU_ENTRY_RESOURCE_RADIO,                                \
          radio_Printer##prn##_callback,                            \
          (ui_callback_data_t)PRINTER_DEVICE_REAL },                \
        SDL_MENU_LIST_END                                           \
    };

#else   /* !HAVE_OPENCBM */

#define VICE_SDL_PRINTER_TYPE_MENU(prn)                             \
    UI_MENU_DEFINE_RADIO(Printer##prn)                              \
    static const ui_menu_entry_t printer_##prn##_type_submenu[] = { \
        { "None",                                                   \
          MENU_ENTRY_RESOURCE_RADIO,                                \
          radio_Printer##prn##_callback,                            \
          (ui_callback_data_t)PRINTER_DEVICE_NONE },                \
        { "File system access",                                     \
          MENU_ENTRY_RESOURCE_RADIO,                                \
          radio_Printer##prn##_callback,                            \
          (ui_callback_data_t)PRINTER_DEVICE_FS },                  \
        SDL_MENU_LIST_END                                           \
    };

#define VICE_SDL_DEVICE_TYPE_MENU(prn)

#endif

VICE_SDL_PRINTER_TYPE_MENU(4)
VICE_SDL_PRINTER_TYPE_MENU(5)
VICE_SDL_PRINTER_TYPE_MENU(6)

#ifdef HAVE_OPENCBM
VICE_SDL_DEVICE_TYPE_MENU(7)
UI_MENU_DEFINE_TOGGLE(IECDevice7)
#endif

UI_MENU_DEFINE_TOGGLE(IECDevice4)
UI_MENU_DEFINE_TOGGLE(IECDevice5)
UI_MENU_DEFINE_TOGGLE(IECDevice6)
UI_MENU_DEFINE_TOGGLE(PrinterUserport)

static UI_MENU_CALLBACK(uiprinter_formfeed_callback)
{
    printer_formfeed(vice_ptr_to_uint(param));
    return NULL;
}


#define VICE_SDL_PRINTER_COMMON_4_MENU_ITEMS          \
    { "Printer #4 emulation",                         \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_4_type_submenu },   \
    { "Printer #4 driver",                            \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_4_driver_submenu }, \
    { "Printer #4 output type",                       \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_4_output_submenu }, \
    { "Printer #4 output device",                     \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_4_device_submenu }, \
    { "Printer #4 form feed",                         \
      MENU_ENTRY_OTHER,                               \
      uiprinter_formfeed_callback,                    \
      (ui_callback_data_t)0 },

#define VICE_SDL_PRINTER_COMMON_5_MENU_ITEMS          \
    { "Printer #5 emulation",                         \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_5_type_submenu },   \
    { "Printer #5 driver",                            \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_5_driver_submenu }, \
    { "Printer #5 output type",                       \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_5_output_submenu }, \
    { "Printer #5 output device",                     \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_5_device_submenu }, \
    { "Printer #5 form feed",                         \
      MENU_ENTRY_OTHER,                               \
      uiprinter_formfeed_callback,                    \
      (ui_callback_data_t)1 },

#define VICE_SDL_PRINTER_COMMON_6_MENU_ITEMS          \
    { "Printer #6 emulation",                         \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_6_type_submenu },   \
    { "Printer #6 driver",                            \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_6_driver_submenu }, \
    { "Printer #6 output type",                       \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_6_output_submenu }, \
    { "Printer #6 output device",                     \
      MENU_ENTRY_SUBMENU,                             \
      submenu_radio_callback,                         \
      (ui_callback_data_t)printer_6_device_submenu }, \
    { "Printer #6 form feed",                         \
      MENU_ENTRY_OTHER,                               \
      uiprinter_formfeed_callback,                    \
      (ui_callback_data_t)2 },

#ifdef HAVE_OPENCBM
#define VICE_SDL_DEVICE_COMMON_7_MENU_ITEMS \
    { "Device #7 emulation",                \
      MENU_ENTRY_SUBMENU,                   \
      submenu_radio_callback,               \
      (ui_callback_data_t)device_7_type_submenu },
#endif

#define VICE_SDL_PRINTER_USERPORT_MENU_ITEMS                 \
    { "Userport printer emulation",                          \
      MENU_ENTRY_RESOURCE_TOGGLE,                            \
      toggle_PrinterUserport_callback,                       \
      NULL },                                                \
    { "Userport printer driver",                             \
      MENU_ENTRY_SUBMENU,                                    \
      submenu_radio_callback,                                \
      (ui_callback_data_t)printer_Userport_driver_submenu }, \
    { "Userport printer output type",                        \
      MENU_ENTRY_SUBMENU,                                    \
      submenu_radio_callback,                                \
      (ui_callback_data_t)printer_Userport_output_submenu }, \
    { "Userport printer output device",                      \
      MENU_ENTRY_SUBMENU,                                    \
      submenu_radio_callback,                                \
      (ui_callback_data_t)printer_Userport_device_submenu }, \
    { "Userport printer form feed",                          \
      MENU_ENTRY_OTHER,                                      \
      uiprinter_formfeed_callback,                           \
      (ui_callback_data_t)3 },

UI_MENU_DEFINE_STRING(PrinterTextDevice1)
UI_MENU_DEFINE_STRING(PrinterTextDevice2)
UI_MENU_DEFINE_STRING(PrinterTextDevice3)

#define VICE_SDL_PRINTER_DEVICEFILE_MENU_ITEMS       \
    { "Device 1",                                    \
      MENU_ENTRY_RESOURCE_STRING,                    \
      string_PrinterTextDevice1_callback,            \
      (ui_callback_data_t)"Printer device 1 file" }, \
    { "Device 2",                                    \
      MENU_ENTRY_RESOURCE_STRING,                    \
      string_PrinterTextDevice2_callback,            \
      (ui_callback_data_t)"Printer device 2 file" }, \
    { "Device 3",                                    \
      MENU_ENTRY_RESOURCE_STRING,                    \
      string_PrinterTextDevice3_callback,            \
      (ui_callback_data_t)"Printer device 3 file" },

const ui_menu_entry_t printer_iec_menu[] = {
    VICE_SDL_PRINTER_COMMON_4_MENU_ITEMS
    { "Printer #4 enable IEC device",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IECDevice4_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_COMMON_5_MENU_ITEMS
    { "Printer #5 enable IEC device",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IECDevice5_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_COMMON_6_MENU_ITEMS
    { "Printer #6 enable IEC device",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IECDevice6_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
#ifdef HAVE_OPENCBM
    VICE_SDL_DEVICE_COMMON_7_MENU_ITEMS
    { "Device #7 enable IEC device",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IECDevice7_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
#endif
    VICE_SDL_PRINTER_USERPORT_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_DEVICEFILE_MENU_ITEMS
    SDL_MENU_LIST_END
};

const ui_menu_entry_t printer_ieee_menu[] = {
    VICE_SDL_PRINTER_COMMON_4_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_COMMON_5_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_COMMON_6_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_USERPORT_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_DEVICEFILE_MENU_ITEMS
    SDL_MENU_LIST_END
};

const ui_menu_entry_t printer_iec_nouserport_menu[] = {
    VICE_SDL_PRINTER_COMMON_4_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_COMMON_5_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
    VICE_SDL_PRINTER_COMMON_6_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
#ifdef HAVE_OPENCBM
    VICE_SDL_DEVICE_COMMON_7_MENU_ITEMS
    SDL_MENU_ITEM_SEPARATOR,
#endif
    VICE_SDL_PRINTER_DEVICEFILE_MENU_ITEMS
    SDL_MENU_LIST_END
};
