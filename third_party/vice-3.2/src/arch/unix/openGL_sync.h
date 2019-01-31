/*
 * openGL_sync.h
 *
 * Written by
 * pottendo <pottendo@gmx.net>
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

#ifndef VICE__openGL_sync_h__
#define VICE__openGL_sync_h__

#include "vice.h"
#include "videoarch.h"

struct ui_menu_entry_s;

extern void openGL_sync_init(video_canvas_t *c);
extern void openGL_sync_with_raster(void);
extern void openGL_sync_shutdown(void);
extern int openGL_available(int val);
extern void init_openGL(void);
extern void openGL_register_resources(void);
extern int openGL_sync_enabled(void);
extern void openGL_set_canvas_refreshrate(float rr);
extern float openGL_get_canvas_refreshrate(void);

#endif /* __openGL_sync_h__ */
