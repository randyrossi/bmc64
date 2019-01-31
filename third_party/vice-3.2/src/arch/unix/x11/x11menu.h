/*
 * x11menu.h - Common X11 menu functions.
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

#ifndef VICE_X11MENU_H
#define VICE_X11MENU_H

#include "vice.h"

#include "uihotkey.h"

#ifdef USE_GNOMEUI
#include "gnome/uiarch.h"
#else
#ifdef VMS
#include "xawuiarch.h"
#else
#include "xaw/uiarch.h"
#endif
#endif

typedef enum {
    UI_MENU_TYPE_NORMAL = 0,
    UI_MENU_TYPE_TICK,
    UI_MENU_TYPE_DOTS,
    UI_MENU_TYPE_TICKDOTS,
    UI_MENU_TYPE_SEPARATOR,
    UI_MENU_TYPE_NONE,
    UI_MENU_TYPE_BL_SUB		/* special submenu type, which can receive callbacks to be blocked */
} ui_menu_type_t;

typedef struct ui_menu_entry_s {
    char *string;
    ui_menu_type_t type;
    ui_callback_t callback;
    ui_callback_data_t callback_data;
    struct ui_menu_entry_s *sub_menu;
    ui_keysym_t hotkey_keysym;
    ui_hotkey_modifier_t hotkey_modifier;
} ui_menu_entry_t;

#define UI_MENU_ENTRY_SEPERATOR { "--", UI_MENU_TYPE_SEPARATOR, NULL, NULL, NULL, (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }
#define UI_MENU_ENTRY_LIST_END { NULL, (ui_menu_type_t)0, NULL, NULL, NULL, (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }

extern void ui_set_drive_menu(int drvnr, ui_menu_entry_t *menu);
extern void ui_destroy_drive_menu(int drive);
extern void ui_set_tape_menu(ui_menu_entry_t *menu);
extern void ui_set_topmenu(ui_menu_entry_t *menu);
extern void ui_set_speedmenu(ui_menu_entry_t *menu);
extern void ui_set_left_menu(ui_menu_entry_t *menu);
extern void ui_set_right_menu(ui_menu_entry_t *menu);

#endif
