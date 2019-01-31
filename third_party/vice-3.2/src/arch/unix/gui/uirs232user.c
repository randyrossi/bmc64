/*
 * uirs232user.c
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

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)

#include <stdio.h>

#include "uimenu.h"
#include "uirs232user.h"

UI_MENU_DEFINE_RADIO(RsUserDev)

ui_menu_entry_t rs232user_device_submenu[] = {
    { N_("Serial 1"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserDev, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Serial 2"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserDev, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Dump to file"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserDev, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Exec process"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserDev, (ui_callback_data_t)3, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(RsUserBaud)

ui_menu_entry_t rs232user_baudrate_submenu[] = {
    { "300", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserBaud, (ui_callback_data_t)300, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "600", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserBaud, (ui_callback_data_t)600, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1200", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserBaud, (ui_callback_data_t)1200, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "2400", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserBaud, (ui_callback_data_t)2400, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "4800", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserBaud, (ui_callback_data_t)4800, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "9600", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RsUserBaud, (ui_callback_data_t)9600, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif
