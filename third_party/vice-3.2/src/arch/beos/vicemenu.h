/*
 * vicemenu.h - Interface of the BeVICE's menubar
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifndef VICE_VICEMENU_H
#define VICE_VICEMENU_H

class BMenuBar;

extern BMenuBar *menu_create(int machine_class, int window_nr);

extern "C" {

#include "cartridge.h"
#include "joyport.h"

extern void vicemenu_set_joyport_func(joyport_desc_t *(*gd)(int port), char *(*gn)(int port), int port1, int port2, int port3, int port4, int port5);
extern void vicemenu_set_cart_func(cartridge_info_t *(*cgil)(void));
extern void vicemenu_free_tune_menu(void);
extern void vicemenu_tune_menu_add(int tune);
}

#endif
