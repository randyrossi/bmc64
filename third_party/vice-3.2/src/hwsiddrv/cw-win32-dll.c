/*
 * cw-win32-dll.c - Windows specific cw3 driver.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Dirk Jagdmann <doj@cubic.org>
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

 - Windows 98SE
 - Windows ME
 - Windows 2000
 */

#include "vice.h"

#ifdef WIN32_COMPILE

#ifdef HAVE_CATWEASELMKIII

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef HAVE_WINIOCTL_H
#include <winioctl.h>
#endif

#include "catweaselmkiii.h"
#include "cw-win32.h"
#include "log.h"
#include "types.h"

/* defined for CatWeasel MK3 PCI device driver */
#define SID_SID_PEEK_POKE CTL_CODE(FILE_DEVICE_SOUND, 0x0800UL + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SID_SET_CLOCK     CTL_CODE(FILE_DEVICE_SOUND, 0x0800UL + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SID_CMD_READ      32

static int sids_found = -1;

/* array containing file handles for up to MAXCARD CatWeasels */
static HANDLE sidhandle[CW_MAXCARDS] = {
    INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,
};

/* set all CatWeasels frequency to buf. buf=0: pal ; buf=1: ntsc */
static void setfreq(uint8_t buf)
{
    int i;

    for (i = 0; i < CW_MAXCARDS; i++) {
        if (sidhandle[i] != INVALID_HANDLE_VALUE) {
            uint32_t w;

            DeviceIoControl(sidhandle[i], SID_SET_CLOCK, &buf, sizeof(buf), 0L, 0UL, (LPDWORD)&w, 0L);
        }
    }
}

/* silent al SIDs by setting all registers to 0 */
static void mutethem(void)
{
    uint32_t w;
    int i;
    uint8_t buf[0x19 * 2];

    for (i = 0; i <= 0x18; i++) {
        buf[i * 2] = i;
        buf[i * 2 + 1] = 0;
    }
    for (i = 0; i < CW_MAXCARDS; i++) {
        if (sidhandle[i] != INVALID_HANDLE_VALUE) {
            DeviceIoControl(sidhandle[i], SID_SID_PEEK_POKE, buf, sizeof(buf), 0L, 0UL, (LPDWORD)&w, 0L);
        }
    }
}

/* open all available CatWeasel devices */
int cw_dll_open(void)
{
    int i;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting PCI CatWeasel boards.");

    /* close any open handles */
    for (i = 0; i < CW_MAXCARDS; i++) {
        if (sidhandle[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(sidhandle[i]), sidhandle[i] = INVALID_HANDLE_VALUE;
        }
    }

    /* find up to four CatWeasel cards */
    for (i = 0; i < CW_MAXCARDS; i++) {
        char buf[32];

        sprintf(buf, "\\\\.\\SID6581_%u", i + 1);
        sidhandle[sids_found] = CreateFile(buf, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 0L, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L);
        if (sidhandle[sids_found] != INVALID_HANDLE_VALUE) {
            log_message(LOG_DEFAULT, "Found PCI CatWeasel #%i.", sids_found + 1);
            sids_found++;
        }
    }

    /* if cards were found */
    if (sids_found > 0) {
        log_message(LOG_DEFAULT, "Found and opened a PCI CatWeasel SID.");

        /* silent all found cards */
        mutethem();

        /* set frequeny of found cards */
        setfreq((uint8_t)catweaselmkiii_get_ntsc());

        return 0;
    }

    log_message(LOG_DEFAULT, "No PCI CatWeasel found.");

    /* no CatWeasels were found */
    return -1;
}

/* close all open device handles */
int cw_dll_close(void)
{
    int i;

    mutethem();
    for (i = 0; i < CW_MAXCARDS; i++) {
        if (sidhandle[i] != INVALID_HANDLE_VALUE) {
            CloseHandle(sidhandle[i]);
            sidhandle[i] = INVALID_HANDLE_VALUE;
        }
    }

    log_message(LOG_DEFAULT, "Closed PCI CatWeasel SID.");

    sids_found = -1;

    return 0;
}

/* read register from sid. only the four read registers from $19 to
   $1C are read from the real SID chip, all other bytes are read from
   sidbuf[] */
int cw_dll_read(uint16_t addr, int chipno)
{
    /* check if chipno is valid */
    if (chipno < CW_MAXCARDS) {
        /* check if addr is in read-only range, and that a card was found */
        if (addr >= 0x19 && addr <= 0x1C && sidhandle[chipno] != INVALID_HANDLE_VALUE) {
            /* do real read */
            uint32_t w;
            uint8_t buf[2];

            buf[0] = SID_CMD_READ;
            buf[1] = (uint8_t)(addr & 0xff);
            DeviceIoControl(sidhandle[chipno], SID_SID_PEEK_POKE, buf, 2, buf, 1, (LPDWORD)&w, 0L);
            return buf[0];
        }
    }
    log_error(LOG_ERR, "PCI CatWeasel does not support SID #%i.", chipno);
    return 0;
}

/* write a value */
void cw_dll_store(uint16_t addr, uint8_t val, int chipno)
{
    /* check if chipno is valid */
    if (chipno < CW_MAXCARDS) {
        /* check if addr is valid */
        if (addr <= 0x18) {
            /* check if device handle is valid */
            if (sidhandle[chipno] != INVALID_HANDLE_VALUE) {
                /* perform real write */
                uint32_t w;
                uint8_t buf[2];

                buf[0] = (uint8_t)(addr & 0xff);
                buf[1] = val;
                DeviceIoControl(sidhandle[chipno], SID_SID_PEEK_POKE, buf, sizeof(buf), 0L, 0UL, (LPDWORD)&w, 0L);
            }
            return;
        }
        log_error(LOG_ERR, "PCI CatWeasel store at address %04x.", addr);
        return;
    }
    log_error(LOG_ERR, "PCI CatWeasel does not support SID #%i.", chipno);
}

/* set current main clock frequency, which gives us the possibilty to
   choose between pal and ntsc frequencies */
void cw_dll_set_machine_parameter(long cycles_per_sec)
{
    uint8_t ntsc = (uint8_t)((cycles_per_sec <= 1000000) ? 0 : 1);
    setfreq(ntsc);
}

int cw_dll_available(void)
{
    return sids_found;
}
#endif
#endif
