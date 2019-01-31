/*
 * menudefs.h - Definition of menu commands and settings.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_MENUDEFS_H
#define VICE_MENUDEFS_H

#include "tuimenu.h"

extern tui_menu_t ui_main_menu;
extern tui_menu_t ui_attach_submenu;
extern tui_menu_t ui_detach_submenu;
extern tui_menu_t ui_video_submenu;
extern tui_menu_t ui_drive_submenu;
extern tui_menu_t ui_rom_submenu;
extern tui_menu_t ui_sound_submenu;
extern tui_menu_t ui_speed_limit_submenu;
extern tui_menu_t ui_special_submenu;
extern tui_menu_t ui_reset_submenu;
extern tui_menu_t ui_quit_submenu;
extern tui_menu_t ui_info_submenu;

extern void ui_create_main_menu(int has_tape, int has_true1541, int has_serial_traps, int num_joysticks, int has_datasette, const tui_menu_item_def_t *d);

#endif /* _MENUDEFS_H */
