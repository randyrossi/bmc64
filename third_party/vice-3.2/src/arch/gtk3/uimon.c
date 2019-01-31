/** \file   uimon.c
 * \brief   Native GTK3 UI monitor stuff
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 */

/*
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

#include "not_implemented.h"

#include "console.h"
#include "debug_gtk3.h"
#include "monitor.h"
#include "kbd.h"
#include "resources.h"
#include "ui.h"
#include "uimon.h"
#include "uimonarch.h"
#include "videoarch.h"
#include "vsync.h"

#ifdef HAVE_VTE

char *uimon_get_in(char **ppchCommandLine, const char *prompt)
{
    NOT_IMPLEMENTED();
    return NULL;
}

void uimon_notify_change(void)
{
    NOT_IMPLEMENTED();
}

int uimon_out(const char *buffer)
{
    NOT_IMPLEMENTED();
    return 0;
}

void uimon_set_interface(monitor_interface_t **monitor_interface_init, int count)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

void uimon_window_close(void)
{
    NOT_IMPLEMENTED();
}

static void window_destroy_cb(void)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

console_t *uimon_window_open(void)
{
    NOT_IMPLEMENTED();
    return NULL;
}

console_t *uimon_window_resume(void)
{
    NOT_IMPLEMENTED();
    return NULL;
}

#endif

/** \brief  Callback to activate the ML-monitor
 *
 * \param[in,out]   widget      widget triggering the event
 * \param[in]       user_data   data for the event (unused)
 */
void ui_monitor_activate_callback(GtkWidget *widget, gpointer user_data)
{
    int v;
    int native = 0;

    /*
     * Determine if we use the spawing terminal or the (yet to write) Gtk3
     * base monitor
     */
    if (resources_get_int("NativeMonitor", &native) < 0) {
        debug_gtk3("failed to get value of resource 'NativeMonitor'\n");
    }
    debug_gtk3("called, native monitor = %s (completely ignored for now)\n",
            native ? "true" : "false");

    resources_get_int("MonitorServer", &v);

    if (v == 0) {
#ifdef HAVE_FULLSCREEN
        fullscreen_suspend(0);
#endif
        vsync_suspend_speed_eval();
        /* ui_autorepeat_on(); */

#ifdef HAVE_MOUSE
        /* FIXME: restore mouse in case it was grabbed */
        /* ui_restore_mouse(); */
#endif
        if (!ui_emulation_is_paused()) {
            monitor_startup_trap();
        } else {
            monitor_startup(e_default_space);
#ifdef HAVE_FULLSCREEN
            fullscreen_resume();
#endif
        }
    }
}
