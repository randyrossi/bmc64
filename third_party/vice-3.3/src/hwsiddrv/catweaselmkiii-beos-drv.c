/*
 * catweaselmkiii-beos-drv.c - BeOS specific cw3 driver.
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

 - BeOS Max V4b1 x86
 - Zeta 1.21
 */

#include "vice.h"

#ifdef BEOS_COMPILE

#ifdef HAVE_CATWEASELMKIII

#include <OS.h>

#include "catweaselmkiii.h"
#include "io-access.h"
#include "log.h"
#include "pci-beos-drv.h"
#include "types.h"

#define CW_SID_DAT 0xd8
#define CW_SID_CMD 0xdc

#define MAXSID 1

static int sids_found = -1;
static int base = -1;

/* input/output functions */
static void cw_outb(uint16_t port, uint8_t val)
{
    io_access_store_byte(port, val);
}

static uint8_t cw_inb(uint16_t port)
{
    return io_access_read_byte(port);
}

int catweaselmkiii_drv_read(uint16_t addr, int chipno)
{
    unsigned char cmd;

    if (chipno < MAXSID && base != -1 && addr < 0x20) {
        cmd = (addr & 0x1f) | 0x20;
        if (catweaselmkiii_get_ntsc()) {
            cmd |= 0x40;
        }
        cw_outb(base + CW_SID_CMD, cmd);
        snooze(1);
        return cw_inb(base + CW_SID_DAT);
    }
    return 0;
}

void catweaselmkiii_drv_store(uint16_t addr, uint8_t outval, int chipno)
{
    unsigned char cmd;

    if (chipno < MAXSID && base != -1 && addr < 0x20) {
        cmd = addr & 0x1f;
        if (catweaselmkiii_get_ntsc()) {
            cmd |= 0x40;
        }
        cw_outb(base + CW_SID_DAT, outval);
        cw_outb(base + CW_SID_CMD, cmd);
        snooze(1);
    }
}

static int detect_sid(void)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        catweaselmkiii_drv_store((uint16_t)i, 0, 0);
    }

    catweaselmkiii_drv_store(0x12, 0xff, 0);

    for (i = 0; i < 100; ++i) {
        if (catweaselmkiii_drv_read(0x1b, 0)) {
            return 0;
        }
    }

    catweaselmkiii_drv_store(0x0e, 0xff, 0);
    catweaselmkiii_drv_store(0x0f, 0xff, 0);
    catweaselmkiii_drv_store(0x12, 0x20, 0);

    for (i = 0; i < 100; ++i) {
        if (catweaselmkiii_drv_read(0x1b, 0)) {
            return 1;
        }
    }
    return 0;
}

int catweaselmkiii_drv_open(void)
{
    int i;
    uint32_t b1 = 0;
    uint32_t b2 = 0;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting PCI CatWeasel boards.");

    if (io_access_init() < 0) {
        log_message(LOG_DEFAULT, "Cannot get access to $%X.", base);
        return -1;
    }

    i = pci_get_base(0xe159, 0x0001, &b1, &b2);

    if (i < 0) {
        log_message(LOG_DEFAULT, "No PCI CatWeasel found.");
        io_access_shutdown();
        return -1;
    }

    base = b1 & 0xfffc;


    log_message(LOG_DEFAULT, "PCI CatWeasel board found at $%04X.", base);

    if (detect_sid()) {
        sids_found++;
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No PCI CatWeasel found.");
        io_access_shutdown();
        return -1;
    }

    log_message(LOG_DEFAULT, "PCI CatWeasel: opened, found %d SIDs.", sids_found);

    return 0;
}

int catweaselmkiii_drv_close(void)
{
    io_access_shutdown();

    base = -1;

    sids_found = -1;

    log_message(LOG_DEFAULT, "PCI CatWeasel: closed");

    return 0;
}

int catweaselmkiii_drv_available(void)
{
    return sids_found;
}

void catweaselmkiii_drv_set_machine_parameter(long cycles_per_sec)
{
}
#endif
#endif
