/*
 * menu_keyset.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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

#ifndef RASPI_MENU_KEYSET_H
#define RASPI_MENU_KEYSET_H

#include "ui.h"

// Indices into keyset_codes array in menu.c
#define KEYSET_UP 0
#define KEYSET_DOWN 1
#define KEYSET_LEFT 2
#define KEYSET_RIGHT 3
#define KEYSET_FIRE 4
#define KEYSET_POTX 5
#define KEYSET_POTY 6

void build_keyset_menu(int num, struct menu_item *parent);

#endif
