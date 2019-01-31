/*
 * hs-amiga-openpci.c - Amiga specific OpenPCI hardsid driver.
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

#ifdef AMIGA_SUPPORT

#if defined(HAVE_PROTO_OPENPCI_H) && defined(HAVE_HARDSID)

#include <stdlib.h>
#include <string.h>

#include "hardsid.h"

#if !defined(USE_SDLUI) && !defined(USE_SDLUI2)
#include "loadlibs.h"
#else
#include "archdep.h"
#endif

#include "log.h"
#include "types.h"

#define MAXSID 4

static int sids_found = -1;

static int hssids[MAXSID] = {-1, -1, -1, -1};

static unsigned char read_sid(unsigned char reg, int chipno); /* Read a SID register */
static void write_sid(unsigned char reg, unsigned char data, int chipno); /* Write a SID register */

static unsigned char *HSbase = NULL;

/* read value from SIDs */
int hs_openpci_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        return read_sid(addr, hssids[chipno]);
    }

    return 0;
}

/* write value into SID */
void hs_openpci_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        write_sid(addr, val, hssids[chipno]);
    }
}

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/openpci.h>
#include <libraries/openpci.h>

#if defined(pci_obtain_card) && defined(pci_release_card)
static int HSLock = FALSE;
#endif

static struct pci_dev *dev = NULL;

static int detect_sid(int chipno)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        write_sid((unsigned short)i, 0, chipno);
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

#if defined(pci_obtain_card) && defined(pci_release_card)
static void close_device(void)
{
    if (HSLock) {
        pci_release_card(dev);
    }
}
#endif

int hs_openpci_open(void)
{
    unsigned int i, j;
    unsigned char bus = 0;

    if (sids_found > 0) {
        return 0;
    }

    if (!sids_found) {
        return -1;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting PCI HardSID boards.");

    if (!pci_lib_loaded) {
        log_message(LOG_DEFAULT, "OpenPCI library not found.");
        return -1;
    }

    bus = pci_bus();

    if (!bus) {
        log_message(LOG_DEFAULT, "No PCI bus found.");
        return -1;
    }

    dev = pci_find_device(0x6581, 0x8580, NULL);

    if (dev == NULL) {
        log_message( LOG_DEFAULT, "Unable to find a PCI HardSID board." );
        return -1;
    }

#if defined(pci_obtain_card) && defined(pci_release_card)
    /* Lock the device, since we're a driver */
    HSLock = pci_obtain_card(dev);
    if (!HSLock) {
        log_message(LOG_DEFAULT, "Unable to lock the HardSID. Another driver may have an exclusive lock." );
        return -1;
    }
#endif

    HSbase = (char *)dev->base_address[0];

    /* Reset the hardsid PCI interface (as per hardsid linux driver) */
    pci_outb(0xff, HSbase + 0x00);
    pci_outb(0x00, HSbase + 0x02);
    usleep(100);
    pci_outb(0x24, HSbase + 0x02);

    for (j = 0; j < MAXSID; ++j) {
        if (detect_sid(j)) {
            hssids[sids_found] = j;
            sids_found++;
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No PCI HardSID boards found.");
#if defined(pci_obtain_card) && defined(pci_release_card)
        close_device();
#endif
        return -1;
    }

    /* mute all sids */
    for (j = 0; j < MAXSID; ++j) {
        if (hssids[j] != -1) {
            for (i = 0; i < 32; i++) {
                write_sid(i, 0, hssids[j]);
            }
        }
    }

    log_message(LOG_DEFAULT, "PCI HardSID: opened at $%X, found %d SIDs.", HSbase, sids_found);

    return 0;
}

static unsigned char read_sid(unsigned char reg, int chipno)
{
    unsigned char ret;

    pci_outb((chipno << 6) | (reg & 0x1f) | 0x20, HSbase + 4);
    usleep(2);
    pci_outb(0x20, HSbase);
    ret = pci_inb(HSbase);
    pci_outb(0x80, HSbase);
    return ret;
}

static void write_sid(unsigned char reg, unsigned char data, int chipno)
{
    pci_outw((chipno << 14) | ((reg & 0x1f) << 8) | data, HSbase + 3);
}

int hs_openpci_close(void)
{
    unsigned int i, j;

    /* mute all sids */
    for (j = 0; j < MAXSID; ++j) {
        if (hssids[j] != -1) {
            for (i = 0; i < 32; i++) {
                write_sid(i, 0, hssids[j]);
            }
        }
        hssids[j] = -1;
    }

#if defined(pci_obtain_card) && defined(pci_release_card)
    close_device();
#endif

    log_message(LOG_DEFAULT, "PCI HardSID: closed.");

    sids_found = -1;

    return 0;
}

int hs_openpci_available(void)
{
    return sids_found;
}
#endif
#endif
