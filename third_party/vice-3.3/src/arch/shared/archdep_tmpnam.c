/** \file   archdep_tmpnam.c
 * \brief   Generate a unique, temporary filename
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
#include "archdep_defs.h"

#include "lib.h"
#include "log.h"
#include "ioutil.h"
#include "util.h"
#include "archdep_atexit.h"
#include "archdep_boot_path.h"
#include "archdep_join_paths.h"

/* TODO: #include for AmigaOS tmpnam() */
/* TODO: #include for BeOS/Haiku tmpnam() */
/* TODO: #include for OS/2 tmpnam() */

/* Seems like tmpnam() is available in stdio.h for Linux, BSD, Windows and
 * perhaps others
 */
#include <stdio.h>
#include <string.h>
#ifdef ARCHDEP_OS_UNIX
# include <unistd.h>
#endif
#ifdef ARCHDEP_OS_WINDOWS
# include "windows.h"
#endif

#include "archdep_tmpnam.h"


char *archdep_tmpnam(void)
{
#ifdef ARCHDEP_OS_AMIGA
    return lib_stralloc(tmpnam(NULL));
#elif defined(ARCHDEP_OS_BEOS)
    return lib_stralloc(tmpnam(NULL));
#elif defined(ARCHDEP_OS_OS2)
    return = lib_stralloc(tmpnam(NULL));
#elif defined(ARCHDEP_OS_UNIX)
    /*
     * Linux manpage for tmpnam(3) says to never use it, FreeBSD indicates the
     * same.
     */


    size_t maxlen = ioutil_maxpathlen();
# ifdef HAVE_MKSTEMP
    char *tmp_name;
    const char *mkstemp_template = "/vice.XXXXXX";
    int fd;
    char *tmp;
    char *final_name;

    tmp_name = lib_malloc(maxlen);

#  ifdef USE_EXE_RELATIVE_TMP
    fprintf("USING EXE REL CRAP\n");
    strcpy(tmp_name, archdep_boot_path());
    strcat(tmp_name, "/tmp");
#  else
    tmp = getenv("TMPDIR");
    if (tmp != NULL) {
        strncpy(tmp_name, tmp, maxlen);
        tmp_name[maxlen - sizeof(mkstemp_template)] = '\0';
    } else {
        /* fall back to /tmp */
        strcpy(tmp_name, "/tmp");
    }
#  endif
    strcat(tmp_name, mkstemp_template);
    fd = mkstemp(tmp_name);
    if (fd < 0) {
        tmp_name[0] = '\0';
    } else {
        close(fd);
    }

    /* reduce memory usage, not strictly required since I think on Linux
     * MAXPATH is 4096 or so, not a big slab memory */
    final_name = lib_stralloc(tmp_name);
    lib_free(tmp_name);
    return final_name;
# else
    return lib_stralloc(tmpnam(NULL));
# endif
#elif defined(ARCHDEP_OS_WINDOWS)
    /*
     * This blows and should probably be replaced with GetTempFileName() or
     * something similar
     */
    char *temp_path;
    char *temp_name;
    size_t maxlen = ioutil_maxpathlen();

    temp_path = lib_malloc(maxlen);
    temp_name = lib_malloc(maxlen);

    if (GetTempPath(maxlen, temp_path) == 0) {
        log_error(LOG_ERR, "failed to get Windows temp dir.");
        lib_free(temp_path);
        lib_free(temp_name);
        archdep_vice_exit(1);
    }


    if (GetTempFileName(temp_path, "vic", 0, temp_name) == 0) {
        log_error(LOG_ERR, "failed to construct as Windows temp file.");
        lib_free(temp_path);
        lib_free(temp_name);
        archdep_vice_exit(1);
    }

    printf("GOT TEMP FILE '%s'\n", temp_name);

    lib_free(temp_path);
    return temp_name;
#endif
}
