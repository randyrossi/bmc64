/*
 * uiattach.c - Implementation of the disk/tape attach dialog box.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Tibor Biczo <crown@mail.matav.hu>
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
#include <windows.h>
#include <tchar.h>

#include "attach.h"
#include "autostart.h"
#include "fullscrn.h"
#include "intl.h"
#include "lib.h"
#include "res.h"
#include "system.h"
#include "tape.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uilib.h"

static void uiattach_disk_dialog(HWND hwnd, WPARAM wparam)
{
    TCHAR *st_title;
    char *name;
    char *resource;
    int unit = 8;
    int autostart_index = -1;

    SuspendFullscreenModeKeep(hwnd);

    st_title = intl_translate_tcs(IDS_ATTACH_DISK_IMAGE);

    switch (wparam & 0xffff) {
        case IDM_ATTACH_8:
            unit = 8;
            break;
        case IDM_ATTACH_9:
            unit = 9;
            break;
        case IDM_ATTACH_10:
            unit = 10;
            break;
        case IDM_ATTACH_11:
            unit = 11;
            break;
    }
    resource = lib_msprintf("AttachDevice%dReadonly", unit);
    if ((name = uilib_select_file_autostart(hwnd,
                                               st_title,
                                               UILIB_FILTER_DISK |
#ifdef HAVE_ZLIB
                                               UILIB_FILTER_ZIP |
#endif
                                               UILIB_FILTER_ALL,
                                               UILIB_SELECTOR_TYPE_FILE_LOAD,
                                               UILIB_SELECTOR_STYLE_DISK,
                                               &autostart_index, resource)) != NULL) {
        if (autostart_index >= 0) {
            if (autostart_autodetect(name, NULL, autostart_index, AUTOSTART_MODE_RUN) < 0) {
                ui_error(translate_text(IDS_CANNOT_AUTOSTART_FILE));
            }
        } else {
            if (file_system_attach_disk(unit, name) < 0) {
                ui_error(translate_text(IDS_CANNOT_ATTACH_FILE));
            }
        }
        lib_free(name);
    }
    ResumeFullscreenModeKeep(hwnd);
    lib_free(resource);
}

static void uiattach_tape_dialog(HWND hwnd)
{
    TCHAR *st_title;
    char *name;
    int autostart_index = -1;

    SuspendFullscreenModeKeep(hwnd);

    st_title = intl_translate_tcs(IDS_ATTACH_TAPE_IMAGE);

    if ((name = uilib_select_file_autostart(hwnd,
                                            st_title,
                                            UILIB_FILTER_TAPE |
#ifdef HAVE_ZLIB
                                            UILIB_FILTER_ZIP |
#endif
                                            UILIB_FILTER_ALL,
                                            UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_TAPE,
                                            &autostart_index, NULL)) != NULL) {
        if (autostart_index >= 0) {
            if (autostart_autodetect(name, NULL, autostart_index, AUTOSTART_MODE_RUN) < 0) {
                ui_error(translate_text(IDS_CANNOT_AUTOSTART_FILE));
            }
        } else {
            if (tape_image_attach(1, name) < 0) {
                ui_error(translate_text(IDS_CANNOT_ATTACH_FILE));
            }
        }
        lib_free(name);
    }
    ResumeFullscreenModeKeep(hwnd);
}

static void uiattach_autostart_dialog(HWND hwnd)
{
    TCHAR *st_title;
    char *name;
    int autostart_index = 0;
    char *resource;

    st_title = intl_translate_tcs(IDS_AUTOSTART_IMAGE);

    resource = lib_msprintf("AttachDevice%dReadonly", 8);
    if ((name = uilib_select_file_autostart(hwnd,
                                            st_title,
                                            UILIB_FILTER_CBM |
                                            UILIB_FILTER_DISK |
                                            UILIB_FILTER_TAPE |
#ifdef HAVE_ZLIB
                                            UILIB_FILTER_ZIP |
#endif
                                            UILIB_FILTER_PRGP00 |
                                            UILIB_FILTER_ALL,
                                            UILIB_SELECTOR_TYPE_FILE_LOAD,
                                            UILIB_SELECTOR_STYLE_DISK_AND_TAPE,
                                            &autostart_index, resource)) != NULL) {
        if (autostart_autodetect(name, NULL, autostart_index, AUTOSTART_MODE_RUN) < 0) {
            ui_error(translate_text(IDS_CANNOT_AUTOSTART_FILE));
        }
        lib_free(name);
    }
    lib_free(resource);
}

void uiattach_command(HWND hwnd, WPARAM wparam)
{
    switch (wparam & 0xffff) {
        case IDM_ATTACH_8:
        case IDM_ATTACH_9:
        case IDM_ATTACH_10:
        case IDM_ATTACH_11:
            uiattach_disk_dialog(hwnd, wparam);
            break;
        case IDM_DETACH_8:
            file_system_detach_disk(8);
            break;
        case IDM_DETACH_9:
            file_system_detach_disk(9);
            break;
        case IDM_DETACH_10:
            file_system_detach_disk(10);
            break;
        case IDM_DETACH_11:
            file_system_detach_disk(11);
            break;
        case IDM_DETACH_ALL:
            file_system_detach_disk(8);
            file_system_detach_disk(9);
            file_system_detach_disk(10);
            file_system_detach_disk(11);
            break;
        case IDM_ATTACH_TAPE:
            uiattach_tape_dialog(hwnd);
            break;
        case IDM_DETACH_TAPE:
            tape_image_detach(1);
            break;
        case IDM_AUTOSTART:
            uiattach_autostart_dialog(hwnd);
            break;
    }
}
