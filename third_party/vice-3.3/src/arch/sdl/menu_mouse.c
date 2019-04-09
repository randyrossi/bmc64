/*
 * menu_mouse.c - Mouse menu for SDL UI.
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

#include <stdio.h>

#include "types.h"

#ifdef HAVE_MOUSE

#include "menu_common.h"
#include "mouse.h"
#include "uimenu.h"

UI_MENU_DEFINE_TOGGLE(Mouse)
UI_MENU_DEFINE_TOGGLE(SmartMouseRTCSave)

const ui_menu_entry_t mouse_menu[] = {
    { "Grab mouse events",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Mouse_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Save Smart Mouse RTC data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SmartMouseRTCSave_callback,
      NULL },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t mouse_grab_menu[] = {
    { "Grab mouse events",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Mouse_callback,
      NULL },
    SDL_MENU_LIST_END
};
#endif
