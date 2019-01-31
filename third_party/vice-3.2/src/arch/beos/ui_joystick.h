/*
 * ui_joystick.h - Joystick settings
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifndef VICE__UI_JOYSTICK_H__
#define VICE__UI_JOYSTICK_H__

const uint32 JOYMESSAGE_DEVPORT = 'J000';
const uint32 JOYMESSAGE_KEYSET1 = 'J001';
const uint32 JOYMESSAGE_KEYSET2 = 'J002';
const uint32 JOYMESSAGE_KEYTEXT = 'J003';
const uint32 JOYMESSAGE_DISPLAY = 'J004';

extern void ui_joystick(int first_port, int second_port);

#endif
