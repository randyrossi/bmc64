/*
 * hardsid-win32-drv.c - Windows specific hardsid driver.
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

 - Windows 95C (ISA HardSID, hardsid.dll)
 - Windows 95C (ISA HardSID Quattro, hardsid.dll)
 - Windows 95C (ISA HardSID, Direct ISA I/O, inpout32.dll is incompatible with windows 95)
 - Windows 95C (ISA HardSID Quattro, Direct ISA I/O, inpout32.dll is incompatible with windows 95)
 - Windows 95C (PCI HardSID, Direct PCI I/O)
 - Windows 95C (PCI HardSID Quattro, Direct PCI I/O)
 - Windows 98SE (ISA HardSID, hardsid.dll)
 - Windows 98SE (ISA HardSID Quattro, hardsid.dll)
 - Windows 98SE (ISA HardSID, winio.dll ISA I/O)
 - Windows 98SE (ISA HardSID Quattro, winio.dll ISA I/O)
 - Windows 98SE (ISA HardSID, inpout32.dll ISA I/O)
 - Windows 98SE (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows 98SE (ISA HardSID, Direct ISA I/O)
 - Windows 98SE (ISA HardSID Quattro, Direct ISA I/O)
 - Windows 98SE (PCI HardSID, hardsid.dll)
 - Windows 98SE (PCI HardSID, Direct PCI I/O)
 - Windows 98SE (PCI HardSID Quattro, hardsid.dll)
 - Windows 98SE (PCI HardSID Quattro, Direct PCI I/O)
 - Windows ME (ISA HardSID, hardsid.dll)
 - Windows ME (ISA HardSID Quattro, hardsid.dll)
 - Windows ME (ISA HardSID, winio.dll ISA I/O)
 - Windows ME (ISA HardSID Quattro, winio.dll ISA I/O)
 - Windows ME (ISA HardSID, inpout32.dll ISA I/O)
 - Windows ME (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows ME (ISA HardSID, Direct ISA I/O)
 - Windows ME (ISA HardSID Quattro, Direct ISA I/O)
 - Windows ME (PCI HardSID, hardsid.dll)
 - Windows ME (PCI HardSID Quattro, hardsid.dll)
 - Windows ME (PCI HardSID, Direct PCI I/O)
 - Windows ME (PCI HardSID Quattro, Direct PCI I/O)
 - Windows NT 3.51 (ISA HardSID, inpout32.dll ISA I/O)
 - Windows NT 3.51 (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID, hardsid.dll)
 - Windows NT 4 (ISA HardSID Quattro, hardsid.dll)
 - Windows NT 4 (ISA HardSID, winio32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID, inpout32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows NT 4 (PCI HardSID, winio32.dll PCI I/O)
 - Windows NT 4 (PCI HardSID Quattro, winio32.dll PCI I/O)
 - Windows 2000 (ISA HardSID, hardsid.dll)
 - Windows 2000 (ISA HardSID Quattro, hardsid.dll)
 - Windows 2000 (ISA HardSID, winio32.dll ISA I/O)
 - Windows 2000 (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows 2000 (ISA HardSID, inpout32.dll ISA I/O)
 - Windows 2000 (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows 2000 (PCI HardSID, hardsid.dll)
 - Windows 2000 (PCI HardSID Quattro, hardsid.dll)
 - Windows 2000 (PCI HardSID, winio32.dll PCI I/O)
 - Windows 2000 (PCI HardSID Quattro, winio32.dll PCI I/O)
 - Windows XP (ISA HardSID, winio32.dll ISA I/O)
 - Windows XP (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows XP (ISA HardSID, inpout32.dll ISA I/O)
 - Windows XP (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows XP (PCI HardSID, winio32.dll PCI I/O)
 - Windows XP (PCI HardSID Quattro, winio32.dll PCI I/O)
 - Windows 2003 Server (ISA HardSID, winio32.dll ISA I/O)
 - Windows 2003 Server (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows 2003 Server (ISA HardSID, inpout32.dll ISA I/O)
 - Windows 2003 Server (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows 2003 Server (PCI HardSID, winio32.dll PCI I/O)
 - Windows 2003 Server (PCI HardSID Quattro, winio32.dll PCI I/O)
 */

#include "vice.h"

#ifdef WIN32_COMPILE

#ifdef HAVE_HARDSID

#include "hardsid.h"
#include "hs-win32.h"
#include "types.h"

static int use_hs_isa = 0;
static int use_hs_pci = 0;
static int use_hs_dll = 0;

void hardsid_drv_reset(void)
{
    if (use_hs_dll) {
        hs_dll_reset();
    }
}

int hardsid_drv_open(void)
{
    int retval;

    retval = hs_dll_open();
    if (!retval) {
        use_hs_dll = 1;
        return 0;
    }

    retval = hs_isa_open();
    if (!retval) {
        use_hs_isa = 1;
        return 0;
    }

    retval = hs_pci_open();
    if (!retval) {
        use_hs_pci = 1;
        return 0;
    }

    return -1;
}

int hardsid_drv_close(void)
{
    if (use_hs_pci) {
        use_hs_pci = 0;
        hs_pci_close();
    }
    if (use_hs_isa) {
        use_hs_isa = 0;
        hs_isa_close();
    }
    if (use_hs_dll) {
        use_hs_dll = 0;
        hs_dll_close();
    }
    return 0;
}

int hardsid_drv_read(uint16_t addr, int chipno)
{
    if (use_hs_pci) {
        return hs_pci_read(addr, chipno);
    }
    if (use_hs_isa) {
        return hs_isa_read(addr, chipno);
    }
    if (use_hs_dll) {
        return hs_dll_read(addr, chipno);
    }
    return 0;
}

void hardsid_drv_store(uint16_t addr, uint8_t val, int chipno)
{
    if (use_hs_pci) {
        hs_pci_store(addr, val, chipno);
    }
    if (use_hs_isa) {
        hs_isa_store(addr, val, chipno);
    }
    if (use_hs_dll) {
        hs_dll_store(addr, val, chipno);
    }
}

int hardsid_drv_available(void)
{
    if (use_hs_pci) {
        return hs_pci_available();
    }

    if (use_hs_isa) {
        return hs_isa_available();
    }

    if (use_hs_dll) {
        return hs_dll_available();
    }
    return 0;
}

void hardsid_drv_set_device(unsigned int chipno, unsigned int device)
{
    if (use_hs_dll) {
        hs_dll_set_device(chipno, device);
    }
}

/* ---------------------------------------------------------------------*/

void hardsid_drv_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    if (use_hs_pci) {
        hs_pci_state_read(chipno, sid_state);
    }

    if (use_hs_isa) {
        hs_isa_state_read(chipno, sid_state);
    }

    if (use_hs_dll) {
        hs_dll_state_read(chipno, sid_state);
    }
}

void hardsid_drv_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    if (use_hs_pci) {
        hs_pci_state_write(chipno, sid_state);
    }

    if (use_hs_isa) {
        hs_isa_state_write(chipno, sid_state);
    }

    if (use_hs_dll) {
        hs_dll_state_write(chipno, sid_state);
    }
}
#endif
#endif
