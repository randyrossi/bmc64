/*
 * kbd.c - Keyboard emulation for BeOS.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "joy.h"
#include "kbd.h"
#include "keyboard.h"

int kbd_handle_keydown(int kcode)
{
    if (!joystick_handle_key(kcode, 1)) {
        keyboard_key_pressed((signed long)kcode);
    }

    return 0;
}

int kbd_handle_keyup(int kcode)
{
    if (!joystick_handle_key(kcode, 0)) {
        keyboard_key_released((signed long)kcode);
    }

    return 0;
}

const char *kbd_code_to_string(int kcode)
{
    static char *tab[128] = {
        "<None>", "Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", 
        "F11", "F12", "PrtScr", "Scroll Lock", "Pause", "`", "1", "2", "3", "4",
        "5", "6", "7", "8", "9", "0", "-", "=", "Backspace", "Ins", "Home",
        "PgUp", "Num Lock", "Numpad /", "Numpad *", "Numpad -", "Tab", "Q", "W", "E", "R", 
        "T", "Y", "U", "I", "O", "P", "{", "}", "\\", "Del", "End",
        "PgDown", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad +", "Caps Lock", "A", "S", "D", "F",
        "G", "H", "J", "K", "L", ";", "'", "Enter", "Numpad 4", "Numpad 5", "Numpad 6",
        "Left Shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", 
        "Right Shift", "Up", "Numpad 1", "Numpad 2", "Numpad 3", "Numpad Enter", "Left Ctrl", "Left ALt",
        "Space", "Right Alt", "Right Ctrl", "Left", "Down", "Right", "Numpad 0", "Numpad Dot",
        "Left Opt", "Right Opt", "Menu", "Euro", "Numpad =", "Power"
    };

    if (kcode < 0 || kcode >= 128) {
        return NULL;
    }
    return tab[(int)kcode];
}

/* ------------------------------------------------------------------------ */

void kbd_initialize_numpad_joykeys(int* joykeys)
{
    joykeys[0] = K_RIGHTCTRL;
    joykeys[1] = K_KP1;
    joykeys[2] = K_KP2;
    joykeys[3] = K_KP3;
    joykeys[4] = K_KP4;
    joykeys[5] = K_KP6;
    joykeys[6] = K_KP7;
    joykeys[7] = K_KP8;
    joykeys[8] = K_KP9;
}

void kbd_arch_init(void)
{
}

signed long kbd_arch_keyname_to_keynum(char *keyname)
{
    return (signed long)atoi(keyname);
}

const char *kbd_arch_keynum_to_keyname(signed long keynum)
{
    static char keyname[20];

    memset(keyname, 0, 20);

    sprintf(keyname, "%li", keynum);

    return keyname;
}

/* returns host keyboard mapping. used to initialize the keyboard map when
   starting with a black (default) config, so an educated guess works good
   enough most of the time :)

   FIXME: add more languages/actual detection
*/
int kbd_arch_get_host_mapping(void)
{
    return KBD_MAPPING_US;
}
