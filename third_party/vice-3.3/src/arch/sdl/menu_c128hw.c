/*
 * menu_c128hw.c - C128 HW menu for SDL UI.
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

#include "c128model.h"
#include "cartridge.h"
#include "cia.h"
#include "menu_c64_common_expansions.h"
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
#include "menu_tape.h"

#ifdef HAVE_RAWNET
#include "menu_ethernet.h"
#include "menu_ethernetcart.h"
#endif

#include "uimenu.h"

#define CIA_MODEL_MENU(xyz)                                     \
    UI_MENU_DEFINE_RADIO(CIA##xyz##Model)                       \
    static const ui_menu_entry_t cia##xyz##_model_submenu[] = { \
        { "6526  (old)",                                        \
          MENU_ENTRY_RESOURCE_TOGGLE,                           \
          radio_CIA##xyz##Model_callback,                       \
          (ui_callback_data_t)CIA_MODEL_6526 },                 \
        { "8521 (new)",                                        \
          MENU_ENTRY_RESOURCE_TOGGLE,                           \
          radio_CIA##xyz##Model_callback,                       \
          (ui_callback_data_t)CIA_MODEL_6526A },                \
        SDL_MENU_LIST_END                                       \
    };

CIA_MODEL_MENU(1)
CIA_MODEL_MENU(2)

/* C128 MODEL SELECTION */

static UI_MENU_CALLBACK(select_c128_model_callback)
{
    int model, selected;

    selected = vice_ptr_to_int(param);

    if (activated) {
        c128model_set(selected);
    } else {
        model = c128model_get();

        if (selected == model) {
            return sdl_menu_text_tick;
        }
    }

    return NULL;
}

static const ui_menu_entry_t c128_model_menu[] = {
    { "C128 (PAL)", MENU_ENTRY_OTHER, select_c128_model_callback, (ui_callback_data_t)C128MODEL_C128_PAL },
    { "C128 DCR (PAL)", MENU_ENTRY_OTHER, select_c128_model_callback, (ui_callback_data_t)C128MODEL_C128DCR_PAL },
    { "C128 (NTSC)", MENU_ENTRY_OTHER, select_c128_model_callback, (ui_callback_data_t)C128MODEL_C128_NTSC },
    { "C128 DCR (NTSC)", MENU_ENTRY_OTHER, select_c128_model_callback, (ui_callback_data_t)C128MODEL_C128DCR_NTSC },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(VDC64KB)
UI_MENU_DEFINE_RADIO(VDCRevision)

static const ui_menu_entry_t vdc_menu[] = {
    SDL_MENU_ITEM_TITLE("VDC revision"),
    { "Rev 0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDCRevision_callback,
      (ui_callback_data_t)0 },
    { "Rev 1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDCRevision_callback,
      (ui_callback_data_t)1 },
    { "Rev 2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDCRevision_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("VDC memory size"),
    { "16kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDC64KB_callback,
      (ui_callback_data_t)0 },
    { "64kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDC64KB_callback,
      (ui_callback_data_t)1 },
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

UI_MENU_DEFINE_TOGGLE(IEEE488)
UI_MENU_DEFINE_TOGGLE(C128FullBanks)
UI_MENU_DEFINE_TOGGLE(Go64Mode)

const ui_menu_entry_t c128_hardware_menu[] = {
    { "Select C128 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)c128_model_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Joyport settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joyport_menu },
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_c64_menu },
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_c128_menu },
    SDL_MENU_ITEM_TITLE("CIA models"),
    { "CIA 1 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia1_model_submenu },
    { "CIA 2 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia2_model_submenu },
    { "VDC settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)vdc_menu },
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
    { "RAM banks 2 and 3",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_C128FullBanks_callback,
      NULL },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)c128_rom_menu },
    { "Switch to C64 mode on reset",
        MENU_ENTRY_RESOURCE_TOGGLE,
        toggle_Go64Mode_callback,
        NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Hardware expansions"),
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "RS232 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)rs232_c128_menu },
#endif
    { CARTRIDGE_NAME_DIGIMAX " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)digimax_menu },
    { CARTRIDGE_NAME_DS12C887RTC " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ds12c887rtc_c128_menu },
    { "IEEE488 interface",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IEEE488_callback,
      NULL },
    { "Userport devices",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)userport_menu },
    { "Tape port devices",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)tapeport_devices_menu },
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
    SDL_MENU_LIST_END
};
