/*
 * uilib.h
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

#ifndef VICE_UILIB_H
#define VICE_UILIB_H

/* this must be in sync with x11/gnome/uifileentry.c and x11/xaw/x11ui.c */
typedef enum uilib_file_filter_enum_s {
    UILIB_FILTER_ALL,
    UILIB_FILTER_PALETTE,
    UILIB_FILTER_SNAPSHOT,
    UILIB_FILTER_DISK,
    UILIB_FILTER_TAPE,
    UILIB_FILTER_CARTRIDGE,
    UILIB_FILTER_CRT_CARTRIDGE,
    UILIB_FILTER_FLIPLIST,
    UILIB_FILTER_ROMSET_FILE,
    UILIB_FILTER_ROMSET_ARCHIVE,
    UILIB_FILTER_KEYMAP,
    UILIB_FILTER_PRGP00,
    UILIB_FILTER_WAV,
    UILIB_FILTER_VOC,
    UILIB_FILTER_IFF,
    UILIB_FILTER_AIFF,
    UILIB_FILTER_MP3,
    UILIB_FILTER_FLAC,
    UILIB_FILTER_VORBIS,
    UILIB_FILTER_SERIAL,
    UILIB_FILTER_VIC20CART,
    UILIB_FILTER_SID,
    UILIB_FILTER_DTVROM,
    UILIB_FILTER_COMPRESSED,
    UILIB_FILTER_ETH,
    UILIB_FILTER_MIDI,
    UILIB_FILTER_HD_IMAGE
} uilib_file_filter_enum_t;

#include "ui.h"

extern void uilib_select_string(const char *resname, const char *title, const char *name);
extern void uilib_select_file(const char *resname, const char *title, uilib_file_filter_enum_t filter);
extern ui_button_t uilib_change_dir(const char *title, const char *prompt, char *buf, unsigned int buflen);
extern void uilib_select_dir(const char *resname, const char *title, const char *name);
extern void uilib_select_dev(const char *resource, const char *title, uilib_file_filter_enum_t filter);
extern ui_button_t uilib_input_file(const char *title, const char *prompt, char *buf, unsigned int buflen);

#endif
