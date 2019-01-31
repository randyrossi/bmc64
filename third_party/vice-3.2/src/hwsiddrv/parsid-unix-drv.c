/*
 * parsid-unix-drv.c - PARallel port SID support for UNIX.
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

 - Linux 2.6 (/dev/port based ISA I/O)
 - Linux 2.6 (permission based ISA I/O)
 - FreeBSD (/dev/io based ISA I/O)
 - NetBSD (permission based ISA I/O)
 - OpenBSD (permission bases ISA I/O)
 */

#include "vice.h"

#ifdef UNIX_COMPILE

#ifdef HAVE_PARSID

#include <unistd.h>

#include "ps-unix.h"
#include "types.h"

#include "parsid.h"

static int use_io = 0;

#ifdef HAVE_PORTSID
static int use_port = 0;
#endif

#ifdef HAVE_LIBIEEE1284
static int use_ieee1284 = 0;
#endif

void parsid_drv_out_ctr(uint8_t parsid_ctrport, int chipno)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        ps_ieee1284_out_ctr(parsid_ctrport, chipno);
    }
#endif

#ifdef HAVE_PORTSID
    if (use_port) {
        ps_file_out_ctr(parsid_ctrport, chipno);
    }
#endif

    if (use_io) {
        ps_io_out_ctr(parsid_ctrport, chipno);
    }
}

uint8_t parsid_drv_in_ctr(int chipno)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        return ps_ieee1284_in_ctr(chipno);
    }
#endif

#ifdef HAVE_PORTSID
    if (use_port) {
        return ps_file_in_ctr(chipno);
    }
#endif

    if (use_io) {
        return ps_io_in_ctr(chipno);
    }
    return 0;
}

int parsid_drv_open(void)
{
    int i;

#ifdef HAVE_LIBIEEE1284
    i = ps_ieee1284_open();
    if (!i) {
        use_ieee1284 = 1;
        return 0;
    }
#endif

#ifdef HAVE_PORTSID
    i = ps_file_open();
    if (!i) {
        use_port = 1;
        return 0;
    }
#endif

    i = ps_io_open();
    if (!i) {
        use_io = 1;
        return 0;
    }
    return -1;
}

uint8_t parsid_drv_in_data(int chipno)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        return ps_ieee1284_in_data(chipno);
    }
#endif

#ifdef HAVE_PORTSID
    if (use_port) {
        return ps_file_in_data(chipno);
    }
#endif

    if (use_io) {
        return ps_io_in_data(chipno);
    }
    return 0;
}

void parsid_drv_out_data(uint8_t outval, int chipno)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        ps_ieee1284_out_data(outval, chipno);
    }
#endif

#ifdef HAVE_PORTSID
    if (use_port) {
        ps_file_out_data(outval, chipno);
    }
#endif

    if (use_io) {
        ps_io_out_data(outval, chipno);
    }
}

int parsid_drv_close(void)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        ps_ieee1284_close();
        use_ieee1284 = 0;
    }
#endif

#ifdef HAVE_PORTSID
    if (use_port) {
        ps_file_close();
        use_port = 0;
    }
#endif

    if (use_io) {
        ps_io_close();
        use_io = 0;
    }

    return 0;
}

void parsid_drv_sleep(int amount)
{
    usleep(amount);
}

int parsid_drv_available(void)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        return ps_ieee1284_available();
    }
#endif

#ifdef HAVE_PORTSID
    if (use_port) {
        return ps_file_available();
    }
#endif

    if (use_io) {
        return ps_io_available();
    }

    return 0;
}
#endif
#endif
