/*
 * emu_api.h - emulator specific API functions
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

// API between common code and emulator specific impl

#ifndef RASPI_EMUX_API_H_
#define RASPI_EMUX_API_H_

#include <stdint.h>

// Pause emulator main loop and run our ui loop. 
void emux_trap_main_loop_ui(void);

void emux_trap_main_loop(void (*trap_func)(uint16_t, void *data), void* data);

// Press/release key by row/col in keyboard matrix.
void emux_kbd_set_latch_keyarr(int row, int col, int value);

// Attach a disk image to a drive
// Return negative on error.
int emux_attach_disk_image(int unit, char *filename);

// Detach a disk drive image or mounted file system.
void emux_detach_disk(int unit);

// Attach a cart image
// Return negative on error.
int emux_attach_cart(int bank, char *filename);

// Detach a cart image
void emux_detach_cart(int bank);

// Reset machine
void emux_reset(int isSoft);

// Save state of machine
int emux_save_state(char *filename);

// Restore state of machine
int emux_load_state(char *filename);

#endif
