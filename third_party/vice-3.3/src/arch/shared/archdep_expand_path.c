/** \file   archdep_expand_path.c
 * \brief   Expand a path into an absolute path
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ioutil.h"
#include "lib.h"
#include "log.h"
#include "util.h"
#include "archdep_defs.h"

#include "archdep_atexit.h"
#include "archdep_home_path.h"

/* TODO:    Include required headers for AmigaOS */

#include "archdep_expand_path.h"


/** \brief  Generate heap-allocated full pathname of \a orig_name
 *
 * Returns the absolute path of \a orig_name.
 * Expands '~' to the user's home path on Unix.
 * If the prefix in \a orig_name is not '\' and not '/' and not '~/' (Unix)
 * the file is assumed to reside in the current working directory, whatever
 * that may be.
 *
 * \param[out]  return_path pointer to expand path destination
 * \param[in]   orig_name   original path
 *
 * \return  0
 */
int archdep_expand_path(char **return_path, const char *orig_name)
{
#ifdef ARCHDEP_OS_UNIX
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
#elif defined(ARCHDEP_OS_WINDOWS)
    /* taken from the old WinVICE port (src/arch/win32/archdep.c): */
    *return_path = lib_stralloc(orig_name);
#elif defined(ARCHDEP_OS_AMIGA)
    /* taken from src/arch/sdl/archdep_amiga.c: */
    BPTR lock;

    lock = Lock(orig_name, ACCESS_READ);
    if (lock) {
        char name[1024];
        LONG rc;
        rc = NameFromLock(lock, name, 1024);
        UnLock(lock);
        if (rc != 0) {
            *return_path = lib_stralloc(name);
            return 0;
        }
    }
    *return_path = lib_stralloc(orig_name);
#elif defined(ARCHDEP_OS_BEOS)
    /* taken from src/arch/sdl/archdep_beos.c: */
    *return_path = lib_stralloc(orig_name);
#elif defined(ARCHDEP_OS_OS2)
    /* the OS/2 code is too terrible to include, so just exit: */
    log_err(LOG_ERR, "OS/2 code is too screwed up, sorry.");
    archdep_vice_exit(1);
# if 0
    if (filename[0] == '\\' || filename[1] == ':') {
        *return_path = lib_stralloc(filename);
    } else {
        char *p = (char *)malloc(512);
        while (getcwd(p, 512) == NULL) {
            return 0;
        }

        *return_path = util_concat(p, "\\", filename, NULL);
        lib_free(p);
    }
    return 0;
# endif

#else
    /* fallback */
    log_error(LOG_ERR, "unsupported OS: just returning input.");
    *return_path = lib_stralloc(orig_name);
#endif
    return 0;
}
