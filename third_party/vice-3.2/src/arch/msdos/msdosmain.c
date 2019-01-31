/*
 * msdosmain.c - MSDOS startup.
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

#include <stdio.h>
#include <signal.h>

#include "log.h"
#include "machine.h"
#include "main.h"
#include "video.h"


int main(int argc, char **argv)
{
    return main_program(argc, argv);
}

void main_exit(void)
{
    /* Disable SIGINT.  This is done to prevent the user from keeping C-c
       pressed and thus breaking the cleanup process, which might be
       dangerous.  */
    signal(SIGINT, SIG_IGN);

    log_message(LOG_DEFAULT, "\nExiting...");

    machine_shutdown();

    putchar ('\n');
}
