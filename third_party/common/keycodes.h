/*
 * keycodes.h
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

#ifndef RASPI_KEYCODES_H
#define RASPI_KEYCODES_H

// These are set according to the keyboard mapping for each emualtor
// so the UI and other functions know what keys the user wanted these
// to be instead of hard coded values.
extern signed long commodore_key_sym;
extern signed long restore_key_sym;
extern signed long ctrl_key_sym;

// Used to track whether these are set.
extern int commodore_key_sym_set;
extern int restore_key_sym_set;
extern int ctrl_key_sym_set;

#define KEYCODE_NONE 0x00
#define KEYCODE_a 0x04
#define KEYCODE_b 0x05
#define KEYCODE_c 0x06
#define KEYCODE_d 0x07
#define KEYCODE_e 0x08
#define KEYCODE_f 0x09
#define KEYCODE_g 0x0A
#define KEYCODE_h 0x0B
#define KEYCODE_i 0x0C
#define KEYCODE_j 0x0D
#define KEYCODE_k 0x0E
#define KEYCODE_l 0x0F
#define KEYCODE_m 0x10
#define KEYCODE_n 0x11
#define KEYCODE_o 0x12
#define KEYCODE_p 0x13
#define KEYCODE_q 0x14
#define KEYCODE_r 0x15
#define KEYCODE_s 0x16
#define KEYCODE_t 0x17
#define KEYCODE_u 0x18
#define KEYCODE_v 0x19
#define KEYCODE_w 0x1A
#define KEYCODE_x 0x1B
#define KEYCODE_y 0x1C
#define KEYCODE_z 0x1D
#define KEYCODE_1 0x1E
#define KEYCODE_2 0x1F
#define KEYCODE_3 0x20
#define KEYCODE_4 0x21
#define KEYCODE_5 0x22
#define KEYCODE_6 0x23
#define KEYCODE_7 0x24
#define KEYCODE_8 0x25
#define KEYCODE_9 0x26
#define KEYCODE_0 0x27
#define KEYCODE_Return 0x28
#define KEYCODE_Escape 0x29
#define KEYCODE_Backspace 0x2A
#define KEYCODE_Tab 0x2B
#define KEYCODE_Space 0x2C
#define KEYCODE_Dash 0x2D
#define KEYCODE_Equals 0x2E
#define KEYCODE_LeftBracket 0x2F
#define KEYCODE_RightBracket 0x30
#define KEYCODE_BackSlash 0x31
#define KEYCODE_Pound 0x32
#define KEYCODE_SemiColon 0x33
#define KEYCODE_SingleQuote 0x34
#define KEYCODE_BackQuote 0x35
#define KEYCODE_Comma 0x36
#define KEYCODE_Period 0x37
#define KEYCODE_Slash 0x38
#define KEYCODE_CapsLock 0x39
#define KEYCODE_F1 0x3A
#define KEYCODE_F2 0x3B
#define KEYCODE_F3 0x3C
#define KEYCODE_F4 0x3D
#define KEYCODE_F5 0x3E
#define KEYCODE_F6 0x3F
#define KEYCODE_F7 0x40
#define KEYCODE_F8 0x41
#define KEYCODE_F9 0x42
#define KEYCODE_F10 0x43
#define KEYCODE_F11 0x44
#define KEYCODE_F12 0x45
#define KEYCODE_ScrollLock 0x47
#define KEYCODE_Pause 0x48
#define KEYCODE_Insert 0x49
#define KEYCODE_Home 0x4A
#define KEYCODE_PageUp 0x4B
#define KEYCODE_Delete 0x4C
#define KEYCODE_End 0x4D
#define KEYCODE_PageDown 0x4E
#define KEYCODE_Right 0x4F
#define KEYCODE_Left 0x50
#define KEYCODE_Down 0x51
#define KEYCODE_Up 0x52
#define KEYCODE_NumLock 0x53
#define KEYCODE_KP_Divide 0x54
#define KEYCODE_KP_Multiply 0x55
#define KEYCODE_KP_Subtract 0x56
#define KEYCODE_KP_Add 0x57
#define KEYCODE_KP_Enter 0x58
#define KEYCODE_KP1 0x59
#define KEYCODE_KP2 0x5a
#define KEYCODE_KP3 0x5b
#define KEYCODE_KP4 0x5c
#define KEYCODE_KP5 0x5d
#define KEYCODE_KP6 0x5e
#define KEYCODE_KP7 0x5f
#define KEYCODE_KP8 0x60
#define KEYCODE_KP9 0x61
#define KEYCODE_KP0 0x62
#define KEYCODE_KP_Decimal 0x63
#define KEYCODE_KP_BackSlash 0x64
#define KEYCODE_Application 0x65

#define KEYCODE_LeftShift 0x100
#define KEYCODE_RightShift 0x101
#define KEYCODE_LeftControl 0x102
#define KEYCODE_RightControl 0x103
#define KEYCODE_LeftAlt 0x104
#define KEYCODE_RightAlt 0x105
#define KEYCODE_LeftSuper 0x106
#define KEYCODE_RightSuper 0x107

// If not 0, will intercept all usb key events and
// forward to this function.  Used to listen to keys
// during keyset or key binding assignments.
typedef void (*raw_keycode_func_t)(long key);

extern raw_keycode_func_t raw_keycode_func;
const char* keycode_to_string(long keycode);

#endif
