/** \file   archdep_create_user_config_dir.c
 * \brief   Create user config dir if it doesn't exist already
 *
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
#include <errno.h>
#include <string.h>

#include "lib.h"
#include "log.h"
#include "archdep_atexit.h"
#include "archdep_home_path.h"
#include "archdep_join_paths.h"
#include "archdep_mkdir.h"
#include "archdep_user_config_path.h"

#ifdef ARCHDEP_OS_UNIX
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include "archdep_create_user_config_dir.h"


void archdep_create_user_config_dir(void)
{
    char *cfg = archdep_user_config_path();

#if defined(ARCHDEP_OS_UNIX)
    const char *home = archdep_home_path();
    char *tmp;

    /*
     * Brute force create XDG ~/.config dir
     * Some systems without X11 don't have ~/.config, which makes sense since
     * XDG is a Freedesktop spec. We use it however for vicerc and other files
     * and it expected to be there
     */
    tmp = archdep_join_paths(home, ".config", NULL);    /* TODO: use define */
    mkdir(tmp, 0755);
    errno = 0;
    lib_free(tmp);
#endif

    if (archdep_mkdir(cfg, 0755) == 0) {
        return;     /* we created the dir */
    } else if (errno != EEXIST) {
        log_error(LOG_ERR, "failed to create user config dir '%s': %d: %s.",
                cfg, errno, strerror(errno));
        archdep_vice_exit(1);
    }
}
