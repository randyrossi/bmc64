/*
 * menu_media.c - SDL media saving menu
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
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

/** \file   src/arch/sdl/menu_media.c
 * \brief   Media submenu for SDL
 *
 * Organizes media recording into screenshot, sound and video.
 */

#include "vice.h"

#include "machine.h"
#include "uimenu.h"

#include "menu_common.h"
#include "menu_ffmpeg.h"
#include "menu_screenshot.h"
#include "menu_sound.h"
#include "menu_media.h"


/** \brief  Generic media menu */
ui_menu_entry_t media_menu[] = {
    { "Create screenshot",
        MENU_ENTRY_SUBMENU,
        submenu_callback,
        NULL },             /* set by uimedia_menu_create() */
    { "Create sound recording",
        MENU_ENTRY_SUBMENU,
        submenu_callback,
        (ui_callback_data_t)sound_record_menu },
#ifdef HAVE_FFMPEG
    { "Create video recording",
        MENU_ENTRY_SUBMENU,
        submenu_callback,
        (ui_callback_data_t)ffmpeg_menu },
#endif
    SDL_MENU_LIST_END
};


/** \brief  Set proper screenshot submenu, depending on machine
 *
 * \return  0 (OK)
 */
int uimedia_menu_create(void)
{
    switch (machine_class) {

        /* VIC/VICII/VDC */
        case VICE_MACHINE_C64:      /* fallthrough */
        case VICE_MACHINE_C64SC:    /* fallthrough */
        case VICE_MACHINE_C64DTV:   /* fallthrough */
        case VICE_MACHINE_C128:     /* fallthrough */
        case VICE_MACHINE_VIC20:    /* fallthrough */
        case VICE_MACHINE_SCPU64:   /* fallthrough */
        case VICE_MACHINE_CBM5x0:
            /* here be dragons: change the index if changing the order of the
             * items in `media_menu`
             */
            media_menu[0].data = (ui_callback_data_t)screenshot_vic_vicii_vdc_menu;
            break;

        /* CRTC */
        case VICE_MACHINE_PET:      /* fallthrough */
        case VICE_MACHINE_CBM6x0:
            /* again: nuclear missiles might launch if changing this */
            media_menu[0].data = (ui_callback_data_t)screenshot_crtc_menu;
            break;

        /* TED */
        case VICE_MACHINE_PLUS4:
            /* your C64 might change into a speccy if you change this */
            media_menu[0].data = (ui_callback_data_t)screenshot_ted_menu;
            break;
        default:
            /* VSID */
            break;
    }
    return 0;
}


/** \brief  Shutdown media menu
 *
 * Doesn't do anything right now, but perhaps later this is required.
 */
void uimedia_menu_shutdown(void)
{
    /* NOP */
}

