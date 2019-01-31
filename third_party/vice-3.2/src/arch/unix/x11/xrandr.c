/*
 * xrandr.c
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

#include "vice.h"

#ifndef HAVE_XRANDR
#error "XRandR extension not available in config.h, check config.log"
#endif

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <string.h>

#include "video.h"
#include "videoarch.h"
#include "x11ui.h"
#include "log.h"
#include "resources.h"
#include "uiapi.h"
#include "uimenu.h"
#include "lib.h"
#include "vsync.h"
#include "xrandr.h"
#include "fullscreenarch.h"
#ifdef HAVE_OPENGL_SYNC
#include "openGL_sync.h"
#endif

/* #define DEBUG_XRANDR */

static log_t xrandr_log = LOG_ERR;
static int no_xrandr = 1;
static int xrandr_active = 0;
static int xrandr_selected_mode = 0;
static int xrandr_is_suspended = 0;
static ui_callback_t menu_callback;
static ui_menu_entry_t *resolutions_submenu;
static struct video_canvas_s *current_canvas;

static int init_XRandR(Display *dpy);
static int set_xrandr(int mode);
static int set_fullscreen(int enable);

typedef struct {
    char *mode_string;
    int index;
    short rate;
} XRRMode_t;

typedef struct {
    XRRScreenConfiguration *config;

    SizeID current_size;
    short current_rate;

    Rotation current_rotation;
    Rotation rotations;

    XRRScreenSize *sizes;
    int n_sizes;

    XRRMode_t *all_modes;
    int n_all_modes;

} screen_info_t;

static screen_info_t screen_info;

int xrandr_mode(struct video_canvas_s *canvas, int mode)
{
    if (mode < 0) {
        return -1;
    }

    if (xrandr_log != LOG_ERR) {
        log_message(xrandr_log, "Selected mode: %s", screen_info.all_modes[mode].mode_string);
    }
    xrandr_selected_mode = mode;
    current_canvas = canvas;

    return 0;
}

int xrandr_enable(struct video_canvas_s *canvas, int activate)
{
#ifdef HAVE_FULLSCREEN
    if (canvas->fullscreenconfig->double_size) {
        log_message(xrandr_log, "double size not implemented - use standard double size from menu.");
    }
    return set_fullscreen(activate);
#else
    return -1;
#endif
}

void xrandr_suspend(int level)
{
    if (!xrandr_active) {
        return;
    }
    if (level > 0) {
        return;
    }
    if (xrandr_is_suspended > 0) {
        return;
    }
    xrandr_is_suspended=1;
    set_fullscreen(0);
}

void xrandr_resume(void)
{
    if (xrandr_active) {
        return;
    }
    if (!xrandr_is_suspended) {
        return;
    }
    xrandr_is_suspended = 0;
    set_fullscreen(1);
}

int xrandr_init(void)
{
    Display *dpy;

    if (xrandr_log == LOG_ERR) {
        xrandr_log = log_open("XRandR");
    }

    dpy = x11ui_get_display_ptr();
    if (init_XRandR(dpy)) {
        no_xrandr = 1;
        log_message(xrandr_log, "X11 XRandR extension not available, using default display mode.");
        return -1;
    }
    no_xrandr = 0;

    return 0;
}

int xrandr_available(void)
{
    if (no_xrandr) {
        return 0;
    }
    return 1;
}

void xrandr_menu_create(struct ui_menu_entry_s *menu)
{
    int i;

    if (no_xrandr) {
        return;
    }

    if (!resolutions_submenu) {
        resolutions_submenu = lib_calloc((size_t)(screen_info.n_all_modes + 1), sizeof(ui_menu_entry_t));

        for (i = 0; i < screen_info.n_all_modes; i++) {
            resolutions_submenu[i].string = screen_info.all_modes[i].mode_string;
            resolutions_submenu[i].type = UI_MENU_TYPE_TICK;
            resolutions_submenu[i].callback = (ui_callback_t)menu_callback;
            resolutions_submenu[i].callback_data = (ui_callback_data_t)(long)i;
        }

        /* finalize menu */
        memset(&(resolutions_submenu[screen_info.n_all_modes]), 0, sizeof(ui_menu_entry_t));
    } /* resuse menu for second resolution menu e.g. VDC in x128 */

    for (i = 0; menu[i].string; i++) {
        if (strncmp(menu[i].string, "XRandR", 6) == 0) {
            if (screen_info.n_all_modes > 0) {
                menu[i].sub_menu = resolutions_submenu;
            }
            break;
        }
    }
}

void xrandr_mode_callback(ui_callback_t cb)
{
    menu_callback = cb;
}

void xrandr_mouse_moved(struct video_canvas_s *canvas, int x, int y, int leave)
{
}

void xrandr_resize(struct video_canvas_s *canvas, int uienable)
{
}

void xrandr_shutdown(void)
{
    if (xrandr_active) {
        /* calling set_xrandr directly (instead of set_fullscreen) here to
           force disabling and to avoid unnecessary calls to ui_update_menus */
        set_xrandr(0);
    }
}

void xrandr_menu_shutdown(struct ui_menu_entry_s *menu)
{
    int i;

    xrandr_shutdown(); /* early shutdown, otherwhise
                          screen_info.all_modes[0] is freed  */
    if (resolutions_submenu) {
        lib_free(resolutions_submenu);
        resolutions_submenu = NULL;
        for (i = 0; i < screen_info.n_all_modes; i++) {
            lib_free(screen_info.all_modes[i].mode_string);
        }
        if (screen_info.n_all_modes) {
            lib_free(screen_info.all_modes);
        }
    }
}

/* ---------------------------------------------------------------------*/
/* initialize XRandR */

static int init_XRandR(Display *dpy)
{
    int event_base, error_base;
    int major, minor, i, j;

    if (!XRRQueryExtension(dpy, &event_base, &error_base) || XRRQueryVersion(dpy, &major, &minor) == 0) {
        return 1;
    }

    screen_info.config = XRRGetScreenInfo(dpy, DefaultRootWindow(dpy));
    screen_info.current_rate = XRRConfigCurrentRate(screen_info.config);
    screen_info.current_size = XRRConfigCurrentConfiguration(screen_info.config, &screen_info.current_rotation);
    screen_info.sizes = XRRConfigSizes(screen_info.config, &screen_info.n_sizes);
    screen_info.rotations = XRRConfigRotations(screen_info.config, &screen_info.current_rotation);

    log_message(xrandr_log, "XRandR reports current display: %dx%d@%d", screen_info.sizes[screen_info.current_size].width,
                screen_info.sizes[screen_info.current_size].height, screen_info.current_rate);

    /* remember all modes: resolutions x rates */
    screen_info.n_all_modes = 1; /* index 0 is Desktop default resolution */
    for (i = 0; i < screen_info.n_sizes; i++) {
        int n_rates;

        XRRRates(dpy, XRRRootToScreen(dpy, DefaultRootWindow(dpy)), i, &n_rates);
        screen_info.n_all_modes += n_rates;
    }

    if (screen_info.n_all_modes) {
        int current = 0;

        screen_info.all_modes = lib_calloc((size_t) screen_info.n_all_modes, sizeof(XRRMode_t));

        /* remember Desktop resolution in index 0 */
        screen_info.all_modes[current].rate = screen_info.current_rate;
        screen_info.all_modes[current].index = screen_info.current_size;
        screen_info.all_modes[current].mode_string = lib_stralloc(_("Desktop"));
        current_canvas->refreshrate = (float)screen_info.current_rate;
        ++current;

        /* now iterate again and fill the allocated array */
        for (i = 0; i < screen_info.n_sizes; i++) {
            int n_rates;
            short *rates;
            rates = XRRRates(dpy, XRRRootToScreen(dpy, DefaultRootWindow(dpy)), i, &n_rates);
            for (j = 0; j < n_rates; j++) {
                screen_info.all_modes[current].rate = rates[j];
                screen_info.all_modes[current].index = i;
                screen_info.all_modes[current].mode_string = lib_msprintf("%dx%d@%dHz", screen_info.sizes[i].width, screen_info.sizes[i].height, rates[j]);
                current++;
            }
        }
    }
    return 0;
}

static int set_xrandr(int val)
{
    Status status;
    int ret = 0;

    if (no_xrandr) {
        return -1;
    }

    xrandr_active = 0;
    log_message(xrandr_log, "%s XRandR", val ? "enabling" : "disabling");
    vsync_suspend_speed_eval();

    if (val) {
        status = XRRSetScreenConfigAndRate(x11ui_get_display_ptr(), screen_info.config, x11ui_get_X11_window(), screen_info.all_modes[xrandr_selected_mode].index,
                                           screen_info.current_rotation, screen_info.all_modes[xrandr_selected_mode].rate, 0);
        if (status) {
            log_message(xrandr_log, "XRandR setting failed: %d", status);
            ret = -1;
        } else {
            current_canvas->refreshrate = screen_info.all_modes[xrandr_selected_mode].rate;
#ifdef HAVE_OPENGL_SYNC
            init_openGL();
#endif
            xrandr_active = 1;
        }
    } else {
        status = XRRSetScreenConfigAndRate(x11ui_get_display_ptr(), screen_info.config, x11ui_get_X11_window(), screen_info.all_modes[0].index,
                                           screen_info.current_rotation, screen_info.all_modes[0].rate, 0);
        if (status) {
            log_message(xrandr_log, "XRandR setting failed: %d", status);
            ret = -1;
        } else {
            current_canvas->refreshrate = screen_info.all_modes[0].rate;
        }
    }
    return ret;
}

static volatile int fslock = 0;
static int set_fullscreen(int enable)
{
    int ret = 0;

    ui_dispatch_events();
    if (xrandr_active == enable) {
        log_message(xrandr_log, "set_fullscreen (%d->%d) skipped", xrandr_active, enable);
        return 0;
    }
    if (fslock) {
        log_message(xrandr_log, "set_fullscreen (%d->%d) failed (locked)", xrandr_active, enable);
        return -1;
    }
    fslock = 1;
#ifdef DEBUG_XRANDR
    log_message(xrandr_log, "set_fullscreen (%d->%d)", xrandr_active, enable);
#endif
    ui_update_menus();

    /* try setting up XRandR */
    if (set_xrandr(enable) < 0) {
        /* if it failed for some reason, switch to windowed UI */
        log_message(xrandr_log, "set_xrandr (%s) failed.", enable ? "enable" : "disable");
        x11ui_fullscreen(0);
        ret = -1;
    } else {
        /* if it worked, try to setup the UI accordingly */
        if (x11ui_fullscreen(enable) < 0) {
            log_message(xrandr_log, "x11ui_fullscreen (%s) failed.", enable ? "enable" : "disable");
            /* it failed, and we were trying to enable fullscreen
               switch back to windowed UI */
            if (enable) {
                set_xrandr(0);
                x11ui_fullscreen(0);
            }
            ret = -1;
        }
    }

    ui_update_menus();

    fslock = 0;
    return ret;
}
