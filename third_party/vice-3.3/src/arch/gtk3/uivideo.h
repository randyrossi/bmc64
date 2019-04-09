/** \file   uivideo.h
 * \brief   Video chip/config helpers - header
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

#ifndef VICE_UIVIDEO_H
#define VICE_UIVIDEO_H

#include "vice.h"


/** \brief  Video chip ID enum
 */
enum {
    UI_VIDEO_CHIP_NONE = -1,    /**< invalid chip */
    UI_VIDEO_CHIP_VICII,        /**< VIC-II */
    UI_VIDEO_CHIP_VIC,          /**< VIC */
    UI_VIDEO_CHIP_CRTC,         /**< CRTC */
    UI_VIDEO_CHIP_TED           /**< TED */
};


const char *uivideo_chip_name(void);
int         uivideo_chip_id  (void);
int         uivideo_chip_has_vert_stretch(const char *chip);
int         uivideo_chip_has_border_mode(const char *chip);
int         uivideo_chip_has_vsp_bug(const char *chip);
int         uivideo_chip_has_sprites(const char *chip);

#endif
