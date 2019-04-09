/*
 * catweaselmkiii-unix-drv.c - Unix specific cw3 driver.
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

 - Linux 2.6 (driver)
 - Linux 2.6 (/dev/port PCI I/O access)
 - Linux 2.6 (permission based PCI I/O access)
 */

#include "vice.h"

#ifdef UNIX_COMPILE

#ifdef HAVE_CATWEASELMKIII

#include "catweaselmkiii.h"
#include "cw-unix.h"
#include "types.h"

#ifdef HAVE_CWSID_H
static int use_cw_device = 0;
#endif

#ifdef HAVE_CATWEASELMKIII_IO
static int use_cw_pci = 0;
#endif

int catweaselmkiii_drv_open(void)
{
#ifdef HAVE_CWSID_H
    if (!cw_device_open()) {
        use_cw_device = 1;
        return 0;
    }
#endif

#ifdef HAVE_CATWEASELMKIII_IO
    if (!cw_pci_open()) {
        use_cw_pci = 1;
        return 0;
    }
#endif

    return -1;
}

int catweaselmkiii_drv_available(void)
{
#ifdef HAVE_CWSID_H
    if (use_cw_device) {
        return cw_device_available();
    }
#endif

#ifdef HAVE_CATWEASELMKIII_IO
    if (use_cw_pci) {
        return cw_pci_available();
    }
#endif

    return 0;
}

int catweaselmkiii_drv_close(void)
{
#ifdef HAVE_CWSID_H
    if (use_cw_device) {
        cw_device_close();
        use_cw_device = 0;
    }
#endif

#ifdef HAVE_CATWEASELMKIII_IO
    if (use_cw_pci) {
        cw_pci_close();
        use_cw_pci = 0;
    }
#endif

    return 0;
}

int catweaselmkiii_drv_read(uint16_t addr, int chipno)
{
#ifdef HAVE_CWSID_H
    if (use_cw_device) {
        return cw_device_read(addr, chipno);
    }
#endif

#ifdef HAVE_CATWEASELMKIII_IO
    if (use_cw_pci) {
        return cw_pci_read(addr, chipno);
    }
#endif

    return 0;
}

void catweaselmkiii_drv_store(uint16_t addr, uint8_t val, int chipno)
{
#ifdef HAVE_CWSID_H
    if (use_cw_device) {
        cw_device_store(addr, val, chipno);
    }
#endif

#ifdef HAVE_CATWEASELMKIII_IO
    if (use_cw_pci) {
        cw_pci_store(addr, val, chipno);
    }
#endif
}

void catweaselmkiii_drv_set_machine_parameter(long cycles_per_sec)
{
#ifdef HAVE_CWSID_H
    if (use_cw_device) {
        cw_device_set_machine_parameter(cycles_per_sec);
    }
#endif
}
#endif
#endif
