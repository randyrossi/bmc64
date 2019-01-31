/*
 * uips2mouse.c
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

#ifdef HAVE_MOUSE

#include <stdio.h>

#include "uimenu.h"
#include "uips2mouse.h"

UI_MENU_DEFINE_TOGGLE(Mouse)
UI_MENU_DEFINE_TOGGLE(ps2mouse)

ui_menu_entry_t ps2_mouse_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_ps2mouse, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Grab mouse events"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Mouse, NULL, NULL,
      KEYSYM_m, UI_HOTMOD_META },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ps2_mouse_menu[] = {
    { N_("PS/2 mouse on Userport"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ps2_mouse_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif
