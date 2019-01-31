/*
 * cbmcharsets.c - CBM lookalike character sets.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_CBMCHARSETS_H
#define VICE_CBMCHARSETS_H

#include "types.h"

extern uint8_t cbm_charset_1[];
extern uint8_t cbm_charset_2[];

extern uint8_t cbm_petscii_business_to_charset[0x100];
extern uint8_t cbm_petscii_graphics_to_charset[0x100];

/* FIXME: These are currently unused.  */
extern uint8_t cbm_ascii_to_petscii[0x100];
extern uint8_t cbm_petscii_business_to_ascii[0x100];
extern uint8_t cbm_petscii_graphics_to_ascii[0x100];

#endif
