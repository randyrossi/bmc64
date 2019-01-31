/*
 * signals.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "monitor.h"
#include "signals.h"


static RETSIGTYPE break64(int sig)
{
#ifdef SYS_SIGLIST_DECLARED
    log_message(LOG_DEFAULT, "Received signal %d (%s).", sig, sys_siglist[sig]);
#else
    log_message(LOG_DEFAULT, "Received signal %d.", sig);
#endif

    exit (-1);
}

/*
    used once at init time to setup all signal handlers
*/
void signals_init(int do_core_dumps)
{
    signal(SIGINT, SIG_IGN);

    if (!do_core_dumps) {
        signal(SIGSEGV, break64);
        signal(SIGILL, break64);
        signal(SIGPIPE, break64);
        signal(SIGHUP, break64);
        signal(SIGQUIT, break64);
    }
}

/*
    these two are used for socket send/recv. in this case we might
    get SIGPIPE if the connection is unexpectedly closed.
*/
/*
    FIXME: confirm wether SIGPIPE must be handled or not. if the emulator quits
           or crashes when the connection is closed, you might have to install
           a signal handler which calls monitor_abort().

           see arch/unix/signals.c and bug #3201796
*/
void signals_pipe_set(void)
{
}

void signals_pipe_unset(void)
{
}
