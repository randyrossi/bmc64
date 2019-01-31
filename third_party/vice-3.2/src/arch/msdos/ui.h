/*
 * ui.h - A (very) simple user interface for MS-DOS.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_UI_DOS_H
#define VICE_UI_DOS_H

#include "types.h"
#include "uiapi.h"


extern void ui_show_text(const char *title, const char *text);
extern void ui_main(char hotkey);
extern void ui_set_warp_status(int status);
extern void ui_dispatch_events(void);
extern void ui_display_speed(float percent, float framerate, int warp_flag);
extern void ui_message(const char *format,...);

extern void ui_pause_emulation(int flag);
extern int ui_emulation_is_paused(void);

#endif
