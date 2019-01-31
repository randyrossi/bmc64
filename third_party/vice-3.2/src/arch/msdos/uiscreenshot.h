/*
 * uiscreenshot.h - Screenshot save dialog for the MS-DOS version of VICE.
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

#ifndef UISCREENSHOT_H
#define UISCREENSHOT_H

struct tui_menu_item_def_s;

extern struct tui_menu_item_def_s ui_screenshot_menu_def_vic_vicii_vdc[];
extern struct tui_menu_item_def_s ui_screenshot_menu_def_ted[];
extern struct tui_menu_item_def_s ui_screenshot_menu_def_crtc[];

#endif
