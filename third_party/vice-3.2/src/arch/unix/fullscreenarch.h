/*
 * fullscreenarch.h
 *
 * Written by
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

#ifndef VICE_FULLSCREENARCH_H
#define VICE_FULLSCREENARCH_H

struct video_canvas_s;

#define FSDRIVER_XRANDR   1
#define FSDRIVER_VIDMODE  2

struct fullscreenconfig_s {
    int enable;
    char *device;
    int double_size;
    int double_scan;
    int mode;
    int driver;
    /* extra space taken up by UI elements */
    unsigned int ui_border_top;
    unsigned int ui_border_bottom;
};
typedef struct fullscreenconfig_s fullscreenconfig_t;

struct ui_menu_entry_s;

extern int fullscreen_is_enabled; 
extern int fullscreen_available(void);

extern void fullscreen_shutdown(void);
extern void fullscreen_suspend(int level);
extern void fullscreen_resume(void);
extern void fullscreen_set_mouse_timeout(void);
extern void fullscreen_mode_callback(const char *device, void *callback);
extern void fullscreen_menu_create(struct ui_menu_entry_s *menu);
extern void fullscreen_menu_shutdown(struct ui_menu_entry_s *menu);
extern int fullscreen_init(void);
extern int fullscreen_init_alloc_hooks(struct video_canvas_s *canvas);
extern void fullscreen_shutdown_alloc_hooks(struct video_canvas_s *canvas);
extern void fullscreen_mouse_moved(struct video_canvas_s *canvas, int x, int y, int leave);
extern void fullscreen_resize(struct video_canvas_s *canvas, int uienable);

#endif
