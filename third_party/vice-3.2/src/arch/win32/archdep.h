/*
 * archdep.h - Architecture dependant defines.
 *
 * Written by
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

#ifndef VICE_ARCHDEP_H
#define VICE_ARCHDEP_H

#include "archapi.h"

#include "sound.h"

#include "types.h"

/* Filesystem dependant operators.  */
#define FSDEVICE_DEFAULT_DIR "."
#define FSDEV_DIR_SEP_STR    "\\"
#define FSDEV_DIR_SEP_CHR    '\\'
#define FSDEV_EXT_SEP_STR    "."
#define FSDEV_EXT_SEP_CHR    '.'

/* Path separator.  */
#define ARCHDEP_FINDPATH_SEPARATOR_CHAR   ';'
#define ARCHDEP_FINDPATH_SEPARATOR_STRING ";"

/* Modes for fopen().  */
#define MODE_READ              "r"
#define MODE_READ_TEXT         "rt"
#define MODE_READ_WRITE        "r+"
#define MODE_WRITE             "w"
#define MODE_WRITE_TEXT        "wt"
#define MODE_APPEND            "a"
#define MODE_APPEND_READ_WRITE "a+"

/* Printer default devices.  */
#define ARCHDEP_PRINTER_DEFAULT_DEV1 "viceprnt.out"
#define ARCHDEP_PRINTER_DEFAULT_DEV2 "LPT1:"
#define ARCHDEP_PRINTER_DEFAULT_DEV3 "LPT2:"

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

/* Default RS232 devices.  */
#define ARCHDEP_RS232_DEV1 "10.0.0.1:25232"
#define ARCHDEP_RS232_DEV2 "10.0.0.1:25232"
#define ARCHDEP_RS232_DEV3 "10.0.0.1:25232"
#define ARCHDEP_RS232_DEV4 "10.0.0.1:25232"

/* Default location of raw disk images.  */
#define ARCHDEP_RAWDRIVE_DEFAULT "A:"

/* Access types */
#define ARCHDEP_R_OK 4
#define ARCHDEP_W_OK 2
#define ARCHDEP_X_OK 1
#define ARCHDEP_F_OK 0

/* Standard line delimiter.  */
#define ARCHDEP_LINE_DELIMITER "\r\n"

/* Ethernet default device */
#define ARCHDEP_ETHERNET_DEFAULT_DEVICE ""

/* No key symcode.  */
#define ARCHDEP_KEYBOARD_SYM_NONE 0

/* Default sound fragment size */
#define ARCHDEP_SOUND_FRAGMENT_SIZE 2

extern void archdep_workaround_nop(const char *otto);
 
#define MAKE_SO_NAME_VERSION_PROTO(name, version)  #name "-" #version ".dll"

/* add second level macro to allow expansion and stringification */
#define ARCHDEP_MAKE_SO_NAME_VERSION(n, v) MAKE_SO_NAME_VERSION_PROTO(n, v)

#define ARCHDEP_OPENCBM_SO_NAME  "opencbm.dll"
#define ARCHDEP_LAME_SO_NAME     "lame.dll"

/* Define HAVE_GIF when not defined, and define USE_GIF_DLL */
#ifndef HAVE_GIF
#define HAVE_GIF
#define USE_GIF_DLL
#endif

/* ffmpeg headers for windows don't seem to have some of the av_ prefixes */
#define ARCHDEP_AV_PREFIX_NEEDED

/* Needs extra call to log_archdep() even when logfile is already opened */
#ifndef WATCOM_COMPILE
#define ARCHDEP_EXTRA_LOG_CALL
#endif

/* When using the ascii printer driver we need a return before the newline */
#define ARCHDEP_PRINTER_RETURN_BEFORE_NEWLINE

/* what to use to return an error when a socket error happens */
#define ARCHDEP_SOCKET_ERROR WSAGetLastError()

extern struct console_s *uimon_console_open_mdi(const char *id, void *,
                                                void *, void *,
                                                uint32_t dwStyle,
                                                int x, int y, int dx, int dy );

/* Default sound output mode */
#define ARCHDEP_SOUND_OUTPUT_MODE SOUND_OUTPUT_SYSTEM

/* Keyword to use for a static prototype */
#define STATIC_PROTOTYPE static

/* define if the platform supports the monitor in a seperate window */
#define ARCHDEP_SEPERATE_MONITOR_WINDOW

extern void vice_usleep(uint64_t waitTime);

extern int is_windows_nt(void);

#endif
