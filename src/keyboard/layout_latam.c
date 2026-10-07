#include "keyboard_layout.h"
#include "../../third_party/common/keycodes.h"

#if BMC64_NEW_KEYBOARD_INPUT
static const KeyboardLayoutKey keys[] = {
   {KEYCODE_1, '1', '!', 0, 0}, {KEYCODE_2, '2', '"', 0, 0},
   {KEYCODE_3, '3', '#', 0, 0}, {KEYCODE_4, '4', '$', 0, 0},
   {KEYCODE_5, '5', '%', 0, 0}, {KEYCODE_6, '6', '&', 0, 0},
   {KEYCODE_7, '7', '/', 0, 0}, {KEYCODE_8, '8', '(', 0, 0},
   {KEYCODE_9, '9', ')', 0, 0}, {KEYCODE_0, '0', '=', 0, 0},
   {KEYCODE_Dash, '\'', '?', '\\', 0}, {KEYCODE_Equals, 0xBF, 0xA1, 0, 0},
   {KEYCODE_LeftBracket, 0xB4, 0xA8, 0, 0}, {KEYCODE_RightBracket, '+', '*', '~', 0},
   {KEYCODE_BackSlash, '}', ']', '`', 0}, {KEYCODE_Pound, '}', ']', '`', 0},
   {KEYCODE_SemiColon, 0xF1, 0xD1, 0, 0}, {KEYCODE_SingleQuote, '{', '[', '^', 0},
   {KEYCODE_BackQuote, '|', 0xB0, 0xAC, 0}, {KEYCODE_Comma, ',', ';', 0, 0},
   {KEYCODE_Period, '.', ':', 0, 0}, {KEYCODE_Slash, '-', '_', 0, 0},
   {KEYCODE_NonUSBackSlash, '<', '>', 0, 0}, {KEYCODE_q, 'q', 'Q', '@', 0},
   {KEYCODE_Space, ' ', ' ', 0, 0},
};

unsigned int keyboard_layout_latam_char(long key, int shifted, int altgr) {
   return keyboard_layout_lookup(keys, sizeof(keys) / sizeof(keys[0]), key, shifted, altgr);
}
#endif