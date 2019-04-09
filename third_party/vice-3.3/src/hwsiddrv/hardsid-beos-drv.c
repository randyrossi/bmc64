/*
 * hardsid-beos-drv.c - BeOS specific hardsid driver.
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

 - BeOS Max V4b1 x86 (ISA HardSID)
 - BeOS Max V4b1 x86 (ISA HardSID Quattro)
 - BeOS Max V4b1 x86 (PCI HardSID)
 - BeOS Max V4b1 x86 (PCI HardSID Quattro)
 - Zeta 1.21 (ISA HardSID)
 - Zeta 1.21 (ISA HardSID Quattro)
 - Zeta 1.21 (PCI HardSID)
 - Zeta 1.21 (PCI HardSID Quattro)
 */

#include "vice.h"

#ifdef BEOS_COMPILE

#ifdef HAVE_HARDSID

#include "hardsid.h"
#include "hs-beos.h"
#include "types.h"

static int use_hs_isa = 0;
static int use_hs_pci = 0;

void hardsid_drv_reset(void)
{
}

int hardsid_drv_open(void)
{
    if (!hs_isa_open()) {
        use_hs_isa = 1;
        return 0;
    }
    if (!hs_pci_open()) {
        use_hs_pci = 1;
        return 0;
    }
    return -1;
}

int hardsid_drv_close(void)
{
    if (use_hs_isa) {
        use_hs_isa = 0;
        hs_isa_close();
    }

    if (use_hs_pci) {
        use_hs_pci = 0;
        hs_pci_close();
    }

    return 0;
}

int hardsid_drv_read(uint16_t addr, int chipno)
{
    if (use_hs_isa) {
        return hs_isa_read(addr, chipno);
    }

    if (use_hs_pci) {
        return hs_pci_read(addr, chipno);
    }

    return 0;
}

void hardsid_drv_store(uint16_t addr, uint8_t val, int chipno)
{
    if (use_hs_isa) {
        hs_isa_store(addr, val, chipno);
    }

    if (use_hs_pci) {
        hs_pci_store(addr, val, chipno);
    }
}

int hardsid_drv_available(void)
{
    if (use_hs_isa) {
        return hs_isa_available();
    }

    if (use_hs_pci) {
        return hs_pci_available();
    }

    return 0;
}

void hardsid_drv_set_device(unsigned int chipno, unsigned int device)
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
