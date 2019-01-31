/*
 * statusbar.c
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

#include "vice.h"

#include <string.h>

#include "private.h"
#include "statusbar.h"
#include "lib.h"
#include "ui.h"
#include "util.h"
#include "datasette.h"
#include "intl.h"
#include "translate.h"

#include <proto/diskfont.h>

#define _Move(rp, x, y) Move(rp,basex + (x), basey + (y))
#define _Draw(rp, x, y) Draw(rp,basex + (x), basey + (y))
#define _RectFill(rp, x1, y1, x2, y2) RectFill(rp, basex + (x1), basey + (y1), basex + (x2), basey + (y2))

#define LINE(x1, y1, x2, y2) do {                              \
                                 _Move(window->RPort, x1, y1); \
                                 _Draw(window->RPort, x2, y2); \
                                 } while (0)

static int enabled_drives = 0;
static ui_drive_enable_t status_enabled = UI_DRIVE_ENABLE_NONE;
static int status_led[DRIVE_NUM];
/* Translate from window index -> drive index */
static int status_map[DRIVE_NUM];
static double status_track[DRIVE_NUM];
static int *drive_active_led;

static int statusbar_statustext_time=0;

#define TAPE_NUM (DRIVE_NUM)

static int tape_enabled = 0;
static int tape_motor;
static int tape_counter;
static int tape_control;

#define b_green (0)
#define b_red (1)
#define b_black (2)
#define b_yellow (3)
#define b_grey (4)
#define b_num (5)

struct {
    int red, green, blue;
} palette[b_num] = {
    { 0, 255, 0 },
    { 255, 0, 0 },
    { 0, 0, 0 },
    { 255, 255, 0 },
    { 128, 128, 128 },
};

const struct TextAttr led_font_attr = {
#ifdef AMIGA_MORPHOS
    "XHelvetica.font", 11, FS_NORMAL, 0
#else
    "Helvetica.font", 11, FS_NORMAL, 0
#endif
};

int statusbar_create(video_canvas_t *canvas)
{
    int i;

    canvas->os->font = OpenDiskFont((struct TextAttr *)&led_font_attr);
    if (canvas->os->font) {
        SetFont(canvas->os->window->RPort, canvas->os->font);
    }
    /* else we use system default font */

    for (i = 0; i < b_num; i++) {
#ifndef AMIGAOS4
        canvas->os->pens[i] = ObtainBestPen(canvas->os->window->WScreen->ViewPort.ColorMap, palette[i].red * 0x01010101, palette[i].green * 0x01010101, palette[i].blue * 0x01010101, TAG_DONE);
#else
        canvas->os->pens[i] = ObtainBestPen(canvas->os->window->WScreen->ViewPort.ColorMap, palette[i].red * 0x01010101, palette[i].green * 0x01010101, palette[i].blue * 0x01010101);
#endif
    }

#define MAXSTR "00: 00.0"
    canvas->os->disk_width = TextLength(canvas->os->window->RPort, MAXSTR, strlen(MAXSTR)) + 14 + 7;
#undef MAXSTR

#define MAXSTR "000"
    canvas->os->tape_width = TextLength(canvas->os->window->RPort, MAXSTR, strlen(MAXSTR)) + 28 + 7 ;
#undef MAXSTR

    canvas->os->has_statusbar = 1;

    statusbar_refresh(REFRESH_ALL);

    return 0;
}

void statusbar_destroy(video_canvas_t *canvas)
{
    int i;

    for (i = 0; i < b_num; i++) {
        if (canvas->os->pens[i] != -1) {
            ReleasePen(canvas->os->window->WScreen->ViewPort.ColorMap, canvas->os->pens[i]);
            canvas->os->pens[i] = -1;
        }
    }
    if (canvas->os->font != NULL) {
        CloseFont(canvas->os->font);
        canvas->os->font = NULL;
    }
    canvas->os->has_statusbar = 0;
}

int statusbar_get_status_height(void)
{
    return 11 + (2 * 4);
}

void statusbar_set_statustext(const char *text, int text_time)
{
    video_canvas_t *canvas;

    for (canvas = canvaslist; canvas; canvas = canvas->next) {
        struct Window *window;
        struct TextExtent te;
        int x, width, height, max_chars;
        int basex, basey;

        if (!canvas->os->has_statusbar || canvas->waiting_for_resize) {
            continue;
        }

        window = canvas->os->window;
        width = window->Width - window->BorderLeft - window->BorderRight;
        height = window->Height - window->BorderTop - window->BorderBottom - statusbar_get_status_height();
        basex = window->BorderLeft;
        basey = window->BorderTop;

        if (canvas->os->font) {
            SetFont(window->RPort, canvas->os->font);
        }
        /* else we use system default font */

        x = width - (canvas->os->disk_width * enabled_drives) - (tape_enabled ? canvas->os->tape_width : 0);

        SetAPen(window->RPort, 0);
        SetBPen(window->RPort, 0);
        _RectFill(window->RPort, 0, height, x - 1, height + statusbar_get_status_height() - 1);

        max_chars = TextFit(window->RPort, text, strlen(text), &te, NULL, 1, x - (7 * 2), 20);

        SetAPen(window->RPort, canvas->os->pens[b_black]);
        _Move(window->RPort, 7, height + 13);
        Text(window->RPort, text, max_chars /* strlen(text) */);
    }
    statusbar_statustext_time = text_time;
}

void statusbar_statustext_update(void)
{
    if (statusbar_statustext_time > 0) {
        statusbar_statustext_time--;
        if (statusbar_statustext_time == 0) {
            statusbar_set_statustext("", 0);
        }
    }
}

void statusbar_refresh(int drive_number)
{
    video_canvas_t *canvas;
    int i;

    if (drive_number == REFRESH_ALL) {
        enabled_drives = 0;
        for (i=0; i<DRIVE_NUM; i++) {
            if (status_enabled & (1 << i)) {
                status_map[enabled_drives++] = i;
            }
        }
    }

    for (canvas = canvaslist; canvas; canvas = canvas->next) {
        struct Window *window;
        char str[32];
        int x, width, height;
        int basex, basey;

        if (!canvas->os->has_statusbar || canvas->waiting_for_resize) {
            continue;
        }

        window = canvas->os->window;
        width = window->Width - window->BorderLeft - window->BorderRight;
        height = window->Height - window->BorderTop - window->BorderBottom - statusbar_get_status_height();
        basex = window->BorderLeft;
        basey = window->BorderTop;

        if (canvas->os->font) {
            SetFont(window->RPort, canvas->os->font);
        }
        /* else we use system default font */

        if (drive_number == REFRESH_ALL) {
            x = width - (canvas->os->disk_width * enabled_drives) - (tape_enabled ? canvas->os->tape_width : 0);
            SetAPen(window->RPort, 0);
            SetBPen(window->RPort, 0);
            _RectFill(window->RPort, 0, height, x - 1, height + statusbar_get_status_height() - 1);
        }

        for (i = 0; i < enabled_drives; i++) {
            int drive = status_map[i];

            if ((drive_number != REFRESH_ALL) && (drive_number != drive)) {
                continue;
            }

            sprintf(str, "%2d: %2d.%01d", drive + 8, (int)status_track[drive], (int)(status_track[drive] * 10 - (int)status_track[drive] * 10));

            x = width - (canvas->os->disk_width * (enabled_drives - i));

            SetAPen(window->RPort, 0);
            SetBPen(window->RPort, 0);
            _RectFill(window->RPort, x, height, x + canvas->os->disk_width - 1, height + statusbar_get_status_height() - 1);

            SetAPen(window->RPort, canvas->os->pens[b_black]);
            _Move(window->RPort, x, height + 13);
            Text(window->RPort, str, strlen(str));
            x += canvas->os->disk_width - (10 + 7);

            SetAPen(window->RPort, canvas->os->pens[(status_led[drive] ? (drive_active_led[drive] ? b_green : b_red) : b_black)]);
            _RectFill(window->RPort, x, height + 4, x + 10, height + 14);
        }

        if ((tape_enabled) && ((drive_number == REFRESH_ALL) || (drive_number == TAPE_NUM))) {
            int motor_color;
            int record_led;

            /* motor */
            if (tape_motor) {
                motor_color = b_yellow;
            } else {
                motor_color = b_grey;
            }

            x = width - (canvas->os->disk_width * enabled_drives) - (tape_enabled ? canvas->os->tape_width : 0);

            SetAPen(window->RPort, 0);
            SetBPen(window->RPort, 0);
            _RectFill(window->RPort, x, height, x + canvas->os->tape_width - 1, height + statusbar_get_status_height() - 1);

            SetAPen(window->RPort, canvas->os->pens[motor_color]);
            _RectFill(window->RPort, x, height + 4, x + 10, height + 14);

            /* control */
            record_led = b_black;
            switch (tape_control) {
                case DATASETTE_CONTROL_STOP:
                    SetAPen(window->RPort, canvas->os->pens[b_black]);
                    _RectFill(window->RPort, x + 2, height + 4 + 2, x + 10 - 2, height + 14 - 2);
                    break;
                case DATASETTE_CONTROL_RECORD:
                    record_led = b_red;
                case DATASETTE_CONTROL_START:
                    SetAPen(window->RPort, canvas->os->pens[b_black]);
                    LINE(x + 2, height + 4 + 2, x + 2, height + 14 - 2);
                    LINE(x + 3, height + 4 + 3, x + 3, height + 14 - 3);
                    LINE(x + 4, height + 4 + 4, x + 4, height + 14 - 4);
                    LINE(x + 5, height + 4 + 5, x + 5, height + 14 - 5);
                    break;
                case DATASETTE_CONTROL_REWIND:
                    SetAPen(window->RPort, canvas->os->pens[b_black]);
                    LINE(x + 5, height + 4 + 2, x + 2, height + 4 + 5);
                    LINE(x + 2, height + 4 + 5, x + 5, height + 14 - 2);
                    LINE(x + 5 + 3, height + 4 + 2, x + 2 + 3, height + 4 + 5);
                    LINE(x + 2 + 3, height + 4 + 5, x + 5 + 3, height + 14 - 2);
                    break;
                case DATASETTE_CONTROL_FORWARD:
                    SetAPen(window->RPort, canvas->os->pens[b_black]);
                    LINE(x + 2, height + 4 + 2, x + 5, height + 4 + 5);
                    LINE(x + 5, height + 4 + 5, x + 2, height + 14 - 2);
                    LINE(x + 2 + 3, height + 4 + 2, x + 5 + 3, height + 4 + 5);
                    LINE(x + 5 + 3, height + 4 + 5, x + 2 + 3, height + 14 - 2);
                    break;
            }
            SetAPen(window->RPort, canvas->os->pens[record_led]);
            _RectFill(window->RPort, x + 2 + 14, height + 4 + 2, x + 10 - 2 + 14, height + 14 - 2);

            /* counter */
            sprintf(str, "%03d", tape_counter);
            SetAPen(window->RPort, canvas->os->pens[b_black]);
            _Move(window->RPort, x + 28, height + 13);
            Text(window->RPort, str, strlen(str));
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Dispay the drive status.  */
void ui_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color)
{
    status_enabled = enable;
    drive_active_led = drive_led_color;
    statusbar_refresh(REFRESH_ALL);
}

/* Toggle displaying of the drive track.  */
/* drive_base is either 8 or 0 depending on unit or drive display.
   Dual drives display drive 0: and 1: instead of unit 8: and 9: */
void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base, unsigned int half_track_number)
{
    double track_number = (double)half_track_number / 2.0;

    status_track[drive_number] = track_number;

    statusbar_refresh(drive_number);
}

/* Toggle displaying of the drive LED.  */
void ui_display_drive_led(int drivenum, unsigned int led_pwm1,
                          unsigned int led_pwm2)
{
    int status = 0;

    if (led_pwm1 > 100) {
        status |= 1;
    }
    if (led_pwm2 > 100) {
        status |= 2;
    }

    status_led[drivenum] = status;

    statusbar_refresh(drivenum);
}

/* display current image */
void ui_display_drive_current_image(unsigned int drivenum, const char *image)
{
    char *directory_name, *image_name;
    char text[200];

    if (image == NULL || image[0] == 0) {
        sprintf(text, translate_text(IDS_DETACHED_DEVICE_D), drivenum + 8);
    } else {
        util_fname_split(image, &directory_name, &image_name);
        sprintf(text, translate_text(IDS_ATTACHED_S_TO_DEVICE_D), image_name, drivenum + 8);
        lib_free(image_name);
        lib_free(directory_name);
    }

    statusbar_set_statustext(text, 5);
}

int ui_extend_image_dialog(void)
{
    return ui_requester(translate_text(IDS_VICE_QUESTION), translate_text(IDS_EXTEND_TO_40_TRACK), translate_text(IDS_YES_NO), 0);
}

/* ------------------------------------------------------------------------- */
/* Dispay the tape status.  */
void ui_set_tape_status(int tape_status)
{
    tape_enabled = tape_status;

    statusbar_refresh(TAPE_NUM);
}

void ui_display_tape_motor_status(int motor)
{
    tape_motor = motor;

    statusbar_refresh(TAPE_NUM);
}

void ui_display_tape_control_status(int control)
{
    tape_control = control;

    statusbar_refresh(TAPE_NUM);
}

void ui_display_tape_counter(int counter)
{
    if (counter != tape_counter) {
        tape_counter = counter;
        statusbar_refresh(TAPE_NUM);
    }
}

/* display the attched tape image */
void ui_display_tape_current_image(const char *image)
{
    char *directory_name, *image_name;
    char text[200];

    if (image == NULL || image[0] == 0) {
        sprintf(text, "%s", translate_text(IDS_DETACHED_TAPE));
    } else {
        util_fname_split(image, &directory_name, &image_name);
        sprintf(text, translate_text(IDS_ATTACHED_TAPE_S), image_name);
        lib_free(image_name);
        lib_free(directory_name);
    }

    statusbar_set_statustext(text, 5);
}
