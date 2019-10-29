/*
 * mousedrv.c
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

#include "mousedrv.h"

#include <stdio.h>

// RASPI includes
#include "videoarch.h"

static int mouse_x, mouse_y;
static unsigned long mouse_timestamp = 0;
static mouse_func_t mouse_funcs;

int mousedrv_resources_init(mouse_func_t *funcs) {
  mouse_funcs.mbl = funcs->mbl;
  mouse_funcs.mbr = funcs->mbr;
  mouse_funcs.mbm = funcs->mbm;
  mouse_funcs.mbu = funcs->mbu;
  mouse_funcs.mbd = funcs->mbd;

  return 0;
}

int mousedrv_cmdline_options_init(void) { return 0; }

void mousedrv_init(void) {}

void mousedrv_mouse_changed(void) {}

int mousedrv_get_x(void) { return mouse_x; }

int mousedrv_get_y(void) { return mouse_y; }

unsigned long mousedrv_get_timestamp(void) { return mouse_timestamp; }

void emu_mouse_move(int x, int y) {
  mouse_x += x;
  mouse_y -= y;
  mouse_timestamp = vsyncarch_gettime();
}

void emu_mouse_button_left(int pressed) {
  mouse_funcs.mbl(pressed);
}

void emu_mouse_button_right(int pressed) {
  mouse_funcs.mbr(pressed);
}

void emu_mouse_button_middle(int pressed) {
  mouse_funcs.mbm(pressed);
}

void emu_mouse_wheel_up(int pressed) {
  mouse_funcs.mbu(pressed);
}

void emu_mouse_wheel_down(int pressed) {
  mouse_funcs.mbd(pressed);
}
