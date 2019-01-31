/*
 * vidmode.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>

#include "lib.h"
#include "log.h"
#include "fullscreenarch.h"
#include "resources.h"
#include "types.h"
#include "uimenu.h"
#include "uisettings.h"
#include "videoarch.h"
#include "vidmode.h"
#include "x11ui.h"

int vm_is_enabled = 0;
int vm_is_suspended = 0;

static log_t vidmode_log = LOG_ERR;
int vm_mode_count;
static unsigned int vm_index = 0;
static int vm_available = 0;
#ifdef HAVE_FULLSCREEN
static int saved_h, saved_w;
#endif
static int vidmode_selected_mode = 0;

XF86VidModeModeInfo **vm_modes;
vm_bestvideomode_t *vm_bestmodes = NULL;

extern int screen;

int vidmode_init(void)
{
    unsigned int hz;
    int i;
    Display *display;

    vidmode_log = log_open("VidMode");

    display = x11ui_get_display_ptr();

    if (!XF86VidModeGetAllModeLines(display, screen, &vm_mode_count, &vm_modes)) {
        log_error(vidmode_log, "Error getting video mode information - disabling vidmode extension.");
        vm_available = 0;
        return 0;
    }

    for (i = 0; i < vm_mode_count; i++) {
        if (vm_modes[i]->hdisplay <= 800 && vm_modes[i]->hdisplay >= 320 && vm_modes[i]->vdisplay <= 600 && vm_modes[i]->vdisplay >= 200) {
            vm_bestmodes = (vm_bestvideomode_t *)lib_realloc(vm_bestmodes, (vm_index + 1) * sizeof(vm_bestvideomode_t));
            vm_bestmodes[vm_index].modeindex = i;

            if (vm_modes[i]->vtotal * vm_modes[i]->htotal) {
                hz = vm_modes[i]->dotclock * 1000 / (vm_modes[i]->vtotal * vm_modes[i]->htotal);
            } else {
                hz = 0;
            }

            vm_bestmodes[vm_index].name = lib_msprintf(" %ix%i-%iHz", vm_modes[i]->hdisplay, vm_modes[i]->vdisplay, hz);
            if (++vm_index > 29) {
                break;
            }
        }
    }

    if (vm_index == 0) {
        return 0;
    }

    vm_available = 1;
    return 0;
}

unsigned int vidmode_available_modes(void)
{
    return vm_index;
}

#if 0
static float get_aspect(video_canvas_t *canvas)
{
    int keep_aspect_ratio, true_aspect_ratio;
    resources_get_int("KeepAspectRatio", &keep_aspect_ratio);
    if (keep_aspect_ratio) {
        resources_get_int("TrueAspectRatio", &true_aspect_ratio);
        if (true_aspect_ratio) {
#ifdef HAVE_HWSCALE
            if (canvas->videoconfig->hwscale) {
                return canvas->geometry->pixel_aspect_ratio;
            }
#endif
        }
    }
    return 1.0f;
}
#endif

/* FIXME: Limiting vidmode to one active fullscreen window.  */
static video_canvas_t *active_canvas;
static int x = 0, y = 0;
static int xoffs, yoffs;
static XF86VidModeModeInfo *vm;

static void vidmode_center_canvas(struct video_canvas_s *canvas)
{
    Display *vm_display;

    if (vm_available == 0) {
        return;
    }
    if (active_canvas != canvas) {
        return;
    }

    vm_display = x11ui_get_display_ptr();

    x11ui_move_canvas_window(canvas->emuwindow, 0, 0);
    ui_dispatch_events();
    x11ui_canvas_position(canvas->emuwindow, &x, &y);
    XF86VidModeSetViewPort(vm_display, screen, x + xoffs, y + yoffs);
    ui_dispatch_events();
}

static void vidmode_resize_canvas(struct video_canvas_s *canvas, int uienable)
{
    int status_h = 0;
    int fs_h, fs_w;

    if (vm_available == 0) {
        return;
    }

#ifdef HAVE_FULLSCREEN
    if (uienable) {
        status_h = canvas->fullscreenconfig->ui_border_top + canvas->fullscreenconfig->ui_border_bottom;
    }
#endif

    /* fs_w = ((float)vm->hdisplay * get_aspect(canvas)); */
    fs_h = vm->vdisplay;
    fs_w = vm->hdisplay;

    xoffs = 0;
    fs_w /= canvas->videoconfig->scalex;

    yoffs = 0;
    fs_h -= status_h;
    fs_h /= canvas->videoconfig->scaley;

    canvas->draw_buffer->canvas_width = fs_w;
    canvas->draw_buffer->canvas_height = fs_h;
    video_viewport_resize(canvas, 1);
    ui_dispatch_events();
}

static int delayed_center = 0;

void vidmode_mouse_moved(struct video_canvas_s *canvas, int x, int y, int leave)
{
    static int lastx, lasty;
    int winx, winy;
    int menu_h = 0;
    int status_h = 0;
    int wrap;

    Display *vm_display;
    Window shellwin;

    if (vm_available == 0) {
        return;
    }

    if (canvas->emuwindow == NULL) {
        return;
    }

    vm_display = x11ui_get_display_ptr();
    if (vm_display == NULL) {
        return;
    }

    if (leave) {
        shellwin = x11ui_get_X11_window();
        if (shellwin == 0)  {
            return;
        }
    }

    if (leave == 1) { /* pointer left canvas */
#ifdef HAVE_FULLSCREEN
        menu_h = canvas->fullscreenconfig->ui_border_top;
        status_h = canvas->fullscreenconfig->ui_border_bottom;
#endif
        x11ui_canvas_position(canvas->emuwindow, &winx, &winy);

        if (lastx < 20) {
            lastx = (vm->hdisplay + xoffs) - 2;
        } else if (lastx > ((vm->hdisplay + xoffs) - 20)) {
            lastx = 2;
        }

        /* only wrap around at y if menu/status is disabled */
        wrap = 1;
        if (menu_h == 0) {
            if (lasty < 20) {
                lasty = (vm->vdisplay + yoffs) - 2;
            } else if (lasty > ((vm->vdisplay + yoffs) - 20)) {
                lasty = 2;
            }
        } else {
            if (lasty < 20) {
                wrap = 0;
            } else if (lasty > ((vm->vdisplay + yoffs) - (menu_h + status_h + 20))) { /* FIXME */
                wrap = 0;
            }
        }
        lastx += (winx + xoffs);
        lasty += (winy + yoffs + menu_h);

        if (wrap) {
            XWarpPointer(vm_display, None, DefaultRootWindow(vm_display), 0, 0, vm->hdisplay, vm->vdisplay, lastx, lasty);
#if 0
            /* grab the pointer and keyboard */
            XGrabPointer(vm_display, shellwin, 1, PointerMotionMask, GrabModeAsync, GrabModeAsync, shellwin, None, CurrentTime);
#endif
            vidmode_center_canvas(canvas);
            /* ui_dispatch_events(); */
        }
    } else if (leave == 2) { /* enter the canvas */
#if 0
        /* grab the pointer and keyboard */
        XGrabPointer(vm_display, shellwin, 1, PointerMotionMask, GrabModeAsync, GrabModeAsync, shellwin, None, CurrentTime);
#endif
        /* vidmode_center_canvas(canvas); */ /* this crashes ? */
        delayed_center = 1;
    } else {
        lastx = x;
        lasty = y;

#ifdef HAVE_FULLSCREEN
        menu_h = canvas->fullscreenconfig->ui_border_top;
#endif
        y -= (menu_h + yoffs);
        x -= (xoffs);

        wrap = 0;
        if (x < 1) {
            x =  vm->hdisplay - 10;
            wrap = 1;
        } else if (x >= (vm->hdisplay-1)) {
            x =  xoffs + 10;
            wrap = 1;
        }

        /* only wrap around at y if menu/status is disabled */
        if (menu_h == 0) {
            if (y < 1) {
                y =  vm->vdisplay - 10;
                wrap = 1;
            } else if (y >= (vm->vdisplay-1)) {
                y =  ((menu_h + yoffs) * 2) + 50; /* FIXME */
                wrap = 1;
            }
        }

        if (wrap) {
            XWarpPointer(vm_display, None, DefaultRootWindow(vm_display), 0, 0, vm->hdisplay, vm->vdisplay, x, y);
            vidmode_center_canvas(canvas);
        } else if (delayed_center) {
            delayed_center = 0;
            vidmode_center_canvas(canvas);
        }
    }
}

void vidmode_resize(struct video_canvas_s *canvas, int uienable)
{
    vidmode_resize_canvas(canvas, uienable);
    vidmode_center_canvas(canvas);
    ui_dispatch_events();
}

int vidmode_enable(struct video_canvas_s *canvas, int enable)
{
#ifdef HAVE_FULLSCREEN
    Display *vm_display;
    /* Window shellwin; */

    if (vm_available == 0) {
        return -1;
    }

    ui_dispatch_events();

    if (vm_is_enabled == enable) {
        return 0;
    }

    vm_display = x11ui_get_display_ptr();

    if (enable) {

        log_message(vidmode_log, "Enabling Vidmode with%s", vm_bestmodes[vidmode_selected_mode].name);
        vm = vm_modes[vm_bestmodes[vidmode_selected_mode].modeindex];

        saved_w = canvas->draw_buffer->canvas_width;
        saved_h = canvas->draw_buffer->canvas_height;

        vidmode_resize_canvas(canvas, canvas->fullscreenconfig->ui_border_top > 0);
        XF86VidModeSwitchToMode(vm_display, screen, vm);
        vidmode_center_canvas(canvas);

        XWarpPointer(vm_display, None, DefaultRootWindow(vm_display), 0, 0, vm->hdisplay, vm->vdisplay, x + vm->hdisplay / 2, y + vm->vdisplay / 2);
#if 0
        /* grab the pointer */
        shellwin = x11ui_get_X11_window();
        XGrabPointer(vm_display, shellwin, 1, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, shellwin, None, CurrentTime);
#endif
        active_canvas = canvas;
        vm_is_suspended = 0;
    } else {
        log_message(vidmode_log, "Disabling Vidmode");

        XF86VidModeSwitchToMode(vm_display, screen, vm_modes[0]);
#if 1
        /* FIXME: don't ungrab if either mouse or lightpen emulation is enabled.
         * 
         *        it should not be necessary to do this here, but without it
         *        the pointer stays grabbed
         */
        XUngrabPointer(vm_display, CurrentTime);
        XUngrabKeyboard(vm_display, CurrentTime);
#endif
        /* restore canvas size for windowed mode */
        canvas->draw_buffer->canvas_width = saved_w;
        canvas->draw_buffer->canvas_height = saved_h;
        video_viewport_resize(canvas, 1);
    }
    ui_dispatch_events();
    vm_is_enabled = enable;
#endif
    return 0;
}

int vidmode_mode(struct video_canvas_s *canvas, int mode)
{
    if (mode < 0) {
        return 0;
    }

    if (vidmode_log != LOG_ERR) {
        log_message(vidmode_log, "Selected mode: %s", vm_bestmodes[mode].name);
    }
    vidmode_selected_mode = mode;

    return 0;
}

void vidmode_shutdown(void)
{
    unsigned int i;

    if (vm_is_enabled > 0 && vm_is_suspended == 0) {
        log_message(vidmode_log, "Disabling Vidmode");
        XF86VidModeSwitchToMode(x11ui_get_display_ptr(), screen, vm_modes[0]);
    }

    if (vm_available == 0) {
        return;
    }

    for (i = 0; i < vm_index; i++) {
        lib_free(vm_bestmodes[i].name);
    }

    lib_free(vm_bestmodes);
}

void vidmode_suspend(int level)
{
    if (vm_is_enabled == 0) {
        return;
    }

    if (level > 0) {
        return;
    }

    if (vm_is_suspended > 0) {
        return;
    }

    vm_is_suspended = 1;
    vidmode_enable(active_canvas, 0);
}

void vidmode_resume(void)
{
    if (vm_is_enabled == 0) {
        return;
    }

    if (vm_is_suspended == 0) {
        return;
    }

    vm_is_suspended = 0;
    vidmode_enable(active_canvas, 1);
}

void vidmode_set_mouse_timeout(void)
{
}

int vidmode_available(void)
{
    return vm_available;
}

static void *mode_callback;

void vidmode_mode_callback(void *callback)
{
    mode_callback = callback;
}

void vidmode_menu_create(struct ui_menu_entry_s *menu)
{
    unsigned int i, amodes;
    ui_menu_entry_t *resolutions_submenu;

    amodes = vidmode_available_modes();

    resolutions_submenu = lib_calloc((size_t)(amodes + 1), sizeof(ui_menu_entry_t));

    for (i = 0; i < amodes ; i++) {
        resolutions_submenu[i].string = (ui_callback_data_t)lib_msprintf("%s", vm_bestmodes[i].name);
        resolutions_submenu[i].type = UI_MENU_TYPE_TICK;
        resolutions_submenu[i].callback = (ui_callback_t)mode_callback;
        resolutions_submenu[i].callback_data = (ui_callback_data_t)(unsigned long)i;
    }

    for (i = 0; menu[i].string; i++) {
        if (strncmp(menu[i].string, "VidMode", 7) == 0) {
            if (amodes > 0) {
                menu[i].sub_menu = resolutions_submenu;
            }
            break;
        }
    }
}

void vidmode_menu_shutdown(struct ui_menu_entry_s *menu)
{
    unsigned int i, amodes;
    ui_menu_entry_t *resolutions_submenu = NULL;

    amodes = vidmode_available_modes();

    if (amodes == 0) {
        return;
    }

    for (i = 0; menu[i].string; i++) {
        if (strncmp(menu[i].string, "VidMode", 7) == 0) {
            resolutions_submenu = menu[i].sub_menu;
            break;
        }
    }

    menu[i].sub_menu = NULL;

    if (resolutions_submenu != NULL) {
        for (i = 0; i < amodes ; i++) {
            lib_free(resolutions_submenu[i].string);
        }
    }

    lib_free(resolutions_submenu);
}
