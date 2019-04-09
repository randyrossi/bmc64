/** \file   archdep_vice_get_docsdir.c
 * \brief   Get path to VICE doc/ dir
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

#include <stdlib.h>

#include "archdep_defs.h"
#include "lib.h"
#include "archdep_boot_path.h"
#include "archdep_join_paths.h"

#include "archdep_get_vice_docsdir.h"


/** \brief  Get path to VICE doc dir
 *
 * \return  heap-allocated string, to be freed after use with lib_free()
 */
char *archdep_get_vice_docsdir(void)
{
    char *path;

#ifdef ARCHDEP_OS_UNIX
# ifdef MACOSX_BUNDLE
    /*    debug_gtk3("FIXME: MACOSX: archdep_get_vice_docsdir '%s%s'.",
            archdep_boot_path(), "/../doc/"); */
    path = archdep_join_paths(archdep_boot_path(), "..", "doc", NULL);
# else
    path = lib_stralloc(DOCDIR);
# endif
#else
    path = archdep_join_paths(archdep_boot_path(), "doc", NULL);
#endif
    return path;
}
