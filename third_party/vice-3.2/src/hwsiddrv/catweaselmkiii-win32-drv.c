/*
 * catweaselmkiii-win32-drv.c - Windows specific cw3 driver.
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

 - Windows 95C (Direct PCI I/O)
 - Windows 98SE (Driver)
 - Windows 98SE (Direct PCI I/O)
 - Windows ME (Driver)
 - Windows ME (Direct PCI I/O)
 - Windows NT 4.0 (winio32.dll PCI I/O)
 - Windows 2000 (Driver)
 - Windows 2000 (winio32.dll PCI I/O)
 - Windows XP (Driver)
 - Windows XP (winio32.dll PCI I/O)
 */

#include "vice.h"

#ifdef WIN32_COMPILE

#ifdef HAVE_CATWEASELMKIII

#include "catweaselmkiii.h"
#include "cw-win32.h"
#include "types.h"

static int use_cw_pci = 0;
static int use_cw_dll = 0;

int catweaselmkiii_drv_open(void)
{
    int retval;

    retval = cw_dll_open();
    if (!retval) {
        use_cw_dll = 1;
        return 0;
    }

    retval = cw_pci_open();
    if (!retval) {
        use_cw_pci = 1;
        return 0;
    }

    return -1;
}

int catweaselmkiii_drv_close(void)
{
    if (use_cw_pci) {
        use_cw_pci = 0;
        cw_pci_close();
    }
    if (use_cw_dll) {
        use_cw_dll = 0;
        cw_dll_close();
    }
    return 0;
}

int catweaselmkiii_drv_read(uint16_t addr, int chipno)
{
    if (use_cw_pci) {
        return cw_pci_read(addr, chipno);
    }
    if (use_cw_dll) {
        return cw_dll_read(addr, chipno);
    }
    return 0;
}

void catweaselmkiii_drv_store(uint16_t addr, uint8_t val, int chipno)
{
    if (use_cw_pci) {
        cw_pci_store(addr, val, chipno);
    }
    if (use_cw_dll) {
        cw_dll_store(addr, val, chipno);
    }
}

void catweaselmkiii_drv_set_machine_parameter(long cycles_per_sec)
{
    if (use_cw_dll) {
        cw_dll_set_machine_parameter(cycles_per_sec);
    }
}

int catweaselmkiii_drv_available(void)
{
    if (use_cw_pci) {
        return cw_pci_available();
    }
    if (use_cw_dll) {
        return cw_dll_available();
    }
    return 0;
}
#endif
#endif
