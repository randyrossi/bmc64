/** \file   rs232-win32-dev.c
 * \brief   RS232 Device emulation
 *
 * \author  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
 *
 * The RS232 emulation captures the bytes sent to the RS232 interfaces
 * available (currently, ACIA 6551, std. C64, and Daniel Dallmann's fast RS232
 * with 9600 BPS).
 *
 * I/O is done to a physical COM port.
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

#undef        DEBUG
/* #define DEBUG */

#define DEBUG_FAKE_INPUT_OUTPUT (5 * 80)
#undef DEBUG_FAKE_INPUT_OUTPUT

#include "vice.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <winsock.h>

#ifdef HAVE_IO_H
#include <io.h>
#endif

#include "log.h"
#include "rs232.h"
#include "rs232dev.h"
#include "types.h"
#include "util.h"

#ifdef DEBUG
# define DEBUG_LOG_MESSAGE(_xxx) log_message _xxx
#else
# define DEBUG_LOG_MESSAGE(_xxx)
#endif

/* ------------------------------------------------------------------------- */

int rs232dev_resources_init(void)
{
    return 0;
}

void rs232dev_resources_shutdown(void)
{
}

int rs232dev_cmdline_options_init(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */

typedef struct rs232dev {
    int inuse;
    HANDLE fd;
    char *file;
    DCB restore_dcb;
    int rts;
    int dtr;
} rs232dev_t;

static rs232dev_t fds[RS232_NUM_DEVICES];

static log_t rs232dev_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

void rs232dev_close(int fd);

/* initializes all RS232 stuff */
void rs232dev_init(void)
{
    rs232dev_log = log_open("RS232DEV");
}

/* reset RS232 stuff */
void rs232dev_reset(void)
{
    int i;

    for (i = 0; i < RS232_NUM_DEVICES; i++) {
        if (fds[i].inuse) {
            rs232dev_close(i);
        }
    }
}

/* opens a rs232 window, returns handle to give to functions below. */
int rs232dev_open(int device)
{
    HANDLE serial_port;
    int ret = -1;
    int i;

    for (i = 0; i < RS232_NUM_DEVICES; i++) {
        if (!fds[i].inuse) {
            break;
        }
    }
    if (i >= RS232_NUM_DEVICES) {
        log_error(rs232dev_log, "No more devices available.");
        return -1;
    }

    DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev_open(device=%d).", device));

    do {
        DCB dcb;
        COMMTIMEOUTS comm_timeouts;
        char *mode_string = strchr(rs232_devfile[device], ':');

        if (mode_string != NULL) {
            *mode_string = 0;
        }

        serial_port = CreateFile(rs232_devfile[device], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

        if (mode_string != NULL) {
            *mode_string = ':';
        }


        if (serial_port == INVALID_HANDLE_VALUE) {
            DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: CreateFile '%s' failed: %d.\n", rs232_devfile[device], GetLastError()));
            break;
        }

        memset(&dcb, 0, sizeof dcb);
        dcb.DCBlength = sizeof dcb;

        if (!GetCommState(serial_port, &dcb)) {
            DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: GetCommState '%s' failed: %d.\n", rs232_devfile[device], GetLastError()));
            break;
        }

        fds[i].restore_dcb = dcb;

        if (mode_string != NULL) {
            ++mode_string;

            while (*mode_string == ' ') {
                ++mode_string;
            }

            if ( ! BuildCommDCB(mode_string, &dcb) ) {
                DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: BuildCommDCB '%s' for device '%s' failed: %d.\n", mode_string + 1, rs232_devfile[device], GetLastError()));
                break;
            }
        }

        if (! SetCommState(serial_port, &dcb)) {
            DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: SetCommState '%s' failed: %d.\n", rs232_devfile[device], GetLastError()));
            break;
        }

        memset(&comm_timeouts, 0, sizeof comm_timeouts);

        /*
         * ensure that a read will always terminate and only return
         * what is already in the buffers
         */
        comm_timeouts.ReadIntervalTimeout = UINT32_MAX;
        comm_timeouts.ReadTotalTimeoutMultiplier = 0;
        comm_timeouts.ReadTotalTimeoutConstant = 0;

        /*
         * Do not use total timeouts for write operations
         */
        comm_timeouts.WriteTotalTimeoutConstant = 0;
        comm_timeouts.WriteTotalTimeoutMultiplier = 0;

        if (!SetCommTimeouts(serial_port, &comm_timeouts)) {
            DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: SetCommTimeouts '%s' failed: %d.\n", rs232_devfile[device], GetLastError()));
            break;
        }

        fds[i].inuse = 1;
        fds[i].fd = serial_port;
        fds[i].file = rs232_devfile[device];
        ret = i;
        serial_port = INVALID_HANDLE_VALUE;

    } while (0);

    if (serial_port != INVALID_HANDLE_VALUE) {
        CloseHandle(serial_port);
    }

    return ret;
}

/* closes the rs232 window again */
void rs232dev_close(int fd)
{
    DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: close(fd=%d).", fd));

    if (fd < 0 || fd >= RS232_NUM_DEVICES) {
        log_error(rs232dev_log, "Attempt to close invalid fd %d.", fd);
        return;
    }
    if (!fds[fd].inuse) {
        log_error(rs232dev_log, "Attempt to close non-open fd %d.", fd);
        return;
    }

    if (!SetCommState(fds[fd].fd, &fds[fd].restore_dcb)) {
        DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: SetCommState '%s' on close failed: %d.\n", rs232_devfile[fd], GetLastError()));
    }

    CloseHandle(fds[fd].fd);
    fds[fd].inuse = 0;
}

#if DEBUG_FAKE_INPUT_OUTPUT
static char rs232_debug_fake_input = 0;
static unsigned rs232_debug_fake_input_available = 0;
#endif

/* sends a byte to the RS232 line */
int rs232dev_putc(int fd, uint8_t b)
{
    uint32_t number_of_bytes = 1;

    DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: Output %u = `%c'.", (unsigned)b, b));

#if DEBUG_FAKE_INPUT_OUTPUT

    rs232_debug_fake_input_available = DEBUG_FAKE_INPUT_OUTPUT;
    rs232_debug_fake_input = b;

#else
    if (WriteFile(fds[fd].fd, &b, (DWORD)1, (LPDWORD)&number_of_bytes, NULL) == 0) {
        return -1;
    }

    if (number_of_bytes == 0) {
        return -1;
    }
#endif

    return 0;
}

/* gets a byte from the RS232 line, returns !=0 if byte received, byte in *b. */
int rs232dev_getc(int fd, uint8_t *b)
{
    uint32_t number_of_bytes = 1;

#if DEBUG_FAKE_INPUT_OUTPUT

    if (rs232_debug_fake_input_available != 0) {
        --rs232_debug_fake_input_available;
        *b = rs232_debug_fake_input;
    } else {
        number_of_bytes = 0;
    }

#else
    if (ReadFile(fds[fd].fd, b, (DWORD)1, (LPDWORD)&number_of_bytes, NULL) == 0) {
        return -1;
    }
#endif

    if (number_of_bytes) {
        DEBUG_LOG_MESSAGE((rs232dev_log, "rs232dev: Input %u = `%c'.", (unsigned)*b, *b));
        return 1;
    }

    return 0;
}

/* set the status lines of the RS232 device */
int rs232dev_set_status(int fd, enum rs232handshake_out status)
{
    int new_rts = (status & RS232_HSO_RTS) ? 1 : 0;
    int new_dtr = (status & RS232_HSO_DTR) ? 1 : 0;

    /* signal the RS232 device the current status, too */

    if ( new_rts != fds[fd].rts ) {
        EscapeCommFunction(fds[fd].fd, new_rts ? SETRTS : CLRRTS);
        fds[fd].rts = new_rts;
    }

    if ( new_dtr != fds[fd].dtr ) {
        EscapeCommFunction(fds[fd].fd, new_dtr ? SETDTR : CLRDTR);
        fds[fd].dtr = new_dtr;
    }

    return 0;
}

/* get the status lines of the RS232 device */
enum rs232handshake_in rs232dev_get_status(int fd)
{
    enum rs232handshake_in modem_status = 0;

    do {
        uint32_t modemstat = 0;
        if (GetCommModemStatus(fds[fd].fd, (LPDWORD)&modemstat) == 0) {
            DEBUG_LOG_MESSAGE((rs232dev_log, "Could not get modem status for device %d.", device));
            break;
        }

        if (modemstat & MS_CTS_ON) {
            modem_status |= RS232_HSI_CTS;
        }

        if (modemstat & MS_DSR_ON) {
            modem_status |= RS232_HSI_DSR;
        }
    } while (0);

    return modem_status;
}

/* set the bps rate of the physical device */
void rs232dev_set_bps(int fd, unsigned int bps)
{
    /*! \todo set the physical bps rate */
    DEBUG_LOG_MESSAGE((rs232dev_log, "Setting bps to %u", bps));
}
