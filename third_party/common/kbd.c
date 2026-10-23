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
 * Modifications Copyright (C) 2026 aminch <https://github.com/aminch>
 * Modified for BMC64: keyboard handling rework, ISO keys, C128 keys and German
 * layout, profiles.
 *
 */

#include "kbd.h"

#include <stdio.h>
#include <string.h>

// RASPI includes
#include "emux_api.h"
#include "circle.h"
#include "demo.h"
#include "joy.h"
#include "menu.h"
#include "menu_keyset.h"
#include "menu_switch.h"
#include "ui.h"

void kbd_arch_init(void) {}


// Tests keyname var against given string
#define KCMP(x) (strcmp(keyname, x) == 0)

// v2.5 made keycodes consistently BMC64 usb keycode
// names rather than a mix of codes and c64 labels. But
// to keep compatibility with existing files out there,
// we still match the old names. The most unfortunate
// one was "Delete" which has to stay. The new code is
// "Del".
#define LEGACY_KCMP(x) (strcmp(keyname, x) == 0)

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
  } else if (KCMP("Return")) {
    return (long)KEYCODE_Return;
  } else if (KCMP("BackSpace")) {
    return (long)KEYCODE_Backspace;
  } else if (KCMP("PageUp") || LEGACY_KCMP("Delete")) {
    return (long)KEYCODE_PageUp;
  } else if (KCMP("PageDown")) {
    return (long)KEYCODE_PageDown;
  } else if (KCMP("CapsLock")) {
    return (long)KEYCODE_CapsLock;
  } else if (KCMP("Up")) {
    return (long)KEYCODE_Up;
  } else if (KCMP("Down")) {
    return (long)KEYCODE_Down;
  } else if (KCMP("Left")) {
    return (long)KEYCODE_Left;
  } else if (KCMP("Right")) {
    return (long)KEYCODE_Right;
  } else if (KCMP("Up")) {
    return (long)KEYCODE_Up;
  } else if (KCMP("Comma") || LEGACY_KCMP("comma")) {
    return (long)KEYCODE_Comma;
  } else if (KCMP("Period") || LEGACY_KCMP("period")) {
    return (long)KEYCODE_Period;
  } else if (KCMP("Space") || LEGACY_KCMP("space")) {
    return (long)KEYCODE_Space;
  } else if (KCMP("RightBracket") || LEGACY_KCMP("asterisk")) {
    return (long)KEYCODE_RightBracket;
  } else if (KCMP("Del") || LEGACY_KCMP("arrowup")) {
    return (long)KEYCODE_Delete;
  } else if (KCMP("Shift_L")) {
    return (long)KEYCODE_LeftShift;
  } else if (KCMP("Shift_R")) {
    return (long)KEYCODE_RightShift;
  } else if (KCMP("Dash") || LEGACY_KCMP("plus")) {
    return (long)KEYCODE_Dash;
  } else if (KCMP("BackQuote") || LEGACY_KCMP("arrowleft")) {
    return (long)KEYCODE_BackQuote;
  } else if (KCMP("Equals") || LEGACY_KCMP("minus")) {
    return (long)KEYCODE_Equals;
  } else if (KCMP("SemiColon") || LEGACY_KCMP("colon")) {
    return (long)KEYCODE_SemiColon;
  } else if (KCMP("Home")) {
    return (long)KEYCODE_Home;
  } else if (KCMP("End")) {
    return (long)KEYCODE_End;
  } else if (KCMP("Slash") || LEGACY_KCMP("slash")) {
    return (long)KEYCODE_Slash;
  } else if (KCMP("BackSlash")) {
    return (long)KEYCODE_BackSlash;
  } else if (KCMP("Pound")) {
    return (long)KEYCODE_Pound;
  } else if (KCMP("Insert") || LEGACY_KCMP("sterling")) {
    return (long)KEYCODE_Insert;
  } else if (KCMP("SingleQuote") || LEGACY_KCMP("semicolon")) {
    return (long)KEYCODE_SingleQuote;
  } else if (KCMP("Tab")) {
    return (long)KEYCODE_Tab;
  } else if (KCMP("Control_L")) {
    return (long)KEYCODE_LeftControl;
  } else if (KCMP("Control_R")) {
    return (long)KEYCODE_RightControl;
  } else if (KCMP("Alt_L")) {
    return (long)KEYCODE_LeftAlt;
  } else if (KCMP("Alt_R")) {
    return (long)KEYCODE_RightAlt;
  } else if (KCMP("Super_L")) {
    return (long)KEYCODE_LeftSuper;
  } else if (KCMP("Super_R")) {
    return (long)KEYCODE_RightSuper;
  } else if (KCMP("Escape")) {
    return (long)KEYCODE_Escape;
  } else if (KCMP("LeftBracket") || LEGACY_KCMP("at")) {
    return (long)KEYCODE_LeftBracket;
  } else if (KCMP("F1")) {
    return (long)KEYCODE_F1;
  } else if (KCMP("F2")) {
    return (long)KEYCODE_F2;
  } else if (KCMP("F3")) {
    return (long)KEYCODE_F3;
  } else if (KCMP("F4")) {
    return (long)KEYCODE_F4;
  } else if (KCMP("F5")) {
    return (long)KEYCODE_F5;
  } else if (KCMP("F6")) {
    return (long)KEYCODE_F6;
  } else if (KCMP("F7")) {
    return (long)KEYCODE_F7;
  } else if (KCMP("F8")) {
    return (long)KEYCODE_F8;
  } else if (KCMP("F9")) {
    return (long)KEYCODE_F9;
  } else if (KCMP("F10")) {
    return (long)KEYCODE_F10;
  } else if (KCMP("F11")) {
    return (long)KEYCODE_F11;
  } else if (KCMP("ScrollLock")) {
    return (long)KEYCODE_ScrollLock;
  } else if (KCMP("Pause")) {
    return (long)KEYCODE_Pause;
  } else if (KCMP("KP_Divide")) {
    return (long)KEYCODE_KP_Divide;
  } else if (KCMP("KP_Decimal")) {
    return (long)KEYCODE_KP_Decimal;
  } else if (KCMP("NonUSBackSlash") || KCMP("KP_BackSlash")) {
    // ISO key next to left Shift. KP_BackSlash is the old name, kept so
    // existing user vkm files still load.
    return (long)KEYCODE_NonUSBackSlash;
  } else if (KCMP("KP_Multiply")) {
    return (long)KEYCODE_KP_Multiply;
  } else if (KCMP("KP_Subtract")) {
    return (long)KEYCODE_KP_Subtract;
  } else if (KCMP("KP_Add")) {
    return (long)KEYCODE_KP_Add;
  } else if (KCMP("KP_Enter")) {
    return (long)KEYCODE_KP_Enter;
  } else if (KCMP("KP_1")) {
    return (long)KEYCODE_KP1;
  } else if (KCMP("KP_2")) {
    return (long)KEYCODE_KP2;
  } else if (KCMP("KP_3")) {
    return (long)KEYCODE_KP3;
  } else if (KCMP("KP_4")) {
    return (long)KEYCODE_KP4;
  } else if (KCMP("KP_5")) {
    return (long)KEYCODE_KP5;
  } else if (KCMP("KP_6")) {
    return (long)KEYCODE_KP6;
  } else if (KCMP("KP_7")) {
    return (long)KEYCODE_KP7;
  } else if (KCMP("KP_8")) {
    return (long)KEYCODE_KP8;
  } else if (KCMP("KP_9")) {
    return (long)KEYCODE_KP9;
  } else if (KCMP("KP_0")) {
    return (long)KEYCODE_KP0;
  }

  return 0;
}

const char *kbd_arch_keynum_to_keyname(signed long keynum) { return 0; }

void kbd_initialize_numpad_joykeys(int *joykeys) {}

