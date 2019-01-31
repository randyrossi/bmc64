/** \file   archdep_unix.c
 * \brief   Miscellaneous UNIX-specific stuff
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>

#include "ioutil.h"
#include "lib.h"
#include "util.h"

#include "not_implemented.h"

#include "archdep.h"


/* fix VICE userdir */
#ifdef VICEUSERDIR
# undef VICEUSERDIR
#endif
#define VICEUSERDIR "vice"



/** \brief  Path separator used in GLib code
 */
static const gchar *path_separator = "/";


/** \brief  Tokens that are illegal in a path/filename
 */
static const char *illegal_name_tokens = "/";



/** \brief  String containing search paths
 *
 * Allocated in the first call to archdep_default_sysfile_pathlist(),
 * deallocated in archdep_shutdown().
 */
static char *default_path = NULL;


/** \brief  Path to the binary
 */
static char *boot_path_bin = NULL;


/** \brief  Write log message to stdout
 *
 * param[in]    level_string    log level string
 * param[in]    txt             log message
 *
 * \note    Shamelessly copied from unix/archdep.c
 *
 * \return  0 on success, < 0 on failure
 */
int archdep_default_logger(const char *level_string, const char *txt)
{
    if (fputs(level_string, stdout) == EOF || fprintf(stdout, "%s", txt) < 0 || fputc ('\n', stdout) == EOF) {
        return -1;
    }
    return 0;
}


/** \brief  Generate path to vicerc
 *
 * The value returned needs to be freed using lib_free()
 *
 * \return  absolute path to vice.ini
 */
char *archdep_default_resource_file_name(void)
{
    char *cfg;
    gchar *tmp;
    char *path;

    cfg = archdep_user_config_path();
    tmp = g_build_path(path_separator, cfg, "vicerc", NULL);
    /* transfer ownership to VICE */
    path = lib_stralloc(tmp);
    g_free(tmp);
    lib_free(cfg);
    return path;
}


/** \brief  Get default settings file name
 *
 * \return  path to default settings file
 */
char *archdep_default_save_resource_file_name(void)
{
    char *fname;
    const char *home;
    const char *viceuserdir;

    if (archdep_pref_path == NULL) {
        home = archdep_home_path();
        /* XDG spec */
        viceuserdir = util_concat(home, "/.config/vice", NULL);
    } else {
        viceuserdir = archdep_pref_path;
    }

    if (access(viceuserdir, F_OK) != 0) {
        mkdir(viceuserdir, 0700);
    }

    fname = util_concat(viceuserdir, "/vicerc", NULL);

    if (archdep_pref_path == NULL) {
        lib_free(viceuserdir);
    }

    return fname;
}


/** \brief  Build a list of search paths for emulator \a emu_id
 *
 * \param[in]   emu_id  emulator name
 *
 * \return  string containing search paths
 */
char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    if (default_path == NULL) {
        const char *boot_path;
        const char *config_path;

        boot_path = archdep_boot_path();
        config_path = archdep_user_config_path();

        /* First search in the `LIBDIR' then the $HOME/.config/vice/ dir (config_path)
           and then in the `boot_path'.  */

#if defined(MACOSX_BUNDLE)
        /* Mac OS X Bundles keep their ROMS in Resources/bin/../ROM */
# if defined(MACOSX_COCOA)
#  define MACOSX_ROMDIR "/../Resources/ROM/"
# else
#  define MACOSX_ROMDIR "/../ROM/"
# endif
        default_path = util_concat(
                boot_path, MACOSX_ROMDIR, emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                config_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, MACOSX_ROMDIR, "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                config_path, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, MACOSX_ROMDIR, "PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                config_path, "/PRINTER", NULL);
#else
        default_path = util_concat(
                LIBDIR, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                config_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                LIBDIR, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                config_path, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "/DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                LIBDIR, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                config_path, "/PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "/PRINTER", NULL);
#endif
        lib_free(config_path);
    }

    /* We allocate a fresh string here because sysfile.c, among other
     * places, takes ownership of the result. */
    return lib_stralloc(default_path);
}


/** \brief  Generate heap-allocated full pathname of \a orig_name
 *
 * Returns the absolute path of \a orig_name. Expands '~' to the user's home
 * path. If the prefix in \a orig_name is not '~/', the file is assumed to
 * reside in the current working directory whichever that may be.
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
        static char *cwd;

        cwd = ioutil_current_dir();
        *return_path = util_concat(cwd, "/", orig_name, NULL);
        lib_free(cwd);
    }
    return 0;
}


/** \brief  Get the absolute path to the VICE dir
 *
 * \return  Path to VICE's directory
 */
const char *archdep_boot_path(void)
{
    char *sep;

    if (boot_path_bin == NULL) {
        boot_path_bin = findpath(argv0, getenv("PATH"), IOUTIL_ACCESS_X_OK);

        /* Remove the program name.  */
        sep = strrchr(boot_path_bin, '/');
        if (sep != NULL) {
           *sep = '\0';
        }
    }

    return boot_path_bin;
}

char *archdep_make_backup_filename(const char *fname)
{
    return util_concat(fname, "~", NULL);
}


void archdep_shutdown(void)
{
    if (default_path != NULL) {
        lib_free(default_path);
    }
    if (argv0 != NULL) {
        lib_free(argv0);
        argv0 = NULL;
    }

    if (boot_path_bin != NULL) {
        lib_free(boot_path_bin);
        boot_path_bin = NULL;
    }
    if (program_name != NULL) {
        lib_free(program_name);
        program_name = NULL;
    }

    archdep_network_shutdown();

    /* partially implemented */
    INCOMPLETE_IMPLEMENTATION();
}

int archdep_spawn(const char *name, char **argv,
                  char **pstdout_redir, const char *stderr_redir)
{
    pid_t child_pid;
    int child_status;
    char *stdout_redir;


    if (pstdout_redir != NULL) {
        if (*pstdout_redir == NULL) {
            *pstdout_redir = archdep_tmpnam();
        }
        stdout_redir = *pstdout_redir;
    } else {
        stdout_redir = NULL;
    }

    child_pid = vfork();
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

/* for when I figure this out: */
#if 0
    char *stdout_redir;
    gboolean result;
    GPid child_pid;
    GError *err;

    if (pstdout_redir != NULL) {
        if (*pstdout_redir == NULL) {
            *pstdout_redir = archdep_tmpnam();
        }
        stdout_redir = *pstdout_redir;
    } else {
        stdout_redir = NULL;
    }

    result = g_spawn_async(NULL,        /* working_directory, NULL = inherit */
                           argv,        /* argv */
                           NULL,        /* envp, NULL = inherit */
                           G_SPAWN_DEFAULT,   /* flags */
                           NULL,        /* child_setup */
                           NULL,        /* user_data */
                           &child_pid,  /* child PID object */
                           &err);
#endif


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

/** \brief  Create a unique temporary filename
 *
 * Uses mkstemp(3) when available.
 *
 * \return  temporary filename
 */
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


void archdep_signals_pipe_set(void)
{
    NOT_IMPLEMENTED();
}

void archdep_signals_pipe_unset(void)
{
    NOT_IMPLEMENTED();
}

char *archdep_default_rtc_file_name(void)
{
    if (archdep_pref_path == NULL) {
        /* XDG-spec path */
        return util_concat(archdep_home_path(), "/.config/vice/vice.rtc", NULL);
    } else {
        return util_concat(archdep_pref_path, "/vice-rtc", NULL);
    }
}

int archdep_file_is_chardev(const char *name)
{
    INCOMPLETE_IMPLEMENTATION();
    return 0;
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

int archdep_fix_permissions(const char *file_name)
{
    NOT_IMPLEMENTED();
    return 0;
}


