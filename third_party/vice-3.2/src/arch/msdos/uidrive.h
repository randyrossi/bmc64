/*
 * uidrive.h
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

#ifndef UIDRIVE_H
#define UIDRIVE_H

#include "tuimenu.h"

struct tui_menu;

extern tui_menu_item_def_t drivec64_settings_submenu[];
extern tui_menu_item_def_t drivec128_settings_submenu[];
extern tui_menu_item_def_t drivec64dtv_settings_submenu[];
extern tui_menu_item_def_t driveplus4_settings_submenu[];
extern tui_menu_item_def_t drivevic20_settings_submenu[];
extern tui_menu_item_def_t driveieee_settings_submenu[];

extern void uidrive_init(struct tui_menu *parent_submenu, const tui_menu_item_def_t *d);

#endif
