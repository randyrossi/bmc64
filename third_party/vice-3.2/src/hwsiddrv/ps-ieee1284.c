/*
 * ps-ieee1284.c - Generic libieee1284 based PARallel port SID support.
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

#if defined(HAVE_PARSID) && defined(HAVE_LIBIEEE1284)

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>

#undef HAVE_IEEE1284_H
#include <ieee1284.h>

#include "alarm.h"
#include "log.h"
#include "parsid.h"
#include "sid-resources.h"
#include "types.h"

#define MAXSID 3

static int sids_found = -1;

static int pssids[MAXSID] = {-1, -1, -1};

static struct parport_list parlist;

static void parsid_ieee1284_outb_ctr(struct parport *port, uint8_t value)
{
    ieee1284_write_control(port, value);
}

static void parsid_ieee1284_outb_data(struct parport *port, uint8_t value)
{
    ieee1284_write_data(port, value);
}

static int parsid_ieee1284_inb_ctr(struct parport *port)
{
    return ieee1284_read_control(port);
}

static int parsid_ieee1284_inb_data(struct parport *port)
{
    return ieee1284_read_data(port);
}

static uint8_t detect_sid_read(struct parport *port, uint8_t addr)
{
    uint8_t value = 0;
    uint8_t ctl = parsid_ieee1284_inb_ctr(port);

    parsid_ieee1284_outb_data(port, addr & 0x1f);

    ctl &= ~parsid_AUTOFEED;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl |= parsid_AUTOFEED;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl |= parsid_PCD;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl |= parsid_nINIT;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl |= parsid_STROBE;
    parsid_ieee1284_outb_ctr(port, ctl);

    value = parsid_ieee1284_inb_data(port);

    ctl &= ~parsid_STROBE;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl &= ~parsid_PCD;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl &= ~parsid_nINIT;
    parsid_ieee1284_outb_ctr(port, ctl);

    return value;
}

static void detect_sid_store(struct parport *port, uint16_t addr, uint8_t outval)
{
    uint8_t ctl = parsid_ieee1284_inb_ctr(port);

    parsid_ieee1284_outb_data(port, (addr & 0x1f));

    ctl &= ~parsid_AUTOFEED;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl |= parsid_AUTOFEED;
    parsid_ieee1284_outb_ctr(port, ctl);

    parsid_ieee1284_outb_data(port, outval);

    ctl |= parsid_STROBE;
    parsid_ieee1284_outb_ctr(port, ctl);

    ctl &= ~parsid_STROBE;
    parsid_ieee1284_outb_ctr(port, ctl);
}

static int detect_sid(struct parport *port)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        detect_sid_store(port, i, 0);
    }

    detect_sid_store(port, 0x12, 0xff);

    for (i = 0; i < 100; ++i) {
        if (detect_sid_read(port, 0x1b)) {
            return 0;
        }
    }

    detect_sid_store(port, 0x0e, 0xff);
    detect_sid_store(port, 0x0f, 0xff);
    detect_sid_store(port, 0x12, 0x20);

    for (i = 0; i < 100; ++i) {
        if (detect_sid_read(port, 0x1b)) {
            return 1;
        }
    }
    return 0;
}

int ps_ieee1284_open(void)
{
    int i = 0;
    int cap;
    int retval;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting libieee1284 ParSIDs.");

    if (ieee1284_find_ports(&parlist, 0) != E1284_OK) {
        return -1;
    }

    if (parlist.portv) {
        for (i = 0; i < MAXSID && i < parlist.portc; ++i) {
            retval = ieee1284_open(parlist.portv[i], F1284_EXCL, &cap);
            if (retval == E1284_OK) {
                retval = ieee1284_claim(parlist.portv[i]);
                if (retval == E1284_OK) {
                    if (detect_sid(parlist.portv[i])) {
                        pssids[sids_found] = i;
                        sids_found++;
                    }
                } else {
                    ieee1284_close(parlist.portv[i]);
                }
            }
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No libieee1284 ParSIDs found.");
        return -1;
    }

    log_message(LOG_DEFAULT, "Libieee1284 ParSID: opened, found %d SIDs.", sids_found);

    return 0;
}

int ps_ieee1284_close(void)
{
    int i;

    for (i = 0; i < sids_found; ++i) {
        ieee1284_release(parlist.portv[pssids[i]]);
        ieee1284_close(parlist.portv[pssids[i]]);
        pssids[i] = -1;
    }
    log_message(LOG_DEFAULT, "Libieee1284 ParSID: closed.");

    return 0;
}

void ps_ieee1284_out_ctr(uint8_t parsid_ctrport, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        parsid_ieee1284_outb_ctr(parlist.portv[pssids[chipno]], parsid_ctrport);
    }
}

uint8_t ps_ieee1284_in_ctr(int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        return parsid_ieee1284_inb_ctr(parlist.portv[pssids[chipno]]);
    }
    return 0;
}

void ps_ieee1284_out_data(uint8_t data, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        parsid_ieee1284_outb_data(parlist.portv[pssids[chipno]], data);
    }
}

uint8_t ps_ieee1284_in_data(int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        return parsid_ieee1284_inb_data(parlist.portv[pssids[chipno]]);
    }
    return 0;
}

int ps_ieee1284_available(void)
{
    return sids_found;
}
#endif
