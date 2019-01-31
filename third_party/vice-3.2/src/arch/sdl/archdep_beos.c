/*
 * archdep_beos.c - Miscellaneous system-specific stuff.
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

#include "vice.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <sys/utsname.h>

#include "ui.h"

#ifdef HAVE_DIR_H
#include <dir.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
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

#ifdef __HAIKU__
#include <sys/wait.h>
#endif

#include "archdep.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "util.h"


/** \brief  Tokens that are illegal in a path/filename
 *
 * FIXME: taken from Unix
 */
static const char *illegal_name_tokens = "/";


static char *orig_workdir;
static char *argv0 = NULL;

static int archdep_init_extra(int *argc, char **argv)
{
    argv0 = lib_stralloc(argv[0]);
    orig_workdir = getcwd(NULL, PATH_MAX);

    return 0;
}

static char *program_name = NULL;

char *archdep_program_name(void)
{
    if (program_name == NULL) {
        char *s, *e;
        int len;

        s = strrchr(argv0, '/');
        if (s == NULL) {
            s = argv0;
        } else {
            s++;
        }
        e = strchr(s, '.');
        if (e == NULL) {
            e = argv0 + strlen(argv0);
        }

        len = e - s + 1;
        program_name = lib_malloc(len);
        memcpy(program_name, s, len - 1);
        program_name[len - 1] = 0;
    }

    return program_name;
}

static char *boot_path = NULL;

const char *archdep_boot_path(void)
{
    if (boot_path == NULL) {
        util_fname_split(argv0, &boot_path, NULL);

        /* This should not happen, but you never know...  */
        if (boot_path == NULL) {
            boot_path = lib_stralloc("./xxx");
        }
    }

    return boot_path;
}

char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    static char *default_path;

    if (default_path == NULL) {
        const char *boot_path = archdep_boot_path();
        default_path = util_concat(boot_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/", "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/", "PRINTER", NULL);
    }

    return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname)
{
    char *tmp;

    tmp = util_concat(fname, NULL);
    tmp[strlen(tmp) - 1] = '~';
    return tmp;
}

char *archdep_default_save_resource_file_name(void)
{
    return archdep_default_resource_file_name();
}

char *archdep_default_resource_file_name(void)
{
    return util_concat(archdep_boot_path(), "/vice-sdl.ini", NULL);
}


/** \brief  Get path to VICE session file
 *
 * The 'session file' is a file that is used to store settings between VICE
 * runs, storing things like the last used directory.
 *
 * \return  path to session file
 */
char *archdep_default_session_file_name(void)
{
    return util_concat(archdep_boot_path(), "/vice-sdl-session.ini", NULL);
}


char *archdep_default_fliplist_file_name(void)
{
    static char *fname;

    lib_free(fname);
    fname = util_concat(archdep_boot_path(), "/fliplist-", machine_get_name(), ".vfl", NULL);
    return fname;
}

char *archdep_default_rtc_file_name(void)
{
    static char *fname;

    lib_free(fname);
    fname = util_concat(archdep_boot_path(), "/vice-sdl.rtc", NULL);
    return fname;
}

char *archdep_default_autostart_disk_image_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "/autostart-", machine_get_name(), ".d64", NULL);
}

char *archdep_default_hotkey_file_name(void)
{
    static char *fname;

    lib_free(fname);
    fname = util_concat(archdep_boot_path(), "/sdl-hotkey-", machine_get_name(), ".vkm", NULL);
    return fname;
}

char *archdep_default_joymap_file_name(void)
{
    static char *fname;

    lib_free(fname);
    fname = util_concat(archdep_boot_path(), "/sdl-joymap-", machine_get_name(), ".vjm", NULL);
    return fname;
}

FILE *archdep_open_default_log_file(void)
{
    char *fname;
    FILE *f;

    fname = util_concat(archdep_boot_path(), "/vice.log", NULL);
    f = fopen(fname, "wt");
    lib_free(fname);

    return f;
}

int archdep_default_logger(const char *level_string, const char *txt)
{
    if (fputs(level_string, stdout) == EOF || fprintf(stdout, txt) < 0 || fputc ('\n', stdout) == EOF) {
        return -1;
    }
    return 0;
}

int archdep_path_is_relative(const char *path)
{
    if (path == NULL) {
        return 0;
    }

    return *path != '/';
}

int archdep_spawn(const char *name, char **argv, char **pstdout_redir, const char *stderr_redir)
{
    pid_t child_pid;
    int child_status;
    char *stdout_redir = NULL;

    if (pstdout_redir != NULL) {
        if (*pstdout_redir == NULL) {
            *pstdout_redir = archdep_tmpnam();
        }
        stdout_redir = *pstdout_redir;
    }

#ifdef WORDS_BIGENDIAN
    child_pid = -1;
#else
    child_pid = vfork();
#endif

    if (child_pid < 0) {
        log_error(LOG_DEFAULT, "vfork() failed: %s.", strerror(errno));
        return -1;
    } else {
        if (child_pid == 0) {
            if (stdout_redir && freopen(stdout_redir, "w", stdout) == NULL) {
                log_error(LOG_DEFAULT, "freopen(\"%s\") failed: %s.", stdout_redir, strerror(errno));
                _exit(-1);
            }
            if (stderr_redir && freopen(stderr_redir, "w", stderr) == NULL) {
                log_error(LOG_DEFAULT, "freopen(\"%s\") failed: %s.", stderr_redir, strerror(errno));
                _exit(-1);
            }
            execvp(name, argv);
            _exit(-1);
        }
    }

    if (waitpid(child_pid, &child_status, 0) != child_pid) {
        log_error(LOG_DEFAULT, "waitpid() failed: %s", strerror(errno));
        return -1;
    }

    if (WIFEXITED(child_status)) {
        return WEXITSTATUS(child_status);
    } else {
        return -1;
    }
}

/* return malloced version of full pathname of orig_name */
int archdep_expand_path(char **return_path, const char *orig_name)
{
    /*  BeOS version   */
    *return_path = lib_stralloc(orig_name);
    return 0;
}

void archdep_startup_log_error(const char *format, ...)
{
    char *tmp;
    va_list args;

    va_start(args, format);
    tmp = lib_mvsprintf(format, args);
    va_end(args);
    printf(tmp);

    lib_free(tmp);
}

char *archdep_quote_parameter(const char *name)
{
    return lib_stralloc(name);
}

char *archdep_filename_parameter(const char *name)
{
    char *exp;
    char *a;

    archdep_expand_path(&exp, name);
    a = archdep_quote_parameter(exp);
    lib_free(exp);
    return a;
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

int archdep_require_vkbd(void)
{
    return 0;
}

int archdep_rename(const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
}

static void archdep_shutdown_extra(void)
{
    lib_free(argv0);
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


#ifdef USE_SDLUI2
char *archdep_sdl2_default_renderers[] = {
    "software", "opengl", NULL
};
#endif


/* This check is needed for haiku, since it always returns 1 on
   SupportsWindowMode() */
int CheckForHaiku(void)
{
    struct utsname name;

    uname(&name);
    if (!strncasecmp(name.sysname, "Haiku", 5)) {
        return -1;
    }
    return 0;
}


