/*
 * menu_text_layout.c
 *
 * BMC64 - A bare metal Commodore emulator for Raspberry Pi
 * Copyright (C) 2026 aminch <https://github.com/aminch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * BMC64 menu text character mapping.
 */

#include "menu_text_layout.h"

#include <stddef.h>

#include "keycodes.h"
#include "menu.h"

typedef struct {
  long key;
  char normal;
  char shifted;
} menu_text_key_t;

static const menu_text_key_t us_text_keys[] = {
 {KEYCODE_1, '1', '!'}, {KEYCODE_2, '2', '@'}, {KEYCODE_3, '3', '#'},
 {KEYCODE_4, '4', '$'}, {KEYCODE_5, '5', '%'}, {KEYCODE_6, '6', '^'},
 {KEYCODE_7, '7', '&'}, {KEYCODE_8, '8', '*'}, {KEYCODE_9, '9', '('},
 {KEYCODE_0, '0', ')'}, {KEYCODE_Dash, '-', '_'}, {KEYCODE_Equals, '=', '+'},
 {KEYCODE_LeftBracket, '[', '{'}, {KEYCODE_RightBracket, ']', '}'},
 {KEYCODE_BackSlash, '\\', '|'}, {KEYCODE_SemiColon, ';', ':'},
 {KEYCODE_SingleQuote, '\'', '"'}, {KEYCODE_BackQuote, '`', '~'},
 {KEYCODE_Comma, ',', '<'}, {KEYCODE_Period, '.', '>'}, {KEYCODE_Slash, '/', '?'},
 {KEYCODE_Space, ' ', ' '},
};

static const menu_text_key_t positional_text_keys[] = {
 {KEYCODE_1, '1', '!'}, {KEYCODE_2, '2', '"'}, {KEYCODE_3, '3', '#'},
 {KEYCODE_4, '4', '$'}, {KEYCODE_5, '5', '%'}, {KEYCODE_6, '6', '&'},
 {KEYCODE_7, '7', '\''}, {KEYCODE_8, '8', '('}, {KEYCODE_9, '9', ')'},
 {KEYCODE_0, '0', '0'}, {KEYCODE_Dash, '+', '+'}, {KEYCODE_Equals, '-', '-'},
 {KEYCODE_LeftBracket, '@', '@'}, {KEYCODE_RightBracket, '*', '*'},
 {KEYCODE_BackSlash, '=', '='}, {KEYCODE_SemiColon, ':', '['},
 {KEYCODE_SingleQuote, ';', ']'}, {KEYCODE_Comma, ',', '<'},
 {KEYCODE_Period, '.', '>'}, {KEYCODE_Slash, '/', '?'}, {KEYCODE_Space, ' ', ' '},
};

static const menu_text_key_t maxi_text_keys[] = {
 {KEYCODE_1, '1', '!'}, {KEYCODE_2, '2', '"'}, {KEYCODE_3, '3', '#'},
 {KEYCODE_4, '4', '$'}, {KEYCODE_5, '5', '%'}, {KEYCODE_6, '6', '&'},
 {KEYCODE_7, '7', '\''}, {KEYCODE_8, '8', '('}, {KEYCODE_9, '9', ')'},
 {KEYCODE_0, '0', '0'}, {KEYCODE_KP_Add, '+', '+'},
 {KEYCODE_KP_Subtract, '-', '-'}, {KEYCODE_LeftBracket, ':', '['},
 {KEYCODE_SemiColon, '*', '*'}, {KEYCODE_BackSlash, '@', '@'},
 {KEYCODE_Equals, '=', '='}, {KEYCODE_RightBracket, ';', ']'},
 {KEYCODE_Comma, ',', '<'}, {KEYCODE_Period, '.', '>'},
 {KEYCODE_Slash, '/', '?'}, {KEYCODE_Space, ' ', ' '},
};

char menu_text_layout_key_to_char(long key, int shifted, int keyboard_mapping) {
  const menu_text_key_t *text_keys = us_text_keys;
  size_t text_key_count = sizeof(us_text_keys) / sizeof(us_text_keys[0]);

  if (keyboard_mapping == KEYBOARD_MAPPING_POS) {
    text_keys = positional_text_keys;
    text_key_count = sizeof(positional_text_keys) / sizeof(positional_text_keys[0]);
  } else if (keyboard_mapping == KEYBOARD_MAPPING_MAXI) {
    text_keys = maxi_text_keys;
    text_key_count = sizeof(maxi_text_keys) / sizeof(maxi_text_keys[0]);
  }

  for (size_t index = 0; index < text_key_count; index++) {
    if (text_keys[index].key == key) {
      return shifted ? text_keys[index].shifted : text_keys[index].normal;
    }
  }
  return '\0';
}
