/*
 * hs-unix.h - Linux hardsid specific prototypes.
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

#ifndef VICE_HS_UNIX_H
#define VICE_HS_UNIX_H

#include "sid-snapshot.h"
#include "types.h"

#define HS_MAXSID 2

extern int hs_linux_open(void);
extern int hs_isa_open(void);
extern int hs_pci_open(void);

extern int hs_linux_close(void);
extern int hs_isa_close(void);
extern int hs_pci_close(void);

extern void hs_linux_reset(void);

extern int hs_linux_read(uint16_t addr, int chipno);
extern int hs_isa_read(uint16_t addr, int chipno);
extern int hs_pci_read(uint16_t addr, int chipno);

extern void hs_linux_store(uint16_t addr, uint8_t val, int chipno);
extern void hs_isa_store(uint16_t addr, uint8_t val, int chipno);
extern void hs_pci_store(uint16_t addr, uint8_t val, int chipno);

extern int hs_linux_available(void);
extern int hs_isa_available(void);
extern int hs_pci_available(void);

extern void hs_linux_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state);
extern void hs_isa_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state);
extern void hs_pci_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state);

extern void hs_linux_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state);
extern void hs_isa_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state);
extern void hs_pci_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state);

#endif
