/*
 * uiromset.h
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

#ifndef VICE_UIROMSET_H
#define VICE_UIROMSET_H

#include "uimenu.h"

extern struct ui_menu_entry_s uiromset_type_submenu[];
extern struct ui_menu_entry_s uiromset_file_submenu[];
extern struct ui_menu_entry_s uiromset_archive_submenu[];

extern UI_CALLBACK(ui_set_romset);
extern UI_CALLBACK(ui_load_rom_file);
extern UI_CALLBACK(ui_unload_rom_file);

extern void uiromset_menu_init(void);

#endif
