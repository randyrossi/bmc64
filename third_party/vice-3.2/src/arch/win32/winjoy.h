/*
 * winjoy.h - Joystick support for Windows.
 *
 * Written by
 *  Spiro Trikaliotis
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

#ifndef VICE_WINJOY_H
#define VICE_WINJOY_H

#include <windows.h>

/*  These are in joystick.c . */
extern void joystick_calibrate(HWND hwnd);
extern void joystick_ui_get_device_list(HWND joy_hwnd);
extern void joystick_ui_get_autofire_axes(HWND joy_hwnd, int device);
extern void joystick_ui_get_autofire_buttons(HWND joy_hwnd, int device);

#endif
