/*
 * menu_cbm2hw.c - CBM2 HW menu for SDL UI.
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

#include "cbm2mem.h"
#include "cbm2model.h"
#include "cia.h"
#include "menu_cbm2hw.h"
#include "menu_common.h"
#include "menu_joyport.h"
#include "menu_joystick.h"
#include "menu_ram.h"
#include "menu_rom.h"

#ifdef HAVE_MOUSE
#include "menu_mouse.h"
#endif

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
#include "menu_rs232.h"
#endif

#include "menu_sid.h"
#include "menu_tape.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(RamSize)
UI_MENU_DEFINE_TOGGLE(Ram08)
UI_MENU_DEFINE_TOGGLE(Ram1)
UI_MENU_DEFINE_TOGGLE(Ram2)
UI_MENU_DEFINE_TOGGLE(Ram4)
UI_MENU_DEFINE_TOGGLE(Ram6)
UI_MENU_DEFINE_TOGGLE(RamC)

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

/* CBM2 MODEL SELECTION */

static UI_MENU_CALLBACK(select_cbm2_model_callback)
{
    int model, selected;

    selected = vice_ptr_to_int(param);

    if (activated) {
        cbm2model_set(selected);
    } else {
        model = cbm2model_get();

        if (selected == model) {
            return sdl_menu_text_tick;
        }
    }

    return NULL;
}

static const ui_menu_entry_t cbm2_model_menu[] = {
    { "CBM 610 (PAL)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_610_PAL },
    { "CBM 610 (NTSC)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_610_NTSC },
    { "CBM 620 (PAL)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_620_PAL },
    { "CBM 620 (NTSC)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_620_NTSC },
    { "CBM 620+ (PAL)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_620PLUS_PAL },
    { "CBM 620+ (NTSC)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_620PLUS_NTSC },
    { "CBM 710 (NTSC)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_710_NTSC },
    { "CBM 720 (NTSC)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_720_NTSC },
    { "CBM 720+ (NTSC)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_720PLUS_NTSC },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t cbm5x0_model_menu[] = {
    { "CBM 510 (PAL)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_510_PAL },
    { "CBM 510 (NTSC)", MENU_ENTRY_OTHER, select_cbm2_model_callback, (ui_callback_data_t)CBM2MODEL_510_NTSC },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t cbm2_memory_menu[] = {
    SDL_MENU_ITEM_TITLE("CBM2 memory size"),
    { "128kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)128 },
    { "256kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)256 },
    { "512kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)512 },
    { "1024kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)1024 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("CBM2 memory blocks"),
    { "RAM at $0800-$0FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram08_callback,
      NULL },
    { "RAM at $1000-$1FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram1_callback,
      NULL },
    { "RAM at $2000-$3FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram2_callback,
      NULL },
    { "RAM at $4000-$5FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram4_callback,
      NULL },
    { "RAM at $6000-$7FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram6_callback,
      NULL },
    { "RAM at $C000-$CFFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RamC_callback,
      NULL },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t cbm5x0_memory_menu[] = {
    SDL_MENU_ITEM_TITLE("CBM2 memory size"),
    { "64kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)64 },
    { "128kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)128 },
    { "256kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)256 },
    { "512kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)512 },
    { "1024kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)1024 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("CBM2 memory blocks"),
    { "RAM at $0800-$0FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram08_callback,
      NULL },
    { "RAM at $1000-$1FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram1_callback,
      NULL },
    { "RAM at $2000-$3FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram2_callback,
      NULL },
    { "RAM at $4000-$5FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram4_callback,
      NULL },
    { "RAM at $6000-$7FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram6_callback,
      NULL },
    { "RAM at $C000-$CFFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RamC_callback,
      NULL },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t cbm5x0_hardware_menu[] = {
    { "Select CBM2 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cbm5x0_model_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Tape port devices",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)tapeport_devices_menu },
    { "Joyport settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joyport_menu },
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_menu },
#ifdef HAVE_MOUSE
    { "Mouse emulation",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mouse_menu },
#endif
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_cbm2_menu },
    { "CIA model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia1_model_submenu },
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_rom_menu },
    { "CBM2 memory setting",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm5x0_memory_menu },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "RS232 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)rs232_nouser_menu },
#endif
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
    { "RTC (DS1307) enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportRTCDS1307_callback,
      NULL },
    { "Save RTC (DS1307) data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportRTCDS1307Save_callback,
      NULL },
    { "4 bit sampler enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Userport4bitSampler_callback,
      NULL },
    { "8 bit stereo sampler enable",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Userport8BSS_callback,
      NULL },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t cbm6x0_7x0_hardware_menu[] = {
    { "Select CBM2 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cbm2_model_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Joyport settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joyport_menu },
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_userport_only_menu },
#ifdef HAVE_MOUSE
    { "Mouse emulation",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mouse_grab_menu },
#endif
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_cbm2_menu },
    { "CIA model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia1_model_submenu },
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_rom_menu },
    { "CBM2 memory setting",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_memory_menu },
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
    SDL_MENU_LIST_END
};
