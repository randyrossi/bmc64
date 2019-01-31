/*
 * uicart.h
 *
 * Written by
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

#ifndef VICE_UICART_H
#define VICE_UICART_H

#include <windows.h>
#include <tchar.h>

struct uicart_params_s {
    WPARAM wparam;
    int type;
    int title;
    char *trans_title;
    DWORD filter;
};
typedef struct uicart_params_s uicart_params_t;

extern void uicart_attach(WPARAM wparam, HWND hwnd, const uicart_params_t *cartridges);
extern void uicart_attach_special(HWND hwnd, const TCHAR *title, DWORD filterlist, unsigned int type);

#endif
