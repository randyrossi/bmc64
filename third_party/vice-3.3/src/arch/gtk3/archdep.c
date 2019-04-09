/** \file   archdep.c
 * \brief   Wrappers for architecture/OS-specific code
 *
 * I've decided to use GLib's use of the XDG specification and the standard
 * way of using paths on Windows. So some files may not be where the older
 * ports expect them to be. For example, vicerc will be in $HOME/.config/vice
 * now, not $HOME/.vice. -- compyx
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
#include <glib.h>
#include <glib/gstdio.h>

#include "debug_gtk3.h"
#include "findpath.h"
#include "ioutil.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "util.h"
#include "uiapi.h"

/* Will get fixed once the code in this file gets moved to its proper location */
#include "../shared/archdep_xdg.h"
#include "../shared/archdep_defs.h"
#include "../shared/archdep_create_user_config_dir.h"
#include "../shared/archdep_join_paths.h"
#include "../shared/archdep_get_vice_docsdir.h"

#if 0
/** \brief  Prefix used for autostart disk images
 */
#define AUTOSTART_FILENAME_PREFIX   "autostart-"


/** \brief  Suffix used for autostart disk images
 */
#define AUTOSTART_FILENAME_SUFFIX   ".d64"
#endif

/** \brief  Reference to argv[0]
 *
 * FIXME: this is only used twice I think, better pass this as an argument to
 *        the functions using it
 */
static char *argv0 = NULL;


/** \brief  Path to the preferences directory of the emu
 */
const char *archdep_pref_path = NULL;


#ifdef UNIX_COMPILE
#include "archdep_unix.c"
#endif

#ifdef WIN32_COMPILE
#include "archdep_win32.c"
#endif


#if 0
/** \brief  Create and open temp file
 *
 * \param[in]   filename    pointer to object to store name of temp file
 * \param[in]   mode        mode to open file with (see fopen(3))
 *
 * \return  pointer to new file or `NULL` on error
 */
FILE *archdep_mkstemp_fd(char **filename, const char *mode)
{
    GError *err = NULL;
    /* this function already uses the OS's tmp dir as a prefix, so no need to
     * do stuff like getenv("TMP")
     */
    int fd = g_file_open_tmp("vice.XXXXXX", filename, &err);
    if (fd < 0) {
        return NULL;
    }
    return fdopen(fd, mode);
}
#endif


/** \brief  Arch-dependent init
 *
 * \param[in]   argc    pointer to argument count
 * \param[in]   argv    argument vector
 *
 * \return  0
 */
int archdep_init(int *argc, char **argv)
{
#ifdef HAVE_DEBUG_GTK3UI
    const char *prg_name;
    char *cfg_path;
    char *searchpath;
    char *vice_ini;
    char *datadir;
    char *docsdir;
# if defined(ARCHDEP_OS_LINUX) || defined(ARCHDEP_OS_BSD)
    char *xdg_cache;
    char *xdg_config;
    char *xdg_data;
# endif
#endif
    argv0 = lib_stralloc(argv[0]);

    /* set argv0 for program_name()/boot_path() calls (yes this sucks) */
    archdep_program_path_set_argv0(argv[0]);

    archdep_create_user_config_dir();

#ifdef HAVE_DEBUG_GTK3UI
    /* sanity checks, to remove later: */
    prg_name = archdep_program_name();
    searchpath = archdep_default_sysfile_pathlist(machine_name);
    cfg_path = archdep_user_config_path();
    vice_ini = archdep_default_resource_file_name();
    datadir = archdep_get_vice_datadir();
    docsdir = archdep_get_vice_docsdir();

# if defined(ARCHDEP_OS_LINUX) && defined(ARCHDEP_OS_BSD)
    xdg_cache = archdep_xdg_cache_home();
    xdg_config = archdep_xdg_config_home();
    xdg_data = archdep_xdg_data_home()l
# endif 

    debug_gtk3("program name    = \"%s\"", prg_name);
    debug_gtk3("user home dir   = \"%s\"", archdep_home_path());
    debug_gtk3("user config dir = \"%s\"", cfg_path);
    debug_gtk3("prg boot path   = \"%s\"", archdep_boot_path());
    debug_gtk3("VICE searchpath = \"%s\"", searchpath);
    debug_gtk3("VICE gui data   = \"%s\"", datadir);
    debug_gtk3("VICE docs path  = \"%s\"", docsdir);
    debug_gtk3("vice.ini path   = \"%s\"", vice_ini);

# if defined(ARCHDEP_OS_LINUX) || defined(ARCHDEP_OS_BSD)
    xdg_cache = archdep_xdg_cache_home();
    xdg_config = archdep_xdg_config_home();
    xdg_data = archdep_xdg_data_home();

    debug_gtk3("XDG_CACHE_HOME  = '%s'.", xdg_cache);
    debug_gtk3("XDG_CONFIG_HOME = '%s'.", xdg_config);
    debug_gtk3("XDG_DATA_HOME   = '%s'.", xdg_data);

    lib_free(xdg_cache);
    lib_free(xdg_config);
    lib_free(xdg_data);
# endif

    lib_free(searchpath);
    lib_free(vice_ini);
# if 0
    lib_free(cfg_path);
# endif
    lib_free(datadir);
    lib_free(docsdir);
#endif

    /* needed for early log control (parses for -silent/-verbose) */
    log_verbose_init(*argc, argv);

    return 0;
}

