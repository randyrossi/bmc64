/*
 * uiacia1.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
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

#include "uiacia1.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(Acia1Dev)

ui_menu_entry_t uiacia1_device_submenu[] = {
    { N_("Serial 1"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_Acia1Dev, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Serial 2"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_Acia1Dev, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Dump to file"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_Acia1Dev, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Exec process"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_Acia1Dev, (ui_callback_data_t)3, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
