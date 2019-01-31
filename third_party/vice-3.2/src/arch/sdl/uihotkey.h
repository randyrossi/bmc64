/*
 * uihotkey.h - UI hotkey functions.
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

#ifndef VICE_UIHOTKEY_H
#define VICE_UIHOTKEY_H

#include "vice.h"
#include "types.h"

#include "uimenu.h"

#define SDL_UI_HOTKEY_DELIM "&"

extern ui_menu_entry_t *sdl_ui_hotkey_action(char *path);
extern char *sdl_ui_hotkey_path(ui_menu_entry_t *action);
extern int sdl_ui_hotkey_map(ui_menu_entry_t *item);

extern int sdl_ui_hotkey(ui_menu_entry_t *item); /* implemented in uimenu.c */

#endif
