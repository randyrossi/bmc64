/*
 * archdep_win32.c - Miscellaneous system-specific stuff.
 *
 * Written by
 *  Tibor Biczo <crown@mail.matav.hu>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "vice.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <winsock.h>
#include <tchar.h>

#include "ui.h"

#ifdef HAVE_DIR_H
#include <dir.h>
#endif

#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if defined(HAVE_IO_H) && !defined(WINMIPS)
#include <io.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "archdep.h"
#include "ioutil.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "util.h"

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#ifndef O_BINARY
#define O_BINARY 0x8000
#endif

#ifndef _O_BINARY
#define _O_BINARY O_BINARY
#endif

#ifndef _O_WRONLY
#define _O_WRONLY O_WRONLY
#endif

#ifndef _O_TRUNC
#define _O_TRUNC O_TRUNC
#endif

#ifndef _O_CREAT
#define _O_CREAT O_CREAT
#endif

#ifndef S_IWRITE
#define S_IWRITE 0x200
#endif

#ifndef S_IREAD
#define S_IREAD 0x400
#endif

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


static char *argv0;


static char *system_mbstowcs_alloc(const char *mbs)
{
    char *wcs;
    size_t len;

    if (mbs == NULL) {
        return NULL;
    }

    len = strlen(mbs);

    wcs = lib_malloc(len + 1);
    return memcpy(wcs, mbs, len + 1);
}

static void system_mbstowcs_free(char *wcs)
{
    lib_free(wcs);
}


static int archdep_init_extra(int *argc, char **argv)
{
    _fmode = O_BINARY;

    _setmode(_fileno(stdin), O_BINARY);
    _setmode(_fileno(stdout), O_BINARY);

    argv0 = lib_stralloc(argv[0]);

    return 0;
}


char *archdep_default_hotkey_file_name(void)
{
    return util_concat(archdep_boot_path(), "\\sdl-hotkey-", machine_get_name(), ".vkm", NULL);
}

char *archdep_default_joymap_file_name(void)
{
    return util_concat(archdep_boot_path(), "\\sdl-joymap-", machine_get_name(), ".vjm", NULL);
}


int archdep_default_logger(const char *level_string, const char *txt)
{
    TCHAR *st_out;
    char *out = lib_msprintf("*** %s %s\n", level_string, txt);

    st_out = system_mbstowcs_alloc(out);
    OutputDebugString(st_out);
    system_mbstowcs_free(st_out);
    lib_free(out);
    return 0;
}

/* FIXME: do we still support Watcom? --compyx */
#ifndef WATCOM_COMPILE
#ifndef _S_IREAD
#define _S_IREAD S_IREAD
#endif
#ifndef _S_IWRITE
#define _S_IWRITE S_IWRITE
#endif
#endif

int archdep_spawn(const char *name, char **argv, char **pstdout_redir, const char *stderr_redir)
{
#ifndef WATCOM_COMPILE
    int new_stdout, new_stderr;
    int old_stdout_mode, old_stderr_mode;
    int old_stdout, old_stderr;
    int retval;
    char *stdout_redir = NULL;

    if (pstdout_redir != NULL) {
        if (*pstdout_redir == NULL) {
            *pstdout_redir = archdep_tmpnam();
        }
        stdout_redir = *pstdout_redir;
    }

    new_stdout = new_stderr = old_stdout = old_stderr = -1;

    /* Make sure we are in binary mode.  */
    old_stdout_mode = _setmode(STDOUT_FILENO, _O_BINARY);
    old_stderr_mode = _setmode(STDERR_FILENO, _O_BINARY);

    /* Redirect stdout and stderr as requested, saving the old
       descriptors.  */
    if (stdout_redir != NULL) {
        old_stdout = _dup(STDOUT_FILENO);
        new_stdout = _open(stdout_redir, _O_WRONLY | _O_TRUNC | _O_CREAT, _S_IWRITE | _S_IREAD);
        if (new_stdout == -1) {
            log_error(LOG_DEFAULT, "open(\"%s\") failed: %s.", stdout_redir, strerror(errno));
            retval = -1;
            goto cleanup;
        }
        _dup2(new_stdout, STDOUT_FILENO);
    }
    if (stderr_redir != NULL) {
        old_stderr = _dup(STDERR_FILENO);
        new_stderr = _open(stderr_redir, _O_WRONLY | _O_TRUNC | _O_CREAT, _S_IWRITE | _S_IREAD);
        if (new_stderr == -1) {
            log_error(LOG_DEFAULT, "open(\"%s\") failed: %s.", stderr_redir, strerror(errno));
            retval = -1;
            goto cleanup;
        }
        _dup2(new_stderr, STDERR_FILENO);
    }

    /* Spawn the child process.  */
    retval = (int)_spawnvp(_P_WAIT, name, (const char **)argv);

cleanup:
    if (old_stdout >= 0) {
        _dup2(old_stdout, STDOUT_FILENO);
        _close(old_stdout);
    }
    if (old_stderr >= 0) {
        _dup2(old_stderr, STDERR_FILENO);
        _close(old_stderr);
    }
    if (old_stdout_mode >= 0) {
        _setmode(STDOUT_FILENO, old_stdout_mode);
    }
    if (old_stderr_mode >= 0) {
        _setmode(STDERR_FILENO, old_stderr_mode);
    }
    if (new_stdout >= 0) {
        _close(new_stdout);
    }
    if (new_stderr >= 0) {
        _close(new_stderr);
    }

    return retval;
#else
    return -1;
#endif
}

#if 0
char *archdep_tmpnam(void)
{
    if (getenv("temp")) {
        return util_concat(getenv("temp"), tmpnam(NULL), NULL);
    } else if (getenv("tmp")) {
        return util_concat(getenv("tmp"), tmpnam(NULL), NULL);
    } else {
        return lib_stralloc(tmpnam(NULL));
    }
}
#endif

FILE *archdep_mkstemp_fd(char **filename, const char *mode)
{
    char *tmp;
    FILE *fd;

    if (getenv("temp")) {
        tmp = util_concat(getenv("temp"), tmpnam(NULL), NULL);
    } else if (getenv("tmp")) {
        tmp = util_concat(getenv("tmp"), tmpnam(NULL), NULL);
    } else {
        tmp = lib_stralloc(tmpnam(NULL));
    }

    fd = fopen(tmp, mode);

    if (fd == NULL) {
        return NULL;
    }

    *filename = tmp;

    return fd;
}


/* set permissions of given file to rw, respecting current umask */
int archdep_fix_permissions(const char *file_name)
{
    return _chmod(file_name, _S_IREAD | _S_IWRITE);
}


#ifdef SDL_CHOOSE_DRIVES
char **archdep_list_drives(void)
{
    DWORD bits, mask;
    int drive_count = 1, i = 0;
    char **result, **p;

    bits = GetLogicalDrives();
    mask = 1;
    while (mask != 0) {
        if (bits & mask) {
            ++drive_count;
        }
        mask <<= 1;
    }
    result = lib_malloc(sizeof(char*) * drive_count);
    p = result;
    mask = 1;
    while (mask != 0) {
        if (bits & mask) {
            char buf[16];
            sprintf(buf, "%c:/", 'a' + i);
            *p++ = lib_stralloc(buf);
        }
        mask <<= 1;
        ++i;
    }
    *p = NULL;

    return result;
}

char *archdep_get_current_drive(void)
{
    char *p = ioutil_current_dir();
    char *p2 = strchr(p, '\\');
    p2[0] = '/';
    p2[1] = '\0';
    return p;
}

void archdep_set_current_drive(const char *drive)
{
    if (_chdir(drive)) {
        ui_error("Failed to change drive to %s", drive);
    }
}
#endif

int archdep_require_vkbd(void)
{
    return 0;
}


static void archdep_shutdown_extra(void)
{
    lib_free(argv0);
}

void archdep_workaround_nop(const char *otto)
{
}

int archdep_rtc_get_centisecond(void)
{
    SYSTEMTIME t;

    GetSystemTime(&t);
    return (int)(t.wMilliseconds / 10);
}

#if defined(_MSC_VER) && !defined(WATCOM_COMPILE)
#include "dirent.h"

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
#endif

int is_windows_nt(void)
{
    OSVERSIONINFO os_version_info;

    ZeroMemory(&os_version_info, sizeof(os_version_info));
    os_version_info.dwOSVersionInfoSize = sizeof(os_version_info);

    GetVersionEx(&os_version_info);

    if (os_version_info.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        return 1;
    }
    return 0;
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
        KBD_MAPPING_NO, KBD_MAPPING_FI, KBD_MAPPING_IT, KBD_MAPPING_NL };
    int langids[KBD_MAPPING_NUM] = {
        MAKELANGID(LANG_ENGLISH,    SUBLANG_ENGLISH_US),
        MAKELANGID(LANG_ENGLISH,    SUBLANG_ENGLISH_UK),
        MAKELANGID(LANG_GERMAN,     SUBLANG_GERMAN),
        MAKELANGID(LANG_DANISH,     SUBLANG_DANISH_DENMARK),
        MAKELANGID(LANG_NORWEGIAN,  SUBLANG_NORWEGIAN_BOKMAL),
        MAKELANGID(LANG_FINNISH,    SUBLANG_FINNISH_FINLAND),
        MAKELANGID(LANG_ITALIAN,    SUBLANG_ITALIAN),
        MAKELANGID(LANG_DUTCH,      SUBLANG_DUTCH)
    };

    /* GetKeyboardLayout returns a pointer, but the first 16 bits of it return a 'language identfier',
     * whatever that is. This is seriously fucked */
    uint64_t lang = (uint64_t)(void *)GetKeyboardLayout(0);

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

/* Provide a usleep replacement */
void vice_usleep(uint64_t waitTime)
{ 
    uint64_t time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < waitTime);
}
