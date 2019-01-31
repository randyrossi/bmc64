/*
 * blockdev.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include <stdio.h>

#include "blockdev.h"

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

int blockdev_open(const char *name, unsigned int *read_only)
{
    return -1;
}

int blockdev_close(void)
{
    return -1;
}

int blockdev_read_sector(uint8_t *buf, unsigned int track, unsigned int sector)
{
    return -1;
}

int blockdev_write_sector(const uint8_t *buf, unsigned int track, unsigned int sector)
{
    return -1;
}
