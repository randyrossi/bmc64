/*
 * hs-beos.h - BeOS hardsid specific prototypes.
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

#ifndef VICE_HS_BEOS_H
#define VICE_HS_BEOS_H

#include "types.h"

extern int hs_isa_open(void);
extern int hs_pci_open(void);

extern int hs_isa_close(void);
extern int hs_pci_close(void);

extern uint8_t hs_isa_read(uint16_t addr, int chipno);
extern uint8_t hs_pci_read(uint16_t addr, int chipno);

extern void hs_isa_store(uint16_t addr, uint8_t val, int chipno);
extern void hs_pci_store(uint16_t addr, uint8_t val, int chipno);

extern int hs_isa_available(void);
extern int hs_pci_available(void);

#endif
