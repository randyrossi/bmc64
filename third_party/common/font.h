/*
 * font.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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
 * Modifications Copyright (C) 2026 aminch <https://github.com/aminch>
 * Modified for BMC64: see Q5.
 *
 */
#ifndef RASPI_FONT_H_
#define RASPI_FONT_H_

// 256-entry 8x8 font indexed by Unicode code point U+0000 - U+00FF.
// See font.c for the layout (ASCII, .notdef box at 0x7F, Latin-1 at 0xA0+).
extern char font8x8_basic[256][8];

#endif
