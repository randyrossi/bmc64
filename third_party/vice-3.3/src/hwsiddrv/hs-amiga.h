/*
 * hs-amiga.h
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

#ifndef VICE_HS_AMIGA_H
#define VICE_HS_AMIGA_H

#include "types.h"
#include "hardsid.h"

extern int hs_openpci_open(void);
extern int hs_os4_open(void);
extern int hs_gg2_isa_open(void);

extern int hs_openpci_close(void);
extern int hs_os4_close(void);
extern int hs_gg2_isa_close(void);

extern int hs_openpci_read(uint16_t addr, int chipno);
extern int hs_os4_read(uint16_t addr, int chipno);
extern int hs_gg2_isa_read(uint16_t addr, int chipno);

extern void hs_openpci_store(uint16_t addr, uint8_t val, int chipno);
extern void hs_os4_store(uint16_t addr, uint8_t val, int chipno);
extern void hs_gg2_isa_store(uint16_t addr, uint8_t val, int chipno);

extern void hs_device_set_machine_parameter(long cycles_per_sec);

extern int hs_gg2_isa_available(void);
extern int hs_openpci_available(void);
extern int hs_os4_available(void);

#endif
