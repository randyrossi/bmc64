/*
 * catweaselmkiii-dos-drv.c - DOS specific cw3 driver.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * Based on code by
 *  Timothy Mann
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
 - MSDOS 6.00
 - Windows 95C
 - Windows 98SE
 - Windows ME
 */

#include "vice.h"

#ifdef __MSDOS__

#ifdef HAVE_CATWEASELMKIII

#include <stdio.h>
#include <dpmi.h>
#include <string.h>
#include <pc.h>
#include <unistd.h>

#include "catweaselmkiii.h"
#include "log.h"
#include "pci-dos-drv.h"
#include "types.h"

#define MAXSID 1

typedef unsigned short uint16;
typedef unsigned long uint32;

static int base;

static int sids_found = -1;

#define CW_SID_DAT 0xd8
#define CW_SID_CMD 0xdc

static int vice_pci_find_catweasel(int index)
{
    int i = 0, j = 0, res;
    int bus, device, func;
    uint32 subsysID, baseAddr;

    if (vice_pci_install_check() != 0) {
        return -1;
    }

    while (i <= index) {

        /* Find the next card that uses the Tiger Jet Networks Tiger320 PCI chip */
        res = vice_pci_find(CW_VENDOR, CW_DEVICE, j++, &bus, &device, &func);
        if (res != 0) {
            return -1;
        }

        /* Read the subsystem vendor ID + subsystem ID */
        res = vice_pci_read_config_dword(bus, device, func, 0x2c, &subsysID);
        if (res != 0) {
            continue;
        }

        /* Check if they match the Catweasel */
        switch (subsysID) {
            case (CW_MK4_SUBDEVICE1 << 16) | CW_MK3_SUBVENDOR: 	/* Catweasel MK3 */
            case (CW_MK4_SUBDEVICE2 << 16) | CW_MK3_SUBVENDOR: 	/* Catweasel MK3 alternate */
            case (CW_MK4_SUBDEVICE1 << 16) | CW_MK4_SUBVENDOR1: /* Catweasel MK4 */
            case (CW_MK4_SUBDEVICE2 << 16) | CW_MK4_SUBVENDOR1: /* Catweasel MK4 alternate */
                break;
            default:
                continue;
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
            return baseAddr & ~3;
        }
    }

    return -1;
}

static unsigned char read_sid(unsigned char reg)
{
    unsigned char cmd;

    cmd = (reg & 0x1f) | 0x20;	// Read command & address
    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  // Make sure its correct frequency
    }

    // Write command to the SID
    outportb(base + CW_SID_CMD, cmd);

    // Waste 1ms
    usleep(1);

    return inportb(base + CW_SID_DAT);
}

static void write_sid(unsigned char reg, unsigned char data)
{
    unsigned char cmd;

    cmd = reg & 0x1f;
    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  // Make sure its correct frequency
    }

    // Write data to the SID
    outportb(base + CW_SID_DAT, data);
    outportb(base + CW_SID_CMD, cmd);

    // Waste 1ms
    usleep(1);
}

int catweaselmkiii_drv_open(void)
{
    int i;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting PCI CatWeasel boards.");

    base = vice_pci_find_catweasel(0);

    if (base == -1) {
        log_message(LOG_DEFAULT, "Unable to find a PCI CatWeasel board.");
        return -1;
    }

    // Reset the catweasel PCI interface (as per the CW programming docs)
    outportb(base + 0x00, 0xf1);
    outportb(base + 0x01, 0x00);
    outportb(base + 0x02, 0x00);
    outportb(base + 0x04, 0x00);
    outportb(base + 0x05, 0x00);
    outportb(base + 0x29, 0x00);
    outportb(base + 0x2b, 0x00);                                      

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    log_message(LOG_DEFAULT, "PCI CatWeasel SID: opened at $%X.", base);

    sids_found = 1;

    return 0;
}

int catweaselmkiii_drv_close(void)
{
    unsigned int i;

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    log_message(LOG_DEFAULT, "PCI CatWeasel SID: closed");

    sids_found = -1;

    return 0;
}

/* read value from SIDs */
int catweaselmkiii_drv_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        return read_sid(addr);
    }

    return 0;
}

/* write value into SID */
void catweaselmkiii_drv_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        write_sid(addr, val);
    }
}

/* set current main clock frequency, which gives us the possibilty to
   choose between pal and ntsc frequencies */
void catweaselmkiii_drv_set_machine_parameter(long cycles_per_sec)
{
}

int catweaselmkiii_drv_available(void)
{
    return sids_found;
}
#endif
#endif
