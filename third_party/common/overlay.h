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

#ifndef RASPI_OVERLAY_H
#define RASPI_OVERLAY_H

#include <sys/types.h>

#include "circle.h"

#define OVERLAY_WIDTH 896
#define OVERLAY_HEIGHT 240

// Holds overlay graphics buffer
extern uint8_t *overlay_buf;

extern int vkbd_enabled;
extern int vkbd_showing;
extern int vkbd_press[JOYDEV_NUM_JOYDEVS];
extern int vkbd_up[JOYDEV_NUM_JOYDEVS];
extern int vkbd_down[JOYDEV_NUM_JOYDEVS];
extern int vkbd_left[JOYDEV_NUM_JOYDEVS];
extern int vkbd_right[JOYDEV_NUM_JOYDEVS];

extern int overlay_dirty;

extern int statusbar_enabled;
extern int statusbar_showing;

uint8_t *overlay_init(int padding, int c40_80_state, int vkbd_transparency);
void overlay_check(void);
void overlay_activate(void);
void overlay_warp_changed(int warp);
void overlay_joyswap_changed(int swap);
void overlay_statusbar_dismiss(void);
void overlay_statusbar_enable(void);
void overlay_statusbar_disable(void);
void overlay_change_padding(int padding);
void overlay_change_vkbd_transparency(int transparency);
void overlay_40_80_columns_changed(int value);
void vkbd_nav_up(void);
void vkbd_nav_down(void);
void vkbd_nav_left(void);
void vkbd_nav_right(void);
void vkbd_enable(void);
void vkbd_disable(void);
void vkbd_nav_press(int pressed, int device);
void vkbd_sync_event(long key, int pressed);

#endif
