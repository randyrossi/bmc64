/*
 * uids12c887rtc.h - DS12C887 RTC cartridge UI interface for MS-DOS.
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

#ifndef UIDS12C887RTC_H
#define UIDS12C887RTC_H

struct tui_menu;

extern void uids12c887rtc_c64_init(struct tui_menu *parent_submenu);
extern void uids12c887rtc_c128_init(struct tui_menu *parent_submenu);
extern void uids12c887rtc_vic20_init(struct tui_menu *parent_submenu);

#endif
