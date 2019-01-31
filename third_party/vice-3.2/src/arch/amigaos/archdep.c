/*
 * archdep.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#define __USE_INLINE__

#include <proto/dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef AMIGA_M68K
#define CLIB_SOCKET_PROTOS_H
#endif

#include "archdep.h"
#include "findpath.h"
#include "ioutil.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "ui.h"
#include "util.h"

#ifdef AMIGA_M68K
#include <math.h>

double log1p(double x)
{
    return log(1 + x) - (((1 + x) - 1) - x) / (1 + x);
}
#endif

#if defined(AMIGA_M68K) && !defined(HAVE_GETTIMEOFDAY)
struct Library *TimerBase = NULL;
struct MsgPort *TimerMP = NULL;
struct timerequest *TimerIO = NULL;

static void gettimeofday_init(void)
{
    if (TimerMP = CreatePort(NULL, NULL)) {
        if (TimerIO = (struct timerequest *)CreateExtIO(TimerMP, sizeof(struct timerequest))) {
            if (!(OpenDevice(TIMERNAME, UNIT_MICROHZ, TimerIO, 0))) {
                TimerBase = (struct Library *)TimerIO->tr_node.io_Device;
            }
        }
    }
}

static void gettimeofday_shutdown(void)
{
    if (TimerIO) {
        CloseDevice(TimerIO);
        DeleteExtIO(TimerIO);
    }

    if (TimerMP) {
      DeletePort(TimerMP);
    }
    TimerBase = NULL;
}
#endif


/** \brief  Tokens that are illegal in a path/filename
 *
 */
static const char *illegal_name_tokens = "/?*:";


static char *boot_path = NULL;
static int run_from_wb = 0;

int archdep_init(int *argc, char **argv)
{
    if (*argc == 0) { /* run from WB */
        run_from_wb = 1;
    } else { /* run from CLI */
        run_from_wb = 0;
    }

#if defined(AMIGA_M68K) && !defined(HAVE_GETTIMEOFDAY)
    gettimeofday_init();
#endif

    return 0;
}

char *archdep_program_name(void)
{
    static char *program_name = NULL;

    if (program_name == NULL) {
        char *p, name[1024];

#ifndef GetProgramName
        GetCliProgramName(name, 1024);
#else
        GetProgramName(name, 1024);
#endif
        p = FilePart(name);

        if (p != NULL) {
            program_name = lib_stralloc(p);
        }
    }

    return program_name;
}

const char *archdep_boot_path(void)
{
    if (boot_path == NULL) {
        char cwd[1024];
        BPTR lock;

        lock = GetProgramDir();
        if (NameFromLock(lock, cwd, 1024)) {
            if (cwd[strlen(cwd) - 1] != ':') {
                strcat(cwd, "/");
            }
            boot_path = lib_stralloc(cwd);
        }
    }

    return boot_path;
}

char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    static char *default_path;

    if (default_path == NULL) {
      const char *boot_path;

      boot_path = archdep_boot_path();

      default_path = util_concat(emu_id,
                                 ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                 boot_path, emu_id,
                                 ARCHDEP_FINDPATH_SEPARATOR_STRING,

                                 "DRIVES",
                                 ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                 boot_path, "DRIVES",
                                 ARCHDEP_FINDPATH_SEPARATOR_STRING,

                                 "PRINTER",
                                 ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                 boot_path, "PRINTER",
                                 NULL);
    }

    return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname)
{
    return util_concat(fname, "~", NULL);
}

char *archdep_default_resource_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "vice.ini", NULL);
}

char *archdep_default_fliplist_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "fliplist-", machine_get_name(), ".vfl", NULL);
}

char *archdep_default_rtc_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "vice.rtc", NULL);
}

char *archdep_default_autostart_disk_image_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "autostart-", machine_get_name(), ".d64", NULL);
}

char *archdep_default_save_resource_file_name(void)
{
    return archdep_default_resource_file_name();
}

FILE *archdep_open_default_log_file(void)
{
    if (run_from_wb) {
        char *fname;
        FILE *f;

        fname = util_concat(archdep_boot_path(), "vice.log", NULL);
        f = fopen(fname, MODE_WRITE_TEXT);

        lib_free(fname);

        if (f == NULL) {
            return stdout;
        }

        return f;
    } else {
        return stdout;
    }
}

int archdep_default_logger(const char *level_string, const char *txt)
{
    return 0;
}

int archdep_path_is_relative(const char *path)
{
    if (path == NULL) {
        return 0;
    }

    return (strchr(path, ':') == NULL);
}

int archdep_spawn(const char *name, char **argv, char **stdout_redir, const char *stderr_redir)
{
    return -1;
}

/* return malloc'd version of full pathname of orig_name */
int archdep_expand_path(char **return_path, const char *orig_name)
{
    BPTR lock;

    lock = Lock(orig_name, ACCESS_READ);
    if (lock) {
        char name[1024];
        LONG rc;

        rc = NameFromLock(lock, name, 1024);
        UnLock(lock);
        if (rc) {
            *return_path = lib_stralloc(name);
            return 0;
        }
    }
    *return_path = lib_stralloc(orig_name);
    return 0;
}


/** \brief  Sanitize \a name by removing invalid characters for the current OS
 *
 * \param[in,out]   name    0-terminated string
 */
void archdep_sanitize_filename(char *name)
{
    while (*name != '\0') {
        int i = 0;
        while (illegal_name_tokens[i] != '\0') {
            if (illegal_name_tokens[i] == *name) {
                *name = '_';
                break;
            }
            i++;
        }
        name++;
    }
}


void archdep_startup_log_error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

char *archdep_filename_parameter(const char *name)
{
    return lib_stralloc(name);
}

char *archdep_quote_parameter(const char *name)
{
    return lib_stralloc(name);
}

char *archdep_tmpnam(void)
{
    return lib_stralloc(tmpnam(NULL));
}

FILE *archdep_mkstemp_fd(char **filename, const char *mode)
{
    char *tmp;
    FILE *fd;

    tmp = lib_stralloc(tmpnam(NULL));

    fd = fopen(tmp, mode);

    if (fd == NULL) {
        return NULL;
    }

    *filename = tmp;

    return fd;
}


int archdep_mkdir(const char *pathname, int mode)
{
    return mkdir(pathname, (mode_t)mode);
}

int archdep_rmdir(const char *pathname)
{
    return rmdir(pathname);
}

int archdep_stat(const char *file_name, unsigned int *len, unsigned int *isdir)
{
    struct stat statbuf;

    if (stat(file_name, &statbuf) < 0) {
        return -1;
    }

    *len = statbuf.st_size;
    *isdir = S_ISDIR(statbuf.st_mode);

    return 0;
}

/* set permissions of given file to rw, respecting current umask */
int archdep_fix_permissions(const char *file_name)
{
    SetProtection(file_name, 0);
    return 0;
}

int archdep_file_is_blockdev(const char *name)
{
    return 0;
}

int archdep_file_is_chardev(const char *name)
{
    return 0;
}

int archdep_rename(const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
}

void archdep_shutdown(void)
{
    lib_free(boot_path);
#if defined(AMIGA_M68K) && !defined(HAVE_GETTIMEOFDAY)
    gettimeofday_shutdown();
#endif
#ifdef HAVE_NETWORK
    archdep_network_shutdown();
#endif
}

#if defined(AMIGA_M68K) && !defined(HAVE_GETTIMEOFDAY)
int archdep_rtc_get_centisecond(void)
{
    struct timeval now;

    GetSysTime(&now);
    return now.tv_usec / 10;
}
#endif

char *archdep_extra_title_text(void)
{
    return NULL;
}
