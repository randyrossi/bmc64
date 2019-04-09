/** \file   archdep_fix_permissions.c
 * \brief   Update permissions of a file to R+W
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(ARCHDEP_OS_UNIX) || defined(ARCHDEP_OS_WINDOWS)
# include <sys/stat.h>
# include <sys/types.h>
#endif
#ifdef ARCHDEP_OS_WINDOWS
# include <io.h>
# include <windows.h>
#endif

#include "lib.h"
#include "log.h"
#include "util.h"

#include "archdep_fix_permissions.h"


/** \brief  Update permissions of \a name to R+W
 *
 * \param[in]   name    pathname
 *
 * \return  bool
 */
int archdep_fix_permissions(const char *name)
{
#ifdef ARCHDEP_OS_WINDOWS
    return _chmod(name, _S_IREAD|_S_IWRITE);
#elif defined(ARCHDEP_OS_UNIX)
    mode_t mask = umask(0);
    umask(mask);
    return chmod(name, mask ^ 666);
#elif defined(ARCHDEP_OS_AMIGA)
    SetProtection(name, 0);
    return 1;   /* the code in sdl/archdep_amiga.c originally returned 0 here,
                   which doesn't make a lot of sense to me, why return failure
                   but still do some call? So should writing GIF screenshots
                   on AmigaOS suddenly fail, it's all my fault.
                 */

#elif defined(ARCHDEP_OS_BEOS)
    /* there's got to be some beos-ish stuff to change permissions, at least
     * with Haiku */
    return 0;

#endif
    /* OS/2 etc */
    return 0;
}
