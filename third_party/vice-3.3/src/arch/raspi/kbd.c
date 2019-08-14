/*
 * kbd.c
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
 */

#include "kbd.h"

#include "circle.h"
#include "demo.h"
#include "joy.h"
#include "keyboard.h"
#include "menu.h"
#include "menu_keyset.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>

extern struct joydev_config joydevs[MAX_JOY_PORTS];

static int left_control_down = 0;

key_combo_state_t key_combo_states[4];

void kbd_arch_init(void) {}

void kbd_set_hotkey_function(unsigned int slot, long key, int function) {
  if (slot >= NUM_KEY_COMBOS)
    return;
  key_combo_states[slot].second_key = key;
  key_combo_states[slot].invoked = 0;
  key_combo_states[slot].function = function;
}

int kbd_arch_get_host_mapping(void) { return KBD_MAPPING_US; }

signed long kbd_arch_keyname_to_keynum(char *keyname) {
  if (strlen(keyname) == 1) {
    switch (keyname[0]) {
    case 'a':
      return KEYCODE_a;
    case 'b':
      return KEYCODE_b;
    case 'c':
      return KEYCODE_c;
    case 'd':
      return KEYCODE_d;
    case 'e':
      return KEYCODE_e;
    case 'f':
      return KEYCODE_f;
    case 'g':
      return KEYCODE_g;
    case 'h':
      return KEYCODE_h;
    case 'i':
      return KEYCODE_i;
    case 'j':
      return KEYCODE_j;
    case 'k':
      return KEYCODE_k;
    case 'l':
      return KEYCODE_l;
    case 'm':
      return KEYCODE_m;
    case 'n':
      return KEYCODE_n;
    case 'o':
      return KEYCODE_o;
    case 'p':
      return KEYCODE_p;
    case 'q':
      return KEYCODE_q;
    case 'r':
      return KEYCODE_r;
    case 's':
      return KEYCODE_s;
    case 't':
      return KEYCODE_t;
    case 'u':
      return KEYCODE_u;
    case 'v':
      return KEYCODE_v;
    case 'w':
      return KEYCODE_w;
    case 'x':
      return KEYCODE_x;
    case 'y':
      return KEYCODE_y;
    case 'z':
      return KEYCODE_z;
    case '1':
      return KEYCODE_1;
    case '2':
      return KEYCODE_2;
    case '3':
      return KEYCODE_3;
    case '4':
      return KEYCODE_4;
    case '5':
      return KEYCODE_5;
    case '6':
      return KEYCODE_6;
    case '7':
      return KEYCODE_7;
    case '8':
      return KEYCODE_8;
    case '9':
      return KEYCODE_9;
    case '0':
      return KEYCODE_0;
    }
    return 0;
  } else if (strcmp(keyname, "Return") == 0) {
    return (long)KEYCODE_Return;
  } else if (strcmp(keyname, "BackSpace") == 0) {
    return (long)KEYCODE_Backspace;
  } else if (strcmp(keyname, "Delete") == 0) {
    return (long)KEYCODE_PageUp;
  } else if (strcmp(keyname, "CapsLock") == 0) {
    return (long)KEYCODE_CapsLock;
  } else if (strcmp(keyname, "Up") == 0) {
    return (long)KEYCODE_Up;
  } else if (strcmp(keyname, "Down") == 0) {
    return (long)KEYCODE_Down;
  } else if (strcmp(keyname, "Left") == 0) {
    return (long)KEYCODE_Left;
  } else if (strcmp(keyname, "Right") == 0) {
    return (long)KEYCODE_Right;
  } else if (strcmp(keyname, "Up") == 0) {
    return (long)KEYCODE_Up;
  } else if (strcmp(keyname, "comma") == 0) {
    return (long)KEYCODE_Comma;
  } else if (strcmp(keyname, "period") == 0) {
    return (long)KEYCODE_Period;
  } else if (strcmp(keyname, "space") == 0) {
    return (long)KEYCODE_Space;
  } else if (strcmp(keyname, "asterisk") == 0) {
    return (long)KEYCODE_RightBracket;
  } else if (strcmp(keyname, "arrowup") == 0) {
    return (long)KEYCODE_Delete;
  } else if (strcmp(keyname, "Shift_L") == 0) {
    return (long)KEYCODE_LeftShift;
  } else if (strcmp(keyname, "Shift_R") == 0) {
    return (long)KEYCODE_RightShift;
  } else if (strcmp(keyname, "plus") == 0) {
    return (long)KEYCODE_Dash;
  } else if (strcmp(keyname, "arrowleft") == 0) {
    return (long)KEYCODE_BackQuote;
  } else if (strcmp(keyname, "minus") == 0) {
    return (long)KEYCODE_Equals;
  } else if (strcmp(keyname, "colon") == 0) {
    return (long)KEYCODE_SemiColon;
  } else if (strcmp(keyname, "Home") == 0) {
    return (long)KEYCODE_Home;
  } else if (strcmp(keyname, "slash") == 0) {
    return (long)KEYCODE_Slash;
  } else if (strcmp(keyname, "equal") == 0 &&
             menu_get_keyboard_type() == KEYBOARD_TYPE_US) {
    return (long)KEYCODE_BackSlash;
  } else if (strcmp(keyname, "equal") == 0 &&
             menu_get_keyboard_type() == KEYBOARD_TYPE_UK) {
    return (long)KEYCODE_Pound;
  } else if (strcmp(keyname, "sterling") == 0) {
    return (long)KEYCODE_Insert;
  } else if (strcmp(keyname, "semicolon") == 0) {
    return (long)KEYCODE_SingleQuote;
  } else if (strcmp(keyname, "Tab") == 0) {
    return (long)KEYCODE_Tab;
  } else if (strcmp(keyname, "Control_L") == 0) {
    return (long)KEYCODE_LeftControl;
  } else if (strcmp(keyname, "Control_R") == 0) {
    return (long)KEYCODE_RightControl;
  } else if (strcmp(keyname, "Alt_L") == 0) {
    return (long)KEYCODE_LeftAlt;
  } else if (strcmp(keyname, "Escape") == 0) {
    return (long)KEYCODE_Escape;
  } else if (strcmp(keyname, "at") == 0) {
    return (long)KEYCODE_LeftBracket;
  } else if (strcmp(keyname, "F1") == 0) {
    return (long)KEYCODE_F1;
  } else if (strcmp(keyname, "F2") == 0) {
    return (long)KEYCODE_F2;
  } else if (strcmp(keyname, "F3") == 0) {
    return (long)KEYCODE_F3;
  } else if (strcmp(keyname, "F4") == 0) {
    return (long)KEYCODE_F4;
  } else if (strcmp(keyname, "F5") == 0) {
    return (long)KEYCODE_F5;
  } else if (strcmp(keyname, "F6") == 0) {
    return (long)KEYCODE_F6;
  } else if (strcmp(keyname, "F7") == 0) {
    return (long)KEYCODE_F7;
  } else if (strcmp(keyname, "F8") == 0) {
    return (long)KEYCODE_F8;
  } else if (strcmp(keyname, "F9") == 0) {
    return (long)KEYCODE_F9;
  } else if (strcmp(keyname, "F10") == 0) {
    return (long)KEYCODE_F10;
  } else if (strcmp(keyname, "F11") == 0) {
    return (long)KEYCODE_F11;
  } else if (strcmp(keyname, "ScrollLock") == 0) {
    return (long)KEYCODE_ScrollLock;
  }

  return 0;
}

const char *kbd_arch_keynum_to_keyname(signed long keynum) { return 0; }

void kbd_initialize_numpad_joykeys(int *joykeys) {}

// Return 1 if press is consumed
static int handle_key_combo_press(long key) {
  int i;
  for (i = 0; i < NUM_KEY_COMBOS; i++) {
    if (key_combo_states[i].second_key == key && left_control_down) {
      key_combo_states[i].invoked = 1;
      return 1;
    }
  }
  return 0;
}

// Return 1 if release is consumed
// Some things we can do on the up event of the 2nd key
static int handle_key_combo_release(long key) {
  int i;
  for (i = 0; i < NUM_KEY_COMBOS; i++) {
    if (key_combo_states[i].second_key == key && key_combo_states[i].invoked) {
      // Can we do this now?
      // KEEP THIS IN SYNC WITH kernel.cpp
      switch (key_combo_states[i].function) {
      case BTN_ASSIGN_WARP:
      case BTN_ASSIGN_SWAP_PORTS:
      case BTN_ASSIGN_STATUS_TOGGLE:
      case BTN_ASSIGN_CART_FREEZE:
      case BTN_ASSIGN_ACTIVE_DISPLAY:
      case BTN_ASSIGN_PIP_LOCATION:
      case BTN_ASSIGN_PIP_SWAP:
      case BTN_ASSIGN_40_80_COLUMN:
      case BTN_ASSIGN_VKBD_TOGGLE:
        circle_emu_quick_func_interrupt(key_combo_states[i].function);
        key_combo_states[i].invoked = 0;
        return 1;
      default:
        break;
      }
      return 0;
    }
  }
  return 0;
}

// Some things we can only do on the up event of the cntrl key
static void handle_key_combo_function() {
  int i;
  for (i = 0; i < NUM_KEY_COMBOS; i++) {
    if (key_combo_states[i].invoked) {
      key_combo_states[i].invoked = 0;

      switch (key_combo_states[i].function) {
      case BTN_ASSIGN_MENU:
        // When transitioning to the menu, make sure to give emulator
        // at least one pass through main loop after sending the up evt.
        circle_lock_acquire();
        ui_toggle_pending = 2;
        circle_lock_release();
        break;
      case BTN_ASSIGN_RESET_HARD:
      case BTN_ASSIGN_RESET_SOFT:
      case BTN_ASSIGN_TAPE_MENU:
      case BTN_ASSIGN_CART_MENU:
        circle_emu_quick_func_interrupt(key_combo_states[i].function);
        key_combo_states[i].invoked = 0;
        break;
      default:
        break;
      }
    }
  }
}

void circle_key_pressed(long key) {
  if (raw_keycode_func) {
    // Just consume this.
    return;
  }

  if (key == KEYCODE_LeftControl) {
    left_control_down = 1;
  }

  // Intercept keys meant to become joystick values
  if (joydevs[0].device == JOYDEV_NUMS_1 ||
      joydevs[0].device == JOYDEV_NUMS_2 ||
      joydevs[0].device == JOYDEV_CURS_SP ||
      joydevs[0].device == JOYDEV_CURS_LC ||
      joydevs[0].device == JOYDEV_KEYSET1 ||
      joydevs[0].device == JOYDEV_KEYSET2) {
    if (joy_key_down(0, key))
      return;
  }
  if (joydevs[1].device == JOYDEV_NUMS_1 ||
      joydevs[1].device == JOYDEV_NUMS_2 ||
      joydevs[1].device == JOYDEV_CURS_SP ||
      joydevs[1].device == JOYDEV_CURS_LC ||
      joydevs[1].device == JOYDEV_KEYSET1 ||
      joydevs[1].device == JOYDEV_KEYSET2) {
    if (joy_key_down(1, key))
      return;
  }

  if (handle_key_combo_press(key)) {
    return;
  }

  if (ui_enabled) {
    circle_ui_key_interrupt(key, 1 /* down */);
  } else {
    circle_emu_key_interrupt(key, 1 /* down */);
  }
}

void circle_key_released(long key) {
  if (raw_keycode_func) {
    raw_keycode_func(key);
    return;
  }

  if (key == KEYCODE_LeftControl) {
    left_control_down = 0;
  }

  if (key == KEYCODE_F12) {
    if (ui_enabled) {
      // Let the ui handle the menu action as it sees fit.
      circle_ui_key_interrupt(key, 0 /* up */);
    } else {
      // When transitioning to the menu, make sure to give emulator
      // at least one pass through main loop after sending the up event.
      circle_emu_key_interrupt(key, 0 /* up */);
      circle_lock_acquire();
      ui_toggle_pending = 2;
      circle_lock_release();
    }
    return;
  }

  // Intercept keys meant to become joystick values
  if (joydevs[0].device == JOYDEV_NUMS_1 ||
      joydevs[0].device == JOYDEV_NUMS_2 ||
      joydevs[0].device == JOYDEV_CURS_SP ||
      joydevs[0].device == JOYDEV_CURS_LC ||
      joydevs[0].device == JOYDEV_KEYSET1 ||
      joydevs[0].device == JOYDEV_KEYSET2) {
    if (joy_key_up(0, key))
      return;
  }
  if (joydevs[1].device == JOYDEV_NUMS_1 ||
      joydevs[1].device == JOYDEV_NUMS_2 ||
      joydevs[1].device == JOYDEV_CURS_SP ||
      joydevs[1].device == JOYDEV_CURS_LC ||
      joydevs[1].device == JOYDEV_KEYSET1 ||
      joydevs[1].device == JOYDEV_KEYSET2) {
    if (joy_key_up(1, key))
      return;
  }

  if (handle_key_combo_release(key)) {
    return;
  }

  if (ui_enabled) {
    circle_ui_key_interrupt(key, 0 /* up */);
  } else {
    circle_emu_key_interrupt(key, 0 /* up */);
  }

  // Check hotkey combo here
  if (key == KEYCODE_LeftControl) {
    // We invoke the hot key func here after the left control is released
    // so emulator is not left in a weird state.
    handle_key_combo_function();
  }
}

void circle_keyboard_set_latch_keyarr(int row, int col, int value) {
  demo_reset_timeout();
  keyboard_set_keyarr(row, col, value);
}
