/*
 * hs-beos-isa.c - BeOS specific ISA hardsid driver.
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

 - BeOS Max V4b1 x86 (ISA HardSID)
 - BeOS Max V4b1 x86 (ISA HardSID Quattro)
 - Zeta 1.21 (ISA HardSID)
 - Zeta 1.21 (ISA HardSID Quattro)
 */

#include "vice.h"

#ifdef BEOS_COMPILE

#ifdef HAVE_HARDSID

#include <OS.h>

#include "hardsid.h"
#include "hs-beos.h"
#include "io-access.h"
#include "log.h"
#include "types.h"

#define HARDSID_BASE 0x300

#define MAXSID 4

static int sids_found = -1;
static int hssids[MAXSID] = {-1, -1, -1, -1};

void hs_isa_store(uint16_t addr, uint8_t value, int chipno)
{
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        io_access_store_byte(HARDSID_BASE, value);
        io_access_store_byte(HARDSID_BASE + 1, (hssids[chipno] << 6) | (addr & 0x1f));
    }
}

uint8_t hs_isa_read(uint16_t addr, int chipno)
{
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        io_access_store_byte(HARDSID_BASE + 1, (hssids[chipno] << 6) | (addr & 0x1f) | 0x20);
        snooze(2);
        return io_access_read_byte(HARDSID_BASE);
    }
    return 0;
}

static int detect_sid_uno(void)
{
    int i;
    int j;

    for (j = 0; j < 4; ++j) {
        for (i = 0x18; i >= 0; --i) {
            hs_isa_store((uint16_t)i, 0, j);
        }
    }

    hs_isa_store(0x12, 0xff, 0);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, 3)) {
            return 0;
        }
    }

    hs_isa_store(0x0e, 0xff, 0);
    hs_isa_store(0x0f, 0xff, 0);
    hs_isa_store(0x12, 0x20, 0);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, 3)) {
            return 1;
        }
    }
    return 0;
}

static int detect_sid(int chipno)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        hs_isa_store((uint16_t)i, 0, chipno);
    }

    hs_isa_store(0x12, 0xff, chipno);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, chipno)) {
            return 0;
        }
    }

    hs_isa_store(0x0e, 0xff, chipno);
    hs_isa_store(0x0f, 0xff, chipno);
    hs_isa_store(0x12, 0x20, chipno);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, chipno)) {
            return 1;
        }
    }
    return 0;
}

static char *HStype = "ISA HardSID Quattro";

int hs_isa_open(void)
{
    int j;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting ISA HardSID boards.");

    if (io_access_init() < 0) {
        log_message(LOG_DEFAULT, "Cannot get permission to access $%X.", HARDSID_BASE);
        return -1;
    }

    for (j = 0; j < MAXSID; ++j) {
        hssids[sids_found] = j;
        if (detect_sid(j)) {
            sids_found++;
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No ISA HardSID boards found.");
        io_access_shutdown();
        return -1;
    }

    /* Check for classic HardSID if 4 SIDs were found. */
    if (sids_found == 4) {
        if (detect_sid_uno()) {
            HStype = "ISA HardSID";
            sids_found = 1;
        }
    }

    log_message(LOG_DEFAULT, "%s: opened, found %d SIDs.", HStype, sids_found);

    return 0;
}

int hs_isa_close(void)
{
    int i;

    io_access_shutdown();

    for (i = 0; i < MAXSID; ++i) {
        hssids[i] = -1;
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "%s: closed.", HStype);

    return 0;
}

int hs_isa_available(void)
{
    return sids_found;
}
#endif
#endif
