/*
 * uimon.c - Monitor access interface.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
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

#include "console.h"
#if defined(UNIX_COMPILE)
#include "fullscreenarch.h"
#endif
#include "lib.h"
#include "machine.h"
#include "monitor.h"
#include "uimon.h"
#include "ui.h"
#include "uimenu.h"
#include "videoarch.h"

static console_t mon_console = {
    40,
    25,
    0,
    0,
    NULL
};

static menu_draw_t *menu_draw = NULL;

static int x_pos = 0;

static console_t *console_log_local = NULL;

#ifdef ALLOW_NATIVE_MONITOR
static int using_ui_monitor = 0;
#else
static const int using_ui_monitor = 1;
#endif

void uimon_window_close(void)
{
    if (using_ui_monitor) {
        if (menu_draw) {
            sdl_ui_activate_post_action();
        }
        if (machine_class == VICE_MACHINE_VSID) {
            memset(sdl_active_canvas->draw_buffer_vsid->draw_buffer, 0, sdl_active_canvas->draw_buffer_vsid->draw_buffer_width * sdl_active_canvas->draw_buffer_vsid->draw_buffer_height);
            sdl_ui_refresh();
        }
    } else {
        console_close(console_log_local);
        console_log_local = NULL;
    }
}

console_t *uimon_window_open(void)
{
#ifdef ALLOW_NATIVE_MONITOR
    using_ui_monitor = !native_monitor || sdl_active_canvas->fullscreenconfig->enable;
#endif

    if (using_ui_monitor) {
        sdl_ui_activate_pre_action();
        sdl_ui_init_draw_params();
        sdl_ui_clear();
        menu_draw = sdl_ui_get_menu_param();
        mon_console.console_xres = menu_draw->max_text_x;
        mon_console.console_yres = menu_draw->max_text_y;
        x_pos = 0;
        return &mon_console;
    } else {
        console_log_local = console_open("Monitor");
        return console_log_local;
    }
}

void uimon_window_suspend(void)
{
    if (using_ui_monitor && menu_draw) {
        uimon_window_close();
    }
}

console_t *uimon_window_resume(void)
{
    if (using_ui_monitor && menu_draw) {
        return uimon_window_open();
    } else {
        return console_log_local;
    }
}

int uimon_out(const char *buffer)
{
    int rc = 0;

    char *buf = lib_stralloc(buffer);

    if (using_ui_monitor) {
        int y = menu_draw->max_text_y - 1;
        char *p = buf;
        int i = 0;
        char c;

        while ((c = p[i]) != 0) {
            if (c == '\n') {
                p[i] = 0;
                sdl_ui_print(p, x_pos, y);
                sdl_ui_scroll_screen_up();
                x_pos = 0;
                p += i + 1;
                i = 0;
            } else if (c == '\t') {
                /* replace tabs with a single space, so weird 'i' chars don't
                 * show up for tabs (we don't have a lot of room in a 40-col
                 * display, so expanding these tabs won't do much good)
                 */
                p[i++] = ' ';
            } else {
                ++i;
            }
        }

        if (p[0] != 0) {
            x_pos += sdl_ui_print(p, x_pos, y);
        }
        return 0;
    } else {
        if (console_log_local) {
            rc = console_out(console_log_local, "%s", buffer);
        }
    }

    lib_free(buf);

    return rc;
}

char *uimon_get_in(char **ppchCommandLine, const char *prompt)
{
    if (using_ui_monitor) {
        int y, x_off;
        char *input;

        y = menu_draw->max_text_y - 1;
        x_pos = 0;

        x_off = sdl_ui_print(prompt, 0, y);
        input = sdl_ui_readline(NULL, x_off, y);
        sdl_ui_scroll_screen_up();

        if (input == NULL) {
            input = lib_stralloc("x");
        }

        return input;
    } else {
        return console_in(console_log_local, prompt);
    }
}

void uimon_notify_change(void)
{
    if (using_ui_monitor && menu_draw) {
        sdl_ui_refresh();
    }
}

void uimon_set_interface(monitor_interface_t **monitor_interface_init, int count)
{
}
