/*
 * uiffmpeg.c - FFMPEG video recording UI interface for MS-DOS.
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

#ifdef HAVE_FFMPEG

#include <stdio.h>
#include <stdlib.h>

#include "ffmpegdrv.h"
#include "gfxoutput.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "ui.h"
#include "uiffmpeg.h"
#include "util.h"
#include "videoarch.h"

static gfxoutputdrv_t *ffmpeg_drv = NULL;

#define MAX_FORMATS 7
#define MAX_CODECS 15

static tui_menu_item_def_t format_menu[MAX_FORMATS + 1];
static tui_menu_item_def_t video_codec_menu[MAX_CODECS + 1];
static tui_menu_item_def_t audio_codec_menu[MAX_CODECS + 1];

TUI_MENU_DEFINE_RADIO(FFMPEGVideoCodec)
TUI_MENU_DEFINE_RADIO(FFMPEGAudioCodec)

static TUI_MENU_CALLBACK(ui_set_video_bitrate_callback)
{
    if (been_activated) {
        int current_rate, value;
        char buf[10];
        char brmin[10];
        char brmax[10];
        char *message = NULL;

        resources_get_int("FFMPEGVideoBitrate", &current_rate);
        sprintf(buf, "%d", current_rate);

        sprintf(brmin, "%d", VICE_FFMPEG_VIDEO_RATE_MIN);
        sprintf(brmax, "%d", VICE_FFMPEG_VIDEO_RATE_MAX);

        message = util_concat("Enter the video bitrate (", brmin, "-", brmax, "):", NULL);

        if (tui_input_string(message, message, buf, 10) == 0) {
            value = atoi(buf);
            if (value > VICE_FFMPEG_VIDEO_RATE_MAX) {
                value = VICE_FFMPEG_VIDEO_RATE_MAX;
            } else if (value < VICE_FFMPEG_VIDEO_RATE_MIN) {
                value = VICE_FFMPEG_VIDEO_RATE_MIN;
            }
            resources_set_int("FFMPEGVideoBitrate", value);
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_set_audio_bitrate_callback)
{
    if (been_activated) {
        int current_rate, value;
        char buf[10];
        char brmin[10];
        char brmax[10];
        char *message = NULL;

        resources_get_int("FFMPEGAudioBitrate", &current_rate);
        sprintf(buf, "%d", current_rate);

        sprintf(brmin, "%d", VICE_FFMPEG_AUDIO_RATE_MIN);
        sprintf(brmax, "%d", VICE_FFMPEG_AUDIO_RATE_MAX);

        message = util_concat("Enter the audio bitrate (", brmin, "-", brmax, "):", NULL);

        if (tui_input_string(message, message, buf, 10) == 0) {
            value = atoi(buf);
            if (value > VICE_FFMPEG_AUDIO_RATE_MAX) {
                value = VICE_FFMPEG_AUDIO_RATE_MAX;
            } else if (value < VICE_FFMPEG_AUDIO_RATE_MIN) {
                value = VICE_FFMPEG_AUDIO_RATE_MIN;
            }
            resources_set_int("FFMPEGAudioBitrate", value);
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(save_movie_callback)
{
    char *name = NULL;
    char *tmp;

    if (been_activated) {
        const char *drv_name;

        if (ffmpegdrv_formatlist == NULL || ffmpegdrv_formatlist[0].name == NULL) {
            ui_error("FFMPEG not available.");
            return NULL;
        }

        ffmpeg_drv = gfxoutput_get_driver("FFMPEG");
        if (ffmpeg_drv == NULL) {
            ui_error("FFMPEG not available.");
            return NULL;
        }

        name = tui_file_selector("Choose movie file", NULL, "*", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            util_add_extension(&name, ffmpeg_drv->default_extension);
            if (screenshot_save("FFMPEG", name, last_canvas) < 0) {
                ui_error("Cannot save movie.");
            }
            lib_free(name);
        }
    }
    return NULL;
}

static tui_menu_item_def_t ffmpeg_menu_items[] = {
    { "Format:", "Select the format to use",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, format_menu, NULL },
    { "Video codec:", "Select the video codec to use",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, video_codec_menu, NULL },
    { "Audio codec:", "Select the audio codec to use",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, audio_codec_menu, NULL },
    { "Video bitrate",
      "Set the video bitrate",
      ui_set_video_bitrate_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Audio bitrate",
      "Set the audio bitrate",
      ui_set_audio_bitrate_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save movie",
      "Save the movie",
      save_movie_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static void update_codec_menus(const char *current_format)
{
    int i;
    gfxoutputdrv_format_t *format;
    gfxoutputdrv_codec_t *codec;

    int video_codec_id;
    int audio_codec_id;

    int codec_found;

    video_codec_menu[0].label = NULL;
    audio_codec_menu[0].label = NULL;

    if (ffmpegdrv_formatlist == NULL || ffmpegdrv_formatlist[0].name == NULL) {
        return;
    }

    format = NULL;
    for (i = 0; ffmpegdrv_formatlist[i].name != NULL; i++) {
        if (strcmp(ffmpegdrv_formatlist[i].name, current_format) == 0) {
            /* got current format */
            format = &(ffmpegdrv_formatlist[i]);
            break;
        }
    }

    if (!format) {
        return;
    }

    /* Update video codec menu */
    codec = format->video_codecs;
    i = 0;

    if (codec == NULL) {
        video_codec_menu[0].label = NULL;
    } else {

        /* get the currently used video codec */
        resources_get_int("FFMPEGVideoCodec", &video_codec_id);

        codec_found = 0;
        while (codec && codec->name) {
            video_codec_menu[i].label = (char *)(codec->name);
            video_codec_menu[i].help_string = NULL;
            video_codec_menu[i].callback = radio_FFMPEGVideoCodec_callback;
            video_codec_menu[i].callback_param = (void *)codec->id;
            video_codec_menu[i].par_string_max_len = 30;
            video_codec_menu[i].behavior = TUI_MENU_BEH_CLOSE;
            video_codec_menu[i].submenu = NULL;
            video_codec_menu[i].submenu_title = NULL;

            if (codec-> id == video_codec_id) {
                /* old video codec is present in the new codecs */
                codec_found = 1;
            }

            codec++;
            i++;

            if (i == MAX_CODECS) {
                break;
            }
        }
        video_codec_menu[i].label = NULL;

        /* is the old codec still valid for the new driver? */
        if (!codec_found) {
            /* no: default to the first codec in the new submenu */
            resources_set_int("FFMPEGVideoCodec", format->video_codecs[0].id);
        }
    }


    /* Update audio codec menu */
    codec = format->audio_codecs;
    i = 0;

    if (codec == NULL) {
        audio_codec_menu[0].label = NULL;
    } else {

        /* get the currently selected audio codec */
        resources_get_int("FFMPEGAudioCodec", &audio_codec_id);
        codec_found = 0;
        while (codec && codec->name) {
            audio_codec_menu[i].label = (char *)codec->name;
            audio_codec_menu[i].help_string = NULL;
            audio_codec_menu[i].callback = radio_FFMPEGAudioCodec_callback;
            audio_codec_menu[i].callback_param = (void *)codec->id;
            audio_codec_menu[i].par_string_max_len = 30;
            audio_codec_menu[i].behavior = TUI_MENU_BEH_CLOSE;
            audio_codec_menu[i].submenu = NULL;
            audio_codec_menu[i].submenu_title = NULL;

            if (audio_codec_id == codec->id) {
                /*old audio codec is present in the new codecs */
                codec_found = 1;
            }

            codec++;
            i++;

            if (i == MAX_CODECS) {
                break;
            }
        }
        audio_codec_menu[i].label = NULL;

        /* is the old codec still valid for the new driver? */
        if (!codec_found) {
            /* no: default to the first codec in the new submenu */
            resources_set_int("FFMPEGAudioCodec", format->audio_codecs[0].id);
        }
    }
}

static TUI_MENU_CALLBACK(custom_FFMPEGFormat_callback)
{
    if (been_activated) {
        resources_set_string("FFMPEGFormat", (char *)param);
        update_codec_menus((const char *)param);
    }
    return NULL;
}

void uiffmpeg_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ffmpeg_submenu;
    int i;
    int k;
    gfxoutputdrv_format_t *format;
    const char *w;

    if (ffmpegdrv_formatlist == NULL || ffmpegdrv_formatlist[0].name == NULL) {
        return;
    }

    k = 0;
    for (i = 0; ffmpegdrv_formatlist[i].name != NULL && i < MAX_FORMATS; i++) {
        char *name = ffmpegdrv_formatlist[i].name;

        if (ffmpegdrv_formatlist[i].audio_codecs != NULL && ffmpegdrv_formatlist[i].video_codecs != NULL) {
            audio_codec_menu[i].label = (char *)name;
            audio_codec_menu[i].help_string = NULL;
            audio_codec_menu[i].callback = custom_FFMPEGFormat_callback;
            audio_codec_menu[i].callback_param = (void *)name;
            audio_codec_menu[i].par_string_max_len = 30;
            audio_codec_menu[i].behavior = TUI_MENU_BEH_CLOSE;
            audio_codec_menu[i].submenu = NULL;
            audio_codec_menu[i].submenu_title = NULL;
            k++;
        }
    }

    format_menu[k].label = NULL;

    resources_get_string("FFMPEGFormat", &w);
    update_codec_menus(w);

    ui_ffmpeg_submenu = tui_menu_create("FFMPEG settings", 1);

    tui_menu_add(ui_ffmpeg_submenu, ffmpeg_menu_items);

    tui_menu_add_submenu(parent_submenu, "FFMPEG settings...",
                         "FFMPEG settings",
                         ui_ffmpeg_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
#endif
