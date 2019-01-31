/*
 * uidrive.c
 *
 * Written by
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

#include "vice.h"

#include <stdio.h>

#include "drive.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uidrive.h"

void uidrive_init(struct tui_menu *parent_submenu, const tui_menu_item_def_t *d)
{
    tui_menu_t ui_drive_submenu;

    ui_drive_submenu = tui_menu_create("Disk Drive Settings", 1);
    tui_menu_add(ui_drive_submenu, d);
    tui_menu_add_submenu(parent_submenu, "Dis_k Drive Settings...",
                         "Drive emulation settings",
                         ui_drive_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
