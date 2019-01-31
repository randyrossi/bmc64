/*
 * archdep.h - Miscellaneous system-specific stuff.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_ARCHDEP_H
#define VICE_ARCHDEP_H

#include "archapi.h"

#include "sound.h"

#include "vicemaxpath.h"

#include <sys/types.h>

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
#define MODE_READ              "rb"
#define MODE_READ_TEXT         "rt"
#define MODE_READ_WRITE        "r+b"
#define MODE_WRITE             "wb"
#define MODE_WRITE_TEXT        "wt"
#define MODE_APPEND            "ab"
#define MODE_APPEND_READ_WRITE "a+b"

/* Printer default devices.  */
#define ARCHDEP_PRINTER_DEFAULT_DEV1 "viceprnt.out"
#define ARCHDEP_PRINTER_DEFAULT_DEV2 "LPT1:"
#define ARCHDEP_PRINTER_DEFAULT_DEV3 "LPT2:"

/* Default rs232 devices */
#define ARCHDEP_RS232_DEV1 "10.0.0.1:25232"
#define ARCHDEP_RS232_DEV2 "10.0.0.1:25232"
#define ARCHDEP_RS232_DEV3 "10.0.0.1:25232"
#define ARCHDEP_RS232_DEV4 "10.0.0.1:25232"

/* Video chip scaling.  */
#define ARCHDEP_VICII_DSIZE   1
#define ARCHDEP_VICII_DSCAN   1
#define ARCHDEP_VICII_HWSCALE 0
#define ARCHDEP_VDC_DSIZE     1
#define ARCHDEP_VDC_DSCAN     1
#define ARCHDEP_VDC_HWSCALE   0
#define ARCHDEP_VIC_DSIZE     1
#define ARCHDEP_VIC_DSCAN     1
#define ARCHDEP_VIC_HWSCALE   0
#define ARCHDEP_CRTC_DSIZE    1
#define ARCHDEP_CRTC_DSCAN    1
#define ARCHDEP_CRTC_HWSCALE  0
#define ARCHDEP_TED_DSIZE     1
#define ARCHDEP_TED_DSCAN     1
#define ARCHDEP_TED_HWSCALE   0

/* Video chip double buffering.  */
#define ARCHDEP_VICII_DBUF 0
#define ARCHDEP_VDC_DBUF   0
#define ARCHDEP_VIC_DBUF   0
#define ARCHDEP_CRTC_DBUF  0
#define ARCHDEP_TED_DBUF   0

/* Default location of raw disk images.  */
#define ARCHDEP_RAWDRIVE_DEFAULT "A:"

/* Access types */
#define ARCHDEP_R_OK R_OK
#define ARCHDEP_W_OK W_OK
#define ARCHDEP_X_OK X_OK
#define ARCHDEP_F_OK F_OK

/* Standard line delimiter.  */
#define ARCHDEP_LINE_DELIMITER "\r\n"

/* Ethernet default device */
#define ARCHDEP_ETHERNET_DEFAULT_DEVICE ""

/* Default sound fragment size */
#define ARCHDEP_SOUND_FRAGMENT_SIZE 1 

/* No key symcode.  */
#define ARCHDEP_KEYBOARD_SYM_NONE 0

/* When using the ascii printer driver we need a return before the newline */
#define ARCHDEP_PRINTER_RETURN_BEFORE_NEWLINE

/* what to use to return an error when a socket error happens */
#define ARCHDEP_SOCKET_ERROR errno

/* Default sound output mode */
#define ARCHDEP_SOUND_OUTPUT_MODE SOUND_OUTPUT_MONO

/* Keyword to use for a static prototype */
#define STATIC_PROTOTYPE static

/* define if the platform supports the monitor in a seperate window */
/* #define ARCHDEP_SEPERATE_MONITOR_WINDOW */

/* missing in some djgpp devs */
extern int fseeko(FILE *file, off_t offset, int x);
extern off_t ftello(FILE *file);

#endif
