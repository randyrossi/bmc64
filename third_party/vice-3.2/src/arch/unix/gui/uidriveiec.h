/*
 * uidriveiec.h
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

#ifndef VICE_UIDRIVEIEC_H
#define VICE_UIDRIVEIEC_H

#include "uimenu.h"

extern UI_CALLBACK(uidriveiec_expansion_control);
extern UI_CALLBACK(uidriveiec_idle_method_control);

extern struct ui_menu_entry_s uidriveiec_drive0_ram_expansion_submenu[];
extern struct ui_menu_entry_s uidriveiec_drive1_ram_expansion_submenu[];
extern struct ui_menu_entry_s uidriveiec_drive2_ram_expansion_submenu[];
extern struct ui_menu_entry_s uidriveiec_drive3_ram_expansion_submenu[];

extern struct ui_menu_entry_s set_drive0_idle_method_submenu[];
extern struct ui_menu_entry_s set_drive1_idle_method_submenu[];
extern struct ui_menu_entry_s set_drive2_idle_method_submenu[];
extern struct ui_menu_entry_s set_drive3_idle_method_submenu[];

#endif
