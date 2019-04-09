/*
 * ssi2001-amiga-drv.c - ISA (using Golden Gate 2+ bridge) SSI2001 driver.
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

#if defined(HAVE_SSI2001) && defined(AMIGA_M68K)

#include <stdlib.h>
#include <string.h>

#include "ssi2001.h"

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

static uint8_t *SSI2001base = NULL;

/* read value from SIDs */
int ssi2001_drv_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        return read_sid(addr);
    }

    return 0;
}

/* write value into SID */
void ssi2001_drv_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        write_sid(addr, val);
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

static int detect_sid(void)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        write_sid((uint8_t)i, 0);
    }

    write_sid(0x12, 0xff);

    for (i = 0; i < 100; ++i) {
        if (read_sid(0x1b)) {
            return 0;
        }
    }

    write_sid(0x0e, 0xff);
    write_sid(0x0f, 0xff);
    write_sid(0x12, 0x20);

    for (i = 0; i < 100; ++i) {
        if (read_sid(0x1b)) {
            return 1;
        }
    }
    return 0;
}

int ssi2001_drv_open(void)
{
    struct ConfigDev *myCD;
    int i;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting GG2+ ISA SSI2001 boards.");

    if ((ExpansionBase = OpenLibrary("expansion.library", 0L)) == NULL) {
        log_message(LOG_DEFAULT, "Cannot open expansion library.");
        return -1;
    }

    myCD = FindConfigDev(myCD, 2150, 1);

    if (!myCD) {
        log_message(LOG_DEFAULT, "No GG2+ board found.");
        return -1;
    }

    SSI2001base = myCD->cd_BoardAddr;

    if (!SSI2001base) {
        log_message(LOG_DEFAULT, "No GG2+ board found.");
        return -1;
    }

    if (!detect_sid()) {
        log_message(LOG_DEFAULT, "No GG2+ ISA SSI2001 boards found.");
        return -1;
    }

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    log_message(LOG_DEFAULT, "GG2+ ISA SSI2001 SID: opened");

    sids_found = 1; /* ok */

    return 0;
}

static unsigned char read_sid(unsigned char reg)
{
    return SSI2001base[((0x280 + (reg & 0x1f)) * 2) + 1];
}

static void write_sid(unsigned char reg, unsigned char data)
{
    // Write data to the SID
    SSI2001base[((0x280 + (reg & 0x1f)) * 2) + 1] = data;
}

int ssi2001_drv_close(void)
{
    unsigned int i;

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "GG2+ ISA SSI2001 SID: closed.");

    return 0;
}

int ssi2001_drv_available(void)
{
    return sids_found;
}
#endif
#endif
