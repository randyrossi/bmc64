/*
 * parsid-win32-drv.c - PARallel port SID support for WIN32.
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

 - Windows 95C (Direct ISA I/O, inpout32.dll is incompatible with windows 95)
 - Windows 98SE (winio.dll ISA I/O)
 - Windows 98SE (inpout32.dll ISA I/O)
 - Windows 98SE (Direct ISA I/O)
 - Windows ME (winio.dll ISA I/O)
 - Windows ME (inpout32.dll ISA I/O)
 - Windows ME (Direct ISA I/O)
 - Windows NT 4.0 (winio32.dll ISA I/O)
 - Windows NT 4.0 (inpout32.dll ISA I/O)
 - Windows 2000 (winio32.dll ISA I/O)
 - Windows 2000 (inpout32.dll ISA I/O)
 - Windows XP (winio32.dll ISA I/O)
 - Windows XP (inpout32.dll ISA I/O)
 - Windows 2003 (winio32.dll ISA I/O)
 - Windows 2003 (inpout32.dll ISA I/O)
 */

#include "vice.h"

#ifdef WIN32_COMPILE

#ifdef HAVE_PARSID

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "archdep.h"
#include "parsid.h"
#include "ps-win32.h"
#include "types.h"

#ifdef HAVE_LIBIEEE1284
static int use_ieee1284 = 0;
#endif

static int use_dll = 0;
static int use_io = 0;


void parsid_drv_out_ctr(uint8_t parsid_ctrport, int chipno)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        ps_ieee1284_out_ctr(parsid_ctrport, chipno);
    }
#endif

    if (use_dll) {
        ps_dll_out_ctr(parsid_ctrport, chipno);
    }

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

    if (use_dll) {
        return ps_dll_in_ctr(chipno);
    }

    if (use_io) {
        return ps_io_in_ctr(chipno);
    }
    return 0;
}

int parsid_drv_close(void)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        ps_ieee1284_close();
        use_ieee1284 = 0;
    }
#endif

    if (use_dll) {
        ps_dll_close();
        use_dll = 0;
    }

    if (use_io) {
        ps_io_close();
        use_io = 0;
    }

    return 0;
}

uint8_t parsid_drv_in_data(int chipno)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        return ps_ieee1284_in_data(chipno);
    }
#endif

    if (use_dll) {
        return ps_dll_in_data(chipno);
    }

    if (use_io) {
        return ps_io_in_data(chipno);
    }

    return 0;
}

void parsid_drv_out_data(uint8_t addr, int chipno)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        ps_ieee1284_out_data(addr, chipno);
    }
#endif

    if (use_dll) {
        ps_dll_out_data(addr, chipno);
    }

    if (use_io) {
        ps_io_out_data(addr, chipno);
    }
}


void parsid_drv_sleep(int amount)
{
    vice_usleep(amount);
}

int parsid_drv_available(void)
{
#ifdef HAVE_LIBIEEE1284
    if (use_ieee1284) {
        return ps_ieee1284_available();
    }
#endif

    if (use_dll) {
        return ps_dll_available();
    }

    if (use_io) {
        return ps_io_available();
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

    i = ps_dll_open();
    if (!i) {
        use_dll = 1;
        return 0;
    }

    i = ps_io_open();
    if (!i) {
        use_io = 1;
        return 0;
    }

    return -1;
}
#endif
#endif
