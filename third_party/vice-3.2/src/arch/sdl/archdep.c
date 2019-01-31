/*
 * archdep.c - Miscellaneous system-specific stuff.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
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

#include "vice_sdl.h"
#include <stdio.h>

/* These functions are defined in the files included below and
   used lower down. */
static int archdep_init_extra(int *argc, char **argv);
static void archdep_shutdown_extra(void);

#ifdef AMIGA_SUPPORT
#include "archdep_amiga.c"
#endif

#ifdef BEOS_COMPILE
#include "archdep_beos.c"
#endif

#ifdef __OS2__
#include "archdep_os2.c"
#endif

#ifdef UNIX_COMPILE
#include "archdep_unix.c"
#endif

#ifdef WIN32_COMPILE
#include "archdep_win32.c"
#endif

#include "kbd.h"

#ifndef SDL_REALINIT
#define SDL_REALINIT SDL_Init
#endif

int archdep_init(int *argc, char **argv)
{
    if (SDL_REALINIT(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        return 1;
    }

    return archdep_init_extra(argc, argv);
}

static char *extra_title_text = NULL;

void archdep_shutdown(void)
{
    SDL_Quit();
#ifdef HAVE_NETWORK
    archdep_network_shutdown();
#endif
    archdep_shutdown_extra();
    if (extra_title_text) {
        lib_free(extra_title_text);
    }
}

char *archdep_extra_title_text(void)
{
    extra_title_text = util_concat(", press \"", kbd_get_menu_keyname(), "\" for the menu.", NULL);
    return extra_title_text;
}


/** \brief  Sanitize \a name by removing invalid characters for the current OS
 *
 * \param[in,out]   name    0-terminated string
 */
void archdep_sanitize_filename(char *name)
{
    while (*name != '\0') {
        int i = 0;
        while (illegal_name_tokens[i] != '\0') {
            if (illegal_name_tokens[i] == *name) {
                *name = '_';
                break;
            }
            i++;
        }
        name++;
    }
}

