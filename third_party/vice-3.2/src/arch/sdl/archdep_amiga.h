/*
 * archdep_amiga.h
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

#ifndef VICE_ARCHDEP_AMIGA_H
#define VICE_ARCHDEP_AMIGA_H

#include "archapi.h"

#ifdef AMIGA_M68K
#include <sys/unistd.h>
#endif

/* This platform supports choosing drives. */
#define SDL_CHOOSE_DRIVES

/* Filesystem dependant operators.  */
#define FSDEVICE_DEFAULT_DIR "."
#define FSDEV_DIR_SEP_STR    "/"
#define FSDEV_DIR_SEP_CHR    '/'
#define FSDEV_EXT_SEP_STR    "."
#define FSDEV_EXT_SEP_CHR    '.'

/* Path separator.  */
#define ARCHDEP_FINDPATH_SEPARATOR_CHAR   ';'
#define ARCHDEP_FINDPATH_SEPARATOR_STRING ";"

/* Modes for fopen().  */
#define MODE_READ              "r"
#define MODE_READ_TEXT         "r"
#define MODE_READ_WRITE        "r+"
#define MODE_WRITE             "w"
#define MODE_WRITE_TEXT        "w"
#define MODE_APPEND            "a"
#define MODE_APPEND_READ_WRITE "a+"

/* Printer default devices.  */
#define ARCHDEP_PRINTER_DEFAULT_DEV1 "print.dump"
#define ARCHDEP_PRINTER_DEFAULT_DEV2 "PRT:"
#define ARCHDEP_PRINTER_DEFAULT_DEV3 "PAR:"

/* Default RS232 devices.  */
#define ARCHDEP_RS232_DEV1 "rs232.dump"
#define ARCHDEP_RS232_DEV2 "SER:"
#define ARCHDEP_RS232_DEV3 "SER:"
#define ARCHDEP_RS232_DEV4 "SER:"

/* Default location of raw disk images.  */
#define ARCHDEP_RAWDRIVE_DEFAULT "DF0:"

/* Access types */
#define ARCHDEP_R_OK R_OK
#define ARCHDEP_W_OK W_OK
#define ARCHDEP_X_OK X_OK
#define ARCHDEP_F_OK F_OK

/* Standard line delimiter.  */
#define ARCHDEP_LINE_DELIMITER "\n"

/* Ethernet default device */
#define ARCHDEP_ETHERNET_DEFAULT_DEVICE "eth0"

/* Default sound fragment size */
#define ARCHDEP_SOUND_FRAGMENT_SIZE SOUND_FRAGMENT_MEDIUM

/*
    FIXME: confirm wether SIGPIPE must be handled or not. if the emulator quits
           or crashes when the connection is closed, you might have to install
           a signal handler which calls monitor_abort().

           see archdep_unix.c and bug #3201796
*/
#define archdep_signals_init(x)
#define archdep_signals_pipe_set()
#define archdep_signals_pipe_unset()

/* what to use to return an error when a socket error happens */
#define ARCHDEP_SOCKET_ERROR errno

/* Keyword to use for a static prototype */
#define STATIC_PROTOTYPE static

extern int load_libs(void);
extern void close_libs(void);

extern int pci_lib_loaded;

#if defined(HAVE_PROTO_OPENPCI_H) || defined(AMIGA_OS4)
extern int pci_lib_loaded;
#endif

#define DEFAULT_MENU_KEY SDLK_F10

#endif
