/*
 * hardsid-unix-drv.c - Unix specific hardsid driver.
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

 - Linux 2.6 (/dev/port based ISA I/O, ISA HardSID)
 - Linux 2.6 (/dev/port based ISA I/O, ISA HardSID Quattro)
 - Linux 2.6 (permission based ISA I/O, ISA HardSID)
 - Linux 2.6 (permission based ISA I/O, ISA HardSID Quattro)
 - Linux 2.6 (/dev/port based PCI I/O, PCI HardSID)
 - Linux 2.6 (/dev/port based PCI I/O, PCI HardSID Quattro)
 - Linux 2.6 (permission based PCI I/O, PCI HardSID)
 - Linux 2.6 (permission based PCI I/O, PCI HardSID Quattro)
 - NetBSD (permission based ISA I/O, ISA HardSID)
 - NetBSD (permission based ISA I/O, ISA HardSID Quattro)
 - OpenBSD (permission based ISA I/O, ISA HardSID)
 - OpenBSD (permission based ISA I/O, ISA HardSID Quattro)
 - FreeBSD (/dev/io based ISA I/O, ISA HardSID)
 - FreeBSD (/dev/io based ISA I/O, ISA HardSID Quattro)
 */

#include "vice.h"

#ifdef UNIX_COMPILE

#ifdef HAVE_HARDSID

#include "hardsid.h"
#include "hs-unix.h"
#include "types.h"

#ifdef HAVE_HARDSID_IO
static int use_hs_isa = 0;
static int use_hs_pci = 0;
#endif

#ifdef HAVE_LINUX_HARDSID_H
static int use_hs_linux = 0;
#endif

void hardsid_drv_reset(void)
{
#ifdef HAVE_LINUX_HARDSID_H
    if (use_hs_linux) {
        hs_linux_reset();
    }
#endif
}

int hardsid_drv_open(void)
{
#ifdef HAVE_LINUX_HARDSID_H
    if (!hs_linux_open()) {
        use_hs_linux = 1;
        return 0;
    }
#endif
#ifdef HAVE_HARDSID_IO
    if (!hs_isa_open()) {
        use_hs_isa = 1;
        return 0;
    }
    if (!hs_pci_open()) {
        use_hs_pci = 1;
        return 0;
    }
#endif
    return -1;
}

int hardsid_drv_close(void)
{
#ifdef HAVE_HARDSID_IO
    if (use_hs_isa) {
        use_hs_isa = 0;
        hs_isa_close();
    }
    if (use_hs_pci) {
        use_hs_pci = 0;
        hs_pci_close();
    }
#endif
#ifdef HAVE_LINUX_HARDSID_H
    if (use_hs_linux) {
        use_hs_linux = 0;
        hs_linux_close();
    }
#endif
    return 0;
}

int hardsid_drv_read(uint16_t addr, int chipno)
{
#ifdef HAVE_HARDSID_IO
    if (use_hs_isa) {
        return hs_isa_read(addr, chipno);
    }
    if (use_hs_pci) {
        return hs_pci_read(addr, chipno);
    }
#endif
#ifdef HAVE_LINUX_HARDSID_H
    if (use_hs_linux) {
        return hs_linux_read(addr, chipno);
    }
#endif
    return 0;
}

void hardsid_drv_store(uint16_t addr, uint8_t val, int chipno)
{
#ifdef HAVE_HARDSID_IO
    if (use_hs_isa) {
        hs_isa_store(addr, val, chipno);
    }
    if (use_hs_pci) {
        hs_pci_store(addr, val, chipno);
    }
#endif
#ifdef HAVE_LINUX_HARDSID_H
    if (use_hs_linux) {
        hs_linux_store(addr, val, chipno);
    }
#endif
}

int hardsid_drv_available(void)
{
#ifdef HAVE_LINUX_HARDSID_H
    if (use_hs_linux) {
        return hs_linux_available();
    }
#endif
#ifdef HAVE_HARDSID_IO
    if (use_hs_isa) {
        return hs_isa_available();
    }
    if (use_hs_pci) {
        return hs_pci_available();
    }
#endif
    return 0;
}

void hardsid_drv_set_device(unsigned int chipno, unsigned int device)
{
}

/* ---------------------------------------------------------------------*/

void hardsid_drv_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
#ifdef HAVE_HARDSID_IO
    if (use_hs_isa) {
        hs_isa_state_read(chipno, sid_state);
    }
    if (use_hs_pci) {
        hs_pci_state_read(chipno, sid_state);
    }
#endif
#ifdef HAVE_LINUX_HARDSID_H
    if (use_hs_linux) {
        hs_linux_state_read(chipno, sid_state);
    }
#endif
}

void hardsid_drv_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
#ifdef HAVE_HARDSID_IO
    if (use_hs_isa) {
        hs_isa_state_write(chipno, sid_state);
    }
    if (use_hs_pci) {
        hs_pci_state_write(chipno, sid_state);
    }
#endif
#ifdef HAVE_LINUX_HARDSID_H
    if (use_hs_linux) {
        hs_linux_state_write(chipno, sid_state);
    }
#endif
}
#endif
#endif
