/*
 * uimon.c - Monitor access interface.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "console.h"
#include "lib.h"
#include "log.h"
#include "monitor.h"
#include "ui.h"
#include "uimon.h"

static console_t *console_log_local = NULL;

void uimon_window_close(void)
{
    console_close(console_log_local);
    console_log_local = NULL;
}

console_t *uimon_window_open(void)
{
    console_log_local = console_open("Monitor");
    if (!console_log_local) {
        ui_error("BeVICE must be started from a Terminal to use the Monitor.");
    }
    return console_log_local;
}

void uimon_window_suspend(void)
{
}

console_t *uimon_window_resume(void)
{
    if (console_log_local) {
        return console_log_local;
    }
    log_error(LOG_DEFAULT, "uimon_window_resume: log was not opened.");
    return uimon_window_open();
}

int uimon_out(const char *buffer)
{
    int rc = 0;

    if (console_log_local) {
        rc = console_out(console_log_local, "%s", buffer);
    }
    return rc;
}

char *uimon_get_in( char **ppchCommandLine, const char *prompt )
{
    return console_in(console_log_local, prompt);
}

void uimon_notify_change( void )
{
}

void uimon_set_interface(monitor_interface_t **monitor_interface_init, int count)
{
}
