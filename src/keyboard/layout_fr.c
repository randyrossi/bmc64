#include "keyboard_layout.h"
#include "../../third_party/common/keycodes.h"

#if BMC64_NEW_KEYBOARD_INPUT
static const KeyboardLayoutKey keys[] = {
   {KEYCODE_1, '&', '1', 0, 0}, {KEYCODE_2, 0xE9, '2', '~', 0},
   {KEYCODE_3, '"', '3', '#', 0}, {KEYCODE_4, '\'', '4', '{', '$'},
   {KEYCODE_5, '(', '5', '[', 0}, {KEYCODE_6, '-', '6', '|', 0},
   {KEYCODE_7, 0xE8, '7', '`', 0}, {KEYCODE_8, '_', '8', '\\', 0},
   {KEYCODE_9, 0xE7, '9', '^', 0}, {KEYCODE_0, 0xE0, '0', '@', 0},
   {KEYCODE_Dash, ')', 0xB0, ']', 0}, {KEYCODE_Equals, '=', '+', '}', 0},
   {KEYCODE_LeftBracket, '^', 0xA8, 0, 0}, {KEYCODE_RightBracket, '$', 0xA3, 0, 0},
   {KEYCODE_BackSlash, '*', 0xB5, 0, 0}, {KEYCODE_Pound, '*', 0xB5, 0, 0},
   {KEYCODE_SemiColon, 'm', 'M', 0, 0}, {KEYCODE_SingleQuote, 0xF9, '%', 0, 0},
   {KEYCODE_BackQuote, 0xB2, '~', 0, 0}, {KEYCODE_Comma, ';', '.', 0, 0},
   {KEYCODE_Period, ':', '/', 0, 0}, {KEYCODE_Slash, '!', 0xA7, 0, 0},
   {KEYCODE_NonUSBackSlash, '<', '>', 0, 0}, {KEYCODE_a, 'q', 'Q', '@', 0},
   {KEYCODE_q, 'a', 'A', 0, 0}, {KEYCODE_w, 'z', 'Z', 0, 0},
   {KEYCODE_z, 'w', 'W', 0, 0}, {KEYCODE_m, ',', '?', 0, 0},
   {KEYCODE_Space, ' ', ' ', 0, 0},
};

unsigned int keyboard_layout_fr_char(long key, int shifted, int altgr) {
   return keyboard_layout_lookup(keys, sizeof(keys) / sizeof(keys[0]), key, shifted, altgr);
}
#endif