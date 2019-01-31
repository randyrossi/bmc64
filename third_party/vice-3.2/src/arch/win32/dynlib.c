/*
 * dynlib.c - Win32 support for dynamic library loading.
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#include <windows.h>

#include "dynlib.h"

void *vice_dynlib_open(const char *name)
{
    return LoadLibrary(name);
}

void *vice_dynlib_symbol(void *handle,const char *name)
{
    return GetProcAddress((HMODULE)handle, name);
}

char *vice_dynlib_error(void)
{
    return "unknown";
}

int vice_dynlib_close(void *handle)
{
    if (FreeLibrary(handle)) {
        return 0;
    } else {
        return -1;
    }
}
