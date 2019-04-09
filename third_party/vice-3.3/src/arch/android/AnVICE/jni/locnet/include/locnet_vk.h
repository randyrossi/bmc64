/*
 * locnet_vk.h
 *
 * Written by
 *  Locnet <android.locnet@gmail.com>
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

typedef enum {
    K_NONE, K_ESC, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0, K_MINUS,   /*11*/
    K_EQUAL, K_BS, K_TAB, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P,     /*24*/
    K_LEFTBR, K_RIGHTBR, K_ENTER, K_LEFTCTRL, K_A, K_S, K_D, K_F, K_G, K_H,     /*35*/
    K_J, K_K, K_L, K_SEMICOLON, K_GRAVE, K_NUMSGN, K_LEFTSHIFT, K_LTGT, K_Z,    /*44*/
    K_X, K_C, K_V, K_B, K_N, K_M, K_COMMA, K_PERIOD, K_SLASH, K_RIGHTSHIFT,     /*54*/
    K_KPMULT, K_LEFTALT, K_SPACE, K_CAPSLOCK, K_F1, K_F2, K_F3, K_F4, K_F5,     /*63*/
    K_F6, K_F7, K_F8, K_F9, K_F10, K_NUMLOCK, K_SCROLLOCK, K_KP7, K_KP8, K_KP9, /*73*/
    K_KPMINUS, K_KP4, K_KP5, K_KP6, K_KPPLUS, K_KP1, K_KP2, K_KP3, K_KP0,       /*82*/
    K_KPDOT, K_PRTSCR, K_85, K_86, K_F11, K_F12, K_KPENTER, K_RIGHTCTRL,        /*90*/
    K_PAUSE, K_SYSREQ, K_KPDIV, K_RIGHTALT, K_BREAK, K_HOME, K_UP, K_PGUP,      /*98*/
    K_LEFT, K_RIGHT, K_END, K_DOWN, K_PGDOWN, K_INS, K_DEL,                     /*105*/
    K_LEFTW95, K_RIGHTW95                                                       /*107*/
} kbd_code_t;

#define KBD_esc         27
#define KBD_1           49
#define KBD_2           50
#define KBD_3           51
#define KBD_4           52
#define KBD_5           53
#define KBD_6           54
#define KBD_7           55
#define KBD_8           56
#define KBD_9           57
#define KBD_0           48
#define KBD_plus        SDLK_PLUS		//43	should use SDLK_MINUS, bug in sdl_sym.vkm?
#define KBD_minus       SDLK_EQUALS		//61
#define KBD_backspace   8
#define KBD_leftctrl    SDLK_TAB		//9
#define KBD_a           97
#define KBD_b           98
#define KBD_c           99
#define KBD_d           100
#define KBD_e           101
#define KBD_f           102
#define KBD_g           103
#define KBD_h           104
#define KBD_i           105
#define KBD_j           106
#define KBD_k           107
#define KBD_l           108
#define KBD_m           109
#define KBD_n           110
#define KBD_o           111
#define KBD_p           112
#define KBD_q           113
#define KBD_r           114
#define KBD_s           115
#define KBD_t           116
#define KBD_u           117
#define KBD_v           118
#define KBD_w           119
#define KBD_x           120
#define KBD_y           121
#define KBD_z           122
#define KBD_at          SDLK_LEFTBRACKET		//91
#define KBD_star        SDLK_RIGHTBRACKET		//93
#define KBD_enter       13
#define KBD_cbm         SDLK_LCTRL			//306
#define KBD_colon       SDLK_SEMICOLON		//59
#define KBD_semicolon   SDLK_QUOTE			//39
#define KBD_left_arrow  SDLK_BACKQUOTE		//96
#define KBD_equals      SDLK_BACKSLASH		//92
#define KBD_leftshift   304
#define KBD_comma       44
#define KBD_period      46
#define KBD_slash       47
#define KBD_space       32
#define KBD_f1          282
#define KBD_f2          283
#define KBD_f3          284
#define KBD_f4          285
#define KBD_f5          286
#define KBD_f6          287
#define KBD_f7          288
#define KBD_f8          289
#define KBD_f9          290
#define KBD_f10         291
#define KBD_f11         292
#define KBD_f12         293
#define KBD_home        278
#define KBD_up          273
#define KBD_left        276
#define KBD_right       275
#define KBD_down        274
#define KBD_pound       SDLK_INSERT			//277
#define KBD_up_arrow    SDLK_DELETE			//127
#define KBD_restore     SDLK_PAGEUP			//280
