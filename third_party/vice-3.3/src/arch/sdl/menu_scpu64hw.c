/*
 * menu_scpu64hw.c - SCPU64 HW menu for SDL UI.
 *
 * Written by
 *  Marco van den Heuevel <blackystardust68@yahoo.com>
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
#include "menu_c64_common_expansions.h"
#include "menu_c64_expansions.h"
#include "menu_c64model.h"
#include "menu_common.h"
#include "menu_joyport.h"
#include "menu_joystick.h"

#ifdef HAVE_MIDI
#include "menu_midi.h"
#endif

#ifdef HAVE_MOUSE
#include "menu_mouse.h"
#endif

#include "menu_ram.h"
#include "menu_rom.h"

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
#include "menu_rs232.h"
#endif

#include "menu_sid.h"

#ifdef HAVE_RAWNET
#include "menu_ethernet.h"
#include "menu_ethernetcart.h"
#endif

#include "uimenu.h"

UI_MENU_DEFINE_RADIO(BurstMod)

const ui_menu_entry_t scpu64_burstmod_menu[] = {
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_BurstMod_callback,
      (ui_callback_data_t)0 },
    { "CIA1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_BurstMod_callback,
      (ui_callback_data_t)1 },
    { "CIA2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_BurstMod_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(SIMMSize)

const ui_menu_entry_t scpu64_simmsize_menu[] = {
    { "0 MB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SIMMSize_callback,
      (ui_callback_data_t)0 },
    { "1 MB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SIMMSize_callback,
      (ui_callback_data_t)1 },
    { "4 MB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SIMMSize_callback,
      (ui_callback_data_t)4 },
    { "8 MB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SIMMSize_callback,
      (ui_callback_data_t)8 },
    { "16 MB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SIMMSize_callback,
      (ui_callback_data_t)16 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(UserportDAC)
UI_MENU_DEFINE_TOGGLE(UserportDIGIMAX)
UI_MENU_DEFINE_TOGGLE(UserportRTCDS1307)
UI_MENU_DEFINE_TOGGLE(UserportRTCDS1307Save)
UI_MENU_DEFINE_TOGGLE(UserportRTC58321a)
UI_MENU_DEFINE_TOGGLE(UserportRTC58321aSave)
UI_MENU_DEFINE_TOGGLE(Userport4bitSampler)
UI_MENU_DEFINE_TOGGLE(Userport8BSS)

static const ui_menu_entry_t userport_menu[] = {
    SDL_MENU_ITEM_TITLE("Userport devices"),
    { "8 bit DAC enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportDAC_callback,
      NULL },
    { "DigiMAX enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportDIGIMAX_callback,
      NULL },
    { "RTC (58321a) enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportRTC58321a_callback,
      NULL },
    { "Save RTC (58321a) data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportRTC58321aSave_callback,
      NULL },
    { "4 bit sampler enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Userport4bitSampler_callback,
      NULL },
    { "8 bit stereo sampler enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Userport8BSS_callback,
      NULL },
    { "RTC (DS1307) enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportRTCDS1307_callback,
      NULL },
    { "Save RTC (DS1307) data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportRTCDS1307Save_callback,
      NULL },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(JiffySwitch)
UI_MENU_DEFINE_TOGGLE(SpeedSwitch)

const ui_menu_entry_t scpu64_hardware_menu[] = {
    { "Model settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)scpu64_model_menu },
    { "SIMM size",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)scpu64_simmsize_menu },
    { "Jiffy switch enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_JiffySwitch_callback,
      NULL },
    { "Speed switch enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SpeedSwitch_callback,
      NULL },
    { "Joyport settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joyport_menu },
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_c64_menu },
#ifdef HAVE_MOUSE
    { "Mouse emulation",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mouse_menu },
#endif
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)scpu64_rom_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Hardware expansions"),
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "RS232 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)rs232_c64_menu },
#endif
    { CARTRIDGE_NAME_DIGIMAX " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)digimax_menu },
    { CARTRIDGE_NAME_DS12C887RTC " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ds12c887rtc_c64_menu },
#ifdef HAVE_MIDI
    { "MIDI settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)midi_c64_menu },
#endif
#ifdef HAVE_RAWNET
    { "Ethernet settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ethernet_menu },
    { "Ethernet Cart settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ethernetcart_menu },
#endif
    { "Burst Mode Modification",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)scpu64_burstmod_menu },
    { "Userport devices",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)userport_menu },
    SDL_MENU_LIST_END
};
