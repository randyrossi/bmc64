/*
 * cw-amiga-clockport.c - Clockport catweasel driver.
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
#include "log.h"
#include "types.h"

#define MAXSID 1

static unsigned char read_sid(unsigned char reg); // Read a SID register
static void write_sid(unsigned char reg, unsigned char data); // Write a SID register

static int sids_found = -1;

static uint8_t *CWbase = NULL;

/* read value from SIDs */
int cw_clockport_read(uint16_t addr, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        return read_sid(addr);
    }

    return 0;
}

/* write value into SID */
void cw_clockport_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno and addr is valid */
    if (chipno < MAXSID && addr < 0x20) {
        write_sid(addr, val);
    }
}

#define CW_SID_DAT 0x18
#define CW_SID_CMD 0x1c

static char *cp_addresses[] = {
    0xd80001,
    0xd84001,
    0xd88001,
    0xd8c001,
    0xd8d001,
    0xd90001,
    NULL
};

static void write_sid_cp(char *base, uint8_t adr, uint8_t val)
{
    base[CW_SID_DAT] = val;
    usleep(1);
    base[CW_SID_CMD] = adr & 0x1f;
}

static uint8_t read_sid_cp(char *base, uint8_t adr)
{
    base[CW_SID_CMD] = 0x20 | (adr & 0x1f);
    usleep(1);
    return base[CW_SID_DAT];
}

static int detect_sid(char *base)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        write_sid_cp(base, (uint8_t)i, 0);
    }

    write_sid_cp(base, 0x12, 0xff);

    for (i = 0; i < 100; ++i) {
        if (read_sid_cp(base, 0x1b)) {
            return 0;
        }
    }

    write_sid_cp(base, 0x0e, 0xff);
    write_sid_cp(base, 0x0f, 0xff);
    write_sid_cp(base, 0x12, 0x20);

    for (i = 0; i < 100; ++i) {
        if (read_sid_cp(base, 0x1b)) {
            return 1;
        }
    }
    return 0;
}

int cw_clockport_open(void)
{
    int i;

    if (sids_found == 0) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting clockport CatWeasel boards.");

    for (i = 0; cp_addresses[i]; ++i) {
        if (detect_sid(cp_addresses[i])) {
            log_message(LOG_DEFAULT, "Clock port CatWeasel board found at %X.", cp_addresses[i]);
            CWbase = cp_addresses[i];
        } else {
            log_message(LOG_DEFAULT, "No clock port CatWeasel board at %X.", cp_addresses[i]);
        }
    }

    if (!CWbase) {
        log_message(LOG_DEFAULT, "No clockport CatWeasel boards found.");
        return -1;
    }

    sids_found = 1;

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    log_message(LOG_DEFAULT, "CatWeasel clockport SID: opened at $%X.", CWbase);

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

int cw_clockport_close(void)
{
    unsigned int i;

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "Clockport CatWeasel SID: closed.");

    return 0;
}

int cw_clockport_available(void)
{
    return sids_found;
}
#endif
#endif
