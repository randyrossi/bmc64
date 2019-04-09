/*
 * parsid-beos-drv.c - I/O based PARallel port SID support for BeOS.
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

#ifdef HAVE_PARSID

/* for snooze() */
#include <OS.h>

#include "io-access.h"
#include "log.h"
#include "parsid.h"
#include "sid-resources.h"
#include "types.h"

#define MAXSID 3

static unsigned int ports[MAXSID] = {-1, -1, -1};
static int pssids[MAXSID] = {-1, -1, -1};
static int psctrl[MAXSID] = {-1, -1, -1};
static int sids_found = -1;

void parsid_drv_out_ctr(uint8_t parsid_ctrport, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        io_access_store_byte(pssids[chipno] + 2, parsid_ctrport);
        psctrl[chipno] = parsid_ctrport;
    }
}

uint8_t parsid_drv_in_ctr(int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        if (psctrl[chipno] == -1) {
            io_access_store_byte(pssids[chipno] + 2, 0);
            psctrl[chipno] = 0;
        } else {
           return (uint8_t)psctrl[chipno];
        }
    }
    return 0;
}

static void parsid_get_ports(void)
{
    ports[0] = 0x3bc;
    ports[1] = 0x378;
    ports[2] = 0x278;
}

uint8_t parsid_drv_in_data(int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        return io_access_read_byte(pssids[chipno]);
    }
    return 0;
}

void parsid_drv_out_data(uint8_t outval, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        io_access_store_byte(pssids[chipno], outval);
    }
}

static uint8_t detect_sid_read(uint16_t addr, int chipno)
{
    uint8_t value = 0;
    uint8_t ctl = parsid_drv_in_ctr(chipno);

    parsid_drv_out_data(addr & 0x1f, chipno);

    ctl &= ~parsid_AUTOFEED;
    parsid_drv_out_ctr(ctl, chipno);

    ctl |= parsid_AUTOFEED;
    parsid_drv_out_ctr(ctl, chipno);

    ctl |= parsid_PCD;
    parsid_drv_out_ctr(ctl, chipno);

    ctl |= parsid_nINIT;
    parsid_drv_out_ctr(ctl, chipno);

    ctl |= parsid_STROBE;
    parsid_drv_out_ctr(ctl, chipno);

    value = parsid_drv_in_data(chipno);

    ctl &= ~parsid_STROBE;
    parsid_drv_out_ctr(ctl, chipno);

    ctl &= ~parsid_PCD;
    parsid_drv_out_ctr(ctl, chipno);

    ctl &= ~parsid_nINIT;
    parsid_drv_out_ctr(ctl, chipno);

    return value;
}

static void detect_sid_store(uint16_t addr, uint8_t outval, int chipno)
{
    uint8_t ctl = parsid_drv_in_ctr(chipno);

    parsid_drv_out_data(addr & 0x1f, chipno);

    ctl &= ~parsid_AUTOFEED;
    parsid_drv_out_ctr(ctl, chipno);

    ctl |= parsid_AUTOFEED;
    parsid_drv_out_ctr(ctl, chipno);

    parsid_drv_out_data(outval, chipno);

    ctl |= parsid_STROBE;
    parsid_drv_out_ctr(ctl, chipno);

    ctl &= ~parsid_STROBE;
    parsid_drv_out_ctr(ctl, chipno);
}

static int detect_sid(int chipno)
{
    int i;

    psctrl[chipno] = -1;

    for (i = 0x18; i >= 0; --i) {
        detect_sid_store(i, 0, chipno);
    }

    detect_sid_store(0x12, 0xff, chipno);

    for (i = 0; i < 100; ++i) {
        if (detect_sid_read(0x1b, chipno)) {
            return 0;
        }
    }

    detect_sid_store(0x0e, 0xff, chipno);
    detect_sid_store(0x0f, 0xff, chipno);
    detect_sid_store(0x12, 0x20, chipno);

    for (i = 0; i < 100; ++i) {
        if (detect_sid_read(0x1b, chipno)) {
            return 1;
        }
    }
    return 0;
}

int parsid_drv_open(void)
{
    int i = 0;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting direct I/O ParSIDs.");

    parsid_get_ports();

    if (io_access_init() < 0) {
        log_message(LOG_DEFAULT, "Cannot get access to I/O.");
        return -1;
    }

    for (i = 0; i < MAXSID; ++i) {
       pssids[sids_found] = ports[i];
       if (detect_sid(sids_found)) {
            sids_found++;
            log_message(LOG_DEFAULT, "ParSID found at %X.", ports[i]);
        } else {
            log_message(LOG_DEFAULT, "No ParSID at %X.", ports[i]);
        }
    }

    if (!sids_found) {
        io_access_shutdown();
        return -1;
    }

    log_message(LOG_DEFAULT, "Direct I/O ParSID: opened, found %d SIDs.", sids_found);

    return 0;
}

int parsid_drv_close(void)
{
    int i;

    io_access_shutdown();

    for (i = 0; i < MAXSID; ++i) {
        pssids[i] = -1;
        ports[i] = -1;
        psctrl[i] = -1;
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "Direct I/O ParSID: closed.");

    return 0;
}

int parsid_drv_available(void)
{
    return sids_found;
}

void parsid_drv_sleep(int amount)
{
    snooze(amount);
}
#endif
#endif
