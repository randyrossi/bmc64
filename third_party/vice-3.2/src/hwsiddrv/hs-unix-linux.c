/*
 * hs-unix-linux.c - Linux specific hardsid driver.
 *
 * Written by
 *  Simon White <sidplay2@yahoo.com>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, modified from the sidplay2 sources.  It is
 * a one for all driver with real timing support via real time kernel
 * extensions or through the hardware buffering.  It supports the hardsid
 * isa/pci single/quattro and also the catweasel MK3/4.
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

#if defined(HAVE_HARDSID) && defined(HAVE_LINUX_HARDSID_H)

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/hardsid.h>

#include "alarm.h"
#include "hardsid.h"
#include "log.h"
#include "maincpu.h"
#include "sid-resources.h"
#include "types.h"

/* Approx 3 PAL screen updates */
#define HARDSID_DELAY_CYCLES 50000

static int hsid_fd = -1;
static CLOCK hsid_main_clk;
static CLOCK hsid_alarm_clk;
static alarm_t *hsid_alarm = 0;

/* FIXME: currently only 1 SID is supported */
#define MAXSID 1

static int sids_found = -1;

static void hardsid_alarm_handler(CLOCK offset, void *data);

static int hardsid_init(void)
{
    /* Already open */
    if (hsid_fd >= 0) {
        return -1;
    }

    /* Open device */
    hsid_fd = open("/dev/sid", O_RDWR);
    if (hsid_fd < 0) {
        log_error(LOG_DEFAULT, "Could not open SID device /dev/sid.");
        return -1;
    }

    /* Make sure we have atleast sid */
    if (ioctl(hsid_fd, HSID_IOCTL_ALLOCATED, 0) == 0) {
        log_error(LOG_DEFAULT, "Could not find any SID installed.");
        close(hsid_fd);
        hsid_fd = -1;
        return -1;
    }
    return 0;
}

void hs_linux_reset(void)
{
    if (sids_found > 0) {
        hsid_main_clk  = maincpu_clk;
        hsid_alarm_clk = HARDSID_DELAY_CYCLES;
        alarm_set(hsid_alarm, HARDSID_DELAY_CYCLES);
    }
}

int hs_linux_open(void)
{
    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting Linux HardSID boards.");

    if (hardsid_init() < 0) {
        return -1;
    }
    hsid_alarm = alarm_new(maincpu_alarm_context, "hardsid", hardsid_alarm_handler, 0);
    sids_found = 1;
    hardsid_reset();
    log_message(LOG_DEFAULT, "Linux HardSID: opened.");
    return 0;
}

int hs_linux_close(void)
{
    /* Driver cleans up after itself */
    if (hsid_fd >= 0) {
        close(hsid_fd);
    }
    alarm_destroy(hsid_alarm);
    hsid_alarm = 0;
    sids_found = -1;
    log_message(LOG_DEFAULT, "Linux HardSID: closed.");
    return 0;
}

int hs_linux_read(uint16_t addr, int chipno)
{
    if (chipno < MAXSID && addr < 0x20 && hsid_fd >= 0) {
        CLOCK cycles = maincpu_clk - hsid_main_clk - 1;
        hsid_main_clk = maincpu_clk;

        while (cycles > 0xffff) {
            /* delay */
            ioctl(hsid_fd, HSID_IOCTL_DELAY, 0xffff);
            cycles -= 0xffff;
        }

        {
            uint packet = ((cycles & 0xffff) << 16) | ((addr & 0x1f) << 8);
            ioctl(hsid_fd, HSID_IOCTL_READ, &packet);
            return (int)packet;
        }
    }
    return 0;
}

void hs_linux_store(uint16_t addr, uint8_t val, int chipno)
{
    if (chipno < MAXSID && addr < 0x20 && hsid_fd >= 0) {
        CLOCK cycles = maincpu_clk - hsid_main_clk - 1;
        hsid_main_clk = maincpu_clk;

        while (cycles > 0xffff) {
            /* delay */
            ioctl(hsid_fd, HSID_IOCTL_DELAY, 0xffff);
            cycles -= 0xffff;
        }

        uint packet = ((cycles & 0xffff) << 16) | ((addr & 0x1f) << 8) | val;
        write(hsid_fd, &packet, sizeof (packet));
    }
}

unsigned int hs_linux_available(void)
{
    return sids_found;
}

static void hardsid_alarm_handler(CLOCK offset, void *data)
{
    CLOCK cycles = (hsid_alarm_clk + offset) - hsid_main_clk;

    if (cycles < HARDSID_DELAY_CYCLES) {
        hsid_alarm_clk = hsid_main_clk + HARDSID_DELAY_CYCLES;
    } else {
        uint delay = (uint) cycles;
        ioctl(hsid_fd, HSID_IOCTL_DELAY, delay);
        hsid_main_clk   = maincpu_clk - offset;
        hsid_alarm_clk  = hsid_main_clk + HARDSID_DELAY_CYCLES;
    }
    alarm_set(hsid_alarm, hsid_alarm_clk);
}

/* ---------------------------------------------------------------------*/

void hs_linux_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    sid_state->hsid_main_clk = (uint32_t)hsid_main_clk;
    sid_state->hsid_alarm_clk = (uint32_t)hsid_alarm_clk;
    sid_state->lastaccess_clk = 0;
    sid_state->lastaccess_ms = 0;
    sid_state->lastaccess_chipno = 0;
    sid_state->chipused = 0;
    sid_state->device_map[0] = 0;
    sid_state->device_map[1] = 0;
    sid_state->device_map[2] = 0;
    sid_state->device_map[3] = 0;
}

void hs_linux_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    hsid_main_clk = (CLOCK)sid_state->hsid_main_clk;
    hsid_alarm_clk = (CLOCK)sid_state->hsid_alarm_clk;
}
#endif
#endif
