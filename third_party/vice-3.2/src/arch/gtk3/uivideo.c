/** \file   uivideo.c
 * \brief   Video chip/config helpers
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "machine.h"

#include "uivideo.h"


/** \brief  Get video chip name by `machine_class`
 *
 * \return  Video chip name (the prefix used in "${CHIP}FooBar" resources)
 *
 * \warning Returns "VICII" for C128, not VDC
 */
const char *uivideo_chip_name(void)
{
    switch (machine_class) {
        /* VIC */
        case VICE_MACHINE_VIC20:
            return "VIC";

        /* VIC-II */
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_CBM5x0:   /* fall through */
        case VICE_MACHINE_VSID:
            return "VICII";

        /* TED */
        case VICE_MACHINE_PLUS4:
            return "TED";

        /* CRTC */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            return "Crtc";

        default:
            /* should never get here */
            fprintf(stderr, "%s:%d:%s(): error: got machine class %d\n",
                    __FILE__, __LINE__, __func__, machine_class);
            exit(1);
    }
}


/** \brief  Get video chip ID by `machine_class`
 *
 * \return  Video chip ID (\see src/arch/gtk3/uivideo.h)
 *
 * \warning Returns "VICII" for C128, not VDC
 */
int uivideo_chip_id(void)
{
    switch (machine_class) {
        /* VIC */
        case VICE_MACHINE_VIC20:
            return UI_VIDEO_CHIP_VIC;

        /* VIC-II */
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_CBM5x0:   /* fall through */
        case VICE_MACHINE_VSID:
            return UI_VIDEO_CHIP_VICII;

        /* TED */
        case VICE_MACHINE_PLUS4:
            return UI_VIDEO_CHIP_TED;

        /* CRTC */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            return UI_VIDEO_CHIP_CRTC;

        default:
            /* should never get here */
            fprintf(stderr, "%s:%d:%s(): error: got machine class %d\n",
                    __FILE__, __LINE__, __func__, machine_class);
            exit(1);
    }
}


/** \brief  Determine if the current video chip supports VerticalStretch
 *
 * \param[in]   chip    chip name (only used in case of the C128
 *
 * \return  bool
 */
bool uivideo_chip_has_vert_stretch(const char *chip)
{
    switch (machine_class) {
        /* CRTC */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            return true;
        /* VDC */
        case VICE_MACHINE_C128:
            return (strcmp(chip, "VDC") == 0);
        /* VIC, VIC-II, TED */
        default:
            return false;
    }
}


/** \brief  Determine if the current video chip supports BorderMode
 *
 * \param[in]   chip    chip name (only used in case of the C128)
 *
 * \return  bool
 */
bool uivideo_chip_has_border_mode(const char *chip)
{
    switch (machine_class) {
        /* CRTC */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            return false;
        /* VDC */
        case VICE_MACHINE_C128:
            return !(strcmp(chip, "VDC") == 0);
        /* VIC, VIC-II, TED */
        default:
            return true;
    }
}


/** \brief  Determine if the current video chip supports VSP-bug emulation
 *
 * \param[in]   chip    chip name (only used in case of the C128)
 *
 * \return  bool
 */
bool uivideo_chip_has_vsp_bug(const char *chip)
{
    switch (machine_class) {
        case VICE_MACHINE_C64SC:      /* fall through */
        case VICE_MACHINE_SCPU64:
            return true;
        /* VDC */
        case VICE_MACHINE_C128:
            /* not right now, unless VICII emulation in x128 gets it later */
            /* return !(strcmp(chip, "VDC") == 0); */
            return false;

        default:
            return false;
    }
}


/** \brief  Determine if the current video chip supports sprites
 *
 * \param[in]   chip    chip name (only used in case of the C128
 *
 * \return  bool
 */
bool uivideo_chip_has_sprites(const char *chip)
{
    switch (machine_class) {
        /* VIC-II */
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_CBM5x0:
            return true;
        /* VDC or VICII */
        case VICE_MACHINE_C128:
            return !(strcmp(chip, "VDC") == 0);

        default:
            return false;
    }
}
