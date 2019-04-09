/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/main.c
 * \brief   Main/shared library code
 *
 * The information used about the structure of the various HVSC file formats
 * was taken from the files in the DOCUMENTS directory of the HVSC \#68.
 * The HVSC can be found at https://www.hvsc.c64.org
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 *  HVSClib - a library to work with High Voltage SID Collection files
 *  Copyright (C) 2018  Bas Wassink <b.wassink@ziggo.nl>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.*
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "hvsc.h"

#include "hvsc_defs.h"
#include "base.h"
#include "stil.h"
#include "sldb.h"

#include "main.h"


/** \brief  Initialize the library
 *
 * This sets the paths to the HSVC and the SLDB, STIL, and BUGlist files. The
 * \a path is expected to be an absolute path to the HVSC's root directory.
 *
 * \param[in]   path    absolute path to HVSC root directory
 *
 * \return  bool
 *
 * For example:
 * \code{.c}
 *
 *  // Initialize the library for use
 *  hvsc_init("/home/compyx/C64Music");
 *
 *  // Do stuff with hvsclib ...
 *
 *  // Clean up properly
 *  hvsc_exit();
 * \endcode
 *
 * \ingroup main
 */
int hvsc_init(const char *path)
{
    hvsc_errno = 0;
    return hvsc_set_paths(path);
}


/** \brief  Clean up memory used by the library
 *
 * Free all memory used by the library.
 *
 * \ingroup main
 */
void hvsc_exit(void)
{
    hvsc_free_paths();
}


/** \brief  Get library version string
 *
 * \return  version string: "maj.min.rev"
 *
 * \ingroup main
 */
const char *hvsc_lib_version_str(void)
{
    return HVSC_LIB_VERSION_STR;
}


/** \brief  Get library version number
 *
 * \param[out]  major       major version number
 * \param[out]  minor       minor version number
 * \param[out]  revision    revision number
 *
 * \ingroup main
 */
void hvsc_lib_version_num(int *major, int *minor, int *revision)
{
    *major = HVSC_LIB_VERSION_MAJ;
    *minor = HVSC_LIB_VERSION_MIN;
    *revision = HVSC_LIB_VERSION_REV;
}
