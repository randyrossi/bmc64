/*
 * hd-dos-pci.c - MSDOS specific PCI hardsid driver.
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

/* Tested and confirmed working on:
 - MSDOS 6.00 (PCI HardSID)
 - MSDOS 6.00 (PCI HardSID Quattro)
 - Windows 95C (PCI HardSID)
 - Windows 95C (PCI HardSID Quattro)
 */

#include "vice.h"

#ifdef __MSDOS__

#ifdef HAVE_HARDSID

#include <stdio.h>
#include <dpmi.h>
#include <string.h>
#include <pc.h>
#include <unistd.h>

#include "hs-dos.h"
#include "log.h"
#include "pci-dos-drv.h"
#include "sid-snapshot.h"
#include "types.h"

#define MAXSID 4

typedef unsigned short uint16;
typedef unsigned long uint32;

static unsigned int base1;
static unsigned int base2;

static int sids_found = -1;
static int hssids[MAXSID] = {-1, -1, -1, -1};

static uint32 vice_pci_find_hardsid(int index)
{
    int i = 0, j = 0, res;
    int bus, device, func;
    uint32 retval;
    uint32 baseAddr;
    uint32 baseAddr1 = 0;
    uint32 baseAddr2 = 0;

    if (vice_pci_install_check() != 0) {
        return -1;
    }

    while (i <= index) {

        /* Find the HardSID card */
        res = vice_pci_find(0x6581, 0x8580, j++, &bus, &device, &func);
        if (res != 0) {
            return -1;
        }

        i++;
    }

    for (i = 0x10; i <= 0x24; i += 4) {

        /* Read a base address */
        res = vice_pci_read_config_dword(bus, device, func, i, &baseAddr);
        if (res != 0) {
            return -1;
        }

        /* Check for I/O space */
        if (baseAddr & 1) {
            if (baseAddr1 == 0) {
                baseAddr1 = baseAddr;
            } else {
                baseAddr2 = baseAddr;
            }
        }
    }

    baseAddr1 &= 0xfffc;
    baseAddr2 &= 0xfffc;

    retval = (baseAddr1 << 16) | (baseAddr2 & 0xffff);

    return retval;
}

static uint8_t read_sid(uint8_t reg, int chipno)
{
    uint8_t ret;

    outportb(base1 + 4, ((chipno << 6) | (reg & 0x1f) | 0x20));
    usleep(2);
    outportb(base2 + 2, 0x20);
    ret = inportb(base1);
    outportb(base2 + 2, 0x80);

    return ret;
}

static void write_sid(uint8_t reg, uint8_t data, int chipno)
{
    outportb(base1 + 3, data);
    outportb(base1 + 4, (chipno << 6) | (reg & 0x1f));
    usleep(2);
}

static int detect_sid_uno(void)
{
    int i;
    int j;

    for (j = 0; j < 4; ++j) {
        for (i = 0x18; i >= 0; --i) {
            write_sid((unsigned short)i, 0, j);
        }
    }

    write_sid(0x12, 0xff, 0);

    for (i = 0; i < 100; ++i) {
        if (read_sid(0x1b, 3)) {
            return 0;
        }
    }

    write_sid(0x0e, 0xff, 0);
    write_sid(0x0f, 0xff, 0);
    write_sid(0x12, 0x20, 0);

    for (i = 0; i < 100; ++i) {
        if (read_sid(0x1b, 3)) {
            return 1;
        }
    }
    return 0;
}

static int detect_sid(int chipno)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        write_sid((uint8_t)i, 0, chipno);
    }

    write_sid(0x12, 0xff, chipno);

    for (i = 0; i < 100; ++i) {
        if (read_sid(0x1b, chipno)) {
            return 0;
        }
    }

    write_sid(0x0e, 0xff, chipno);
    write_sid(0x0f, 0xff, chipno);
    write_sid(0x12, 0x20, chipno);

    for (i = 0; i < 100; ++i) {
        if (read_sid(0x1b, chipno)) {
            return 1;
        }
    }
    return 0;
}

static char *HStype = "PCI HardSID Quattro";

int hs_pci_open(void)
{
    int i, j;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting PCI HardSID boards.");

    base1 = vice_pci_find_hardsid(0);
    base2 = base1 & 0xffff;
    base1 >>= 16;

    if (base1 == 0 || base2 == 0) {
        log_message(LOG_DEFAULT, "No PCI HardSID boards found.");
        return -1;
    }

    /* Reset the hardsid PCI interface (as per hardsid linux driver) */
    outportb(base1, 0xff);
    outportb(base2 + 2, 0x80);
    outportb(base1 + 2, 0x00);
    usleep(100);
    outportb(base1 + 2, 0x24);

    for (j = 0; j < MAXSID; ++j) {
        hssids[sids_found] = j;
        if (detect_sid(j)) {
            sids_found++;
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No PCI HardSID boards found.");
        return -1;
    }

    /* Check for PCI HardSID if 4 SIDs were found. */
    if (sids_found == 4) {
        if (detect_sid_uno()) {
            HStype = "PCI HardSID";
            sids_found = 1;
        }
    }

    /* mute all sids */
    for (j = 0; j < MAXSID; ++j) {
        if (hssids[j] != -1) {
            for (i = 0; i < 32; i++) {
                write_sid((uint8_t)i, 0, hssids[j]);
            }
        }
    }

    log_message(LOG_DEFAULT, "%s: opened at $%04X and $%04X, found %d SIDs.", HStype, base1, base2, sids_found);

    return 0;
}

int hs_pci_close(void)
{
    int i, j;

    /* mute all sids */
    for (j = 0; j < MAXSID; ++j) {
        if (hssids[j] != -1) {
            for (i = 0; i < 32; i++) {
                write_sid((uint8_t)i, 0, hssids[j]);
            }
        }
        hssids[j] = -1;
    }

    log_message(LOG_DEFAULT, "%s: closed.", HStype);

    sids_found = -1;

    return 0;
}

/* read value from SIDs */
int hs_pci_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        return read_sid(addr, hssids[chipno]);
    }
    return 0;
}

/* write value into SID */
void hs_pci_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        write_sid(addr, val, hssids[chipno]);
    }
}

int hs_pci_available(void)
{
    return sids_found;
}
#endif
#endif
