/*
 * intl.h - Localization routines for Win32.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifndef VICE_INTL_H
#define VICE_INTL_H

#include <windows.h>
#include <tchar.h>
#include <windef.h>

extern TCHAR *intl_speed_at_text;

extern char *intl_translate_text(int en_resource);
#ifdef WIN32_UNICODE_SUPPORT
extern WCHAR *intl_translate_wcs(int en_resource);
#define intl_translate_tcs intl_translate_wcs
#else
#define intl_translate_tcs intl_translate_text
#endif
extern int intl_translate_res(int en_resource);

extern void intl_init(void);
extern void intl_shutdown(void);
extern char *intl_arch_language_init(void);
extern void intl_update_ui(void);
extern char *intl_convert_cp(char *text, int cp);

#endif
