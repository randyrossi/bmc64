/** \file   archdep_user_config_path.c
 * \brief   Retrieve path to the user's configuration directory
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Get path to user's VICE configuration directory, this is where the vice
 * files like vicerc/vice.ini, vice.log, autostart-$emu.d64 will be.
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

#include "lib.h"
#include "log.h"
#include "archdep_boot_path.h"
#include "archdep_home_path.h"
#include "archdep_join_paths.h"
#include "archdep_xdg.h"

#include "archdep_user_config_path.h"


/** \brief  User's VICE configuration directory reference
 *
 * Allocated once in the first call to archdep_user_config_path(), should be
 * freed on emulator exit with archdep_user_config_path_free()
 */
static char *user_config_dir = NULL;


/** \brief  Get path to the user's VICE configuration directory
 *
 * On systems supporting home directories this will return a directory inside
 * the home directory, depending on OS:
 *
 * - Windows: $HOME\\AppData\\Roaming\\vice
 * - Unix: $HOME/.config/vice (rather: XDG_CONFIG_HOME)
 * - BeOS: $HOME/config/settings/vice
 *   (Haiku sets $XDG_CONFIG_HOME to '/boot/home/config/settings')
 *
 * On other systems the path to the executable is returned.
 *
 * Free memory used on emulator exit with archdep_user_config_path_free()
 *
 * \return  path to VICE config directory
 */
char *archdep_user_config_path(void)
{
    if (user_config_dir != NULL) {
        return user_config_dir;
    }

#if defined(ARCHDEP_OS_UNIX)
    char *xdg_config = archdep_xdg_config_home();
    user_config_dir = archdep_join_paths(xdg_config, "vice", NULL);
    lib_free(xdg_config);

#elif defined(ARCHDEP_OS_WINDOWS)
    user_config_dir = archdep_join_paths(archdep_home_path(),
                                         "AppData", "Roaming", "vice", NULL);
#elif defined(ARCHDEP_OS_BEOS)
    user_config_dir = archdep_join_paths(archdep_home_path(),
                                         "config", "settings", "vice", NULL);
#else
    user_config_dir = lib_stralloc(archdep_boot_path());
#endif
    return user_config_dir;
}


/** \brief  Free memory used by the user's config path
 */
void archdep_user_config_path_free(void)
{
    if (user_config_dir != NULL) {
        lib_free(user_config_dir);
        user_config_dir = NULL;
    }
}
