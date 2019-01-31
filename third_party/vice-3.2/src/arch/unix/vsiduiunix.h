/*
 * vsiduiunix.h - Implementation of the VSID-specific part of the UI.
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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

#ifndef VICE_VSIDUIUNIX_H
#define VICE_VSIDUIUNIX_H

#include "ui.h"

extern void vsid_ctrl_widget_set_parent(ui_window_t p);
extern ui_window_t build_vsid_ctrl_widget(void);
extern void shutdown_vsid_ctrl_widget(void);

extern void ui_vsid_setpsid(const char *psid);
extern void ui_vsid_settune(const int tune);
extern void ui_vsid_setdeftune(const int tune);
extern void ui_vsid_setnumtunes(const int count);
extern void ui_vsid_settime(const int sec);
extern void ui_vsid_setcopyright(const char *c);
extern void ui_vsid_setauthor(const char *a);
extern void ui_vsid_setsync(const char *a);
extern void ui_vsid_setmodel(const char *a);
extern void ui_vsid_setirq(const char *a);
extern void ui_vsid_setdrv(const char *info);

extern void ui_vsid_control_shutdown(void);

#endif
