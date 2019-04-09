/** \file   console_unix.c
 * \brief   Unix specific console access interface for SDL.
 *
 * \author  Hannu Nuotio <hannu.nuotio@tut.fi>
 * \author  Andreas Boose <viceteam@t-online.de>
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

#include "console.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "charset.h"

static FILE *mon_input, *mon_output;

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

console_t *console_open(const char *id)
{
    console_t *console;

    console = lib_malloc(sizeof(console_t));

    mon_input = stdin;
    mon_output = stdout;

    console->console_xres = 80;
    console->console_yres = 25;
    console->console_can_stay_open = 1;
    console->console_cannot_output = 0;

    return console;
}

int console_close(console_t *log)
{
    lib_free(log);
    return 0;
}

int console_out(console_t *log, const char *format, ...)
{
    va_list ap;
    char *buf;
    unsigned char c;
    int i;

    va_start(ap, format);
    buf = lib_mvsprintf(format, ap);
    va_end(ap);

    if (buf) {
        for (i = 0; (c = buf[i]) != 0; i++) {
            if (c == '\t') {
                buf[i] = ' ';
            } else if (((c < 32) || (c > 126)) && (c != '\n')) {
                    buf[i] = charset_p_toascii(c, 1);
            }
        }

        fprintf(stdout, "%s", buf);
        lib_free(buf);
    }
    return 0;
}

#ifndef HAVE_READLINE
char *readline(const char *prompt)
{
    char *p = lib_malloc(1024);
    size_t len;

    console_out(NULL, "%s", prompt);

    fflush(mon_output);
    if (fgets(p, 1024, mon_input) == NULL) {
        lib_free(p);
        return NULL;
    }

    /* Remove trailing newlines.  */
    for (len = strlen(p);
            len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n');
            len--) {
        p[len - 1] = '\0';
    }
    return p;
}
#endif

char *console_in(console_t *log, const char *prompt)
{
    char *p, *ret_sting;

    p = readline(prompt);
#ifdef HAVE_READLINE
    if (p && *p) {
        add_history(p);
    }
#endif
    ret_sting = lib_stralloc(p);
    free(p);

    return ret_sting;
}

int console_init(void)
{
#if defined(HAVE_READLINE) && defined(HAVE_RLNAME)
    rl_readline_name = "VICE";
#endif
    return 0;
}

int console_close_all(void)
{
    return 0;
}
