/*
 * vidmode.h
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_VIDMODE_H
#define VICE_VIDMODE_H

struct video_canvas_s;
struct ui_menu_entry_s;

struct vm_bestvideomode_s {
    int modeindex;
    char *name;
};
typedef struct vm_bestvideomode_s vm_bestvideomode_t;

extern vm_bestvideomode_t *vm_bestmodes;

extern int vm_is_enabled;
extern int vm_mode_count;

extern int vidmode_init(void);
extern int vidmode_available(void);
extern unsigned int vidmode_available_modes(void);
extern int vidmode_enable(struct video_canvas_s *canvas, int enable);
extern int vidmode_mode(struct video_canvas_s *canvas, int mode);
extern void vidmode_shutdown(void);
extern void vidmode_suspend(int level);
extern void vidmode_resume(void);
extern void vidmode_set_mouse_timeout(void);
extern void vidmode_mode_callback(void *callback);
extern void vidmode_menu_create(struct ui_menu_entry_s *menu);
extern void vidmode_menu_shutdown(struct ui_menu_entry_s *menu);
extern void vidmode_mouse_moved(struct video_canvas_s *canvas, int x, int y, int leave);
extern void vidmode_resize(struct video_canvas_s *canvas, int uienable);

#endif
