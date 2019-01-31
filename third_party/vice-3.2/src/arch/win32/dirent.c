/*
 * dirent.c - Fake implementation of dirent stuff for Windows.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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
/* This was inspired by Mathias Ortmann's `posixemu.c', used in the Windows
   port of UAE, the <insert your favourite adjective starting by `U' here>
   Amiga Emulator.  */

#include "vice.h"

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

#ifdef HAVE_IO_H
#include <io.h>
#endif

#include "dirent.h"
#include "lib.h"
#include "system.h"
#include "util.h"

struct _vice_dir {
    WIN32_FIND_DATA find_data;
    HANDLE handle;
    int first_passed;
    char *filter;
};

DIR *opendir(const char *path)
{
    DIR *dir;
    TCHAR *st_filter;

    dir = lib_malloc(sizeof(DIR));
    dir->filter = util_concat(path, "\\*", NULL);

    st_filter = system_mbstowcs_alloc(dir->filter);
    dir->handle = FindFirstFile(st_filter, &dir->find_data);
    system_mbstowcs_free(st_filter);
    if (dir->handle == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    dir->first_passed = 0;
    return dir;
}

struct dirent *readdir(DIR *dir)
{
    static struct dirent ret;

    if (dir->first_passed) {
        if (!FindNextFile(dir->handle, &dir->find_data)) {
            return NULL;
        }
    }

    dir->first_passed = 1;
    ret.d_name = dir->find_data.cFileName;
    ret.d_namlen = (int)strlen(ret.d_name);

    return &ret;
}

void closedir(DIR *dir)
{
    FindClose(dir->handle);
    lib_free(dir->filter);
    lib_free(dir);
}
