/*
 * blockdev.c
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdint.h>

#include "vice.h"

#include "vice_sdl.h"

#include "blockdev.h"
#include "types.h"

#ifndef RW_SEEK_SET
#define RW_SEEK_SET SEEK_SET
#endif

static SDL_RWops* device = NULL;

int blockdev_open(const char *name, unsigned int *read_only)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    if (*read_only == 0) {
        device = SDL_RWFromFile(name, "rw");

        if (device == NULL) {
            return -1;
        }
    } else {
        device = SDL_RWFromFile(name, "r");

        if (device == NULL) {
            return -1;
        }
    }

    return 0;
}

int blockdev_close(void)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    if (device && SDL_RWclose(device)) {
        return -1;
    }

    device = NULL;
    return 0;
}

/*-----------------------------------------------------------------------*/

int blockdev_read_sector(uint8_t *buf, unsigned int track, unsigned int sector)
{
    int offset;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    offset = ((track - 1) * 40 + sector) * 256;

    SDL_RWseek(device, offset, RW_SEEK_SET);

    if (SDL_RWread(device, (void *)buf, 256, 1) != 1) {
        return -1;
    }

    return 0;
}

int blockdev_write_sector(const uint8_t *buf, unsigned int track, unsigned int sector)
{
    int offset;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    offset = ((track - 1) * 40 + sector) * 256;

    SDL_RWseek(device, offset, RW_SEEK_SET);

    if (SDL_RWwrite(device, (void *)buf, 256, 1) != 1) {
        return -1;
    }

    return 0;
}

/*-----------------------------------------------------------------------*/

void blockdev_init(void)
{
}

int blockdev_resources_init(void)
{
    return 0;
}

int blockdev_cmdline_options_init(void)
{
    return 0;
}
