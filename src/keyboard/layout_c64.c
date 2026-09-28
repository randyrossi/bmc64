#include "keyboard_layout.h"

#if BMC64_NEW_KEYBOARD_INPUT
char keyboard_layout_c64_char(long key, int shifted) {
   return keyboard_layout_positional_char(key, shifted);
}
#endif