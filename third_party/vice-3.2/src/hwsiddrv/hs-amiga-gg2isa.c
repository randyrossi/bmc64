/*
 * hs-amiga-gg2isa.c - ISA (using Golden Gate 2+ bridge) hardsid driver.
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

#if defined(HAVE_HARDSID) && defined(AMIGA_M68K)

#include <stdlib.h>
#include <string.h>

#include "hs-amiga.h"

#if !defined(USE_SDLUI) && !defined(USE_SDLUI2)
#include "loadlibs.h"
#else
#include "archdep.h"
#endif

#include "log.h"
#include "types.h"

#define MAXSID 4

static unsigned char read_sid(unsigned short reg, int chipno); // Read a SID register
static void write_sid(unsigned short reg, unsigned char data, int chipno); // Write a SID register

static int sids_found = -1;

static unsigned char *HSbase = NULL;

static int hssids[4] = { -1, -1, -1, -1 };

/* read value from SIDs */
int hs_gg2_isa_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        return read_sid(addr, hssids[chipno]);
    }

    return 0;
}

/* write value into SID */
void hs_gg2_isa_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        write_sid(addr, val, hssids[chipno]);
    }
}

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/configvars.h>

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>

static struct Library *ExpansionBase = NULL;

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

int hs_gg2_isa_open(void)
{
    int i, j;
    struct ConfigDev *myCD;
    unsigned char *base = NULL;

    if (sids_found > 0) {
        return 0;
    }

    if (!sids_found) {
        return -1;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting GG2+ ISA HardSID boards.");

    if ((ExpansionBase = OpenLibrary("expansion.library", 0L)) == NULL) {
        log_message(LOG_DEFAULT, "Cannot open expansion library.");
        return -1;
    }

    myCD = FindConfigDev(myCD, 2150, 1);

    if (!myCD) {
        log_message(LOG_DEFAULT, "No GG2+ found.");
        return -1;
    }

    base = myCD->cd_BoardAddr;

    if (!base) {
        log_message(LOG_DEFAULT, "No GG2+ found.");
        return -1;
    }

    for (i = 0; i < MAXSID; ++i) {
        if (detect_sid(i)) {
            hssids[sids_found] = i;
            sids_found++;
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No GG2+ ISA HardSID board found.");
        return -1;
    }

    /* Check for classic HardSID if 4 SIDs were found. */
    if (sids_found == 4) {
        if (detect_sid_uno()) {
            sids_found = 1;
        }
    }

    /* mute all sids */
    for (j = 0; j < MAXSID; ++j) {
        if (hssids[j] != -1) {
            for (i = 0; i < 32; i++) {
                write_sid(i, 0, hssids[j]);
            }
        }
    }

    log_message(LOG_DEFAULT, "GG2+ ISA HardSID: opened, found %d SIDs.", sids_found);

    return 0;
}

static unsigned char read_sid(unsigned short reg, int chipno)
{
    unsigned char cmd;
    BYTE tmp;

    cmd = (chipno << 6) | (reg & 0x1f) | 0x20;   // Read command & address

    // Write command to the SID
    HSbase[0x601] = cmd;

    // Waste 2ms
    usleep(2);

    return HSbase[0x603];
}

static void write_sid(unsigned short reg, unsigned char data, int chipno)
{
    unsigned char cmd;

    cmd = (chipno << 6) | (reg & 0x1f);            // Write command & address

    // Write data to the SID
    HSbase[0x601] = data;
    HSbase[0x603] = cmd;
}

int hs_gg2_isa_close(void)
{
    int i, j;

    /* mute all sids */
    for (j = 0; j < MAXSID; ++j) {
        if (hssids[j] != -1) {
            for (i = 0; i < 32; ++i) {
                write_sid(i, 0, hssids[j]);
            }
            hssids[j] = -1;
        }
    }
    sids_found = -1;

    log_message(LOG_DEFAULT, "GG2+ ISA HardSID: closed.");

    return 0;
}

int hs_gg2_isa_available(void)
{
    return sids_found;
}
#endif
#endif
