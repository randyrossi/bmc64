/*
 * console_amiga.c - AmigaOS specific console access interface for SDL.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#define __USE_INLINE__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>

#ifdef AMIGA_MORPHOS
#include <proto/exec.h>
#endif

#include "console.h"
#include "lib.h"
#include "resources.h"
#include "ui.h"
#include "fullscreenarch.h"

#ifdef AMIGA_OS4
#define Flush(x) FFlush(x)
#endif

static BPTR console_handle;
static int fullscreenwasenabled=0;

#ifdef AMIGA_MORPHOS
static APTR fh_putchproc(APTR putchdata, UBYTE ch)
{
    FPutC((BPTR)putchdata, ch);
    return putchdata;
}
#endif

int console_out(console_t *log, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
#ifndef AMIGA_MORPHOS
#ifdef AMIGA_AROS
    VFPrintf(console_handle, format, (IPTR *)ap);
#else
    VFPrintf(console_handle, format, (CONST APTR)ap);
#endif
#else
    VNewRawDoFmt(format, fh_putchproc, (STRPTR)console_handle, ap);
#endif
    va_end(ap);

    return 0;
}

char *readline(const char *prompt)
{
    char *p = lib_malloc(1024);
    int len;

    console_out(NULL, "%s", prompt);

    Flush(console_handle);
    FGets(console_handle, p, 1024);

    /* Remove trailing newlines.  */
    for (len = strlen(p); len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n'); len--) {
        p[len - 1] = '\0';
    }

    return p;
}

char *console_in(console_t *log, const char *prompt)
{
    char *p;

    p = readline(prompt);

    return p;
}

console_t *console_open(const char *id)
{
    console_t *console;

    console = lib_malloc(sizeof(console_t));

    console->console_xres = 80;
    console->console_yres = 25;
    console->console_can_stay_open = 0;

    console_handle = Open("CON:0/0/700/480/VICE Monitor/Auto", MODE_READWRITE);

    return console;
}

int console_close(console_t *log)
{
    Close(console_handle);

    lib_free(log);

    return 0;
}

int console_init(void)
{
    return 0;
}

int console_close_all(void)
{
    return 0;
}
