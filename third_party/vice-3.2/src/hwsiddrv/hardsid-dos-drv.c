/*
 * hardsid-dos-drv.c - MSDOS specific PCI/ISA hardsid wrapper.
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

 - MSDOS 6.00 (ISA HardSID)
 - MSDOS 6.00 (ISA HardSID Quattro)
 - MSDOS 6.00 (PCI HardSID)
 - MSDOS 6.00 (PCI HardSID Quattro)
 - Windows 95C (ISA HardSID)
 - Windows 95C (ISA HardSID Quattro)
 - Windows 95C (PCI HardSID)
 - Windows 95C (PCI HardSID Quattro)
 - Windows 98SE (ISA HardSID)
 - Windows 98SE (ISA HardSID Quattro)
 - Windows ME (ISA HardSID)
 - Windows ME (ISA HardSID Quattro)
 */

#include "vice.h"

#ifdef __MSDOS__

#ifdef HAVE_HARDSID

#include <dos.h>
#include <stdio.h>
#include <string.h>

#include "hs-dos.h"
#include "log.h"
#include "sid-snapshot.h"
#include "types.h"

static int use_pci = 0;
static int use_isa = 0;

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

int hardsid_drv_open(void)
{
    int rc;

    if (is_windows_nt()) {
        return -1;
    }

    rc = hs_pci_open();
    if (rc != -1) {
        use_pci = 1;
        return 0;
    }

    rc = hs_isa_open();
    if (rc != -1) {
        use_isa = 1;
        return 0;
    }

    return -1;
}

int hardsid_drv_close(void)
{
    if (use_pci) {
        use_pci = 0;
        hs_pci_close();
    }

    if (use_isa) {
        use_isa = 0;
        hs_isa_close();
    }

    return 0;
}

/* read value from SIDs */
int hardsid_drv_read(uint16_t addr, int chipno)
{
    if (use_pci) {
        return hs_pci_read(addr, chipno);
    }

    if (use_isa) {
        return hs_isa_read(addr, chipno);
    }

    return 0;
}

/* write value into SID */
void hardsid_drv_store(uint16_t addr, uint8_t val, int chipno)
{
    if (use_pci) {
        hs_pci_store(addr, val, chipno);
    }

    if (use_isa) {
        hs_isa_store(addr, val, chipno);
    }
}

void hardsid_drv_set_device(unsigned int chipno, unsigned int device)
{
}

int hardsid_drv_available(void)
{
    if (use_pci) {
        return hs_pci_available();
    }

    if (use_isa) {
        return hs_isa_available();
    }
    return 0;
}

void hardsid_drv_reset(void)
{
}

/* ---------------------------------------------------------------------*/

void hardsid_drv_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    sid_state->hsid_main_clk = 0;
    sid_state->hsid_alarm_clk = 0;
    sid_state->lastaccess_clk = 0;
    sid_state->lastaccess_ms = 0;
    sid_state->lastaccess_chipno = 0;
    sid_state->chipused = 0;
    sid_state->device_map[0] = 0;
    sid_state->device_map[1] = 0;
    sid_state->device_map[2] = 0;
    sid_state->device_map[3] = 0;
}

void hardsid_drv_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
}
#endif
#endif
