/*
 * pci-beos-drv.c - BeOS specific PCI driver code.
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

#ifdef BEOS_COMPILE

#include <stdio.h>
#include <string.h>

#include "io-access.h"
#include "log.h"
#include "types.h"

#include "pci-beos-drv.h"

static int pci_get_direct_base(int vendorID, int deviceID, uint32_t *base1, uint32_t *base2)
{
    int bus_index;
    int slot_index;
    int func_index;
    unsigned int address;
    unsigned int device;
    uint32_t tmp;
    int res = 0;

    io_access_store_byte(0xCFB, 0x01);
    tmp = io_access_read_long(0xCF8);
    io_access_store_long(0xCF8, 0x80000000);
    if (io_access_read_long(0xCF8) == 0x80000000) {
        res = 1;
    }
    io_access_store_long(0xCF8, tmp);

    if (!res) {
        return -1;
    }

    for (bus_index = 0; bus_index < 256; ++bus_index) {
        for (slot_index = 0; slot_index < 32; ++slot_index) {
            for (func_index = 0; func_index < 8; ++func_index) {
                address = 0x80000000 | (bus_index << 16) | (slot_index << 11) | (func_index << 8);
                io_access_store_long(0xCF8, address);
                device = io_access_read_long(0xCFC);
                if (device == (vendorID | (deviceID << 16))) {
                    address |= 0x10;
                    io_access_store_long(0xCF8, address);
                    *base1 = io_access_read_long(0xCFC);
                    address |= 0x04;
                    io_access_store_long(0xCF8, address);
                    *base2 = io_access_read_long(0xCFC);
                    return 0;
                }
            }
        }
    }
    return -1;
}

int pci_get_base(int vendorID, int deviceID, uint32_t *base1, uint32_t *base2)
{
    int retval = -1;

    retval = pci_get_direct_base(vendorID, deviceID, base1, base2);

    return retval;
}
#endif
