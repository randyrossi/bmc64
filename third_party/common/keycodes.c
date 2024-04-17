/*
 * keycodes.c
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

#include "keycodes.h"

// Must default these to sane values in case the emulator
// does not start and we do not discover what these
// keys are supposed to be.
signed long commodore_key_sym = KEYCODE_LeftControl;
signed long restore_key_sym = KEYCODE_PageUp;
signed long ctrl_key_sym = KEYCODE_Tab;
int commodore_key_sym_set;
int restore_key_sym_set;
int ctrl_key_sym_set;


raw_keycode_func_t raw_keycode_func = 0;

const char* keycode_to_string(long keycode) {
  switch (keycode) {
    case KEYCODE_a:
       return "a";
    case KEYCODE_b:
       return "b";
    case KEYCODE_c:
       return "c";
    case KEYCODE_d:
       return "d";
    case KEYCODE_e:
       return "e";
    case KEYCODE_f:
       return "f";
    case KEYCODE_g:
       return "g";
    case KEYCODE_h:
       return "h";
    case KEYCODE_i:
       return "i";
    case KEYCODE_j:
       return "j";
    case KEYCODE_k:
       return "k";
    case KEYCODE_l:
       return "l";
    case KEYCODE_m:
       return "m";
    case KEYCODE_n:
       return "n";
    case KEYCODE_o:
       return "o";
    case KEYCODE_p:
       return "p";
    case KEYCODE_q:
       return "q";
    case KEYCODE_r:
       return "r";
    case KEYCODE_s:
       return "s";
    case KEYCODE_t:
       return "t";
    case KEYCODE_u:
       return "u";
    case KEYCODE_v:
       return "v";
    case KEYCODE_w:
       return "w";
    case KEYCODE_x:
       return "x";
    case KEYCODE_y:
       return "y";
    case KEYCODE_z:
       return "z";
    case KEYCODE_1:
       return "1";
    case KEYCODE_2:
       return "2";
    case KEYCODE_3:
       return "3";
    case KEYCODE_4:
       return "4";
    case KEYCODE_5:
       return "5";
    case KEYCODE_6:
       return "6";
    case KEYCODE_7:
       return "7";
    case KEYCODE_8:
       return "8";
    case KEYCODE_9:
       return "9";
    case KEYCODE_0:
       return "0";
    case KEYCODE_Return:
       return "Return";
    case KEYCODE_Escape:
       return "Esc";
    case KEYCODE_Backspace:
       return "Backspace";
    case KEYCODE_Tab:
       return "Tab";
    case KEYCODE_Space:
       return "Space";
    case KEYCODE_Dash:
       return "-";
    case KEYCODE_Equals:
       return "=";
    case KEYCODE_LeftBracket:
       return "{";
    case KEYCODE_RightBracket:
       return "}";
    case KEYCODE_BackSlash:
       return "\\";
    case KEYCODE_Pound:
       return "Pound";
    case KEYCODE_SemiColon:
       return ";";
    case KEYCODE_SingleQuote:
       return "'";
    case KEYCODE_BackQuote:
       return "`";
    case KEYCODE_Comma:
       return ",";
    case KEYCODE_Period:
       return ".";
    case KEYCODE_Slash:
       return "/";
    case KEYCODE_CapsLock:
       return "CapsLock";
    case KEYCODE_F1:
       return "F1";
    case KEYCODE_F2:
       return "F2";
    case KEYCODE_F3:
       return "F3";
    case KEYCODE_F4:
       return "F4";
    case KEYCODE_F5:
       return "F5";
    case KEYCODE_F6:
       return "F6";
    case KEYCODE_F7:
       return "F7";
    case KEYCODE_F8:
       return "F8";
    case KEYCODE_F9:
       return "F9";
    case KEYCODE_F10:
       return "F10";
    case KEYCODE_F11:
       return "F11";
    case KEYCODE_F12:
       return "F12";
    case KEYCODE_ScrollLock:
       return "ScrollLock";
    case KEYCODE_Pause:
       return "Pause";
    case KEYCODE_Insert:
       return "Insert";
    case KEYCODE_Home:
       return "Home";
    case KEYCODE_PageUp:
       return "PgUp";
    case KEYCODE_Delete:
       return "Del";
    case KEYCODE_End:
       return "End";
    case KEYCODE_PageDown:
       return "PgDown";
    case KEYCODE_Right:
       return "Right";
    case KEYCODE_Left:
       return "Left";
    case KEYCODE_Down:
       return "Down";
    case KEYCODE_Up:
       return "Up";
    case KEYCODE_NumLock:
       return "NumLock";
    case KEYCODE_KP_Divide:
       return "KP Divide";
    case KEYCODE_KP_Multiply:
       return "KP Multiply";
    case KEYCODE_KP_Subtract:
       return "KP Subtract";
    case KEYCODE_KP_Add:
       return "KP Add";
    case KEYCODE_KP_Enter:
       return "KP Enter";
    case KEYCODE_KP1:
       return "KP 1";
    case KEYCODE_KP2:
       return "KP 2";
    case KEYCODE_KP3:
       return "KP 3";
    case KEYCODE_KP4:
       return "KP 4";
    case KEYCODE_KP5:
       return "KP 5";
    case KEYCODE_KP6:
       return "KP 6";
    case KEYCODE_KP7:
       return "KP 7";
    case KEYCODE_KP8:
       return "KP 8";
    case KEYCODE_KP9:
       return "KP 9";
    case KEYCODE_KP0:
       return "KP 0";
    case KEYCODE_KP_Decimal:
       return "KP Decimal";
    case KEYCODE_KP_BackSlash:
       return "KP BackSlash";
    case KEYCODE_Application:
       return "App";
    case KEYCODE_LeftShift:
       return "LeftShift";
    case KEYCODE_RightShift:
       return "RightShift";
    case KEYCODE_LeftControl:
       return "LeftControl";
    case KEYCODE_RightControl:
       return "RightControl";
    case KEYCODE_LeftAlt:
       return "LeftAlt";
    case KEYCODE_RightAlt:
       return "RightAlt";
    case KEYCODE_LeftSuper:
       return "LeftSuper";
    case KEYCODE_RightSuper:
       return "RightSuper";
    default:
       return "undefined";
  }
}
