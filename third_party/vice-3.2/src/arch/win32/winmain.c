/*
 * winmain.c - Windows startup.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#ifdef HAVE_CRTDBG
#include <crtdbg.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "lib.h"
#include "log.h"
#include "machine.h"
#include "main.h"
#include "winmain.h"
#include "video.h"

HINSTANCE winmain_instance;
HINSTANCE winmain_prev_instance;
int winmain_cmd_show;

#if !defined(__MSVCRT__) && !defined(_MSC_VER) && !defined(_WIN64) && !defined(__WATCOMC__) && !defined(WATCOM_COMPILE)
extern void __GetMainArgs(int *, char ***, char ***, int);
#endif

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, char *cmd_line, int cmd_show)
{
    winmain_instance = instance;
    winmain_prev_instance = prev_instance;
    winmain_cmd_show = cmd_show;

#if defined(__MSVCRT__) || defined(_MSC_VER) || defined(_WIN64) || defined(__WATCOMC__) || defined(WATCOM_COMPILE)
#  ifdef _DEBUG
    {
        int tmpFlag;

        // Get current flag
        tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

        // Turn on leak-checking bit
        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
        tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

        // Set flag to the new value
        _CrtSetDbgFlag(tmpFlag);
    }
#  endif
    if (!__argc) {
        // For now we always pass 8-bit args to main_program()
        char *vice_cmdline;
        char *vice_argv[256] ;
        int vice_argc = 0;
        int i = 0;

        vice_cmdline = lib_stralloc(GetCommandLineA());

        while (vice_cmdline[i] != 0) {
            if (vice_cmdline[i] == '"') {
                i++;
                vice_argv[vice_argc] = vice_cmdline + i;
                while (vice_cmdline[i] != '"' && vice_cmdline[i] != 0) {
                    i++;
                }
                if (vice_cmdline[i] == '"') {
                    vice_cmdline[i] = 0;
                    i++;
                }
            } else {
                vice_argv[vice_argc] = vice_cmdline + i;
                while (vice_cmdline[i] != ' ' && vice_cmdline[i] != 0) {
                    i++;
                }
                if (vice_cmdline[i] == ' ') {
                    vice_cmdline[i] = 0;
                    i++;
                }
            }
            vice_argc++;
            while (vice_cmdline[i] == ' ') {
                i++;
            }
        }

        main_program(vice_argc, vice_argv);
        lib_free(vice_cmdline);
    } else {
        main_program(__argc, __argv);
    }
#else
    if (_argc) {
        main_program(_argc, _argv);
    } else {
        int vice_argc = 0;
        char **vice_argv = 0;
        char **dummy = 0;

        __GetMainArgs(&vice_argc, &vice_argv, &dummy, -1);
        main_program(vice_argc, vice_argv);
    }
#endif

    return 0;
}

void main_exit(void)
{
#ifdef HAVE_SIGNAL_H
    /* Disable SIGINT.  This is done to prevent the user from keeping C-c
       pressed and thus breaking the cleanup process, which might be
       dangerous.  */
    signal(SIGINT, SIG_IGN);
#endif

    log_message(LOG_DEFAULT, "\nExiting...");

    machine_shutdown();
}
