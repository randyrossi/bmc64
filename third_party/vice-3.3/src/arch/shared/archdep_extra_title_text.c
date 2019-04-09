/** \file   archdep_extra_title_text.c
 *
 * \brief   Extra text to use in the title bar
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Provide extra text for the window title bar indicating which key to press
 * to access the menu. Only used in the SDL port.
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

#include "kbd.h"
#include "lib.h"
#include "util.h"

#include "archdep_extra_title_text.h"


#ifdef SDL_UI_SUPPORT
/** \brief  Extra text for the window title bar
 *
 * Heap-allocated by archdep_extra_title_text(), needs to be freed on emu exit
 * with archdep_extra_title_text_free() (SDL only)
 */
static char *extra_title_text = NULL;
#endif


char *archdep_extra_title_text(void)
{
#ifdef SDL_UI_SUPPORT
    extra_title_text = util_concat(", press \"",
                                   kbd_get_menu_keyname(),
                                   "\" for the menu.",
                                   NULL);
    return extra_title_text;
#else
    return NULL;
#endif
}


void archdep_extra_title_text_free(void)
{
#ifdef SDL_UI_SUPPORT
    if (extra_title_text != NULL) {
        lib_free(extra_title_text);
    }
#endif
}

