/** \file   archdep_default_sysfile_pathlist.c
 * \brief   Get a list of paths of required data files
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
#include <stddef.h>

#include "lib.h"
#include "log.h"
#include "util.h"

#include "archdep_boot_path.h"
#include "archdep_join_paths.h"
#include "archdep_user_config_path.h"


#include "archdep_default_sysfile_pathlist.h"

/** \brief  Total number of pathnames to store in the pathlist
 *
 * 16 seems to be enough, but it can always be increased to support more.
 */
#define TOTAL_PATHS 16


/** \brief  Reference to the sysfile pathlist
 *
 * This keeps a copy of the generated sysfile pathlist so we don't have to
 * generate it each time it is needed.
 */
static char *sysfile_path = NULL;


/** \brief  Generate a list of search paths for VICE system files
 *
 * \param[in]   emu_id  emulator ID (ie 'C64 or 'VSID')
 *
 * \return  heap-allocated string, to be freed by the caller
 */
char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    const char *boot_path = archdep_boot_path();
#ifndef ARCHDEP_OS_WINDOWS
    const char *home_path = archdep_user_config_path();
#endif

    char *lib_root = NULL;
    char *lib_machine_roms = NULL;
    char *lib_drive_roms = NULL;
    char *lib_printer_roms = NULL;
    char *boot_root = NULL;
    char *boot_machine_roms = NULL;
    char *boot_drive_roms = NULL;
    char *boot_printer_roms = NULL;
    char *home_root = NULL;
    char *home_machine_roms = NULL;
    char *home_drive_roms = NULL;
    char *home_printer_roms = NULL;

    const char *paths[TOTAL_PATHS + 1];
    int i;


    if (sysfile_path != NULL) {
        /* sysfile.c appears to free() this */
        return lib_stralloc(sysfile_path);
    }

    /* zero out the array of paths to join later */
    for (i = 0; i <= TOTAL_PATHS; i++) {
        paths[i] = NULL;
    }


#ifdef ARCHDEP_OS_UNIX

# ifdef MACOSX_BUNDLE

    /* ROM paths relative to the Contents/Resources/bin directory 
       in the bundle */
    lib_machine_roms = archdep_join_paths(
            boot_path, "..", "ROM", emu_id, NULL);
    lib_drive_roms = archdep_join_paths(
            boot_path, "..", "ROM", "DRIVES", NULL);
    lib_printer_roms = archdep_join_paths(
            boot_path, "..", "ROM", "PRINTER", NULL);

    /* ROM paths relative to the Contents/MacOS directory in the bundle
       (currently unused, but was used by SDL UI in VICE 3.2) */
    boot_machine_roms = archdep_join_paths(
            boot_path, "..", "Resources", "ROM", emu_id, NULL);
    boot_drive_roms = archdep_join_paths(
            boot_path, "..", "Resources", "ROM", "DRIVES", NULL);
    boot_printer_roms = archdep_join_paths(
            boot_path, "..", "Resources", "ROM", "PRINTER", NULL);

# else
    lib_machine_roms = archdep_join_paths(LIBDIR, emu_id, NULL);
    lib_drive_roms = archdep_join_paths(LIBDIR, "DRIVES", NULL);
    lib_printer_roms = archdep_join_paths(LIBDIR, "PRINTER", NULL);

    boot_machine_roms = archdep_join_paths(boot_path, emu_id, NULL);
    boot_drive_roms = archdep_join_paths(boot_path, "DRIVES", NULL);
    boot_printer_roms = archdep_join_paths(boot_path, "PRINTER", NULL);
# endif

    /* home path based paths */
    home_machine_roms = archdep_join_paths(home_path, emu_id, NULL);
    home_drive_roms = archdep_join_paths(home_path, "DRIVES", NULL);
    home_printer_roms = archdep_join_paths(home_path, "PRINTER", NULL);

#elif defined(ARCHDEP_OS_WINDOWS) || defined(ARCHDEP_OS_OS2) \
    || defined(ARCHDEP_OS_MSDOS)
    boot_machine_roms = archdep_join_paths(boot_path, emu_id, NULL);
    boot_drive_roms = archdep_join_paths(boot_path, "DRIVES", NULL);
    boot_printer_roms = archdep_join_paths(boot_path, "PRINTER", NULL);
#if 0
    home_machine_roms = archdep_join_paths(home_path, emu_id, NULL);
    home_drive_roms = archdep_join_paths(home_path, "DRIVES", NULL);
    home_printer_roms = archdep_join_paths(home_path, "PRINTER", NULL);
#endif
#elif defined(ARCHDEP_OS_AMIGA) || defined(ARCHDEP_OS_BEOS)
    boot_machine_roms = archdep_join_paths(boot_path, emu_id, NULL);
    boot_drive_roms = archdep_join_paths(boot_path, "DRIVES", NULL);
    boot_printer_roms = archdep_join_paths(boot_path, "PRINTER", NULL);
#endif
    /* now join everything together */
    i = 0;

    /* LIBDIR paths */
    if (lib_root != NULL) {
        paths[i++] = lib_root;
    }
    if (lib_machine_roms != NULL) {
        paths[i++] = lib_machine_roms;
    }
    if (lib_drive_roms != NULL) {
        paths[i++] = lib_drive_roms;
    }
    if (lib_printer_roms != NULL) {
        paths[i++] = lib_printer_roms;
    }
    /* boot paths */
    if (boot_root != NULL) {
        paths[i++] = boot_root;
    }
    if (boot_machine_roms != NULL) {
        paths[i++] = boot_machine_roms;
    }
    if (boot_drive_roms != NULL) {
        paths[i++] = boot_drive_roms;
    }
    if (boot_printer_roms != NULL) {
        paths[i++] = boot_printer_roms;
    }

    /* home paths */
    if (home_root != NULL) {
        paths[i++] = home_root;
    }
    if (home_machine_roms != NULL) {
        paths[i++] = home_machine_roms;
    }
    if (home_drive_roms != NULL) {
        paths[i++] = home_drive_roms;
    }
    if (home_printer_roms != NULL) {
        paths[i++] = home_printer_roms;
    }

    /* terminate list */
    paths[i] = NULL;
    sysfile_path = util_strjoin(paths, ARCHDEP_FINDPATH_SEPARATOR_STRING);

    /* TODO: free intermediate strings */
    /* LIBDIR paths */
    if (lib_root != NULL) {
        lib_free(lib_root);
    }
    if (lib_machine_roms != NULL) {
        lib_free(lib_machine_roms);
    }
    if (lib_drive_roms != NULL) {
        lib_free(lib_drive_roms);
    }
    if (lib_printer_roms != NULL) {
        lib_free(lib_printer_roms);
    }
    /* boot paths */
    if (boot_root != NULL) {
        lib_free(boot_root);
    }
    if (boot_machine_roms != NULL) {
        lib_free(boot_machine_roms);
    }
    if (boot_drive_roms != NULL) {
        lib_free(boot_drive_roms);
    }
    if (boot_printer_roms != NULL) {
        lib_free(boot_printer_roms);
    }

    /* home paths */
    if (home_root != NULL) {
        lib_free(home_root);
    }
    if (home_machine_roms != NULL) {
        lib_free(home_machine_roms);
    }
    if (home_drive_roms != NULL) {
        lib_free(home_drive_roms);
    }
    if (home_printer_roms != NULL) {
        lib_free(home_printer_roms);
    }

#if 0
    log_message(LOG_DEFAULT, "Search path = %s", sysfile_path);
    printf("%s(): paths = '%s'\n", __func__, sysfile_path);
#endif
    /* sysfile.c appears to free() this (ie TODO: fix sysfile.c) */
    return lib_stralloc(sysfile_path);
}


/** \brief  Free the internal copy of the sysfile pathlist
 *
 * Call on emulator exit
 */

void archdep_default_sysfile_pathlist_free(void)
{
    if (sysfile_path != NULL) {
        lib_free(sysfile_path);
        sysfile_path = NULL;
    }
}
