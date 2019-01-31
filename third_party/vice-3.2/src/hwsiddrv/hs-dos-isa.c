/*
 * hs-dos-isa.c - MSDOS specific ISA hardsid driver.
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
 - MSDOS 6.00 (ISA HardSID)
 - MSDOS 6.00 (ISA HardSID Quattro)
 - Windows 95C (ISA HardSID)
 - Windows 95C (ISA HardSID Quattro)
 - Windows 98SE (ISA HardSID)
 - Windows 98SE (ISA HardSID Quattro)
 - Windows ME (ISA HardSID)
 - Windows ME (ISA HardSID Quattro)
 */

#include "vice.h"

#ifdef __MSDOS__

#ifdef HAVE_HARDSID

#include <stdio.h>
#include <string.h>
#include <pc.h>
#include <unistd.h>
#include <dos.h>

#include "hs-dos.h"
#include "log.h"
#include "sid-snapshot.h"
#include "types.h"

#define HS_ISA_BASE 0x300

#define MAXSID 4

static int sids_found = -1;

static int hssids[MAXSID] = {-1, -1, -1, -1};

static uint8_t read_sid(uint8_t reg, int chipno)
{
    outportb(HS_ISA_BASE + 1, (chipno << 6) | (reg & 0x1f) | 0x20);
    usleep(2);
    return inportb(HS_ISA_BASE);
}

static void write_sid(uint8_t reg, uint8_t data, int chipno)
{
    outportb(HS_ISA_BASE, data);
    outportb(HS_ISA_BASE + 1, (chipno << 6) | (reg & 0x1f));
    usleep(2);
}

static int is_windows_nt(void)
{
    unsigned short real_version;
    int version_major = -1;
    int version_minor = -1;

    real_version = _get_dos_version(1);
    version_major = real_version >> 8;
    version_minor = real_version & 0xff;

    if (version_major == 5 && version_minor == 50) {
        return 1;
    }
    return 0;
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

    if (is_windows_nt()) {
        log_message(LOG_DEFAULT, "Running on Windows NT, cannot use direct memory access.");
        return 0;
    }

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

static char *HStype = "ISA HardSID Quattro";

int hs_isa_open(void)
{
    int i, j;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting ISA HardSID boards.");

    for (j = 0; j < MAXSID; ++j) {
        if (detect_sid(j)) {
            hssids[sids_found] = j;
            sids_found++;
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No ISA HardSID boards found.");
        return -1;
    }

    /* Check for classic HardSID if 4 SIDs were found. */
    if (sids_found == 4) {
        if (detect_sid_uno()) {
            HStype = "ISA HardSID";
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

    log_message(LOG_DEFAULT, "%s: opened, found %d SIDs", HStype, sids_found);

    return 0;
}

int hs_isa_close(void)
{
    int i, j;

    /* mute all sids */
    for (j = 0; j < MAXSID; ++j) {
        if (hssids[j] != -1) {
            for (i = 0; i < 32; i++) {
                write_sid((uint8_t)i, 0, hssids[j]);
            }
        }
    }

    log_message(LOG_DEFAULT, "%s: closed.", HStype);

    sids_found = -1;

    return 0;
}

/* read value from SIDs */
int hs_isa_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        return read_sid(addr, hssids[chipno]);
    }
    return 0;
}

/* write value into SID */
void hs_isa_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        write_sid(addr, val, hssids[chipno]);
    }
}

int hs_isa_available(void)
{
    return sids_found;
}
#endif
#endif
