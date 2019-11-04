/*
 * emu_api.c - emulator specific API functions
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

#include "emux_api.h"

#include "circle.h"

int emux_machine_class = BMC64_MACHINE_CLASS_UNKNOWN;
int vic_showing;
int vdc_showing;
int vic_enabled;
int vdc_enabled;

// Ring buffer for key latch events
struct pending_emu_key_s pending_emu_key;

// Ring buffer for joy latch events
struct pending_emu_joy_s pending_emu_joy;

// queue a key for press/release for the main loop
void emux_key_interrupt(long key, int pressed) {
  circle_lock_acquire();
  int i = pending_emu_key.tail & 0xf;
  pending_emu_key.key[i] = key;
  pending_emu_key.pressed[i] = pressed;
  pending_emu_key.tail++;
  circle_lock_release();
}

// Same as above except can call while already holding the lock
void emux_key_interrupt_locked(long key, int pressed) {
  int i = pending_emu_key.tail & 0xf;
  pending_emu_key.key[i] = key;
  pending_emu_key.pressed[i] = pressed;
  pending_emu_key.tail++;
}

// Queue a joy latch change for the main loop
void emux_joy_interrupt(int type, int port, int device, int value) {
  circle_lock_acquire();
  int i = pending_emu_joy.tail & 0x7f;
  pending_emu_joy.type[i] = type;
  pending_emu_joy.port[i] = port;
  pending_emu_joy.device[i] = device;
  pending_emu_joy.value[i] = value;
  pending_emu_joy.tail++;
  circle_lock_release();
}
