//
// layout_de.c
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "keyboard_layout.h"
#include "../../third_party/common/keycodes.h"

static const KeyboardLayoutKey keys[] = {
   {KEYCODE_1, '1', '!', 0, 0}, {KEYCODE_2, '2', '"', 0xB2, 0},
   {KEYCODE_3, '3', 0xA7, 0xB3, 0}, {KEYCODE_4, '4', '$', 0, 0},
   {KEYCODE_5, '5', '%', 0, 0}, {KEYCODE_6, '6', '&', 0, 0},
   {KEYCODE_7, '7', '/', '{', 0}, {KEYCODE_8, '8', '(', '[', 0},
   {KEYCODE_9, '9', ')', ']', 0}, {KEYCODE_0, '0', '=', '}', 0},
   {KEYCODE_Dash, 0xDF, '?', '\\', 0}, {KEYCODE_Equals, 0xB4, '`', 0, 0},
   {KEYCODE_LeftBracket, 0xFC, 0xDC, 0, 0}, {KEYCODE_RightBracket, '+', '*', '~', 0},
   {KEYCODE_BackSlash, '#', '\'', 0, 0}, {KEYCODE_Pound, '#', '\'', 0, 0},
   {KEYCODE_SemiColon, 0xF6, 0xD6, 0, 0}, {KEYCODE_SingleQuote, 0xE4, 0xC4, 0, 0},
   {KEYCODE_BackQuote, '^', 0xB0, 0, 0}, {KEYCODE_Comma, ',', ';', 0, 0},
   {KEYCODE_Period, '.', ':', 0, 0}, {KEYCODE_Slash, '-', '_', 0, 0},
   {KEYCODE_NonUSBackSlash, '<', '>', '|', 0}, {KEYCODE_y, 'z', 'Z', 0, 0},
   {KEYCODE_z, 'y', 'Y', 0, 0}, {KEYCODE_q, 'q', 'Q', '@', 0},
   {KEYCODE_e, 'e', 'E', 0x20AC, 0}, {KEYCODE_m, 'm', 'M', 0xB5, 0},
   {KEYCODE_Space, ' ', ' ', 0, 0},
};

unsigned int keyboard_layout_de_char(long key, int shifted, int altgr) {
   return keyboard_layout_lookup(keys, sizeof(keys) / sizeof(keys[0]), key, shifted, altgr);
}