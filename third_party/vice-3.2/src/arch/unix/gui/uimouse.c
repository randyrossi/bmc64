/*
 * uimouse.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#ifdef HAVE_MOUSE

#include <stdio.h>

#include "mouse.h"
#include "uimenu.h"
#include "uimouse.h"

UI_MENU_DEFINE_TOGGLE(Mouse)
UI_MENU_DEFINE_TOGGLE(SmartMouseRTCSave)

ui_menu_entry_t mouse_submenu[] = {
    { N_("Enable mouse grab"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Mouse, NULL, NULL,
      KEYSYM_m, UI_HOTMOD_META },
    { N_("Enable SmartMouse RTC saving"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SmartMouseRTCSave, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t mouse_grab_submenu[] = {
    { N_("Enable mouse grab"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Mouse, NULL, NULL,
      KEYSYM_m, UI_HOTMOD_META },
    UI_MENU_ENTRY_LIST_END
};
#endif
