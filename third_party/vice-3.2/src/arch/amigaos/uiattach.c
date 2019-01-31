/*
 * uiattach.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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
#include "private.h"

#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif

#include <stdio.h>

#include "attach.h"
#include "autostart.h"
#include "intl.h"
#include "lib.h"
#include "tape.h"
#include "translate.h"
#include "ui.h"
#include "uiattach.h"
#include "uilib.h"
#include "uires.h"
#include "mui/mui.h"

#ifdef AMIGA_AROS
void uiattach_aros(video_canvas_t *canvas, int unit)
{
    char *fname = NULL;
    char select_txt[50];

    if (unit == 1) {
        fname = BrowseFile("Select file for tape","#?", canvas);
        if (fname != NULL) {
            if (tape_image_attach(1, fname) < 0) {
                ui_error(translate_text(IDMES_CANNOT_ATTACH_FILE));
            }
        }
    } else {
        sprintf(select_txt, "Select file for unit %d", unit);
        fname = BrowseFile(select_txt, "#?", canvas);
        if (fname != NULL) {
            if (file_system_attach_disk(unit, fname) < 0) {
                ui_error(translate_text(IDMES_CANNOT_ATTACH_FILE));
            }
        }
    }
}
#endif

static void uiattach_disk_dialog(video_canvas_t *canvas, int idm)
{
    char *name;
    char *resource;
    int unit = 8;
    int autostart_index = -1;

    switch (idm) {
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
    if ((name = uilib_select_file_autostart(translate_text(IDMS_ATTACH_DISK_IMAGE),
                                            UILIB_FILTER_DISK | UILIB_FILTER_ZIP | UILIB_FILTER_ALL,
                                            UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DISK,
                                            &autostart_index, resource)) != NULL) {
        if (autostart_index >= 0) {
            if (autostart_autodetect(name, NULL, autostart_index, AUTOSTART_MODE_RUN) < 0) {
                ui_error(translate_text(IDMES_CANNOT_AUTOSTART_FILE));
            }
        } else {
            if (file_system_attach_disk(unit, name) < 0) {
                ui_error(translate_text(IDMES_CANNOT_ATTACH_FILE));
            }
        }
        lib_free(name);
    }
    lib_free(resource);
}

static void uiattach_tape_dialog(video_canvas_t *canvas)
{
    char *name;
    int autostart_index = -1;

    if ((name = uilib_select_file_autostart(translate_text(IDS_ATTACH_TAPE_IMAGE),
                                            UILIB_FILTER_TAPE | UILIB_FILTER_ZIP | UILIB_FILTER_ALL,
                                            UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_TAPE,
                                            &autostart_index, NULL)) != NULL) {
        if (autostart_index >= 0) {
            if (autostart_autodetect(name, NULL, autostart_index, AUTOSTART_MODE_RUN) < 0) {
                ui_error(translate_text(IDMES_CANNOT_AUTOSTART_FILE));
            }
        } else {
            if (tape_image_attach(1, name) < 0) {
                ui_error(translate_text(IDMES_CANNOT_ATTACH_FILE));
            }
        }
        lib_free(name);
    }
}

static void uiattach_autostart_dialog(video_canvas_t *canvas)
{
    char *name;
    int autostart_index = 0;

    if ((name = uilib_select_file_autostart(translate_text(IDS_AUTOSTART_IMAGE),
                                            UILIB_FILTER_DISK | UILIB_FILTER_TAPE | UILIB_FILTER_ZIP | UILIB_FILTER_ALL,
                                            UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DISK_AND_TAPE,
                                            &autostart_index, NULL)) != NULL) {
        if (autostart_autodetect(name, NULL, autostart_index, AUTOSTART_MODE_RUN) < 0) {
            ui_error(translate_text(IDMES_CANNOT_AUTOSTART_FILE));
        }
        lib_free(name);
    }
}

void uiattach_command(video_canvas_t *canvas, int idm)
{
    switch (idm) {
        case IDM_ATTACH_8:
        case IDM_ATTACH_9:
        case IDM_ATTACH_10:
        case IDM_ATTACH_11:
            uiattach_disk_dialog(canvas, idm);
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
            uiattach_tape_dialog(canvas);
            break;
        case IDM_DETACH_TAPE:
            tape_image_detach(1);
            break;
        case IDM_AUTOSTART:
            uiattach_autostart_dialog(canvas);
            break;
    }
}
