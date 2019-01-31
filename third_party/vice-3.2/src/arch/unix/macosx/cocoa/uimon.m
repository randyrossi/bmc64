/*
 * uimon.m - MacVICE monitor interface
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
#include "monitor.h"
#include "uimon.h"

#import "vicemachine.h"

/* make sure to set console_can_stay_open=0 otherwise uimon_window_close will not be called! */
static console_t dummy = { 80,25,0,0,NULL };

console_t *uimon_window_open( void )
{
    // open monitor window
    [[theVICEMachine app] openMonitor];
    return &dummy;
}

void uimon_window_close( void )
{
    // close monitor window
    [[theVICEMachine app] closeMonitor];
}

void uimon_window_suspend( void )
{
    // monitor is temporarly suspended. disable UI of monitor
    [[theVICEMachine app] suspendMonitor];
}

console_t *uimon_window_resume( void )
{
    // monitor is activated after suspend. reenabled UI of monitor.
    [[theVICEMachine app] resumeMonitor];
    return &dummy;
}

int uimon_out(const char *buffer)
{
    NSString *msg = [NSString stringWithCString:buffer encoding:NSUTF8StringEncoding];
    [[theVICEMachine app] printMonitorMessage:msg];
    return 0;
}

char *uimon_get_in(char **ppchCommandLine, const char *prompt)
{
    char *ret;
    NSString *promptString = [NSString stringWithCString:prompt encoding:NSUTF8StringEncoding];
    NSString *lineInput = [theVICEMachine lineInputWithPrompt:promptString timeout:0];
    if (lineInput == nil)
        ret = NULL;
    else {
        ret = lib_stralloc([lineInput cStringUsingEncoding:NSUTF8StringEncoding]);
        [lineInput release];
    }
    return ret;
}

void uimon_notify_change( void )
{
    // report changes of the monitor state to the UI
    [[theVICEMachine app] updateMonitor];
}

void uimon_set_interface(monitor_interface_t **monitor_interface_init,
                         int count )
{
    //TODO
}

