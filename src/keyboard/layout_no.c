#include "keyboard_layout.h"
#include "../../third_party/common/keycodes.h"

#if BMC64_NEW_KEYBOARD_INPUT
static const KeyboardLayoutKey keys[] = {
   {KEYCODE_1, '1', '!', 0, 0}, {KEYCODE_2, '2', '"', '@', 0},
   {KEYCODE_3, '3', '#', 0xA3, 0}, {KEYCODE_4, '4', 0xA4, '$', 0},
   {KEYCODE_5, '5', '%', 0, 0}, {KEYCODE_6, '6', '&', 0, 0},
   {KEYCODE_7, '7', '/', '{', 0}, {KEYCODE_8, '8', '(', '[', 0},
   {KEYCODE_9, '9', ')', ']', 0}, {KEYCODE_0, '0', '=', '}', 0},
   {KEYCODE_Dash, '+', '?', '\\', 0}, {KEYCODE_Equals, '\\', '`', 0, 0},
   {KEYCODE_LeftBracket, 0xE5, 0xC5, 0, 0}, {KEYCODE_RightBracket, 0xA8, '^', 0, 0},
   {KEYCODE_BackSlash, '\'', '*', 0, 0}, {KEYCODE_Pound, '\'', '*', 0, 0},
   {KEYCODE_SemiColon, 0xF8, 0xD8, 0, 0}, {KEYCODE_SingleQuote, 0xE6, 0xC6, 0, 0},
   {KEYCODE_BackQuote, '|', 0xA7, 0, 0}, {KEYCODE_Comma, ',', ';', 0, 0},
   {KEYCODE_Period, '.', ':', 0, 0}, {KEYCODE_Slash, '-', '_', 0, 0},
   {KEYCODE_KP_BackSlash, '<', '>', 0, 0}, {KEYCODE_Space, ' ', ' ', 0, 0},
};

unsigned int keyboard_layout_no_char(long key, int shifted, int altgr) {
   return keyboard_layout_lookup(keys, sizeof(keys) / sizeof(keys[0]), key, shifted, altgr);
}
#endif