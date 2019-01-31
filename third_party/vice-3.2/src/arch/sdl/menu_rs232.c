/*
 * menu_rs232.c - RS-232 menus for SDL UI.
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

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)

#include <stdio.h>

#include "types.h"

#include "menu_common.h"
#include "menu_rs232.h"
#include "resources.h"
#include "uimenu.h"

#define VICE_SDL_RS232_ARCHDEP_ITEMS /**/

/* *nix extra RS232 settings */
#ifdef UNIX_COMPILE

UI_MENU_DEFINE_RADIO(RsDevice1Baud)
UI_MENU_DEFINE_RADIO(RsDevice2Baud)
UI_MENU_DEFINE_RADIO(RsDevice3Baud)
UI_MENU_DEFINE_RADIO(RsDevice4Baud)

#define RS_BAUD_MENU(x)                                 \
    static const ui_menu_entry_t rs##x##baud_menu[] = { \
        { "300",                                        \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)300 },                    \
        { "1200",                                       \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)1200 },                   \
        { "2400",                                       \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)2400 },                   \
        { "9600",                                       \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)9600 },                   \
        { "19200",                                      \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)19200 },                  \
        { "38400 (Swiftlink/Turbo232 only)",            \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)38400 },                  \
        { "57600 (Turbo232 only)",                      \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)57600 },                  \
        { "115200 (Turbo232 only)",                     \
          MENU_ENTRY_RESOURCE_RADIO,                    \
          radio_RsDevice##x##Baud_callback,             \
          (ui_callback_data_t)115200 },                 \
        SDL_MENU_LIST_END                               \
    };

RS_BAUD_MENU(1)
RS_BAUD_MENU(2)
RS_BAUD_MENU(3)
RS_BAUD_MENU(4)

#undef VICE_SDL_RS232_ARCHDEP_ITEMS
#define VICE_SDL_RS232_ARCHDEP_ITEMS      \
    { "Device 1 baud rate",               \
      MENU_ENTRY_SUBMENU,                 \
      submenu_radio_callback,             \
      (ui_callback_data_t)rs1baud_menu }, \
    { "Device 2 baud rate",               \
      MENU_ENTRY_SUBMENU,                 \
      submenu_radio_callback,             \
      (ui_callback_data_t)rs2baud_menu }, \
    { "Device 3 baud rate",               \
      MENU_ENTRY_SUBMENU,                 \
      submenu_radio_callback,             \
      (ui_callback_data_t)rs3baud_menu }, \
    { "Device 4 baud rate",               \
      MENU_ENTRY_SUBMENU,                 \
      submenu_radio_callback,             \
      (ui_callback_data_t)rs4baud_menu },

#endif /* defined(UNIX_COMPILE) */

/* Common menus */

UI_MENU_DEFINE_STRING(RsDevice1)
UI_MENU_DEFINE_STRING(RsDevice2)
UI_MENU_DEFINE_STRING(RsDevice3)
UI_MENU_DEFINE_STRING(RsDevice4)

UI_MENU_DEFINE_TOGGLE(Acia1Enable)
UI_MENU_DEFINE_RADIO(Acia1Dev)

static const ui_menu_entry_t acia1dev_menu[] = {
    { "1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Dev_callback,
      (ui_callback_data_t)0 },
    { "2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Dev_callback,
      (ui_callback_data_t)1 },
    { "3",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Dev_callback,
      (ui_callback_data_t)2 },
    { "4",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Dev_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(Acia1Irq)

static const ui_menu_entry_t acia1irq_menu[] = {
    { "No IRQ/NMI",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Irq_callback,
      (ui_callback_data_t)0 },
    { "IRQ",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Irq_callback,
      (ui_callback_data_t)1 },
    { "NMI",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Irq_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(Acia1Mode)

static const ui_menu_entry_t acia1mode_menu[] = {
    { "Normal",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Mode_callback,
      (ui_callback_data_t)0 },
    { "Swiftlink",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Mode_callback,
      (ui_callback_data_t)1 },
    { "Turbo232",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Mode_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(Acia1Base)

static const ui_menu_entry_t acia1base_c64_menu[] = {
    { "$DE00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Base_callback,
      (ui_callback_data_t)0xde00 },
    { "$DF00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Base_callback,
      (ui_callback_data_t)0xdf00 },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t acia1base_c128_menu[] = {
    { "$D700",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Base_callback,
      (ui_callback_data_t)0xd700 },
    { "$DE00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Base_callback,
      (ui_callback_data_t)0xde00 },
    { "$DF00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Base_callback,
      (ui_callback_data_t)0xdf00 },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t acia1base_vic20_menu[] = {
    { "$9800",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Base_callback,
      (ui_callback_data_t)0x9800 },
    { "$9C00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_Acia1Base_callback,
      (ui_callback_data_t)0x9c00 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(RsUserEnable)
UI_MENU_DEFINE_RADIO(RsUserBaud)

static const ui_menu_entry_t rsuserbaud_menu[] = {
    { "300",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserBaud_callback,
      (ui_callback_data_t)300 },
    { "1200",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserBaud_callback,
      (ui_callback_data_t)1200 },
    { "2400",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserBaud_callback,
      (ui_callback_data_t)2400 },
    { "9600",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserBaud_callback,
      (ui_callback_data_t)9600 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(RsUserDev)

static const ui_menu_entry_t rsuserdev_menu[] = {
    { "1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserDev_callback,
      (ui_callback_data_t)0 },
    { "2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserDev_callback,
      (ui_callback_data_t)1 },
    { "3",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserDev_callback,
      (ui_callback_data_t)2 },
    { "4",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RsUserDev_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t rs232_nouser_menu[] = {
    { "ACIA host device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1dev_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Host settings"),
    { "Device 1",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice1_callback,
      (ui_callback_data_t)"RS232 host device 1" },
    { "Device 2",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice2_callback,
      (ui_callback_data_t)"RS232 host device 2" },
    { "Device 3",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice3_callback,
      (ui_callback_data_t)"RS232 host device 3" },
    { "Device 4",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice4_callback,
      (ui_callback_data_t)"RS232 host device 4" },
    VICE_SDL_RS232_ARCHDEP_ITEMS
    SDL_MENU_LIST_END
};

const ui_menu_entry_t rs232_c64_menu[] = {
    { "ACIA RS232 interface emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Acia1Enable_callback,
      NULL },
    { "ACIA base address",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1base_c64_menu },
    { "ACIA host device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1dev_menu },
    { "ACIA interrupt",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1irq_menu },
    { "ACIA emulation mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1mode_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport RS232 emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RsUserEnable_callback,
      NULL },
    { "Userport RS232 host device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)rsuserdev_menu },
    { "Userport RS232 baud rate",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)rsuserbaud_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Host settings"),
    { "Device 1",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice1_callback,
      (ui_callback_data_t)"RS232 host device 1" },
    { "Device 2",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice2_callback,
      (ui_callback_data_t)"RS232 host device 2" },
    { "Device 3",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice3_callback,
      (ui_callback_data_t)"RS232 host device 3" },
    { "Device 4",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice4_callback,
      (ui_callback_data_t)"RS232 host device 4" },
    VICE_SDL_RS232_ARCHDEP_ITEMS
    SDL_MENU_LIST_END
};

const ui_menu_entry_t rs232_c128_menu[] = {
    { "ACIA RS232 interface emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Acia1Enable_callback,
      NULL },
    { "ACIA base address",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1base_c128_menu },
    { "ACIA host device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1dev_menu },
    { "ACIA interrupt",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1irq_menu },
    { "ACIA emulation mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1mode_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport RS232 emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RsUserEnable_callback,
      NULL },
    { "Userport RS232 host device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)rsuserdev_menu },
    { "Userport RS232 baud rate",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)rsuserbaud_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Host settings"),
    { "Device 1",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice1_callback,
      (ui_callback_data_t)"RS232 host device 1" },
    { "Device 2",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice2_callback,
      (ui_callback_data_t)"RS232 host device 2" },
    { "Device 3",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice3_callback,
      (ui_callback_data_t)"RS232 host device 3" },
    { "Device 4",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice4_callback,
      (ui_callback_data_t)"RS232 host device 4" },
    VICE_SDL_RS232_ARCHDEP_ITEMS
    SDL_MENU_LIST_END
};

const ui_menu_entry_t rs232_vic20_menu[] = {
    { "ACIA RS232 interface emulation (MasC=uerade)",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Acia1Enable_callback,
      NULL },
    { "ACIA base address",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1base_vic20_menu },
    { "ACIA host device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1dev_menu },
    { "ACIA interrupt",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1irq_menu },
    { "ACIA emulation mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)acia1mode_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport RS232 emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RsUserEnable_callback,
      NULL },
    { "Userport RS232 host device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)rsuserdev_menu },
    { "Userport RS232 baud rate",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)rsuserbaud_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Host settings"),
    { "Device 1",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice1_callback,
      (ui_callback_data_t)"RS232 host device 1" },
    { "Device 2",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice2_callback,
      (ui_callback_data_t)"RS232 host device 2" },
    { "Device 3",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice3_callback,
      (ui_callback_data_t)"RS232 host device 3" },
    { "Device 4",
      MENU_ENTRY_RESOURCE_STRING,
      string_RsDevice4_callback,
      (ui_callback_data_t)"RS232 host device 4" },
    VICE_SDL_RS232_ARCHDEP_ITEMS
    SDL_MENU_LIST_END
};
#endif
