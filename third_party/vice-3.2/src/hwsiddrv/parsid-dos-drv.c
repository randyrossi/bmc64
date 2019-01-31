/*
 * parsid-dos-drv.c - PARallel port SID support for MSDOS.
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

#ifdef HAVE_PARSID
#include <dos.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <pc.h>
#include <dpmi.h>
#include <libc/farptrgs.h>
#include <go32.h>

#include "alarm.h"
#include "parsid.h"
#include "log.h"
#include "sid-resources.h"
#include "types.h"

#define MAXSID 3

static unsigned int ports[MAXSID] = {-1, -1, -1};
static int pssids[MAXSID] = {-1, -1, -1};
static int psctrl[MAXSID] = {-1, -1, -1};
static int sids_found = -1;

/* input/output functions */
static uint8_t parsid_inb(uint16_t addr)
{
    return inportb(addr);
}

static void parsid_outb(uint16_t addr, uint8_t value)
{
    outportb(addr, value);
}

void parsid_drv_out_ctr(uint8_t parsid_ctrport, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        parsid_outb(pssids[chipno] + 2, parsid_ctrport);
        psctrl[chipno] = parsid_ctrport;
    }
}

uint8_t parsid_drv_in_ctr(int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        if (psctrl[chipno] == -1) {
            parsid_outb(pssids[chipno] + 2, 0);
            psctrl[chipno] = 0;
        } else {
            return psctrl[chipno];
        }
    }
    return 0;
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

static void parsid_get_ports(void)
{
    unsigned int address;            /* Address of Port */
    int j;
    unsigned long ptraddr = 0x0408;  /* Base Address: segment is zero*/
    int same = 0;

    for (j = 0; j < 4; j++) {
        ports[j] = _farpeekw(_dos_ds, ptraddr);
        ptraddr += 2;
        log_message(LOG_DEFAULT, "Parallel port %d is at $%X.", j, address);
    }

    if (ports[0] == ports[1] && ports[0] == ports[2]) {
        log_message(LOG_DEFAULT, "Addresses are all the same, replacing addresses with $278, $378 and $3BC.");
        ports[0] = 0x278;
        ports[1] = 0x378;
        ports[2] = 0x3BC;
        ports[3] = -1;
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

    if (is_windows_nt()) {
        log_message(LOG_DEFAULT, "Running on Windows NT, cannot use direct memory access.");
        return 0;
    }

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

    log_message(LOG_DEFAULT, "Detecting ParSIDs.");

    parsid_get_ports();

    for (i = 0; i < MAXSID; ++i) {
        if (ports[i] != -1) {
            pssids[sids_found] = ports[i];
            psctrl[sids_found] = -1;
            if (detect_sid(sids_found)) {
                sids_found++;
                log_message(LOG_DEFAULT, "ParSID found on port at address $%X.", ports[i]);
            }
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No ParSIDs found.");
        return -1;
    }

    log_message(LOG_DEFAULT, "ParSID: opened, found %d SIDs.", sids_found);

    return 0;
}

int parsid_drv_close(void)
{
    int i;

    for (i = 0; i < MAXSID; ++i) {
        pssids[i] = -1;
        ports[i] = -1;
        psctrl[i] = -1;
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "ParSID: closed.");

    return 0;
}

uint8_t parsid_drv_in_data(int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        return parsid_inb(pssids[chipno]);
    }
    return 0;
}

void parsid_drv_out_data(uint8_t outval, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != -1) {
        parsid_outb(pssids[chipno], outval);
    }
}

void parsid_drv_sleep(int amount)
{
    sleep(amount);
}

int parsid_drv_available(void)
{
    return sids_found;
}
#endif
#endif
