/*
 * kbd.c - Keyboard emulation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@mail.matav.hu>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kbd.h"
#include "keyboard.h"
#include "types.h"
#include "winkbd.h"


uint8_t _kbd_extended_key_tab[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, K_KPENTER, K_RIGHTCTRL, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, K_KPDIV, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, K_NUMLOCK, 0, K_HOME, K_UP, K_PGUP, 0, K_LEFT, 0, K_RIGHT, 0, K_END,
    K_DOWN, K_PGDOWN, K_INS, K_DEL, 0, 0, 0, 0, 0, 0, 0, K_LEFTW95, K_RIGHTW95, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* ------------------------------------------------------------------------ */

/* Windows would not want us to handle raw scancodes like this...  But we
   need it nevertheless.  */

int kbd_handle_keydown(uint32_t virtual_key, uint32_t key_data)
{
    long kcode = (key_data >> 16) & 0xff;
    int repeated = key_data & 0x40000000 ? 1 : 0;

    if (virtual_key == 0xff) {
        return 0;
    }

    /*  Translate Extended scancodes */
    if (key_data & (1 << 24)) {
        kcode = _kbd_extended_key_tab[kcode];
    }

    if ( ! repeated ) {
        keyboard_key_pressed(kcode);
    }

    return 0;
}

int kbd_handle_keyup(uint32_t virtual_key, uint32_t key_data)
{
    long kcode = (key_data >> 16) & 0xff;
    int repeated = key_data & 0x40000000 ? 0 : 1;

    if (virtual_key == 0xff) {
        return 0;
    }

    /*  Translate Extended scancodes */
    if (key_data & (1 << 24)) {
        kcode = _kbd_extended_key_tab[kcode];
    }

    if ( ! repeated ) {
        keyboard_key_released(kcode);
    }

    return 0;
}

const TCHAR *kbd_code_to_string(kbd_code_t kcode)
{
    static TCHAR *tab[256] = {
        TEXT("None"), TEXT("Esc"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"),
        TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"), TEXT("0"),
        TEXT("-"), TEXT("="), TEXT("Backspace"), TEXT("Tab"), TEXT("Q"),
        TEXT("W"), TEXT("E"), TEXT("R"), TEXT("T"), TEXT("Y"), TEXT("U"),
        TEXT("I"), TEXT("O"), TEXT("P"), TEXT("{"), TEXT("}"), TEXT("Enter"),
        TEXT("Left Ctrl"), TEXT("A"), TEXT("S"), TEXT("D"), TEXT("F"),
        TEXT("G"), TEXT("H"), TEXT("J"), TEXT("K"), TEXT("L"), TEXT(";"),
        TEXT("'"), TEXT("`"), TEXT("Left Shift"), TEXT("\\"), TEXT("Z"),
        TEXT("X"), TEXT("C"), TEXT("V"), TEXT("B"), TEXT("N"), TEXT("M"),
        TEXT(","), TEXT("."), TEXT("/"), TEXT("Right Shift"), TEXT("Numpad *"),
        TEXT("Left Alt"), TEXT("Space"), TEXT("Caps Lock"), TEXT("F1"),
        TEXT("F2"), TEXT("F3"), TEXT("F4"), TEXT("F5"), TEXT("F6"), TEXT("F7"),
        TEXT("F8"), TEXT("F9"), TEXT("F10"), TEXT("Num Lock"),
        TEXT("Scroll Lock"), TEXT("Numpad 7"), TEXT("Numpad 8"), TEXT("Numpad 9"),
        TEXT("Numpad -"), TEXT("Numpad 4"), TEXT("Numpad 5"), TEXT("Numpad 6"),
        TEXT("Numpad +"), TEXT("Numpad 1"), TEXT("Numpad 2"), TEXT("Numpad 3"),
        TEXT("Numpad 0"), TEXT("Numpad ."), TEXT("SysReq"), TEXT("85"), TEXT("86"),
        TEXT("F11"), TEXT("F12"), TEXT("Home"), TEXT("Up"), TEXT("PgUp"),
        TEXT("Left"), TEXT("Right"), TEXT("End"), TEXT("Down"), TEXT("PgDown"),
        TEXT("Ins"), TEXT("Del"), TEXT("Numpad Enter"), TEXT("Right Ctrl"),
        TEXT("Pause"), TEXT("PrtScr"), TEXT("Numpad /"), TEXT("Right Alt"),
        TEXT("Break"), TEXT("Left Win95"), TEXT("Right Win95")
    };

    return tab[(int)kcode];
}

/* ------------------------------------------------------------------------ */

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

/* returns host keyboard mapping. used to initialize the keyboard map when
   starting with a black (default) config, so an educated guess works good
   enough most of the time :)

   FIXME: add more languages, constants are defined in winnt.h

   https://msdn.microsoft.com/en-us/library/windows/desktop/dd318693%28v=vs.85%29.aspx
*/
int kbd_arch_get_host_mapping(void)
{
    int n;
    int maps[KBD_MAPPING_NUM] = {
        KBD_MAPPING_US, KBD_MAPPING_UK, KBD_MAPPING_DE, KBD_MAPPING_DA,
        KBD_MAPPING_NO, KBD_MAPPING_FI, KBD_MAPPING_IT };
    int langids[KBD_MAPPING_NUM] = {
        MAKELANGID(LANG_ENGLISH,   SUBLANG_ENGLISH_US),
        MAKELANGID(LANG_ENGLISH,   SUBLANG_ENGLISH_UK),
        MAKELANGID(LANG_GERMAN,    SUBLANG_GERMAN),
        MAKELANGID(LANG_DANISH,    SUBLANG_DANISH_DENMARK),
        MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL),
        MAKELANGID(LANG_FINNISH,   SUBLANG_FINNISH_FINLAND),
        MAKELANGID(LANG_ITALIAN,   SUBLANG_ITALIAN)
    };
    int lang = (int)LOWORD(GetKeyboardLayout(0));

    /* try full match first */
    lang &= 0xffff; /* lower 16 bit contain the language id */
    for (n = 0; n < KBD_MAPPING_NUM; n++) {
        if (lang == langids[n]) {
            return maps[n];
        }
    }
    /* try only primary language */
    lang &= 0x3ff; /* lower 10 bit contain the primary language id */
    for (n = 0; n < KBD_MAPPING_NUM; n++) {
        if (lang == (langids[n] & 0x3ff)) {
            return maps[n];
        }
    }
    return KBD_MAPPING_US;
}
