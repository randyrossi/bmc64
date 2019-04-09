/*
 * menu_plus4hw.c - PLUS4 HW menu for SDL UI.
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

#include "menu_common.h"
#include "menu_joyport.h"
#include "menu_joystick.h"
#include "menu_ram.h"
#include "menu_rom.h"
#include "plus4memhacks.h"
#include "plus4model.h"

#ifdef HAVE_MOUSE
#include "menu_mouse.h"
#endif

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
#include "menu_rs232.h"
#endif

#include "menu_sid.h"
#include "menu_tape.h"
#include "uimenu.h"

/* PLUS4 MODEL SELECTION */

static UI_MENU_CALLBACK(custom_PLUS4Model_callback)
{
    int model, selected;

    selected = vice_ptr_to_int(param);

    if (activated) {
        plus4model_set(selected);
    } else {
        model = plus4model_get();

        if (selected == model) {
            return sdl_menu_text_tick;
        }
    }

    return NULL;
}

static const ui_menu_entry_t plus4_model_submenu[] = {
    { "C16 PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_PLUS4Model_callback,
      (ui_callback_data_t)PLUS4MODEL_C16_PAL },
    { "C16 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_PLUS4Model_callback,
      (ui_callback_data_t)PLUS4MODEL_C16_NTSC },
    { "Plus4 PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_PLUS4Model_callback,
      (ui_callback_data_t)PLUS4MODEL_PLUS4_PAL },
    { "Plus4 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_PLUS4Model_callback,
      (ui_callback_data_t)PLUS4MODEL_PLUS4_NTSC },
    { "V364 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_PLUS4Model_callback,
      (ui_callback_data_t)PLUS4MODEL_V364_NTSC },
    { "C232 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_PLUS4Model_callback,
      (ui_callback_data_t)PLUS4MODEL_232_NTSC },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(SpeechEnabled)
UI_MENU_DEFINE_FILE_STRING(SpeechImage)

static const ui_menu_entry_t v364speech_menu[] = {
    { "Enable V364 Speech",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SpeechEnabled_callback,
      NULL },
    { "ROM image file",
      MENU_ENTRY_DIALOG,
      file_string_SpeechImage_callback,
      (ui_callback_data_t)"Select Speech ROM image" },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(UserportDAC)

static const ui_menu_entry_t userport_menu[] = {
    SDL_MENU_ITEM_TITLE("Userport devices"),
    { "8 bit DAC enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportDAC_callback,
      NULL },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(MemoryHack)
UI_MENU_DEFINE_RADIO(RamSize)
UI_MENU_DEFINE_TOGGLE(Acia1Enable)

const ui_menu_entry_t plus4_hardware_menu[] = {
    { "Select Plus4 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)plus4_model_submenu },
    { "Joyport settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joyport_menu },
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_plus4_menu },
#ifdef HAVE_MOUSE
    { "Mouse emulation",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mouse_menu },
#endif
    { "SID cart settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_plus4_menu },
    { "V364 Speech settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)v364speech_menu },
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)plus4_rom_menu },
    { "ACIA installed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Acia1Enable_callback,
      NULL },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "RS232 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)rs232_nouser_menu },
#endif
    { "Userport devices",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)userport_menu },
    { "Tape port devices",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)tapeport_devices_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Memory"),
    { "16kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)16 },
    { "32kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)32 },
    { "64kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)64 },
    SDL_MENU_ITEM_TITLE("Memory expansion hack"),
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_NONE},
    { "256kB (CSORY)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_C256K},
    { "256kB (HANNES)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_H256K },
    { "1024kB (HANNES)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_H1024K },
    { "4096kB (HANNES)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MemoryHack_callback,
      (ui_callback_data_t)MEMORY_HACK_H4096K },
    SDL_MENU_LIST_END
};
