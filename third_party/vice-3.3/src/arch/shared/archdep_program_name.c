/** \file   archdep_program_name.c
 * \brief   Retrieve name of currently running binary
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Get name of running executable, stripping path and extension (if present).
 *
 * OS support:
 *  - Linux
 *  - Windows
 *  - MacOS
 *  - BeOS/Haiku
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

#include "lib.h"
#include "log.h"

#ifdef AMIGA_SUPPORT
/* some includes */
#endif

/* for readlink(2) */
#ifdef UNIX_COMPILE
# include <unistd.h>
#endif

#ifdef MACOSX_SUPPORT
# include <libproc.h>
#endif

/* for GetModuleFileName() */
#ifdef WIN32_COMPILE
# include "windows.h"
#endif

#include "archdep_atexit.h"
#include "archdep_program_path.h"

#include "archdep_program_name.h"


/** \brief  Program name
 *
 * Heap allocated on the first call, must be free when exiting the program
 * with lib_free().
 */
static char *program_name = NULL;


#if defined(WIN32_COMPILE) || defined(OS2_COMPILE) || \
    defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__) || defined(__DOS__)
/** \brief  Helper function for Windows and OS/2
 *
 * \param[in]   buf string to parse binary name from
 *
 * \return  heap-allocated binary name, free with lib_free()
 */
static char *prg_name_win32_os2(const char *buf)
{
    const char *s;
    const char *e;
    size_t len;
    char *tmp;

    s = strrchr(buf, '\\');
    if (s == NULL) {
        s = buf;
    } else {
        s++;
    }
    e = strchr(s, '.');
    if (e == NULL) {
        e = buf + strlen(buf);
    }
    len = (int)(e - s + 1);
    tmp = lib_malloc(len);
    memcpy(tmp, s, len - 1);
    tmp[len - 1] = 0;

    return tmp;
}
#endif


#if defined(UNIX_COMPILE) || defined(BEOS_COMPILE)
/** \brief  Helper function for Unix-ish systems
 *
 * \param[in]   buf string to parse binary name from
 *
 * \return  heap-allocated binary name, free with lib_free()
 */
static char *prg_name_unix(const char *buf)
{
    const char *p;
    char *tmp;

    p = strrchr(buf, '/');
    if (p == NULL) {
        tmp = lib_stralloc(buf);
    } else {
        tmp = lib_stralloc(p + 1);
    }
    return tmp;
}
#endif

/** \brief  Get name of the currently running binary
 *
 * Allocates the name on the first call, this must be free with lib_free()
 * when exiting the program.
 *
 * \return  program name
 */
const char *archdep_program_name(void)
{
    const char *execpath;

    /* if we already have found the program name, just return it */
    if (program_name != NULL) {
        return program_name;
    }


    execpath = archdep_program_path();
    if (execpath == NULL) {
        log_error(LOG_ERR, "bollocks");
        archdep_vice_exit(1);
    }

#ifdef AMIGA_SUPPORT
    char *p;

    p = FilePart(execpath);
    if (p != NULL) {
        program_name = lib_stralloc(p);
    } else {
        log_error(LOG_ERR, "failed to retrieve program name.");
        archdep_vice_exit(1);
    }
#endif

#ifdef UNIX_COMPILE
    /* XXX: Only works on Linux, support for *BSD, Solaris and MacOS to be
     *      added later:
     *
     *      MacOS:      _NSGetExecutablePath()
     *      Solaris:    getexecname()
     *      FreeBSD:    sysctl CTL_KERN_PROC KERN_PROC_PATHNAME - 1 (???)
     *      NetBSD:     readlink /proc/curproc/exe
     *      DFlyBSD:    readlink /proc/curproc/file
     *      OpenBSD:    ???
     */

    program_name = prg_name_unix(execpath);
#endif

#if defined(WIN32_COMPILE) || defined(OS2_COMPILE) || \
    defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__) || defined(__DOS__)
    program_name = prg_name_win32_os2(execpath);
#endif

#ifdef BEOS_COMPILE
    program_name = prg_name_unix(execpath);
#endif
#if 0
    printf("%s: got program name '%s'\n", __func__, program_name);
#endif
    return program_name;
}


/** \brief  Free program name
 */
void archdep_program_name_free(void)
{
    if (program_name != NULL) {
        lib_free(program_name);
        program_name = NULL;
    }
}
