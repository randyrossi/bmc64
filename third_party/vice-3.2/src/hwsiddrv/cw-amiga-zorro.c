/*
 * cw-amiga-zorro.c - Zorro catweasel driver.
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

#if defined(HAVE_CATWEASELMKIII) && defined(AMIGA_M68K)

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

static uint8_t *CWbase = NULL;

/* read value from SIDs */
int cw_zorro_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        return read_sid(addr);
    }

    return 0;
}

/* write value into SID */
void cw_zorro_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr <= 0x20) {
        write_sid(addr, val);
    }
}

#define CW_SID_DAT 0xd8
#define CW_SID_CMD 0xdc

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/configvars.h>

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>

static struct Library *ExpansionBase = NULL;

int cw_zorro_open(void)
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

    log_message(LOG_DEFAULT, "Detecting Zorro CatWeasel boards.");

    if ((ExpansionBase = OpenLibrary("expansion.library", 0L)) == NULL) {
        log_message(LOG_DEFAULT, "Cannot open expansion library.");
        return -1;
    }

    myCD = FindConfigDev(myCD, 0x1212, 0x42);

    if (!myCD) {
        log_message(LOG_DEFAULT, "No Zorro CatWeasel boards found.");
        return -1;
    }

    CWbase = myCD->cd_BoardAddr;

    if (!CWbase) {
        log_message(LOG_DEFAULT, "No Zorro CatWeasel boards found.");
        return -1;
    }

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    log_message(LOG_DEFAULT, "Zorro CatWeasel SID: opened at $%X", CWbase);

    sids_found = 1;

    return 1;
}

static unsigned char read_sid(unsigned char reg)
{
    unsigned char cmd;
    uint8_t tmp;

    cmd = (reg & 0x1f) | 0x20;   // Read command & address

    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  // Make sure its correct frequency
    }

    // Write command to the SID
    CWbase[CW_SID_CMD] = cmd;

    // Waste 1ms
    usleep(1);

    return CWbase[CW_SID_DAT];
}

static void write_sid(unsigned char reg, unsigned char data)
{
    unsigned char cmd;
    uint8_t tmp;

    cmd = reg & 0x1f;            // Write command & address
    if (catweaselmkiii_get_ntsc()) {
        cmd |= 0x40;  // Make sure its correct frequency
    }

    // Write data to the SID
    CWbase[CW_SID_DAT] = data;
    CWbase[CW_SID_CMD] = cmd;

    // Waste 1ms
    usleep(1);
}

int cw_zorro_close(void)
{
    unsigned int i;

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "Zorro CatWeasel SID: closed.");

    return 0;
}

int cw_zorro_available(void)
{
    return sids_found;
}
#endif
#endif
