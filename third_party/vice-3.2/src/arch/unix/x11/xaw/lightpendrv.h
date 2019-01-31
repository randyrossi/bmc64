/*
 * lightpendrv.h - Xaw
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
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
#ifndef VICE_UI_XAW_LIGHTPENDRV_H
#define VICE_UI_XAW_LIGHTPENDRV_H

extern void xaw_init_lightpen(Display *disp);
extern void xaw_lightpen_setbutton(int b, int s);
extern void xaw_lightpen_update_xy(int x, int y);
extern void x11_lightpen_update(void);
extern void xaw_lightpen_update_canvas(struct video_canvas_s *p, int enter);

#endif /* VICE_UI_XAW_LIGHTPENDRV_H */
