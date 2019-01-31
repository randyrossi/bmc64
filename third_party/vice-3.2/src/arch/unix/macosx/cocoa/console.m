/*
 * console.m - MacVICE console interface
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *  Michael Klein <michael.klein@puffin.lb.shuttle.de>
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
#include "lib.h"

#import "consolewindow.h"

int console_init(void)
{
    return 0;
}

console_t *console_open(const char *id)
{
    // open console window
    ConsoleWindow *window = [[ConsoleWindow alloc] 
                        initWithContentRect:NSMakeRect(600, 140, 500, 200)
                                      title:[NSString stringWithCString:id
                                            encoding:NSUTF8StringEncoding]];

    // create VICE console structure
    console_t * console;
    console = lib_malloc(sizeof(console_t));
    console->console_xres = 80;
    console->console_yres = 25;
    console->console_can_stay_open = 0;
    console->private = (struct console_private_s*)window;
    return console;
}

int console_close(console_t *log)
{
    // close console window
    ConsoleWindow *window = (ConsoleWindow *)log->private;
    if (window!=nil)
        [window release];

    // close VICE console structure
    lib_free(log);
    return 0;
}

int console_out(console_t *log, const char *format, ...)
{
    char *tmp;
    va_list ap;

    va_start(ap, format);
    tmp = lib_mvsprintf(format, ap);
    va_end(ap);

    if (log && (log->private!=nil) && tmp)
        [(ConsoleWindow*)log->private appendText:[NSString stringWithCString:tmp encoding:NSUTF8StringEncoding]];

    lib_free(tmp);
    return 0;
}

char *console_in(console_t * log, const char * prompt)
{
    // NOT USED
    return NULL;
}

int console_close_all(void)
{
    return 0;
}

