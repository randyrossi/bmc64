/*
 * overlay.h
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

#ifndef VICE_OVERLAY_H
#define VICE_OVERLAY_H

#include <sys/types.h>

// Holds overlay graphics buffer
extern uint8_t *overlay_buf;

uint8_t *overlay_init(int width, int height, int padding, int c40_80_state);
void overlay_check(void);
void overlay_activate(void);
void overlay_warp_changed(int warp);
void overlay_joyswap_changed(int swap);
void overlay_dismiss(void);
void overlay_force_enabled(void);
void overlay_change_padding(int padding);
void overlay_40_80_columns_changed(int value);

#endif
