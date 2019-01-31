/*
 * archdep.c - Miscellaneous system-specific stuff.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <dir.h>
#include <errno.h>
#include <io.h>
#include <process.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#include <allegro.h>

#include <dos.h>

#include "archdep.h"
#include "fcntl.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "util.h"
#include "video.h"


/** \brief  Tokens that are illegal in a path/filename
 */
static const char *illegal_name_tokens = "/\\?*:|\"<>";


static char *orig_workdir;
static char *argv0;

static void restore_workdir(void)
{
    if (orig_workdir) {
        chdir(orig_workdir);
    }
}

int archdep_init(int *argc, char **argv)
{
    allegro_init();

    _fmode = O_BINARY;

    argv0 = lib_stralloc(argv[0]);

    orig_workdir = getcwd(NULL, PATH_MAX);
    atexit(restore_workdir);

    return 0;
}

char *archdep_program_name(void)
{
    static char *program_name = NULL;

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

const char *archdep_boot_path(void)
{
    static char *boot_path;

    if (boot_path == NULL) {
        util_fname_split(argv0, &boot_path, NULL);

        /* This should not happen, but you never know...  */
        if (boot_path == NULL) {
            boot_path = lib_stralloc("./");
        }
    }

    return boot_path;
}

char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    static char *default_path;

    if (default_path == NULL) {
        const char *boot_path = archdep_boot_path();

        default_path = util_concat(boot_path, "/", emu_id,
                                   ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/", "DRIVES",
                                   ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/", "PRINTER",
                                   NULL);
    }

    return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname)
{
    static char backup_name[MAXPATH];
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char name[MAXFILE];
    char ext[MAXEXT];

    fnsplit(fname, drive, dir, name, ext);
    fnmerge(backup_name, drive, dir, name, "BAK");

    return lib_stralloc(backup_name);
}

char *archdep_default_save_resource_file_name(void)
{
    return archdep_default_resource_file_name();
}

char *archdep_default_resource_file_name(void)
{
    return util_concat(archdep_boot_path(), "\\vicerc", NULL);
}

char *archdep_default_fliplist_file_name(void)
{
    return util_concat(archdep_boot_path(), "\\",
                       machine_get_name(), ".vfl", NULL);
}

char *archdep_default_rtc_file_name(void)
{
    return util_concat(archdep_boot_path(), "\\vice.rtc", NULL);
}

char *archdep_default_autostart_disk_image_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "\\auto-", machine_get_name(), ".d64", NULL);
}

FILE *archdep_open_default_log_file(void)
{
    char *fname;
    FILE *f;

    fname = util_concat(archdep_boot_path(), "\\vice.log", NULL);
    f = fopen(fname, "wt");
    lib_free(fname);

    return f;
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

    /* `c:\foo', `c:/foo', `c:foo', `\foo' and `/foo' are absolute.  */

    return !((isalpha(path[0]) && path[1] == ':') || path[0] == '/' || path[0] == '\\');
}

int archdep_spawn(const char *name, char **argv, char **pstdout_redir, const char *stderr_redir)
{
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
    old_stdout_mode = setmode(STDOUT_FILENO, O_BINARY);
    old_stderr_mode = setmode(STDERR_FILENO, O_BINARY);

    /* Redirect stdout and stderr as requested, saving the old
       descriptors.  */
    if (stdout_redir != NULL) {
        old_stdout = dup(STDOUT_FILENO);
        new_stdout = open(stdout_redir, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        if (new_stdout == -1) {
            log_error(LOG_DEFAULT, "open(\"%s\") failed: %s.", stdout_redir, strerror(errno));
            retval = -1;
            goto cleanup;
        }
        dup2(new_stdout, STDOUT_FILENO);
    }
    if (stderr_redir != NULL) {
        old_stderr = dup(STDERR_FILENO);
        new_stderr = open(stderr_redir, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        if (new_stderr == -1) {
            log_error(LOG_DEFAULT, "open(\"%s\") failed: %s.", stderr_redir, strerror(errno));
            retval = -1;
            goto cleanup;
        }
        dup2(new_stderr, STDERR_FILENO);
    }

    /* Spawn the child process.  */
    retval = spawnvp(P_WAIT, name, argv);

cleanup:
    if (old_stdout >= 0) {
        dup2(old_stdout, STDOUT_FILENO);
        close(old_stdout);
    }
    if (old_stderr >= 0) {
        dup2(old_stderr, STDERR_FILENO);
        close(old_stderr);
    }
    if (old_stdout_mode >= 0) {
        setmode(STDOUT_FILENO, old_stdout_mode);
    }
    if (old_stderr_mode >= 0) {
        setmode(STDERR_FILENO, old_stderr_mode);
    }
    if (new_stdout >= 0) {
        close(new_stdout);
    }
    if (new_stderr >= 0) {
        close(new_stderr);
    }

    return retval;
}

/* return malloc'd version of full pathname of orig_name */
int archdep_expand_path(char **return_path, const char *orig_name)
{
    /* MS-DOS version.  */
    char *full_path = _truename(orig_name, NULL);

    if (full_path == NULL) {
        log_error(LOG_ERR, "zfile_list_add: warning, illegal file name `%s'.", orig_name);
        *return_path = lib_stralloc(orig_name);
    }
    *return_path = lib_stralloc(full_path);
    free(full_path); /* not lib_free() */
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
    /* nothing special(?) */
    return lib_stralloc(name);
}

char *archdep_quote_parameter(const char *name)
{
    /*not needed(?) */
    return lib_stralloc(name);
}

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
    return _chmod(file_name, 1, 0);
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
#ifdef HAVE_NETWORK
    archdep_network_shutdown();
#endif
}

char *archdep_extra_title_text(void)
{
    return NULL;
}
