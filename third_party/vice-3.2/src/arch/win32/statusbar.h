/*
 * statusbar.h - Status bar code.
 *
 * Written by
 *  Tibor Biczo <crown@mtavnet.hu>
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

#ifndef VICE_STATUSBAR_H_
#define VICE_STATUSBAR_H_

#define EVENT_OFF       0
#define EVENT_RECORDING 1
#define EVENT_PLAYBACK  2

#define MAX_COLOR 255

#define COLOR_SHADE_RED   0x01
#define COLOR_SHADE_GREEN 0x100


#include "uiapi.h"

extern void statusbar_create(HWND hwnd);
extern void statusbar_destroy(HWND hwnd);
extern void statusbar_create_brushes(void);
extern void statusbar_delete_brushes(void);
extern int statusbar_get_status_height(void);
extern void statusbar_setstatustext(const char *text);
extern void statusbar_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color);
extern void statusbar_display_drive_track(int drivenum, int drive_base, double track_number);
extern void statusbar_display_drive_led(int drivenum, int status);
extern void statusbar_set_tape_status(int tape_status);
extern void statusbar_display_tape_motor_status(int motor);
extern void statusbar_display_tape_control_status(int control);
extern void statusbar_display_tape_counter(int counter);
extern void statusbar_display_joyport(BYTE *ui_joystick_status);
extern void statusbar_event_status(int mode);
extern void statusbar_event_time(unsigned int current, unsigned int total);
extern void statusbar_handle_WMSIZE(UINT msg, WPARAM wparam, LPARAM lparam, int window_index);
extern void statusbar_handle_WMDRAWITEM(WPARAM wparam, LPARAM lparam);
extern void statusbar_notify(HWND window, int window_index, WPARAM wparam, LPARAM lparam);
extern void statusbar_display_volume(int vol);
#endif
