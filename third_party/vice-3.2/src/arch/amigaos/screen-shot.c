/*
 * screenshot.c
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

#include "vice.h"

#define __USE_INLINE__

#ifdef AMIGA_OS4
#define ASL_PRE_V38_NAMES
#endif

#include <string.h>

#include "private.h"
#include "lib.h"
#include "ui.h"
#include "screenshot.h"
#include "intl.h"
#include "translate.h"

enum {
    SS_BMP=1,
    SS_DOODLE,
    SS_DOODLE_COMPRESSED,
    SS_KOALA,
    SS_KOALA_COMPRESSED,
#ifdef HAVE_GIF
    SS_GIF,
#endif
    SS_IFF,
#ifdef HAVE_JPEG
    SS_JPG,
#endif
    SS_PCX,
#ifdef HAVE_PNG
    SS_PNG,
#endif
    SS_PPM,
    SS_4BT
};

#ifndef HAVE_GIF
#define SS_GIF 1000
#endif

#ifndef HAVE_JPEG
#define SS_JPG 1001
#endif

#ifndef HAVE_PNG
#define SS_PNG 1002
#endif

static void save_screenshot_file(char *pattern, char *screenshot_type, video_canvas_t *canvas)
{
    struct FileRequester *request;
    char fname[1024] = "";
    char *save_text;
    char title[100];

    sprintf(title, translate_text(IDS_SAVE_S_SCREENSHOT), screenshot_type);

    request = (struct FileRequester *)AllocAslRequestTags(ASL_FileRequest, ASL_Hail, title, (struct TagItem *)TAG_DONE);

    save_text = translate_text(IDS_SAVE);

    if (AslRequestTags(request, ASLFR_Window, canvas->os->window,
                                ASLFR_InitialDrawer, "PROGDIR:",
                                ASLFR_InitialPattern, pattern,
                                ASLFR_PositiveText, save_text,
                                (struct TagItem *)TAG_DONE)) {
        strcat(fname,request->rf_Dir);
        if (fname[strlen(fname) - 1] != (UBYTE)58) {
            strcat(fname, "/");
        }
        strcat(fname, request->rf_File);
        if (fname[strlen(fname) - 1] != (UBYTE)58 && fname[strlen(fname) - 1] != '/') {
            if (strcasecmp(fname + (strlen(fname) - 4), pattern + 2)) {
                strcat(fname, pattern+2);
            }
            if (screenshot_save(screenshot_type, fname, canvas) < 0) {
                ui_error(translate_text(IDMES_SCREENSHOT_SAVE_S_FAILED), fname);
            } else {
                ui_message(translate_text(IDMES_SCREENSHOT_S_SAVED), fname);
            }
        }
    }
    if (request) {
        FreeAslRequest(request);
    }
}

void ui_screenshot_dialog(video_canvas_t *canvas)
{
    int format;
    char choices[100] = "";

    strcat(choices, "BMP|");
    strcat(choices, "DOODLE|");
    strcat(choices, "DOODLE_COMPRESSED|");
    strcat(choices, "KOALA|");
    strcat(choices, "KOALA_COMPRESSED|");
#ifdef HAVE_GIF
    strcat(choices, "GIF|");
#endif
    strcat(choices, "IFF|");
#ifdef HAVE_JPEG
    strcat(choices, "JPG|");
#endif
    strcat(choices, "PCX|");
#ifdef HAVE_PNG
    strcat(choices, "PNG|");
#endif
    strcat(choices, "PPM|");
    strcat(choices, "4BT|");
    strcat(choices, translate_text(IDS_CANCEL));

    format = ui_requester(translate_text(IDS_SAVE_SCREENSHOT), translate_text(IDS_CHOOSE_SCREENSHOT_FORMAT), choices, 0);

    switch (format) {
        case SS_BMP:
            save_screenshot_file("#?.bmp", "BMP", canvas);
            break;
        case SS_DOODLE:
            save_screenshot_file("#?.dd", "DOODLE", canvas);
            break;
        case SS_DOODLE_COMPRESSED:
            save_screenshot_file("#?.jj", "DOODLE_COMPRESSED", canvas);
            break;
        case SS_KOALA:
            save_screenshot_file("#?.koa", "KOALA", canvas);
            break;
        case SS_KOALA_COMPRESSED:
            save_screenshot_file("#?.gg", "KOALA_COMPRESSED", canvas);
            break;
#ifdef HAVE_GIF
        case SS_GIF:
            save_screenshot_file("#?.gif", "GIF", canvas);
            break;
#endif
        case SS_IFF:
            save_screenshot_file("#?.iff", "IFF", canvas);
            break;
#ifdef HAVE_JPEG
        case SS_JPG:
            save_screenshot_file("#?.jpg", "JPEG", canvas);
            break;
#endif
        case SS_PCX:
            save_screenshot_file("#?.pcx", "PCX", canvas);
            break;
#ifdef HAVE_PNG
        case SS_PNG:
            save_screenshot_file("#?.png", "PNG", canvas);
            break;
#endif
        case SS_PPM:
            save_screenshot_file("#?.ppm", "PPM", canvas);
            break;
        case SS_4BT:
            save_screenshot_file("#?.4bt", "4BT", canvas);
            break;
        default:
            break;
    }
}
