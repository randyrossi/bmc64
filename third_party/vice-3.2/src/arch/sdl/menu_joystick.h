/*
 * menu_joystick.h - Joystick menu for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#ifndef VICE_MENU_JOYSTICK_H
#define VICE_MENU_JOYSTICK_H

#include "vice.h"
#include "types.h"
#include "uimenu.h"

extern const ui_menu_entry_t joystick_menu[];
extern const ui_menu_entry_t joystick_c64_menu[];
extern const ui_menu_entry_t joystick_c64dtv_menu[];
extern const ui_menu_entry_t joystick_plus4_menu[];
extern const ui_menu_entry_t joystick_vic20_menu[];
extern const ui_menu_entry_t joystick_userport_only_menu[];

#endif
