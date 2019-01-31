/*
 * winkbd.h - Joystick support for Windows.
 *
 * Written by
 *  Spiro Trikaliotis
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

#ifndef VICE_WINKBD_H
#define VICE_WINKBD_H

#include <windows.h>

extern const TCHAR *kbd_code_to_string(kbd_code_t kcode);

#ifndef LANG_ENGLISH
#define LANG_ENGLISH 0x09
#endif

#ifndef SUBLANG_ENGLISH_US
#define SUBLANG_ENGLISH_US 0x01
#endif

#ifndef SUBLANG_ENGLISH_UK
#define SUBLANG_ENGLISH_UK 0x02
#endif

#ifndef LANG_GERMAN
#define LANG_GERMAN 0x07
#endif

#ifndef SUBLANG_GERMAN
#define SUBLANG_GERMAN 0x01
#endif

#ifndef LANG_DANISH
#define LANG_DANISH 0x06
#endif

#ifndef SUBLANG_DANISH_DENMARK
#define SUBLANG_DANISH_DENMARK 0x01
#endif

#ifndef LANG_NORWEGIAN
#define LANG_NORWEGIAN  0x14
#endif

#ifndef SUBLANG_NORWEGIAN_BOKMAL
#define SUBLANG_NORWEGIAN_BOKMAL 0x01
#endif

#ifndef LANG_FINNISH
#define LANG_FINNISH 0x0b
#endif

#ifndef SUBLANG_FINNISH_FINLAND
#define SUBLANG_FINNISH_FINLAND 0x01
#endif

#ifndef LANG_ITALIAN
#define LANG_ITALIAN 0x10
#endif

#ifndef SUBLANG_ITALIAN
#define SUBLANG_ITALIAN 0x01
#endif

#endif
