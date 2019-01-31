/*
 * pci-dos-drv.c - MSDOS specific PCI driver code.
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

#ifdef __MSDOS__

#include <stdio.h>
#include <dpmi.h>
#include <string.h>

#include "log.h"
#include "types.h"

int vice_pci_install_check(void)
{
    __dpmi_regs r;

    memset(&r, 0, sizeof(r));

    r.x.ax = 0xb101;
    r.d.edi = 0x0;

    if (__dpmi_int(0x1a, &r) != 0) {
        log_message(LOG_DEFAULT, "BIOS does not support PCI.");
        return -1;
    }

    if (r.h.ah != 0 || r.d.edx != 0x20494350) {
        log_message(LOG_DEFAULT, "BIOS does not support PCI.");
        return -1;
    }

    if (!(r.h.al & 1)) {
        log_message(LOG_DEFAULT, "BIOS does not support PCI access mechanism #1.");
        return -1;
    }

    return 0;
}

int vice_pci_find(int vendorID, int deviceID, int index, int *bus, int *device, int *func)
{
    __dpmi_regs r;

    memset(&r, 0, sizeof(r));

    r.x.ax = 0xb102;
    r.x.cx = deviceID;
    r.x.dx = vendorID;
    r.x.si = index;

    if (__dpmi_int(0x1a, &r) != 0) {
        return -1;
    }

    if (r.h.ah == 0) {
        *bus = r.h.bh;
        *device = (r.h.bl >> 3) & 0x1f;
        *func = r.h.bl & 0x03;
        log_message(LOG_DEFAULT, "Device %04X, vendor %04X found at PCI bus %d, slot %d.", vendorID, deviceID, *bus, *device);
    }

    return r.h.ah;
}

int vice_pci_read_config_dword(int bus, int device, int func, int reg, uint32_t *value)
{
    __dpmi_regs r;

    memset(&r, 0, sizeof(r));

    r.x.ax = 0xb10a;
    r.h.bh = bus;
    r.h.bl = (device << 3) + func;
    r.x.di = reg;

    if (__dpmi_int(0x1a, &r) != 0) {
        return -1;
    }

    if (r.h.ah == 0) {
        *value = r.d.ecx;
    }

    return r.h.ah;
}
#endif
