/*
 * archdep.c - Miscellaneous system-specific stuff.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifdef __NeXT__
#define _POSIX_SOURCE
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "archdep.h"
#include "findpath.h"
#include "ioutil.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "ui.h"
#include "util.h"

#ifdef __NeXT__
#define waitpid(p, s, o) wait3((union wait *)(s), (o), (struct rusage *) 0)
#endif

#if defined(OPENSTEP_COMPILE) || defined(NEXTSTEP_COMPILE)
#ifndef S_ISBLK
#define S_ISBLK(mode)  (((mode) & (0170000)) == (0060000))
#endif
#ifndef S_ISCHR
#define S_ISCHR(mode)  (((mode) & (0170000)) == (0020000))
#endif
#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & (0170000)) == (0040000))
#endif
#endif


/** \brief  Tokens that are illegal in a path/filename
 */
static const char *illegal_name_tokens = "/";


static char *argv0 = NULL;
static char *boot_path = NULL;

/* alternate storage of preferences */
const char *archdep_pref_path = NULL; /* NULL -> use home_path + ".vice" */

int archdep_init(int *argc, char **argv)
{
    argv0 = lib_stralloc(argv[0]);
    log_verbose_init(*argc, argv);
    return 0;
}

char *archdep_program_name(void)
{
    static char *program_name = NULL;

    if (program_name == NULL) {
        char *p;

        p = strrchr(argv0, '/');
        if (p == NULL) {
            program_name = lib_stralloc(argv0);
        } else {
            program_name = lib_stralloc(p + 1);
        }
    }

    return program_name;
}

const char *archdep_boot_path(void)
{
    if (boot_path == NULL) {
        boot_path = findpath(argv0, getenv("PATH"), IOUTIL_ACCESS_X_OK);

        /* Remove the program name.  */
        *strrchr(boot_path, '/') = '\0';
    }

    return boot_path;
}

const char *archdep_home_path(void)
{
    char *home;

    home = getenv("HOME");
    if (home == NULL) {
#ifdef HAVE_GETPWUID
        struct passwd *pwd;

        pwd = getpwuid(getuid());
        if ((pwd == NULL) || ((home = pwd->pw_dir) == NULL)) {
            /* give up */
            home = ".";
        }
#else
        home = ".";
#endif
    }

    return home;
}

char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    static char *default_path;

#if defined(MINIXVMD) || defined(MINIX_SUPPORT)
    static char *default_path_temp;
#endif

    if (default_path == NULL) {
        const char *boot_path;
        const char *home_path;

        boot_path = archdep_boot_path();
        home_path = archdep_home_path();

        /* First search in the `LIBDIR' then the $HOME/.vice/ dir (home_path)
           and then in the `boot_path'.  */

#if defined(MINIXVMD) || defined(MINIX_SUPPORT)
        default_path_temp = util_concat(LIBDIR, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/", emu_id,NULL);

        default_path = util_concat(default_path_temp, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   LIBDIR, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   LIBDIR, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/PRINTER", NULL);
        lib_free(default_path_temp);

#else 
#if defined(MACOSX_BUNDLE)
        /* Mac OS X Bundles keep their ROMS in Resources/bin/../ROM */
#if defined(MACOSX_COCOA)
#define MACOSX_ROMDIR "/../Resources/ROM/"
#else
#define MACOSX_ROMDIR "/../ROM/"
#endif
        default_path = util_concat(boot_path, MACOSX_ROMDIR, emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,

                                   boot_path, MACOSX_ROMDIR, "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,

                                   boot_path, MACOSX_ROMDIR, "PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/PRINTER", NULL);
#else
        default_path = util_concat(LIBDIR, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   LIBDIR, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   LIBDIR, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   home_path, "/", VICEUSERDIR, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "/PRINTER", NULL);
#endif
#endif
    }

    return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname)
{
    return util_concat(fname, "~", NULL);
}

/* create the default directory where prefs, fliplist, autostart images etc are stored
   a pointer to the resulting path is returned, and it should be freed by the caller. */
static char *archdep_make_default_pref_path(int create)
{
    char *path;
    if (archdep_pref_path == NULL) {
        const char *home;
        home = archdep_home_path();
        path = util_concat(home, "/.vice", NULL);
    } else {
        path = lib_stralloc(archdep_pref_path);
    }
    if(create) {
        if (access(path, F_OK)) {
            mkdir(path, S_IRWXU);
        }
    }
    return path;
}

char *archdep_default_resource_file_name(void)
{
    char *fname;
    const char *viceuserdir;

    viceuserdir = archdep_make_default_pref_path(0);
    fname = util_concat(viceuserdir, "/vicerc", NULL);
    lib_free(viceuserdir);

    return fname;
}

char *archdep_default_fliplist_file_name(void)
{
    char *fname;
    const char *viceuserdir;

    viceuserdir = archdep_make_default_pref_path(0);
    fname = util_concat(viceuserdir, "/fliplist-", machine_get_name(), ".vfl", NULL);
    lib_free(viceuserdir);

    return fname;
}

char *archdep_default_rtc_file_name(void)
{
    char *fname;
    const char *viceuserdir;

    viceuserdir = archdep_make_default_pref_path(0);
    fname = util_concat(viceuserdir, "/vice.rtc", NULL);
    lib_free(viceuserdir);

    return fname;
}

char *archdep_default_autostart_disk_image_file_name(void)
{
    char *fname;
    const char *viceuserdir;

    viceuserdir = archdep_make_default_pref_path(0);
    fname = util_concat(viceuserdir, "/autostart-", machine_get_name(), ".d64", NULL);
    lib_free(viceuserdir);

    return fname;
}

/* same as archdep_default_resource_file_name() but also creates the directory */
/* FIXME: checking for existance and creating the directory should perhaps be
          handled in common code instead. for the time being its created always
          here, which isnt 'nice' but shouldnt do any harm either. it is needed
          so on a fresh install the related features work, ie before the settings
          were saved (and thus the directory created) (see bug #629) */
char *archdep_default_save_resource_file_name(void)
{ 
    char *fname;
    const char *viceuserdir;

    viceuserdir = archdep_make_default_pref_path(1);
    fname = util_concat(viceuserdir, "/vicerc", NULL);
    lib_free(viceuserdir);

    return fname;
}

#if defined(MACOSX_COCOA)
int default_log_fd = 0;

FILE *archdep_open_default_log_file(void)
{
    int newFd = dup(default_log_fd);
    FILE *file = fdopen(newFd, "w");
    setlinebuf(file);
    return file;
}

#else
FILE *archdep_open_default_log_file(void)
{
    return stdout;
}
#endif

int archdep_default_logger(const char *level_string, const char *txt)
{
    if (fputs(level_string, stdout) == EOF || fprintf(stdout, "%s", txt) < 0 || fputc ('\n', stdout) == EOF) {
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
#if !defined(OPENSTEP_COMPILE) && !defined(NEXTSTEP_COMPILE)
    pid_t child_pid;
    int child_status;
    char *stdout_redir;

    child_pid = vfork();

    if (pstdout_redir != NULL) {
        if (*pstdout_redir == NULL) {
            *pstdout_redir = archdep_tmpnam();
        }
        stdout_redir = *pstdout_redir;
    } else {
        stdout_redir = NULL;
    }

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
#else
    return -1;
#endif
}

/** \brief  Return malloc'd version of full pathname of orig_name
 *
 * Returns the absolute path of \a orig_name. Expands '~' to the user's home
 * path.
 *
 * \param[out]  return_path pointer to expand path destination
 * \param[in]   orig_name   original path
 *
 * \return  0
 */
int archdep_expand_path(char **return_path, const char *orig_name)
{
    /* Unix version.  */
    if (*orig_name == '/') {
        *return_path = lib_stralloc(orig_name);
    } else if (*orig_name == '~' && *(orig_name +1) == '/') {
        *return_path = util_concat(archdep_home_path(), orig_name + 1, NULL);
    } else {
        char *cwd;

        cwd = ioutil_current_dir();
        *return_path = util_concat(cwd, "/", orig_name, NULL);
        lib_free(cwd);
    }
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

/*
    "special" chars in *unix are:

    "'\[]() and acute/forward tick

    tested unproblematic (no escaping):

    "'() and acute/forward tick

    tested problematic (need escaping):

    \[]
    - if the name of a file _inside_ a .zip file contain \, [ or ], then extracting
      it will fail if they are not escaped.

    several problems on autostart remain, which are not quoting but ascii vs petscii related.
*/
char *archdep_quote_parameter(const char *name)
{
    char *a;

    a = util_subst(name, "\\", "\\\\");
    a = util_subst(a, "[", "\\[");
    a = util_subst(a, "]", "\\]");
    return a;
}

char *archdep_tmpnam(void)
{
#ifdef HAVE_MKSTEMP
    char *tmp_name;
    const char mkstemp_template[] = "/vice.XXXXXX";
    int fd;
    char *tmp;
    char *final_name;

    tmp_name = lib_malloc(ioutil_maxpathlen());
    if ((tmp = getenv("TMPDIR")) != NULL) {
        strncpy(tmp_name, tmp, ioutil_maxpathlen());
        tmp_name[ioutil_maxpathlen() - sizeof(mkstemp_template)] = '\0';
    } else {
        strcpy(tmp_name, "/tmp");
    }
    strcat(tmp_name, mkstemp_template);
    if ((fd = mkstemp(tmp_name)) < 0) {
        tmp_name[0] = '\0';
    } else {
        close(fd);
    }

    final_name = lib_stralloc(tmp_name);
    lib_free(tmp_name);
    return final_name;
#else
    return lib_stralloc(tmpnam(NULL));
#endif
}

FILE *archdep_mkstemp_fd(char **filename, const char *mode)
{
#if defined HAVE_MKSTEMP
    char *tmp;
    const char template[] = "/vice.XXXXXX";
    int fildes;
    FILE *fd;
    char *tmpdir;

    tmpdir = getenv("TMPDIR");

    if (tmpdir != NULL) {
        tmp = util_concat(tmpdir, template, NULL);
    } else {
        tmp = util_concat("/tmp", template, NULL);
    }

    fildes = mkstemp(tmp);

    if (fildes < 0 ) {
        lib_free(tmp);
        return NULL;
    }

    fd = fdopen(fildes, mode);

    if (fd == NULL) {
        lib_free(tmp);
        return NULL;
    }

    *filename = tmp;

    return fd;
#else
    char *tmp;

    tmp = tmpnam(NULL);

    if (tmp == NULL) {
        return NULL;
    }

    fd = fopen(tmp, mode);

    if (fd == NULL) {
        return NULL;
    }

    *filename = lib_stralloc(tmp);

    return fd;
#endif
}


/* called by ioutil_mkdir, modes defined in ioutil.h */
int archdep_mkdir(const char *pathname, int mode)
{
#ifndef __NeXT__
    return mkdir(pathname, (mode_t)mode);
#else
    return mkdir(pathname, mode);
#endif
}

/** \brief  Remove directory \a pathname
 *
 * \return  0 on success, -1 on failure
 */
int archdep_rmdir(const char *pathname)
{
    return rmdir(pathname);
}

int archdep_stat(const char *file_name, unsigned int *len, unsigned int *isdir)
{
    struct stat statbuf;

    if (stat(file_name, &statbuf) < 0) {
        *len = 0;
        *isdir = 0;
        return -1;
    }

    *len = statbuf.st_size;
    *isdir = S_ISDIR(statbuf.st_mode);

    return 0;
}

#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

/* set permissions of given file to rw, respecting current umask */
int archdep_fix_permissions(const char *file_name)
{
    mode_t mask = umask(0);
    umask(mask);
    return chmod(file_name, mask ^ DEFFILEMODE); /* 0666 */
}

int archdep_file_is_blockdev(const char *name)
{
    struct stat buf;

    if (stat(name, &buf) != 0) {
        return 0;
    }

    if (S_ISBLK(buf.st_mode)) {
        return 1;
    }

    return 0;
}

int archdep_file_is_chardev(const char *name)
{
    struct stat buf;

    if (stat(name, &buf) != 0) {
        return 0;
    }

    if (S_ISCHR(buf.st_mode)) {
        return 1;
    }

    return 0;
}

int archdep_rename(const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
}

void archdep_shutdown(void)
{
    lib_free(argv0);
    lib_free(boot_path);
    archdep_network_shutdown();
}

char *archdep_extra_title_text(void)
{
    return NULL;
}
