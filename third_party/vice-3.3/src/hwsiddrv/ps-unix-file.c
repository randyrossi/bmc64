/*
 * ps-unix-file.c - file device based PARallel port SID support for UNIX.
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

#ifdef UNIX_COMPILE

#ifdef HAVE_PORTSID

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#ifdef HAVE_LINUX_PARPORT_HEADERS
#include <linux/ppdev.h>
#include <linux/parport.h>
#endif

#ifdef HAVE_FREEBSD_PARPORT_HEADERS
#include <dev/ppbus/ppi.h>
#include <dev/ppbus/ppbconf.h>
#endif

#include "alarm.h"
#include "log.h"
#include "parsid.h"
#include "ps-unix.h"
#include "sid-resources.h"
#include "types.h"

#define MAXSID 3

#ifdef HAVE_LINUX_PARPORT_HEADERS
static char *parport_name[MAXSID] = { "/dev/parport0", "/dev/parport1", "/dev/parport2" };
#define PARPORT_TYPE int
#define PARPORT_NULL -1
#endif

#ifdef HAVE_FREEBSD_PARPORT_HEADERS
static char *parport_name[MAXSID] = { "/dev/ppi0", "/dev/ppi1", "/dev/ppi2" };
#define PARPORT_TYPE int
#define PARPORT_NULL -1
#endif

#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
#define IOPREAD 1
#define IOPWRITE 2
#define PARPORT_TYPE int
#define PARPORT_NULL -1

typedef struct iopbuf_struct {
    unsigned int port;
    unsigned char port_value;
} iopbuf;

static int fd = -1;

static int ports[MAXSID] = {0x3bc, 0x378, 0x278};
#endif

static int sids_found = -1;
static PARPORT_TYPE pssids[MAXSID] = {PARPORT_NULL, PARPORT_NULL, PARPORT_NULL};
static int psctrl[MAXSID] = {-1, -1, -1};

/* Some prototypes. */
static uint8_t detect_sid_read(uint16_t addr, int chipno);
static void detect_sid_store(uint16_t addr, uint8_t outval, int chipno);
static int detect_sid(int chipno);

#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
uint8_t ps_file_in_data(int chipno)
{
    iopbuf tmpbuf;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        tmpbuf.port_value = 0;
        tmpbuf.port = pssids[chipno];

        ioctl(fd, IOPREAD, &tmpbuf);
    }
    return tmpbuf.port_value;
}

void ps_file_out_data(uint8_t outval, int chipno)
{
    iopbuf tmpbuf;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        tmpbuf.port_value = outval;
        tmpbuf.port = pssids[chipno];

        ioctl(fd, IOPWRITE, &tmpbuf);
    }
}

void ps_file_out_ctr(uint8_t parsid_ctrport, int chipno)
{
    iopbuf tmpbuf;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        tmpbuf.port_value = parsid_ctrport;
        tmpbuf.port = pssids[chipno] + 2;

        ioctl(fd, IOPWRITE, &tmpbuf);
        psctrl[chipno] = parsid_ctrport;
    }
}

uint8_t ps_file_in_ctr(int chipno)
{
    iopbuf tmpbuf;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        if (psctrl[chipno] == -1) {
            tmpbuf.port_value = 0;
            tmpbuf.port = pssids[chipno] + 2;
            ioctl(fd, IOPWRITE, &tmpbuf);
            psctrl[chipno] = 0;
        } else {
            return (uint8_t)psctrl[chipno];
        }
    }
    return 0;
}

int ps_file_open(void)
{
    int i;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting Solaris ParSIDs.");

    fd = open("/devices/pseudo/iop@0:iop", O_RDONLY);

    if (fd < 0) {
        log_message(LOG_DEFAULT, "Solaris IOP device not found.");
        return -1;
    }

    for (i = 0; i < MAXSID; ++i) {
        pssids[sids_found] = ports[i];
        if (detect_sid(sids_found)) {
            sids_found++;
            log_message(LOG_DEFAULT, "ParSID found at $%X.", ports[i]);
        } else {
            log_message(LOG_DEFAULT, "No ParSID at $%X.", ports[i]);
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No Solaris ParSIDs found.");
        close(fd);
        return -1;
    }

    log_message(LOG_DEFAULT, "Solaris ParSID: opened, found %d SIDs.", sids_found);

    return 0;
}

int ps_file_close(void)
{
    close(fd);

    log_message(LOG_DEFAULT, "Solaris ParSID: closed.");

    return 0;
}
#endif

#ifdef HAVE_LINUX_PARPORT_HEADERS
uint8_t ps_file_in_data(int chipno)
{
    uint8_t retval = 0;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        ioctl(pssids[chipno], PPRDATA, &retval);
    }
    return retval;
}

void ps_file_out_data(uint8_t outval, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        ioctl(pssids[chipno], PPWDATA, &outval);
    }
}

void ps_file_out_ctr(uint8_t parsid_ctrport, int chipno)
{
    uint8_t ctl = 0;
    int datadir = 0;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        if (parsid_ctrport & parsid_STROBE) {
            ctl |= PARPORT_CONTROL_STROBE;
        }
        if (parsid_ctrport & parsid_AUTOFEED) {
            ctl |= PARPORT_CONTROL_AUTOFD;
        }
        if (parsid_ctrport & parsid_nINIT) {
            ctl |= PARPORT_CONTROL_INIT;
        }
        if (parsid_ctrport & parsid_SELECTIN) {
            ctl |= PARPORT_CONTROL_SELECT;
        }
        datadir = (parsid_ctrport & parsid_PCD) ? 1 : 0;
        ioctl(pssids[chipno], PPWCONTROL, &ctl);
        ioctl(pssids[chipno], PPDATADIR, &datadir);
        psctrl[chipno] = parsid_ctrport;
    }
}

uint8_t ps_file_in_ctr(int chipno)
{
    uint8_t retval = 0;
    uint8_t ctl;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        ioctl(pssids[chipno], PPRCONTROL, &ctl);
        if (ctl & PARPORT_CONTROL_STROBE) {
            retval |= parsid_STROBE;
        }
        if (ctl & PARPORT_CONTROL_AUTOFD) {
            retval |= parsid_AUTOFEED;
        }
        if (ctl & PARPORT_CONTROL_INIT) {
            retval |= parsid_nINIT;
        }
        if (ctl & PARPORT_CONTROL_SELECT) {
            retval |= parsid_SELECTIN;
        }
    }

    return retval;
}

int ps_file_open(void)
{
    int i;
    int mode = IEEE1284_MODE_COMPAT;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting Linux ParSIDs.");

    for (i = 0; i < MAXSID; ++i) {
        pssids[sids_found] = open(parport_name[i], O_RDWR);
        if (pssids[sids_found] != -1) {
            if (!ioctl(pssids[sids_found], PPCLAIM)) {
                if (!ioctl(pssids[sids_found], PPNEGOT, &mode)) {
                    if (detect_sid(sids_found)) {
                        sids_found++;
                        log_message(LOG_DEFAULT, "PARSID found on %s.", parport_name[i]);
                    } else {
                        log_message(LOG_DEFAULT, "No ParSID on %s.", parport_name[i]);
                        close(pssids[sids_found]);
                        pssids[sids_found] = -1;
                    }
                } else {
                    log_message(LOG_DEFAULT, "Could not set correct mode for %s.", parport_name[i]);
                    close(pssids[sids_found]);
                    pssids[sids_found] = -1;
                }
            } else {
                log_message(LOG_DEFAULT, "Could not claim %s.", parport_name[i]);
                close(pssids[sids_found]);
                pssids[sids_found] = -1;
            }
        } else {
            log_message(LOG_DEFAULT, "Could not open %s.", parport_name[i]);
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No Linux ParSIDs found.");
        return -1;
    }

    log_message(LOG_DEFAULT, "Linux ParSID: opened, found %d SIDs.", sids_found);

    return 0;
}

int ps_file_close(void)
{
    int i;

    for (i = 0; i < MAXSID; ++i) {
        if (pssids[i] != -1) {
            close(pssids[i]);
            pssids[i] = -1;
        }
    }

    log_message(LOG_DEFAULT, "Linux ParSID: closed.");

    return 0;
}
#endif

#ifdef HAVE_FREEBSD_PARPORT_HEADERS
uint8_t ps_file_in_data(int chipno)
{
    uint8_t retval = 0;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        ioctl(pssids[chipno], PPIGDATA, &retval);
    }
    return retval;
}

void ps_file_out_data(uint8_t outval, int chipno)
{
    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        ioctl(pssids[chipno], PPISDATA, &outval);
    }
}


void ps_file_out_ctr(uint8_t parsid_ctrport, int chipno)
{
    uint8_t ctl = 0;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        if (parsid_ctrport & parsid_STROBE) {
            ctl |= STROBE;
        }
        if (parsid_ctrport & parsid_AUTOFEED) {
            ctl |= AUTOFEED;
        }
        if (parsid_ctrport & parsid_nINIT) {
            ctl |= nINIT;
        }
        if (parsid_ctrport & parsid_SELECTIN) {
            ctl |= SELECTIN;
        }
        if (parsid_ctrport & parsid_PCD) {
            ctl |= PCD;
        }

        ioctl(pssids[chipno], PPISCTRL, &ctl);
        psctrl[chipno] = parsid_ctrport;
    }
}

uint8_t ps_file_in_ctr(int chipno)
{
    uint8_t retval = 0;
    uint8_t ctl;

    if (chipno < MAXSID && pssids[chipno] != PARPORT_NULL) {
        ioctl(pssids[chipno], PPIGCTRL, &ctl);
        if (ctl & STROBE) {
            retval |= parsid_STROBE;
        }
        if (ctl & AUTOFEED) {
            retval |= parsid_AUTOFEED;
        }
        if (ctl & nINIT) {
            retval |= parsid_nINIT;
        }
        if (ctl & SELECTIN) {
            retval |= parsid_SELECTIN;
        }
        if (ctl & PCD) {
            retval |= parsid_PCD;
        }
    }

    return retval;
}

int ps_file_open(void)
{
    int i;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting FreeBSD ParSIDs.");

    for (i = 0; i < MAXSID; ++i) {
        pssids[sids_found] = open(parport_name[i], O_RDWR);
        if (pssids[sids_found] != -1) {
            if (detect_sid(sids_found)) {
                sids_found++;
                log_message(LOG_DEFAULT, "PARSID found on %s.", parport_name[i]);
            } else {
                log_message(LOG_DEFAULT, "No ParSID on %s.", parport_name[i]);
                close(pssids[sids_found]);
                pssids[sids_found] = -1;
            }
        } else {
            log_message(LOG_DEFAULT, "Could not open %s.", parport_name[i]);
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No FreeBSD ParSIDs found.");
        return -1;
    }

    log_message(LOG_DEFAULT, "FreeBSD ParSID: opened, found %d SIDs.", sids_found);

    return 0;
}

int ps_file_close(void)
{
    int i;

    for (i = 0; i < MAXSID; ++i) {
        if (pssids[i] != -1) {
            close(pssids[i]);
            pssids[i] = -1;
        }
    }

    log_message(LOG_DEFAULT, "FreeBSD ParSID: closed.");

    return 0;
}
#endif

static uint8_t detect_sid_read(uint16_t addr, int chipno)
{
    uint8_t value = 0;
    uint8_t ctl = ps_file_in_ctr(chipno);

    ps_file_out_data(addr & 0x1f, chipno);

    ctl &= ~parsid_AUTOFEED;
    ps_file_out_ctr(ctl, chipno);

    ctl |= parsid_AUTOFEED;
    ps_file_out_ctr(ctl, chipno);

    ctl |= parsid_PCD;
    ps_file_out_ctr(ctl, chipno);

    ctl |= parsid_nINIT;
    ps_file_out_ctr(ctl, chipno);

    ctl |= parsid_STROBE;
    ps_file_out_ctr(ctl, chipno);

    value = ps_file_in_data(chipno);

    ctl &= ~parsid_STROBE;
    ps_file_out_ctr(ctl, chipno);

    ctl &= ~parsid_PCD;
    ps_file_out_ctr(ctl, chipno);

    ctl &= ~parsid_nINIT;
    ps_file_out_ctr(ctl, chipno);

    return value;
}

static void detect_sid_store(uint16_t addr, uint8_t outval, int chipno)
{
    uint8_t ctl = ps_file_in_ctr(chipno);

    ps_file_out_data(addr & 0x1f, chipno);

    ctl &= ~parsid_AUTOFEED;
    ps_file_out_ctr(ctl, chipno);

    ctl |= parsid_AUTOFEED;
    ps_file_out_ctr(ctl, chipno);

    ps_file_out_data(outval, chipno);

    ctl |= parsid_STROBE;
    ps_file_out_ctr(ctl, chipno);

    ctl &= ~parsid_STROBE;
    ps_file_out_ctr(ctl, chipno);
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

int ps_file_available(void)
{
    return sids_found;
}
#endif
#endif
