/*
 * console.c - Console access interface.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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
#include <string.h>
#include <unistd.h>

#include "console.h"
#include "lib.h"
#include "log.h"

int console_init( void )
{
    return 0;
}

console_t *console_open(const char *id)
{
    console_t *console;

    if (!isatty(fileno(stdin))) {
        log_error(LOG_DEFAULT, "console_open: stdin is not a tty.");
        return NULL;
    }
    if (!isatty(fileno(stdout))) {
        log_error(LOG_DEFAULT, "console_open: stdout is not a tty.");
        return NULL;
    }

    console = lib_malloc(sizeof(console_t));

    console->console_xres = 80;
    console->console_yres = 25;
    console->console_can_stay_open = 1;
    console->console_cannot_output = 0;

    return console;
}

int console_out(console_t *log, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);

    return 0;
}

#ifdef HAVE_READLINE
# include "editline.h"
#else
char *readline(const char *prompt)
{
    char *p, *rv;

    console_out(NULL, "%s", prompt);
    fflush(stdout);

    p = lib_malloc(1024);
    rv = fgets(p, 1024, stdin);
    /*
       fgets returns p on success, or null on EOF.
       If BeVICE is started from Tracker, it will
       always get an EOF from stdin.
    */

    if (!rv) {
        lib_free(p);
    } else {
        /* Remove trailing newlines. */
        int len;

        for (len = strlen(p); len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n'); len--) {
            p[len - 1] = '\0';
        }
    }

    return rv;
}
#endif

char *console_in(console_t *log, const char *prompt)
{
    char *p;

    p = readline(prompt);

    return p;
}

int console_close(console_t *log)
{
    lib_free(log);

    return 0;
}

int console_close_all( void )
{
    return 0;
}
