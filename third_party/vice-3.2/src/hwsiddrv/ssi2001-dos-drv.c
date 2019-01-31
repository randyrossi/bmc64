/*
 * ssi2001-dos-drv.c - MSDOS specific SSI2001 (ISA SID card) driver.
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

 - MSDOS 6.00
 - Windows 95C
 - Windows 98SE
 - Windows ME
 */

#include "vice.h"

#ifdef __MSDOS__

#ifdef HAVE_SSI2001
#include <dos.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "types.h"

#define SSI2008_BASE 0x280

#define MAXSID 1

static int sids_found = -1;

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

static uint8_t read_sid(uint8_t reg)
{
    return inportb(SSI2008_BASE + (reg & 0x1f));
}

static void write_sid(uint8_t reg, uint8_t data)
{
    outportb(SSI2008_BASE + (reg & 0x1f), data);
}

static int detect_sid(void)
{
    int i;

    if (is_windows_nt()) {
        log_message(LOG_DEFAULT, "Running on Windows NT, cannot use direct memory access.");
        return 0;
    }

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
    int i;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "Detecting ISA SSI2001 boards.");

    if (!detect_sid()) {
        log_message(LOG_DEFAULT, "No ISA SSI2001 boards found.");
        return -1;
    }

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    sids_found = 1;

    log_message(LOG_DEFAULT, "ISA SSI2001 SID: opened.");

    return 0;
}

int ssi2001_drv_close(void)
{
    int i;

    /* mute all sids */
    for (i = 0; i < 32; i++) {
        write_sid(i, 0);
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "ISA SSI2001 SID: closed.");

    return 0;
}

/* read value from SIDs */
int ssi2001_drv_read(uint16_t addr, int chipno)
{
    /* if addr is from read-only register, perform a real read */
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

int ssi2001_drv_available(void)
{
    return sids_found;
}
#endif
#endif
