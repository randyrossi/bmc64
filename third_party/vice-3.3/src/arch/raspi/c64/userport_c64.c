/*
 * userport_c64.c
 *
 * Written by
 *  Mark Bush <mark@bushnet.org>
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

#include "userport_c64.h"

#include "cia.h"
#include "c64/c64.h"

uint8_t userport_get_ddr(void) {
  return machine_context.cia2->c_cia[CIA_DDRB];
}
uint8_t userport_get(void) {
  return machine_context.cia2->c_cia[CIA_PRB];
}
void userport_set(uint8_t value) {
  machine_context.cia2->c_cia[CIA_PRB] = value;
}
