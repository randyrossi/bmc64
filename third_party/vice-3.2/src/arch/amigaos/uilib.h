/*
 * uilib.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#define UILIB_SELECTOR_TYPE_FILE_LOAD 0
#define UILIB_SELECTOR_TYPE_FILE_SAVE 1
#define UILIB_SELECTOR_TYPE_DIR_EXIST 2
#define UILIB_SELECTOR_TYPE_DIR_NEW   3

#define UILIB_SELECTOR_STYLE_DEFAULT       0
#define UILIB_SELECTOR_STYLE_TAPE          1
#define UILIB_SELECTOR_STYLE_DISK          2
#define UILIB_SELECTOR_STYLE_DISK_AND_TAPE 3
#define UILIB_SELECTOR_STYLE_CART          4
#define UILIB_SELECTOR_STYLE_SNAPSHOT      5
#define UILIB_SELECTOR_STYLE_EVENT_START   6
#define UILIB_SELECTOR_STYLE_EVENT_END     7
#define UILIB_SELECTOR_STYLES_NUM          8

#define UILIB_FILTER_ALL            (1 << 0)
#define UILIB_FILTER_PALETTE        (1 << 1)
#define UILIB_FILTER_SNAPSHOT       (1 << 2)
#define UILIB_FILTER_DISK           (1 << 3)
#define UILIB_FILTER_TAPE           (1 << 4)
#define UILIB_FILTER_ZIP            (1 << 5)
#define UILIB_FILTER_CRT            (1 << 6)
#define UILIB_FILTER_BIN            (1 << 7)
#define UILIB_FILTER_FLIPLIST       (1 << 8)
#define UILIB_FILTER_ROMSET_FILE    (1 << 9)
#define UILIB_FILTER_ROMSET_ARCHIVE (1 << 10)
#define UILIB_FILTER_KEYMAP         (1 << 11)
#define UILIB_FILTER_LENGTH_MAX     (1 << 12)

extern char *uilib_select_file(const char *title, unsigned int filterlist,
                               unsigned int type, int style);

extern char *uilib_select_file_autostart(const char *title,
                                         unsigned int filterlist, unsigned int type,
                                         int style, int *autostart,
                                         char *resource_readonly);

extern void ui_show_text(const char *szCaption,
                         const char *szHeader, const char *szText);

extern int ui_messagebox(const char *lpText, const char *lpCaption, unsigned int uType);

#endif
