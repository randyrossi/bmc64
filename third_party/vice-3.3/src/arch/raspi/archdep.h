/*
 * archdep.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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

#include <dirent.h>
#include <sys/types.h>

#define VICE_ARCHAPI_PRIVATE_API
#include "archapi.h"
#undef VICE_ARCHAPI_PRIVATE_API

#include "sound.h"

/* Filesystem dependant operators.  */
#define FSDEVICE_DEFAULT_DIR "."
#define FSDEV_DIR_SEP_STR "/"
#define FSDEV_DIR_SEP_CHR '/'
#define FSDEV_EXT_SEP_STR "."
#define FSDEV_EXT_SEP_CHR '.'

/* Path separator.  */
#define ARCHDEP_FINDPATH_SEPARATOR_CHAR ':'
#define ARCHDEP_FINDPATH_SEPARATOR_STRING ":"

/* Modes for fopen().  */
#define MODE_READ "r"
#define MODE_READ_TEXT "r"
#define MODE_READ_WRITE "r+"
#define MODE_WRITE "w"
#define MODE_WRITE_TEXT "w"
#define MODE_APPEND "a"
#define MODE_APPEND_READ_WRITE "a+"

/* Printer default devices.  */
#define ARCHDEP_PRINTER_DEFAULT_DEV1 "print.dump"
#define ARCHDEP_PRINTER_DEFAULT_DEV2 "|lpr"
#define ARCHDEP_PRINTER_DEFAULT_DEV3 "|petlp -F PS|lpr"

/* Video chip scaling.  */
#define ARCHDEP_VICII_DSIZE 1
#define ARCHDEP_VICII_DSCAN 1
#define ARCHDEP_VICII_HWSCALE 1
#define ARCHDEP_VDC_DSIZE 1
#define ARCHDEP_VDC_DSCAN 1
#define ARCHDEP_VDC_HWSCALE 1
#define ARCHDEP_VIC_DSIZE 1
#define ARCHDEP_VIC_DSCAN 1
#define ARCHDEP_VIC_HWSCALE 1
#define ARCHDEP_CRTC_DSIZE 1
#define ARCHDEP_CRTC_DSCAN 1
#define ARCHDEP_CRTC_HWSCALE 1
#define ARCHDEP_TED_DSIZE 1
#define ARCHDEP_TED_DSCAN 1
#define ARCHDEP_TED_HWSCALE 1

/* Video chip double buffering.  */
#define ARCHDEP_VICII_DBUF 0
#define ARCHDEP_VDC_DBUF 0
#define ARCHDEP_VIC_DBUF 0
#define ARCHDEP_CRTC_DBUF 0
#define ARCHDEP_TED_DBUF 0

/* Default RS232 devices.  */
#define ARCHDEP_RS232_DEV1 "/dev/ttyS0"
#define ARCHDEP_RS232_DEV2 "/dev/ttyS1"
#define ARCHDEP_RS232_DEV3 "rs232.dump"
#define ARCHDEP_RS232_DEV4 "|lpr"

/* Default MIDI devices.  */
#define ARCHDEP_MIDI_IN_DEV "/dev/midi"
#define ARCHDEP_MIDI_OUT_DEV "/dev/midi"

/* Default location of raw disk images.  */
#define ARCHDEP_RAWDRIVE_DEFAULT "/dev/fd0"

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
#define ARCHDEP_SOUND_FRAGMENT_SIZE 1

/* No key symcode.  */
#define ARCHDEP_KEYBOARD_SYM_NONE 0

extern const char *archdep_home_path(void);

/* set this path to customize the preference storage */
extern const char *archdep_pref_path;

#define LIBDIR VICEDIR

#define DOCDIR LIBDIR "/doc"

#define VICEUSERDIR ".vice"

/* add second level macro to allow expansion and stringification */
#define ARCHDEP_MAKE_SO_NAME_VERSION(n, v) MAKE_SO_NAME_VERSION_PROTO(n, v)

/* what to use to return an error when a socket error happens */
#define ARCHDEP_SOCKET_ERROR errno

/* Default sound output mode */
#define ARCHDEP_SOUND_OUTPUT_MODE SOUND_OUTPUT_SYSTEM

/* Keyword to use for a static prototype */
#define STATIC_PROTOTYPE static

/* define if the platform supports the monitor in a seperate window */
#define ARCHDEP_SEPERATE_MONITOR_WINDOW

int kbd_arch_get_host_mapping(void);

#endif
