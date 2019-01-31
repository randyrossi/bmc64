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

#ifdef OPENSERVER6_COMPILE
#include <sys/signal.h>
#endif

#include "log.h"
#include "signals.h"

#ifdef SYS_SIGLIST_DECLARED
#define signal_name(x) sys_siglist[x]
#else
static const char *sigstr[7] = {
    "SIGINT", "SIGTERM", "SIGSEGV", "SIGILL", "SIGPIPE", "SIGHUP", "SIGQUIT"
};
static const int signum[7] = {
    SIGINT, SIGTERM, SIGSEGV, SIGILL, SIGPIPE, SIGHUP, SIGQUIT
};
static const char *signal_name(int num)
{
    int i;
    for (i = 0; i < 7; i++) {
        if (num == signum[i]) {
            return sigstr[i];
        }
    }
    return "unknown";
}
#endif

/******************************************************************************/

#ifdef OPENSERVER5_COMPILE
static RETSIGTYPE ignore64(int sig)
{
}
#endif

static RETSIGTYPE break64(int sig)
{
    log_message(LOG_DEFAULT, "Received signal %d (%s), exiting.", sig, signal_name(sig));
    exit (-1);
}

/*
    used once at init time to setup all signal handlers
*/
void signals_init(int do_core_dumps)
{
    signal(SIGINT, break64);
    signal(SIGTERM, break64);

#ifdef OPENSERVER5_COMPILE
    signal(SIGALRM, ignore64);
#endif

    if (!do_core_dumps) {
        signal(SIGSEGV, break64);
        signal(SIGILL, break64);
        signal(SIGPIPE, break64);
        signal(SIGHUP, break64);
        signal(SIGQUIT, break64);
    }
}

/******************************************************************************/

typedef void (*signal_handler_t)(int);

static signal_handler_t old_pipe_handler;

/*
    these two are used for socket send/recv. in this case we might
    get SIGPIPE if the connection is unexpectedly closed.
*/
void signals_pipe_set(void)
{
    old_pipe_handler = signal(SIGPIPE, SIG_IGN);
}

void signals_pipe_unset(void)
{
    signal(SIGPIPE, old_pipe_handler);
}
