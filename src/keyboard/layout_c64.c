#include "keyboard_layout.h"

char keyboard_layout_c64_char(long key, int shifted) {
   return keyboard_layout_positional_char(key, shifted);
}