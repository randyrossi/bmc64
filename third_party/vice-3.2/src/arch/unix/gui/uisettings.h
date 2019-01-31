/*
 * uisettings.h - Implementation of common UI settings.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_UISETTINGS_H
#define VICE_UISETTINGS_H

#include "uimenu.h"

extern struct ui_menu_entry_s ui_performance_settings_menu[];
extern struct ui_menu_entry_s ui_fullscreen_settings_menu[];
extern struct ui_menu_entry_s ui_fullscreen_settings_submenu[];
extern struct ui_menu_entry_s ui_settings_settings_menu[];
extern struct ui_menu_entry_s ui_debug_settings_menu[];
#define UI_DEBUG_SETTINGS_SUBMENU_EXTRA_IDX 8
extern struct ui_menu_entry_s debug_settings_submenu[];

/* VSID debug menu doesn't have the drive CPU items */
extern struct ui_menu_entry_s ui_debug_settings_menu_vsid[];
extern struct ui_menu_entry_s debug_settings_submenu_vsid[];


extern struct ui_menu_entry_s set_maximum_speed_submenu[];
#endif
