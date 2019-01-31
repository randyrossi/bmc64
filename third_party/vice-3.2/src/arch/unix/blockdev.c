/*
 * blockdev.c
 *
 * Written by
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

#include "vice.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include "blockdev.h"
#include "log.h"
#include "types.h"

static int device;

int blockdev_open(const char *name, unsigned int *read_only)
{
    if (*read_only == 0) {
        device = open(name, O_RDWR);

        if (device == -1) {
            device = open(name, O_RDONLY);
            if (device == -1) {
                return -1;
            }
            *read_only = 1;
        }
    } else {
        device = open(name, O_RDONLY);

        if (device == -1) {
            return -1;
        }
    }

    return 0;
}

int blockdev_close(void)
{
    return close(device);
}

/*-----------------------------------------------------------------------*/

int blockdev_read_sector(uint8_t *buf, unsigned int track, unsigned int sector)
{
    off_t offset;

    offset = ((track - 1) * 40 + sector) * 256;

    lseek(device, offset, SEEK_SET);

    if (read(device, (void *)buf, 256) != 256) {
        return -1;
    }

    return 0;
}

int blockdev_write_sector(const uint8_t *buf, unsigned int track, unsigned int sector)
{
    off_t offset;

    offset = ((track - 1) * 40 + sector) * 256;

    lseek(device, offset, SEEK_SET);

    if (write(device, (void *)buf, 256) != 256) {
        return -1;
    }

    return 0;
}

/*-----------------------------------------------------------------------*/

void blockdev_init(void)
{
}

int blockdev_resources_init()
{
    return 0;
}

int blockdev_cmdline_options_init()
{
    return 0;
}
