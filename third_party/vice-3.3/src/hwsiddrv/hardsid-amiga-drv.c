/*
 * hardsid-amiga-drv.c - Amiga specific hardsid driver.
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

#include "vice.h"

#ifdef AMIGA_SUPPORT

#ifdef HAVE_HARDSID

#ifdef AMIGA_M68K
static int hs_use_gg2_isa = 0;
#endif

#ifdef HAVE_PROTO_OPENPCI_H
static int hs_use_openpci = 0;
#endif

#ifdef AMIGA_OS4
static int hs_use_os4 = 0;
#endif

#include "hardsid.h"
#include "hs-amiga.h"

#if !defined(USE_SDLUI) && !defined(USE_SDLUI2)
#include "loadlibs.h"
#else
#include "archdep.h"
#endif

#include "types.h"

/* read value from SIDs */
int hardsid_drv_read(uint16_t addr, int chipno)
{
#ifdef AMIGA_M68K
    if (hs_use_gg2_isa) {
        return hs_gg2_isa_read(addr, chipno);
    }
#endif

#ifdef HAVE_PROTO_OPENPCI_H
    if (hs_use_openpci) {
        return hs_openpci_read(addr, chipno);
    }
#endif

#ifdef AMIGA_OS4
    if (hs_use_os4) {
        return hs_os4_read(addr, chipno);
    }
#endif

    return 0;
}

/* write value into SID */
void hardsid_drv_store(uint16_t addr, uint8_t val, int chipno)
{
#ifdef AMIGA_M68K
    if (hs_use_gg2_isa) {
        hs_gg2_isa_store(addr, val, chipno);
    }
#endif

#ifdef HAVE_PROTO_OPENPCI_H
    if (hs_use_openpci) {
        hs_openpci_store(addr, val, chipno);
    }
#endif

#ifdef AMIGA_OS4
    if (hs_use_os4) {
        hs_os4_store(addr, val, chipno);
    }
#endif
}

int hardsid_drv_open(void)
{
    int rc;

#ifdef AMIGA_M68K
    rc = hs_gg2_isa_open();
    if (!rc) {
        hs_use_gg2_isa = 1;
        return 0;
    }
#endif

#if defined(HAVE_PROTO_OPENPCI_H) || defined(AMIGA_OS4)
    if (!pci_lib_loaded) {
        return -1;
    }
#endif

#ifdef HAVE_PROTO_OPENPCI_H
    rc = hs_openpci_open();
    if (!rc) {
        hs_use_openpci = 1;
        return 0;
    }
#endif

#ifdef AMIGA_OS4
    rc = hs_os4_open();
    if (!rc) {
        hs_use_os4 = 1;
        return 0;
    }
#endif

    return -1;
}

int hardsid_drv_close(void)
{
#ifdef HAVE_PROTO_OPENPCI_H
    if (hs_use_openpci) {
        hs_openpci_close();
        hs_use_openpci = 0;
    }
#endif

#ifdef AMIGA_OS4
    if (hs_use_os4) {
        hs_os4_close();
        hs_use_os4 = 0;
    }
#endif

#ifdef AMIGA_M68K
    if (hs_use_gg2_isa) {
        hs_gg2_isa_close();
        hs_use_gg2_isa = 0;
    }
#endif

    return 0;
}

int hardsid_drv_available(void)
{
#ifdef AMIGA_M68K
    if (hs_use_gg2_isa) {
        return hs_gg2_isa_available();
    }
#endif

#ifdef HAVE_PROTO_OPENPCI_H
    if (hs_use_openpci) {
        return hs_openpci_available();
    }
#endif

#ifdef AMIGA_OS4
    if (hs_use_os4) {
        return hs_os4_available();
    }
#endif
    return 0;
}

void hardsid_drv_set_machine_parameter(long cycles_per_sec)
{
}

void hardsid_drv_reset(void)
{
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
