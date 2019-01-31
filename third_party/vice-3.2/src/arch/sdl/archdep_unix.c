/*
 * archdep_unix.c - Miscellaneous system-specific stuff.
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

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <pwd.h>
#include "vice_sdl.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#if defined(__QNX__) && !defined(__QNXNTO__)
#include <sys/time.h>
#include <sys/timers.h>
#endif

#include "archdep.h"
#include "findpath.h"
#include "ioutil.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "monitor.h"
#include "ui.h"
#include "util.h"

#ifdef __NeXT__
#define waitpid(p, s, o) wait3((union wait *)(s), (o), (struct rusage *) 0)
#endif


/** \brief  Tokens that are illegal in a path/filename
 */
static const char *illegal_name_tokens = "/";


static char *argv0 = NULL;
static char *boot_path = NULL;
static char *program_name = NULL;

/* alternate storage of preferences */
const char *archdep_pref_path = NULL; /* NULL -> use home_path + ".vice" */


#if defined(__QNX__) && !defined(__QNXNTO__)
int archdep_rtc_get_centisecond(void)
{
    struct timespec dtm;
    int status;

    if ((status = clock_gettime(CLOCK_REALTIME, &dtm)) == 0) {
        return dtm.tv_nsec / 10000L;
    }
    return 0;
}
#endif

static int archdep_init_extra(int *argc, char **argv)
{
    ssize_t read;
#if !defined(USE_PROC_SELF_EXE)
    /* first try to get exe name from argv[0] */
    if (argv[0]) {
        argv0 = lib_stralloc(argv[0]);
        return 0;
    }
#endif
    argv0 = lib_malloc(ioutil_maxpathlen());
    read = readlink("/proc/self/exe", argv0, ioutil_maxpathlen() - 1);

    if (read == -1) {
        return 1;
    } else {
        argv0[read] = '\0';
    }
    /* FIXME: bad name of define, this probably should not be here either */
#ifdef USE_PROC_SELF_EXE
    /* set this up now to remove extra .vice directory */
    archdep_pref_path = archdep_boot_path();
#endif
    return 0;
}

char *archdep_program_name(void)
{
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
        /* FIXME: bad name of define */
#ifdef USE_PROC_SELF_EXE
        /* known from setup in archdep_init_extra() so just reuse it */
        boot_path = lib_stralloc(argv0);
#else
        boot_path = findpath(argv0, getenv("PATH"), IOUTIL_ACCESS_X_OK);
#endif

        /* Remove the program name.  */
        *strrchr(boot_path, '/') = '\0';
    }

    return boot_path;
}

const char *archdep_home_path(void)
{
    /* FIXME: bad name of define */
#ifdef USE_PROC_SELF_EXE
    /* everything is relative to the location of the exe which is already known
       from archdep_init_bootpath() so just reuse it */
    return (archdep_boot_path());
#else
    char *home;

    home = getenv("HOME");
    if (home == NULL) {
        struct passwd *pwd;

        pwd = getpwuid(getuid());
        if ((pwd == NULL) || ((home = pwd->pw_dir) == NULL)) {
            /* give up */
            home = ".";
        }
    }

    return home;
#endif
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
                                        home_path, "/", VICEUSERDIR, "/", emu_id, NULL);

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
        #define MACOSX_ROMDIR "/../Resources/ROM/"

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

char *archdep_default_resource_file_name(void)
{
    if (archdep_pref_path == NULL) {
        const char *home;

        home = archdep_home_path();
        return util_concat(home, "/.vice/sdl-vicerc", NULL);
    } else {
        return util_concat(archdep_pref_path, "/sdl-vicerc", NULL);
    }
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
    if (archdep_pref_path == NULL) {
        const char *home;

        home = archdep_home_path();
        return util_concat(home, "/.vice/sdl-vicesession", NULL);
    } else {
        return util_concat(archdep_pref_path, "/sdl-vicesession", NULL);
    }
}


char *archdep_default_fliplist_file_name(void)
{
    if (archdep_pref_path == NULL) {
        const char *home;

        home = archdep_home_path();
        return util_concat(home, "/.vice/fliplist-", machine_get_name(), ".vfl", NULL);
    } else {
        return util_concat(archdep_pref_path, "/fliplist-", machine_get_name(), ".vfl", NULL);
    }
}

char *archdep_default_rtc_file_name(void)
{
    if (archdep_pref_path == NULL) {
        const char *home;

        home = archdep_home_path();
        return util_concat(home, "/.vice/sdl-vice.rtc", NULL);
    } else {
        return util_concat(archdep_pref_path, "/sdl-vice.rtc", NULL);
    }
}

char *archdep_default_autostart_disk_image_file_name(void)
{
    if (archdep_pref_path == NULL) {
        const char *home;

        home = archdep_home_path();
        return util_concat(home, "/.vice/autostart-", machine_get_name(), ".d64", NULL);
    } else {
        return util_concat(archdep_pref_path, "/autostart-", machine_get_name(), ".d64", NULL);
    }
}

char *archdep_default_hotkey_file_name(void)
{
    if (archdep_pref_path == NULL) {
        const char *home;

        home = archdep_home_path();
        return util_concat(home, "/.vice/sdl-hotkey-", machine_get_name(), ".vkm", NULL);
    } else {
        return util_concat(archdep_pref_path, "/sdl-hotkey-", machine_get_name(), ".vkm", NULL);
    }
}

char *archdep_default_joymap_file_name(void)
{
    if (archdep_pref_path == NULL) {
        const char *home;

        home = archdep_home_path();
        return util_concat(home, "/.vice/sdl-joymap-", machine_get_name(), ".vjm", NULL);
    } else {
        return util_concat(archdep_pref_path, "/sdl-joymap-", machine_get_name(), ".vjm", NULL);
    }
}

char *archdep_default_save_resource_file_name(void)
{
    char *fname;
    const char *home;
    const char *viceuserdir;

    if (archdep_pref_path == NULL) {
        home = archdep_home_path();
        viceuserdir = util_concat(home, "/.vice", NULL);
    } else {
        viceuserdir = archdep_pref_path;
    }

    if (access(viceuserdir, F_OK)) {
        mkdir(viceuserdir, 0700);
    }

    fname = util_concat(viceuserdir, "/sdl-vicerc", NULL);

    if (archdep_pref_path == NULL) {
        lib_free(viceuserdir);
    }

    return fname;
}

FILE *archdep_open_default_log_file(void)
{
    return stdout;
}

int archdep_default_logger(const char *level_string, const char *txt)
{
    if (fputs(level_string, stdout) == EOF || fprintf(stdout, "%s", txt) < 0 || fputc('\n', stdout) == EOF) {
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
    } else if (*orig_name == '~' && *(orig_name + 1) == '/') {
        *return_path = util_concat(archdep_home_path(), orig_name + 1, NULL);
    } else {
        static char *cwd;

        cwd = ioutil_current_dir();
        *return_path = util_concat(cwd, "/", orig_name, NULL);
        lib_free(cwd);
    }
    return 0;
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
#ifdef HAVE_MKSTEMP
    char *tmp_name;
    const char mkstemp_template[] = "/vice.XXXXXX";
    int fd;
    char *tmp;
    char *final_name;

    tmp_name = lib_malloc(ioutil_maxpathlen());
#ifdef USE_EXE_RELATIVE_TMP
    strcpy(tmp_name, archdep_boot_path());
    strcat(tmp_name, "/tmp");
#else
    if ((tmp = getenv("TMPDIR")) != NULL) {
        strncpy(tmp_name, tmp, ioutil_maxpathlen());
        tmp_name[ioutil_maxpathlen() - sizeof(mkstemp_template)] = '\0';
    } else {
        strcpy(tmp_name, "/tmp");
    }
#endif
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
#if defined(HAVE_MKSTEMP)
    char *tmp;
    const char template[] = "/vice.XXXXXX";
    int fildes;
    FILE *fd;
    char *tmpdir;

#ifdef USE_EXE_RELATIVE_TMP
    tmp = lib_msprintf("%s/tmp%s", archdep_boot_path(), template);
#else
    tmpdir = getenv("TMPDIR");

    if (tmpdir != NULL) {
        tmp = util_concat(tmpdir, template, NULL);
    } else {
        tmp = util_concat("/tmp", template, NULL);
    }
#endif

    fildes = mkstemp(tmp);

    if (fildes < 0) {
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
    FILE *fd;

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


int archdep_mkdir(const char *pathname, int mode)
{
#ifndef __NeXT__
    return mkdir(pathname, (mode_t)mode);
#else
    return mkdir(pathname, mode);
#endif
}

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

/* set permissions of given file to rw, respecting current umask */
int archdep_fix_permissions(const char *file_name)
{
    mode_t mask = umask(0);
    umask(mask);
    return chmod(file_name, mask ^ 0666);
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

int archdep_require_vkbd(void)
{
    return 0;
}

static void archdep_shutdown_extra(void)
{
    lib_free(argv0);
    lib_free(boot_path);
    if (program_name != NULL) {
        lib_free(program_name);
        program_name = NULL;
    }
}

/******************************************************************************/

static RETSIGTYPE break64(int sig)
{
    log_message(LOG_DEFAULT, "Received signal %d, exiting.", sig);
    exit (-1);
}

/*
    used once at init time to setup all signal handlers
*/
void archdep_signals_init(int do_core_dumps)
{
    if (!do_core_dumps) {
        signal(SIGPIPE, break64);
    }
}

typedef void (*signal_handler_t)(int);

static signal_handler_t old_pipe_handler;

/*
    these two are used for socket send/recv. in this case we might
    get SIGPIPE if the connection is unexpectedly closed.
*/
void archdep_signals_pipe_set(void)
{
    old_pipe_handler = signal(SIGPIPE, SIG_IGN);
}

void archdep_signals_pipe_unset(void)
{
    signal(SIGPIPE, old_pipe_handler);
}

int archdep_rename(const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
}

/* doesn't seem to be required -- compyx */
#if 0
static char *archdep_get_runtime_os(void)
{
/* TODO: add runtime os detection code for other *nix os'es */
#ifndef RUNTIME_OS_CALL
    return "*nix";
#else
    return RUNTIME_OS_CALL();
#endif
}

static char *archdep_get_runtime_cpu(void)
{
/* TODO: add runtime cpu detection code for other cpu's */
#ifndef RUNTIME_CPU_CALL
    return "Unknown CPU";
#else
    return RUNTIME_CPU_CALL();
#endif
}
#endif


/* returns host keyboard mapping. used to initialize the keyboard map when
   starting with a blank (default) config, so an educated guess works good
   enough most of the time :)

   FIXME: add more languages
*/
int kbd_arch_get_host_mapping(void)
{
    int n;
    char *l;
    int maps[KBD_MAPPING_NUM] = {
        KBD_MAPPING_US, KBD_MAPPING_UK, KBD_MAPPING_DE, KBD_MAPPING_DA,
        KBD_MAPPING_NO, KBD_MAPPING_FI, KBD_MAPPING_IT };
    char str[KBD_MAPPING_NUM][6] = {
        "en_US", "en_UK", "de", "da", "no", "fi", "it"};
    /* setup the locale */
    setlocale(LC_ALL, "");
    l = setlocale(LC_ALL, NULL);
    if (l && (strlen(l) > 1)) {
        for (n = 1; n < KBD_MAPPING_NUM; n++) {
            if (strncmp(l, str[n], strlen(str[n])) == 0) {
                return maps[n];
            }
        }
    }
    return KBD_MAPPING_US;
}

#ifdef USE_SDLUI2
char *archdep_sdl2_default_renderers[] = {
    "opengles2",
    "opengles",
    "opengl",
    NULL
};
#endif
