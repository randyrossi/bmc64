/*
 * uicommands.h - Implementation of common UI commands.
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

#ifndef VICE_UICOMMANDS_H
#define VICE_UICOMMANDS_H

#include "uimenu.h"

extern struct ui_menu_entry_s ui_directory_commands_menu[];
extern struct ui_menu_entry_s ui_exit_commands_menu[];
extern struct ui_menu_entry_s ui_help_commands_menu[];
extern struct ui_menu_entry_s ui_run_commands_menu[];
extern struct ui_menu_entry_s ui_runmode_commands_menu[];
extern struct ui_menu_entry_s ui_snapshot_commands_menu[];
extern struct ui_menu_entry_s ui_snapshot_commands_submenu[];
extern struct ui_menu_entry_s ui_sound_record_commands_menu[];
extern struct ui_menu_entry_s ui_sound_record_commands_submenu[];
extern struct ui_menu_entry_s ui_tool_commands_menu[];
extern struct ui_menu_entry_s ui_tool_commands_monitor_menu[];

extern void uicommands_shutdown(void);
extern ui_button_t ui_change_dir(const char *title, const char *prompt, char *buf,unsigned int buflen);
extern void uicommands_sound_record_stop(void);

#endif
