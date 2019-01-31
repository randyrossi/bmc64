/*
 * io-access.h - I/O access prototypes.
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

#ifndef VICE_IO_ACCESS_H
#define VICE_IO_ACCESS_H

#include "types.h"

extern int io_access_init(void);
extern void io_access_shutdown(void);

# ifdef UNIX_COMPILE
extern void io_access_store(uint16_t addr, uint8_t value);
extern uint8_t io_access_read(uint16_t addr);
int io_access_map(uint16_t addr, uint16_t space);
void io_access_unmap(uint16_t addr, uint16_t space);
# else
extern void io_access_store_byte(uint16_t addr, uint8_t value);
extern uint8_t io_access_read_byte(uint16_t addr);
extern void io_access_store_long(uint16_t addr, uint32_t value);
extern uint32_t io_access_read_long(uint16_t addr);
# endif

#endif
