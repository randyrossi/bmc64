/*
 * vmidas.c - MIDAS function wrappers.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include "vmidas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>

#include "log.h"
#include "types.h"
#include "util.h"

#define MIDAS_ERRMSG() MIDASgetErrorMessage(MIDASgetLastError())

int _midas_available = 0;

static int MIDASstartup_called = 0;
static struct {
    DWORD rate;
    BOOL displaySync;
    void (MIDAS_CALL *preVR)();
    void (MIDAS_CALL *immVR)();
    void (MIDAS_CALL *inVR)();
} midas_timer_status;

static char *config_fname;

static log_t vmidas_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

static void open_log(void)
{
    if (vmidas_log == LOG_ERR) {
        vmidas_log = log_open("VMidas");
    }
}

void vmidas_close(void)
{
    if (_midas_available) {
        MIDASclose();
        _midas_available = 0;
    }
}

static void vmidas_set_config_fname(void)
{
    config_fname = util_concat(boot_path, "/midas.cfg", NULL);
}

BOOL vmidas_startup(void)
{
    if (!MIDASstartup_called) {
        open_log();

        if (!MIDASstartup()) {
            log_error(vmidas_log, "MIDASstartup: %s.", MIDAS_ERRMSG());
            return FALSE;
        }
        MIDASstartup_called = 1;
        _midas_available = 1;
        memset(&midas_timer_status, 0, sizeof(midas_timer_status));
        vmidas_set_config_fname();
        if (MIDASloadConfig(config_fname)) {
            log_message(vmidas_log, "Loaded MIDAS configuration file `%s'.", config_fname);
        } else {
            log_error(vmidas_log, "MIDASloadConfig(\"%s\"): %s.", config_fname, MIDAS_ERRMSG());
            log_error(vmidas_log, "Couldn't load MIDAS configuration file `%s'.", config_fname);
        }
        atexit(vmidas_close);
        return TRUE;
    }

    return TRUE;
}

BOOL vmidas_remove_timer_callbacks(void)
{
    if (!_midas_available) {
        return FALSE;
    }
    if (MIDASremoveTimerCallbacks()) {
        midas_timer_status.rate = 0;
        return TRUE;
    } else {
        log_error(vmidas_log, "MIDASremoveTimerCallbacks: %s.", MIDAS_ERRMSG());
        return FALSE;
    }
}

BOOL vmidas_set_timer_callbacks(DWORD rate, BOOL displaySync, void (MIDAS_CALL *preVR)(), void (MIDAS_CALL *immVR)(), void (MIDAS_CALL *inVR)())
{
    if (!_midas_available) {
        return FALSE;
    }
    if (MIDASsetTimerCallbacks(rate, displaySync, preVR, immVR, inVR)) {
        midas_timer_status.rate = rate;
        midas_timer_status.displaySync = displaySync;
        midas_timer_status.preVR = preVR;
        midas_timer_status.immVR = immVR;
        midas_timer_status.inVR = inVR;
        return TRUE;
    } else {
        log_error(vmidas_log, "MIDASsetTimerCallbacks: %s.", MIDAS_ERRMSG());
        return FALSE;
    }
}

BOOL vmidas_init(void)
{
    if (!_midas_available) {
        return FALSE;
    }

    open_log();

    if (!MIDASclose()) {
        log_error(vmidas_log, "MIDASclose: %s\n", MIDAS_ERRMSG());
        return FALSE;
    }
    if (!MIDASinit()) {
        log_error(vmidas_log, "MIDASinit: %s\n", MIDAS_ERRMSG());
        return FALSE;
    }
    if (midas_timer_status.rate != 0) {
        BOOL ret = MIDASsetTimerCallbacks(midas_timer_status.rate, midas_timer_status.displaySync, midas_timer_status.preVR, midas_timer_status.immVR, midas_timer_status.inVR);
        if (!ret) {
            log_error(vmidas_log, "Cannot set MIDAS timer callbacks.");
            log_error(vmidas_log, "MIDASsetTimerCallbacks(rate = %d, displaySync = %d): %s.", midas_timer_status.rate, midas_timer_status.displaySync, MIDAS_ERRMSG());
            exit(EXIT_FAILURE);
        }
        return ret;
    }

    return TRUE;
}

BOOL vmidas_config(void)
{
    open_log();

    if (!MIDASconfig()) {
        log_error(vmidas_log, "MIDASconfig(): %s.", MIDAS_ERRMSG());
        return FALSE;
    }

    if (!vmidas_init()) {
        log_error(vmidas_log, "Cannot reinitialize MIDAS.");
        exit(EXIT_FAILURE);
    }

    /* FIXME: `MIDASsaveConfig()' return value? */
    if (!MIDASsaveConfig(config_fname)) {
        log_message(vmidas_log, "Saved MIDAS configuration in `%s'.", config_fname);
        return TRUE;
    } else {
        log_error(vmidas_log, "MIDASsaveConfig(\"%s\"): %s.", config_fname, MIDAS_ERRMSG());
        log_error(vmidas_log, "Couldn't save MIDAS configuration in `%s'.", config_fname);
        return FALSE;
    }
}
