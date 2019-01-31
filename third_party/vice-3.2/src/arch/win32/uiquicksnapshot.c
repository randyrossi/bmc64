/*
 * uiquicksnapshot.c
 *
 * Written by
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

#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#include "archdep.h"
#include "interrupt.h"
#include "intl.h"
#include "lib.h"
#include "machine.h"
#include "res.h"
#include "snapshot.h"
#include "system.h"
#include "translate.h"
#include "uiapi.h"
#include "uiquicksnapshot.h"
#include "util.h"

typedef struct {
    char name[MAX_PATH];
    int valid;
} snapfiles;

static snapfiles files[10];
static int lastindex;
static int snapcounter;

static void scan_files(void)
{
    WIN32_FIND_DATA file_info;
    HANDLE search_handle;
    int i;
    char *dirname;
    TCHAR *st_dirname;

    dirname = util_concat(archdep_boot_path(), "\\", machine_name, "\\quicksnap?.vsf", NULL);
    st_dirname = system_mbstowcs_alloc(dirname);
    search_handle = FindFirstFile(st_dirname, &file_info);
    system_mbstowcs_free(st_dirname);
    snapcounter = 0;
    lastindex = -1;
    for (i = 0; i < 10; i++) {
        files[i].valid = 0;
    }
    if (search_handle != INVALID_HANDLE_VALUE) {
        do {
            TCHAR c;

            c = file_info.cFileName[_tcslen(file_info.cFileName) - 5];
            if ((c >= '0') && (c <= '9')) {
                strcpy(files[c - '0'].name, file_info.cFileName);
                files[c - '0'].valid = 1;
                if ((c - '0') > lastindex) {
                    lastindex = c - '0';
                }
                snapcounter++;
            }
        } while (FindNextFile(search_handle, &file_info));
        FindClose(search_handle);
    }
    lib_free(dirname);
}

static void save_quicksnapshot_trap(uint16_t unused_addr, void *unused_data)
{
    int i, j;
    char *fullname, *fullname2;
    TCHAR *st_fullname, *st_fullname2;

    if (lastindex == -1) {
        lastindex = 0;
        strcpy(files[lastindex].name, "quicksnap0.vsf");
    } else {
        if (lastindex == 9) {
            if (snapcounter == 10) {
                fullname = util_concat(archdep_boot_path(), "\\", machine_name, "\\", files[0].name, NULL);
                st_fullname = system_mbstowcs_alloc(fullname);
                DeleteFile(st_fullname);
                system_mbstowcs_free(st_fullname);
                lib_free(fullname);
                for (i = 1; i < 10; i++) {
                    fullname = util_concat(archdep_boot_path(), "\\", machine_name, "\\", files[i].name, NULL);
                    fullname2 = util_concat(archdep_boot_path(), "\\", machine_name, "\\", files[i - 1].name, NULL);
                    st_fullname = system_mbstowcs_alloc(fullname);
                    st_fullname2 = system_mbstowcs_alloc(fullname2);
                    MoveFile(st_fullname, st_fullname2);
                    system_mbstowcs_free(st_fullname);
                    system_mbstowcs_free(st_fullname2);
                    lib_free(fullname);
                    lib_free(fullname2);
                }
            } else {
                for (i = 0; i < 10; i++) {
                    if (files[i].valid == 0) {
                        break;
                    }
                }
                for (j = i + 1; j < 10; j++) {
                    if (files[j].valid) {
                        strcpy(files[i].name,files[j].name);
                        files[i].name[strlen(files[i].name) - 5] = '0' + i;
                        fullname = util_concat(archdep_boot_path(), "\\", machine_name, "\\", files[j].name, NULL);
                        fullname2 = util_concat(archdep_boot_path(), "\\", machine_name, "\\", files[i].name, NULL);
                        st_fullname = system_mbstowcs_alloc(fullname);
                        st_fullname2 = system_mbstowcs_alloc(fullname2);
                        MoveFile(st_fullname, st_fullname2);
                        system_mbstowcs_free(st_fullname);
                        system_mbstowcs_free(st_fullname2);
                        lib_free(fullname);
                        lib_free(fullname2);
                        i++;
                    }
                }
                strcpy(files[i].name,files[0].name);
                files[i].name[strlen(files[i].name) - 5] = '0' + i;
                lastindex = i;
            }
        } else {
            strcpy(files[lastindex + 1].name,files[lastindex].name);
            lastindex++;
            files[lastindex].name[strlen(files[lastindex].name) - 5] = '0' + lastindex;
        }
    }

    fullname = util_concat(archdep_boot_path(), "\\", machine_name, "\\", files[lastindex].name, NULL);
    if (machine_write_snapshot(fullname, 0, 0, 0) < 0) {
        snapshot_display_error();
    }
    lib_free(fullname);
}

static void load_quicksnapshot_trap(uint16_t unused_addr, void *unused_data)
{
    char *fullname;

    fullname = util_concat(archdep_boot_path(), "\\", machine_name, "\\", files[lastindex].name, NULL);
    if (machine_read_snapshot(fullname, 0) < 0) {
        snapshot_display_error();
    }
    lib_free(fullname);
}

void ui_quicksnapshot_load(HWND hwnd)
{
    scan_files();
    if (snapcounter > 0) {
        interrupt_maincpu_trigger_trap(load_quicksnapshot_trap, (void *)0);
    }
}

void ui_quicksnapshot_save(HWND hwnd)
{
    scan_files();
    interrupt_maincpu_trigger_trap(save_quicksnapshot_trap, (void *)0);
}
