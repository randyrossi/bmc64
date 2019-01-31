/*
 * uipalcontrol.h - GTK only, UI controls for CRT emu
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
 *  groepaz <groepaz@gmx.net>
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
 
#ifndef UIPALCONTROL_H_
#define UIPALCONTROL_H_

#include "videoarch.h"

extern GtkWidget *build_pal_ctrl_widget(video_canvas_t *canvas, void *data);
extern void shutdown_pal_ctrl_widget(GtkWidget *f, void *ctrldata);
extern void ui_update_palctrl(void);
extern int palctrl_get_height(video_canvas_t *canvas);

#endif
