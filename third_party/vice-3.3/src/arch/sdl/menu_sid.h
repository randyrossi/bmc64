/*
 * menu_sid.h - Implementation of the SID settings menu for the SDL UI.
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

#ifndef VICE_MENU_SID_H
#define VICE_MENU_SID_H

#include "vice.h"

#include "uimenu.h"

extern ui_menu_entry_t sid_c64_menu[];
extern ui_menu_entry_t sid_c128_menu[];
extern ui_menu_entry_t sid_cbm2_menu[];
extern ui_menu_entry_t sid_dtv_menu[];
extern ui_menu_entry_t sid_pet_menu[];
extern ui_menu_entry_t sid_plus4_menu[];
extern ui_menu_entry_t sid_vic_menu[];

extern void uisid_menu_create(void);
extern void uisid_menu_shutdown(void);

#endif
