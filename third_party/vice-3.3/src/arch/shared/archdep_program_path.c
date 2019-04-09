/** \file   archdep_program_path.c
 * \brief   Retrieve path of currently running binary
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Get path to running executable.
 *
 * OS support:
 *  - Linux
 *  - Windows
 *  - MacOS
 *  - BeOS/Haiku (untested)
 *  - AmigaOS (untested)
 *  - OS/2 (untested)
 *  - MS-DOS (untested)
 *
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
#include "archdep_defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lib.h"
#include "log.h"

#ifdef ARCHDEP_OS_AMIGA
/* some includes */
#endif

/* for readlink(2) */
#if defined(ARCHDEP_OS_UNIX) || defined(ARCHDEP_OS_BEOS)
# include <unistd.h>
# ifdef ARCHDEP_OS_BSD_FREE
#  include <sys/sysctl.h>
# endif
# ifdef ARCHDEP_OS_OSX
#  include <libproc.h>
# endif
#endif

/* for GetModuleFileName() */
#ifdef ARCHDEP_OS_WINDOWS
# include "windows.h"
# include <direct.h>
#endif

#include "archdep_atexit.h"
#include "archdep_join_paths.h"
#include "archdep_path_is_relative.h"

#include "archdep_program_path.h"


/** \brief  Size of the buffer used to retrieve the path
 *
 */
#define PATH_BUFSIZE    4096


/** \brief  Reference to program path string
 *
 * Should be freed on emulator exit with archdep_program_path_free()
 */
static char *program_path = NULL;


/** \brief  Reference to argv[0]
 *
 * Do not free this, the C standard guarantees argv is available during a
 * program's lifetime, so this will work.
 */
static char *argv0_ref = NULL;


/** \brief  Buffer used to retrieve pathnames
 *
 * Various OS calls use this buffer to store the path to the running binary, if
 * such a call exists. The buffer should be large enough (I think Linux
 * defines PATH_MAX as 4096 by default, but that can be changed).
 * The various function calls guard against writing past this buffer's end, but
 * should some idiotic OS or user actually need a path larger than 4095 bytes,
 * tell 'em to fuck off or update PATH_BUFSIZE.
 */
static char buffer[PATH_BUFSIZE];


/** \brief  Set reference to argv[0]
 *
 * \param[in]    argv0   argv[0]
 */
void archdep_program_path_set_argv0(char *argv0)
{
    argv0_ref = argv0;
}


/** \brief  Fall back: try to get absolute path to exec via argv[0]
 *
 * This is unreliable and should only be used as a last resort.
 *
 * \return  bool (if this fails, we have to give up)
 */
static int argv_fallback(void)
{
    char cwd_buf[4096];
    char *result;
    size_t res_len;

    if (argv0_ref == NULL) {
        log_error(LOG_ERR, "argv[0] is NULL, giving up.");
        return 0;
    }
    if (*argv0_ref == '\0') {
        log_error(LOG_ERR, "argv[0] is empty, giving up.");
        return 0;
    }

    /* do we have an absolute path in argv[0]? */
    if (!archdep_path_is_relative(argv0_ref)) {
        strcpy(buffer, argv0_ref);
        return 1;
    }

    /*
     * Relative path in argv[0], try to get cwd and join it with argv[0]
     */
    memset(cwd_buf, 0, 4096);

#if defined(ARCHDEP_OS_UNIX) || defined(ARCHDEP_OS_BEOS)
    if (getcwd(cwd_buf, 4096 - 1) == NULL) {
        log_error(LOG_ERR, "failed to get cwd, giving up.");
        return 0;
    }
#elif defined(ARCHDEP_OS_WINDOWS)
    if (_getcwd(cwd_buf, 4096 -1) == NULL) {
        log_error(LOG_ERR, "failed to get cwd, giving up.");
        return 0;
    }
#else
    log_error(LOG_ERR,"no getcwd() support for current OS, giving up.");
    return 0;
#endif

    result = archdep_join_paths(cwd_buf, argv0_ref, NULL);
    res_len = strlen(result);
    if (res_len >= 4096) {
        /* insufficient space */
        log_error(LOG_ERR, "insufficient space for path, giving up.");
        lib_free(result);
        return 0;
    }
    memcpy(buffer, result, res_len + 1);
    lib_free(result);
    return 1;
}



/** \brief  Get absolute path to the running executable
 *
 * Free with archdep_program_path_free() on emulator exit.
 *
 * \return  absolute path to running executable
 */
const char *archdep_program_path(void)
{
    if (program_path != NULL) {
        /* already got it, return */
        return program_path;
    }

    /* zero out the buffer since readlink(2) doesn't add a nul character */
    memset(buffer, 0, PATH_BUFSIZE);


#ifdef ARCHDEP_OS_AMIGA

    /* do I need a header for this? */
    GetProgramName(buffer, PATH_BUFSIZE - 1);

#elif defined(ARCHDEP_OS_WINDOWS)

    if (GetModuleFileName(NULL, buffer, PATH_BUFSIZE - 1) == PATH_BUFSIZE - 1) {
        log_error(LOG_ERR,
                "failed to retrieve executable path, falling back"
                " to getcwd() + argv[0]");
        if (!argv_fallback()) {
            archdep_vice_exit(1);
        }
    }

#elif defined(ARCHDEP_OS_UNIX)

    /* XXX: Only works on Linux and OSX, support for *BSD etc to be added later
     *
     *      MacOS:      _NSGetExecutablePath()
     *      Solaris:    getexecname()
     *      FreeBSD:    sysctl CTL_KERN_PROC KERN_PROC_PATHNAME - 1 (???)
     *      NetBSD:     readlink /proc/curproc/exe
     *      DFlyBSD:    readlink /proc/curproc/file
     *      OpenBSD:    ???
     */

# ifdef ARCHDEP_OS_OSX

    /* get path via libproc */
    pid_t pid = getpid();
    if (proc_pidpath(pid, buffer, PATH_BUFSIZE - 1) <= 0) {
        log_error(LOG_ERR,
                "failed to retrieve executable path, falling back"
                " to getcwd() + argv[0]");
        if (!argv_fallback()) {
            archdep_vice_exit(1);
        }
    }

    /* TODO: other Unices */

# elif defined(ARCHDEP_OS_LINUX)

    /* Linux as a fallback (has it really come to this?) */
    if (readlink("/proc/self/exe", buffer, PATH_BUFSIZE - 1) < 0) {
        log_error(LOG_ERR,
                "failed to retrieve executable path, falling back"
                " to getcwd() + argv[0]");
        if (!argv_fallback()) {
            archdep_vice_exit(1);
        }
    }

    /* BSD's */
# elif defined(ARCHDEP_OS_BSD)
#  if defined(ARCHDEP_OS_BSD_FREE)

    int mib[4];
    size_t bufsize = PATH_BUFSIZE;

    /* /proc may not be available on FreeBSD */
    if (readlink("/proc/curproc/file", buffer, PATH_BUFSIZE - 1) < 0) {
        printf("%s(): failed to read /proc/curproc/file: %d: %s\n",
                __func__, errno, strerror(errno));
        /* try sysctl call */
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;

        if (sysctl(mib, 4, buffer, &bufsize, NULL, 0) < 0) {
            log_error(LOG_ERR,
                    "failed to retrieve executable path, falling back"
                    " to getcwd() + argv[0]");
            if (!argv_fallback()) {
                archdep_vice_exit(1);
            }
        }
#if 0
        printf("SYSCTL: %s\n", buffer);
#endif
    }

#  elif defined(ARCHDEP_OS_BSD_NET)

    if (readlink("/proc/curproc/exe", buffer, PATH_BUFSIZE - 1) < 0) {
        log_error(LOG_ERR,
                "failed to retrieve executable path, falling back"
                " to getcwd() + argv[0]");
        if (!argv_fallback()) {
            archdep_vice_exit(1);
        }
    }

#  elif defined(ARCHDEP_OS_BSD_OPEN)
    /*
     * I couldn't find any non-argv[0] solution for OpenBSD, so this will have
     * to do. --compyx
     */
    if (!argv_fallback()) {
        archdep_vice_exit(1);
    }
#  elif defined(ARCHDEP_OS_BSD_DRAGON)
#   error DragonFly BSD support missing

#  endif    /* end BSD's */

# endif /* end UNIX */
#else

    /*
     * Other systems (BeOS etc)
     */
    if (!argv_fallback()) {
        archdep_vice_exit(1);
    }

#endif
    program_path = lib_stralloc(buffer);
#if 0
    printf("%s(): program_path = %s\n", __func__, program_path);
#endif
    return program_path;
}


/** \brief  Free memory used by path to running executable
 *
 * Call from program exit
 */
void archdep_program_path_free(void)
{
    if (program_path != NULL) {
        lib_free(program_path);
        program_path = NULL;
    }
}
