/** \file   uimon-fallback.c
 * \brief   Fallback implementation for the ML-Monitor for when the VTE library
 *          is not available
 *
 * \author  Fabrizio Gennari <fabrizio.ge@tiscali.it>
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

/* FIXME: this code can probably be shared between GTK3/SDL */

#include "vice.h"

#include "debug_gtk3.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
#include <sys/stat.h>
#endif

#ifdef WIN32_COMPILE
#include <windows.h>
#include <winternl.h>
#endif

#include "console.h"
#include "lib.h"
#include "log.h"
#include "ui.h"
#include "uimon.h"
#include "uimon-fallback.h"


static console_t *console_log_local = NULL;

#if defined(HAVE_READLINE) && defined(HAVE_READLINE_READLINE_H)
#include <readline/readline.h>
#include <readline/history.h>
#else
static FILE *mon_input, *mon_output;
#endif

/** \brief  NOP
 *
 * \return  0
 */
int consolefb_close_all(void)
{
    /* This is a no-op on GNOME, should be fine here too */
    return 0;
}


/** \brief  NOP
 *
 * \return  0
 */
int consolefb_init(void)
{
    NOT_IMPLEMENTED_WARN_ONLY();
    return 0;
}

#if !defined(HAVE_READLINE) || !defined(HAVE_READLINE_READLINE_H)
int consolefb_out(console_t *log, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(mon_output, format, ap);
    va_end(ap);

    return 0;
}
#endif

#ifdef WIN32_COMPILE
static int vice_ismsystty(int fd)
{
    intptr_t h_stdin = _get_osfhandle(fd);
    char ntfn_bytes[sizeof(OBJECT_NAME_INFORMATION) + 256 * sizeof(WCHAR)];
    OBJECT_NAME_INFORMATION *ntfn = (OBJECT_NAME_INFORMATION*) ntfn_bytes;
    NTSTATUS status;
    ULONG ntfn_size = sizeof(ntfn_bytes);
    USHORT i, l;
    wchar_t c, *s0;

    memset(ntfn, 0, ntfn_size);
    status = NtQueryObject((HANDLE)h_stdin, ObjectNameInformation, ntfn, ntfn_size, &ntfn_size);

    if (!NT_SUCCESS(status)) {
        return 0;
    }

    l = ntfn->Name.Length;
    s0 = ntfn->Name.Buffer;
    /* Check for "\Device\NamedPipe" */
    {
        USHORT l1 = l;
        wchar_t *s1 = s0;
        wchar_t expect[] = L"\\Device\\NamedPipe\\";

        if (s0[0] == '\\' && s0[1] == '\\' && s0[2] == '?' && s0[3] == '\\') {
            l1 -= 4;
            s1 += 4;
        }
        for (i = 0; i < l1; i++) {
            wchar_t e = expect[i];
            c = s1[i];
            if (!e) {
                break;
            }
            if (c != e) {
                return 0;
            }
        }
    }
    /* Look for "-pty%d-" */
    for (i = 0; i < l; i++) {
        c = s0[i];
        if (c == '-') {
            wchar_t *s = s0 + i + 1;
            if (s[0] == 'p' && s[1] == 't' && s[2] == 'y' && (c = s[3]) && (c >= '0') && (c <= '9'))
            {
                s += 4;
                while ((c = *s) && (c >= '0') && (c <= '9')) {
                    s++;
                }
                if (c == '-' || c == 0) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

static int vice_isatty(int fd)
{
    if (!isatty(fileno(stdin))) {
        return vice_ismsystty(fileno(stdin));
    }
    return 1;
}
#else
#define vice_isatty isatty
#endif

console_t *uimonfb_window_open(void)
{
#ifdef HAVE_SYS_IOCTL_H
    struct winsize w;
#endif

    if (!vice_isatty(fileno(stdin))) {
        log_error(LOG_DEFAULT, "console_open: stdin is not a tty.");
        console_log_local = NULL;
        return NULL;
    }
    if (!vice_isatty(fileno(stdout))) {
        log_error(LOG_DEFAULT, "console_open: stdout is not a tty.");
        console_log_local = NULL;
        return NULL;
    }
    console_log_local = lib_malloc(sizeof(console_t));
    /* change window title for console identification purposes */
    if (getenv("WINDOWID") == NULL) {
        printf("\033]2;VICE monitor console (%d)\007", (int)getpid());
    }

#if !defined(HAVE_READLINE) || !defined(HAVE_READLINE_READLINE_H)
    mon_input = stdin;
    mon_output = stdout;
#endif

#ifdef HAVE_SYS_IOCTL_H
    if (ioctl(fileno(stdin), TIOCGWINSZ, &w)) {
        console_log_local->console_xres = 80;
        console_log_local->console_yres = 25;
    } else {
        console_log_local->console_xres = w.ws_col >= 40 ? w.ws_col : 40;
        console_log_local->console_yres = w.ws_row >= 22 ? w.ws_row : 22;
    }
#else
    console_log_local->console_xres = 80;
    console_log_local->console_yres = 25;
#endif
    console_log_local->console_can_stay_open = 1;
    console_log_local->console_cannot_output = 0;

    /* partially implemented */
    INCOMPLETE_IMPLEMENTATION();
#ifdef HAVE_MOUSE
    /* ui_restore_mouse(); */
#endif
    /* ui_focus_monitor(); */
    return console_log_local;
}

void uimonfb_window_close(void)
{
    lib_free(console_log_local);
    console_log_local = NULL;

    uimon_window_suspend();
}

void uimonfb_window_suspend( void )
{
    /* ui_restore_focus(); */
#ifdef HAVE_MOUSE
    /* ui_check_mouse_cursor(); */
#endif
    NOT_IMPLEMENTED_WARN_ONLY();
}

console_t *uimonfb_window_resume(void)
{
    if (console_log_local) {
        /* partially implemented */
        INCOMPLETE_IMPLEMENTATION();
#ifdef HAVE_MOUSE
        /* ui_restore_mouse(); */
#endif
        /* ui_focus_monitor(); */
        return console_log_local;
    }
    log_error(LOG_DEFAULT, "uimon_window_resume: log was not opened.");
    return uimon_window_open();
}

int uimonfb_out(const char *buffer)
{
    fprintf(stdout, "%s", buffer);
    return 0;
}

#if !defined(HAVE_READLINE) || !defined(HAVE_READLINE_READLINE_H)
char *readline(const char *prompt)
{
    char *p = malloc(1024);

    consolefb_out(NULL, "%s", prompt);

    fflush(mon_output);
    if (fgets(p, 1024, mon_input) == NULL) {
        /* FIXME: handle error */
    }

    /* Remove trailing newlines.  */
    {
        int len;

        for (len = strlen(p); len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n'); len--) {
            p[len - 1] = '\0';
        }
    }

    return p;
}
#endif

char *uimonfb_get_in(char **ppchCommandLine, const char *prompt)
{
    char *p, *ret_sting;

    p = readline(prompt);
#if defined(HAVE_READLINE) && defined(HAVE_READLINE_READLINE_H)
    if (p && *p) {
        add_history(p);
    }
#endif
    ret_sting = lib_stralloc(p);
    free(p);

    return ret_sting;
}

void uimonfb_notify_change( void )
{
}

void uimonfb_set_interface(struct monitor_interface_s **monitor_interface_init, int count)
{
}
