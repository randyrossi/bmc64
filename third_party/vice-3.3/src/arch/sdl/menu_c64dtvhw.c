/*
 * menu_c64dtvhw.c - C64DTV HW menu for SDL UI.
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
#include "types.h"

#include "c64dtv-resources.h"
#include "c64dtvmodel.h"
#include "menu_common.h"
#include "menu_joyport.h"
#include "menu_joystick.h"
#include "menu_ram.h"
#include "menu_rom.h"
#include "menu_sid.h"
#include "uimenu.h"

/* DTV MODEL SELECTION */

static UI_MENU_CALLBACK(custom_DTVModel_callback)
{
    int model, selected;

    selected = vice_ptr_to_int(param);

    if (activated) {
        dtvmodel_set(selected);
    } else {
        model = dtvmodel_get();

        if (selected == model) {
            return sdl_menu_text_tick;
        }
    }

    return NULL;
}

UI_MENU_DEFINE_TOGGLE(VICIINewLuminances)

static const ui_menu_entry_t dtv_model_submenu[] = {
    { "DTV2 PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_DTVModel_callback,
      (ui_callback_data_t)DTVMODEL_V2_PAL },
    { "DTV2 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_DTVModel_callback,
      (ui_callback_data_t)DTVMODEL_V2_NTSC },
    { "DTV3 PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_DTVModel_callback,
      (ui_callback_data_t)DTVMODEL_V3_PAL },
    { "DTV3 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_DTVModel_callback,
      (ui_callback_data_t)DTVMODEL_V3_NTSC },
    { "Hummer NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_DTVModel_callback,
      (ui_callback_data_t)DTVMODEL_HUMMER_NTSC },
    SDL_MENU_ITEM_SEPARATOR,
    { "Colorfix",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIINewLuminances_callback,
      NULL },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(HummerADC)

#ifdef HAVE_MOUSE
UI_MENU_DEFINE_TOGGLE(ps2mouse)
UI_MENU_DEFINE_TOGGLE(Mouse)
#endif

UI_MENU_DEFINE_FILE_STRING(c64dtvromfilename)
UI_MENU_DEFINE_TOGGLE(c64dtvromrw)
UI_MENU_DEFINE_TOGGLE(FlashTrueFS)
UI_MENU_DEFINE_RADIO(DtvRevision)

const ui_menu_entry_t c64dtv_hardware_menu[] = {
    { "Select DTV model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)dtv_model_submenu },
    { "Joyport settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joyport_menu },
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_c64dtv_menu },
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_dtv_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("C64DTV ROM image"),
    { "File",
      MENU_ENTRY_DIALOG,
      file_string_c64dtvromfilename_callback,
      (ui_callback_data_t)"Select C64DTV ROM image file" },
    { "Enable writes",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_c64dtvromrw_callback,
      NULL },
    { "True flash filesystem",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_FlashTrueFS_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("DTV revision"),
    { "DTV2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DtvRevision_callback,
      (ui_callback_data_t)2 },
    { "DTV3",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DtvRevision_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Enable Hummer ADC",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_HummerADC_callback,
      NULL },
#ifdef HAVE_MOUSE
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("PS/2 mouse on userport"),
    { "Enable PS/2 mouse",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_ps2mouse_callback,
      NULL },
    { "Grab mouse events",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Mouse_callback,
      NULL },
#endif
    SDL_MENU_ITEM_SEPARATOR,
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    { "Fallback ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)c64dtv_rom_menu },
    SDL_MENU_LIST_END
};
