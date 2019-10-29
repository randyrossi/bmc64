/*
 * kbd.h
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

#ifndef RASPI_KBD_H
#define RASPI_KBD_H

#include "circle.h"
#include "keycodes.h"

struct key_combo_state_s {
  long second_key; // the 2nd key to press to invoke this combo
  int invoked;     // set when the combo is completed
  int function;    // what to do
};

typedef struct key_combo_state_s key_combo_state_t;

extern void kbd_arch_init(void);
extern int kbd_arch_get_host_mapping(void);
extern signed long kbd_arch_keyname_to_keynum(char *keyname);
extern const char *kbd_arch_keynum_to_keyname(signed long keynum);
extern void kbd_initialize_numpad_joykeys(int *joykeys);

void kbd_set_hotkey_function(unsigned int slot, long key, int function);

#define KBD_PORT_PREFIX "rpi"

#endif
