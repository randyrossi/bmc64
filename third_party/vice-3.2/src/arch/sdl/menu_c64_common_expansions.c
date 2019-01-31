/*
 * menu_c64_common_expansions.c - C64/C128 expansions menu for SDL UI.
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

#include <stdio.h>

#include "types.h"

#include "cartridge.h"
#include "clockport.h"
#include "menu_c64_common_expansions.h"
#include "menu_common.h"
#include "uimenu.h"


/* DIGIMAX MENU */

UI_MENU_DEFINE_TOGGLE(DIGIMAX)
UI_MENU_DEFINE_RADIO(DIGIMAXbase)

const ui_menu_entry_t digimax_menu[] = {
    { "Enable " CARTRIDGE_NAME_DIGIMAX,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DIGIMAX_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Base address"),
    { "$DE00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xde00 },
    { "$DE20",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xde20 },
    { "$DE40",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xde40 },
    { "$DE60",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xde60 },
    { "$DE80",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xde80 },
    { "$DEA0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdea0 },
    { "$DEC0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdec0 },
    { "$DEE0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdee0 },
    { "$DF00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdf00 },
    { "$DF20",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdf20 },
    { "$DF40",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdf40 },
    { "$DF60",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdf60 },
    { "$DF80",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdf80 },
    { "$DFA0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdfa0 },
    { "$DFC0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdfc0 },
    { "$DFE0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0xdfe0 },
    SDL_MENU_LIST_END
};


/* DS12C887 RTC MENU */

UI_MENU_DEFINE_TOGGLE(DS12C887RTC)
UI_MENU_DEFINE_TOGGLE(DS12C887RTCRunMode)
UI_MENU_DEFINE_RADIO(DS12C887RTCbase)
UI_MENU_DEFINE_TOGGLE(DS12C887RTCSave)

const ui_menu_entry_t ds12c887rtc_c64_menu[] = {
    { "Enable " CARTRIDGE_NAME_DS12C887RTC,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTC_callback,
      NULL },
    { "Start with running oscillator",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTCRunMode_callback,
      NULL },
    { "Save RTC data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTCSave_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Base address"),
    { "$D500",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xd500 },
    { "$D600",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xd600 },
    { "$D700",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xd700 },
    { "$DE00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xde00 },
    { "$DF00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xdf00 },
    SDL_MENU_LIST_END
};


const ui_menu_entry_t ds12c887rtc_c128_menu[] = {
    { "Enable " CARTRIDGE_NAME_DS12C887RTC,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTC_callback,
      NULL },
    { "Start with running oscillator",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTCRunMode_callback,
      NULL },
    { "Save RTC data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTCSave_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Base address"),
    { "$D700",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xd700 },
    { "$DE00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xde00 },
    { "$DF00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0xdf00 },
    SDL_MENU_LIST_END
};


/* IDE64 CART MENU */

UI_MENU_DEFINE_RADIO(IDE64ClockPort)

static ui_menu_entry_t ide64_clockport_device_menu[CLOCKPORT_MAX_ENTRIES + 1];

UI_MENU_DEFINE_TOGGLE(IDE64RTCSave)
UI_MENU_DEFINE_RADIO(IDE64version)
UI_MENU_DEFINE_TOGGLE(IDE64USBServer)
UI_MENU_DEFINE_STRING(IDE64USBServerAddress)
UI_MENU_DEFINE_FILE_STRING(IDE64Image1)
UI_MENU_DEFINE_FILE_STRING(IDE64Image2)
UI_MENU_DEFINE_FILE_STRING(IDE64Image3)
UI_MENU_DEFINE_FILE_STRING(IDE64Image4)
UI_MENU_DEFINE_TOGGLE(IDE64AutodetectSize1)
UI_MENU_DEFINE_TOGGLE(IDE64AutodetectSize2)
UI_MENU_DEFINE_TOGGLE(IDE64AutodetectSize3)
UI_MENU_DEFINE_TOGGLE(IDE64AutodetectSize4)
UI_MENU_DEFINE_INT(IDE64Cylinders1)
UI_MENU_DEFINE_INT(IDE64Cylinders2)
UI_MENU_DEFINE_INT(IDE64Cylinders3)
UI_MENU_DEFINE_INT(IDE64Cylinders4)
UI_MENU_DEFINE_INT(IDE64Heads1)
UI_MENU_DEFINE_INT(IDE64Heads2)
UI_MENU_DEFINE_INT(IDE64Heads3)
UI_MENU_DEFINE_INT(IDE64Heads4)
UI_MENU_DEFINE_INT(IDE64Sectors1)
UI_MENU_DEFINE_INT(IDE64Sectors2)
UI_MENU_DEFINE_INT(IDE64Sectors3)
UI_MENU_DEFINE_INT(IDE64Sectors4)

static const ui_menu_entry_t ide64_menu_HD_1[] = {
    SDL_MENU_ITEM_TITLE("ATA device 1 settings"),
    { "Device 1 image file",
      MENU_ENTRY_DIALOG,
      file_string_IDE64Image1_callback,
      (ui_callback_data_t)"Select Device 1 image" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Device 1 geometry"),
    { "Autodetect geometry",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IDE64AutodetectSize1_callback,
      NULL },
    { "Cylinders",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Cylinders1_callback,
      (ui_callback_data_t)"Enter amount of cylinders (1-65535)" },
    { "Heads",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Heads1_callback,
      (ui_callback_data_t)"Enter amount of heads (1-16)" },
    { "Sectors",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Sectors1_callback,
      (ui_callback_data_t)"Enter amount of sectors (0-63)" },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t ide64_menu_HD_2[] = {
    SDL_MENU_ITEM_TITLE("ATA device 2 settings"),
    { "Device 2 image file",
      MENU_ENTRY_DIALOG,
      file_string_IDE64Image2_callback,
      (ui_callback_data_t)"Select Device 2 image" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Device 2 geometry"),
    { "Autodetect geometry",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IDE64AutodetectSize2_callback,
      NULL },
    { "Cylinders",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Cylinders2_callback,
      (ui_callback_data_t)"Enter amount of cylinders (1-1024)" },
    { "Heads",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Heads2_callback,
      (ui_callback_data_t)"Enter amount of heads (1-16)" },
    { "Sectors",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Sectors2_callback,
      (ui_callback_data_t)"Enter amount of sectors (0-63)" },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t ide64_menu_HD_3[] = {
    SDL_MENU_ITEM_TITLE("ATA device 3 settings"),
    { "Device 3 image file",
      MENU_ENTRY_DIALOG,
      file_string_IDE64Image3_callback,
      (ui_callback_data_t)"Select Device 3 image" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Device 3 geometry"),
    { "Autodetect geometry",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IDE64AutodetectSize3_callback,
      NULL },
    { "Cylinders",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Cylinders3_callback,
      (ui_callback_data_t)"Enter amount of cylinders (1-1024)" },
    { "Heads",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Heads3_callback,
      (ui_callback_data_t)"Enter amount of heads (1-16)" },
    { "Sectors",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Sectors3_callback,
      (ui_callback_data_t)"Enter amount of sectors (0-63)" },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t ide64_menu_HD_4[] = {
    SDL_MENU_ITEM_TITLE("ATA device 4 settings"),
    { "Device 4 image file",
      MENU_ENTRY_DIALOG,
      file_string_IDE64Image4_callback,
      (ui_callback_data_t)"Select Device 4 image" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Device 4 geometry"),
    { "Autodetect geometry",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IDE64AutodetectSize4_callback,
      NULL },
    { "Cylinders",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Cylinders4_callback,
      (ui_callback_data_t)"Enter amount of cylinders (1-1024)" },
    { "Heads",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Heads4_callback,
      (ui_callback_data_t)"Enter amount of heads (1-16)" },
    { "Sectors",
      MENU_ENTRY_RESOURCE_INT,
      int_IDE64Sectors4_callback,
      (ui_callback_data_t)"Enter amount of sectors (0-63)" },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(SBDIGIMAX)
UI_MENU_DEFINE_RADIO(SBDIGIMAXbase)

static const ui_menu_entry_t ide64_digimax_menu[] = {
    SDL_MENU_ITEM_TITLE("DigiMAX settings"),
    { "Enable DigiMAX device",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SBDIGIMAX_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("DigiMAX device address"),
    { "$de40",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SBDIGIMAXbase_callback,
      (ui_callback_data_t)0xde40 },
    { "$de48",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SBDIGIMAXbase_callback,
      (ui_callback_data_t)0xde48 },
    SDL_MENU_LIST_END
};

#ifdef HAVE_RAWNET
UI_MENU_DEFINE_TOGGLE(SBETFE)
UI_MENU_DEFINE_RADIO(SBETFEbase)

static const ui_menu_entry_t ide64_etfe_menu[] = {
    SDL_MENU_ITEM_TITLE("ETFE settings"),
    { "Enable ETFE device",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SBETFE_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("ETFE device address"),
    { "$de00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SBETFEbase_callback,
      (ui_callback_data_t)0xde00 },
    { "$de10",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SBETFEbase_callback,
      (ui_callback_data_t)0xde10 },
    { "$df00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SBETFEbase_callback,
      (ui_callback_data_t)0xdf00 },
    SDL_MENU_LIST_END
};
#endif

const ui_menu_entry_t ide64_menu[] = {
    SDL_MENU_ITEM_TITLE("Cartridge version"),
    { "V3",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IDE64version_callback,
      (ui_callback_data_t)0 },
    { "V4.1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IDE64version_callback,
      (ui_callback_data_t)1 },
    { "V4.2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IDE64version_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Save RTC data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IDE64RTCSave_callback,
      NULL },
#ifdef HAVE_NETWORK
    { "Enable USB server",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IDE64USBServer_callback,
      NULL },
    { "USB server address",
      MENU_ENTRY_RESOURCE_STRING,
      string_IDE64USBServerAddress_callback,
      (ui_callback_data_t)"Set USB server address" },
#endif
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("ATA Device settings"),
    { "ATA Device 1 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu_HD_1 },
    { "ATA Device 2 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu_HD_2 },
    { "ATA Device 3 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu_HD_3 },
    { "ATA Device 4 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu_HD_4 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Clockport device",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_clockport_device_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Shortbus Device settings"),
    { "DigiMAX settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_digimax_menu },
#ifdef HAVE_RAWNET
    { "ETFE settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_etfe_menu },
#endif
    SDL_MENU_LIST_END
};

void uiclockport_ide64_menu_create(void)
{
    int i;

    for (i = 0; clockport_supported_devices[i].name; ++i) {
        ide64_clockport_device_menu[i].string = clockport_supported_devices[i].name;
        ide64_clockport_device_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        ide64_clockport_device_menu[i].callback = radio_IDE64ClockPort_callback;
        ide64_clockport_device_menu[i].data = (ui_callback_data_t)int_to_void_ptr(clockport_supported_devices[i].id);
    }

    ide64_clockport_device_menu[i].string = NULL;
    ide64_clockport_device_menu[i].type = MENU_ENTRY_TEXT;
    ide64_clockport_device_menu[i].callback = NULL;
    ide64_clockport_device_menu[i].data = NULL;
}
