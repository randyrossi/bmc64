/*
 * cw-amiga-openpci.c - PCI catweasel driver.
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

#if defined(HAVE_PROTO_OPENPCI_H) && defined(HAVE_CATWEASELMKIII)

#include <stdlib.h>
#include <string.h>

#include "cw-amiga.h"

#if !defined(USE_SDLUI) && !defined(USE_SDLUI2)
#include "loadlibs.h"
#else
#include "archdep.h"
#endif

#include "log.h"
#include "types.h"

#define MAXSID 1

static unsigned char read_sid(unsigned char reg); // Read a SID register
static void write_sid(unsigned char reg, unsigned char data); // Write a SID register

static int sids_found = -1;

static unsigned long CWbase;

/* read value from SIDs */
int cw_openpci_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        return read_sid(addr);
    }

    return 0;
}

/* write value into SID */
void cw_openpci_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        write_sid(addr, val);
    }
}

#define CW_SID_DAT 0xd8
#define CW_SID_CMD 0xdc

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/openpci.h>
#include <libraries/openpci.h>

#if defined(pci_obtain_card) && defined(pci_release_card)
static int CWLock = FALSE;
#endif

static struct pci_dev *dev = NULL;

#if defined(pci_obtain_card) && defined(pci_release_card)
static void close_device(void)
{
    if (CWLock) {
        pci_release_card(dev);
    }
}
#endif

int cw_openpci_open(void)
{
    unsigned int i;
    unsigned char bus = 0;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting PCI CatWeasel boards.");

    if (!pci_lib_loaded) {
        log_message(LOG_DEFAULT, "No OpenPCI library available.");
        return -1;
    }

    bus = pci_bus();

    if (!bus) {
        log_message(LOG_DEFAULT, "No PCI bus found.");
        return -1;
    }

    dev = pci_find_device(CW_VENDOR, CW_DEVICE, NULL);

    if (dev == NULL) {
        log_message(LOG_DEFAULT, "Unable to find a PCI CatWeasel board.");
        return -1;
    }

#if defined(pci_obtain_card) && defined(pci_release_card)
    /* Lock the device, since we're a driver */
    CWLock = pci_obtain_card(dev);
    if (!CWLock) {
        log_message(LOG_DEFAULT, "Unable to lock the CatWeasel. Another driver may have an exclusive lock." );
        return -1;
    }
#endif

    CWbase = dev->base_address[0];

    // Reset the catweasel PCI interface (as per the CW programming docs)
    pci_outb(0xf1, CWbase + 0x00);
    pci_outb(0x00, CWbase + 0x01);
    pci_outb(0x00, CWbase + 0x02);
    pci_outb(0x00, CWbase + 0x04);
    pci_outb(0x00, CWbase + 0x05);
    pci_outb(0x00, CWbase + 0x29);
    pci_outb(0x00, CWbase + 0x2b);

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    log_message(LOG_DEFAULT, "PCI CatWeasel SID: opened at $%X.", CWbase);

    sids_found = 1;

    return 0;
}

static unsigned char read_sid(unsigned char reg)
{
    unsigned char cmd;

    cmd = (reg & 0x1f) | 0x20;   // Read command & address

    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  // Make sure its correct frequency
    }

    // Write command to the SID
    pci_outb(cmd, CWbase + CW_SID_CMD);

    // Waste 1ms
    usleep(1);

    return pci_inb(CWbase + CW_SID_DAT);
}

static void write_sid(unsigned char reg, unsigned char data)
{
    unsigned char cmd;

    cmd = reg & 0x1f;            // Write command & address
    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  // Make sure its correct frequency
    }

    // Write data to the SID
    pci_outb(data, CWbase + CW_SID_DAT);
    pci_outb(cmd, CWbase + CW_SID_CMD);

    // Waste 1ms
    usleep(1);
}

int cw_openpci_close(void)
{
    unsigned int i;

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

#if defined(pci_obtain_card) && defined(pci_release_card)
    close_device();
#endif

    log_message(LOG_DEFAULT, "PCI CatWeasel SID: closed.");

    sids_found = -1;

    return 0;
}

int cw_openpci_available(void)
{
    return sids_found;
}
#endif
#endif

