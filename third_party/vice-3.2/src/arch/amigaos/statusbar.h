/*
 * statusbar.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#define REFRESH_ALL (-1)

extern int statusbar_create(video_canvas_t *canvas);
extern void statusbar_destroy(video_canvas_t *canvas);
extern int statusbar_get_status_height(void);
extern void statusbar_refresh(int drive_number);
extern void statusbar_set_statustext(const char *text, int text_time);
extern void statusbar_statustext_update(void);

#endif /* _STATUSBAR_H_ */
