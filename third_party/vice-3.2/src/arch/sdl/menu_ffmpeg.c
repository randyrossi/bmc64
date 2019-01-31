/*
 * menu_ffmpeg.c - FFMPEG menu for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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
#include <string.h>

#include "types.h"

#include "ffmpegdrv.h"
#include "gfxoutput.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_ffmpeg.h"
#include "resources.h"
#include "screenshot.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "util.h"
#include "videoarch.h"


static gfxoutputdrv_t *ffmpeg_drv = NULL;

#define MAX_FORMATS 7
#define MAX_CODECS 15

static ui_menu_entry_t format_menu[MAX_FORMATS + 1];
static ui_menu_entry_t video_codec_menu[MAX_CODECS + 1];
static ui_menu_entry_t audio_codec_menu[MAX_CODECS + 1];

UI_MENU_DEFINE_RADIO(FFMPEGVideoCodec)
UI_MENU_DEFINE_RADIO(FFMPEGAudioCodec)

UI_MENU_DEFINE_SLIDER(FFMPEGVideoBitrate, VICE_FFMPEG_VIDEO_RATE_MIN, VICE_FFMPEG_VIDEO_RATE_MIN)
UI_MENU_DEFINE_SLIDER(FFMPEGAudioBitrate, VICE_FFMPEG_AUDIO_RATE_MIN, VICE_FFMPEG_AUDIO_RATE_MAX)

static UI_MENU_CALLBACK(save_movie_callback)
{
    char *name = NULL;
    int width;
    int height;

    if (activated) {

        if (ffmpegdrv_formatlist == NULL || ffmpegdrv_formatlist[0].name == NULL) {
            ui_error("FFMPEG not available.");
            return NULL;
        }

        ffmpeg_drv = gfxoutput_get_driver("FFMPEG");
        if (ffmpeg_drv == NULL) {
            ui_error("FFMPEG not available.");
            return NULL;
        }

        name = sdl_ui_file_selection_dialog("Choose movie file", FILEREQ_MODE_SAVE_FILE);
        if (name != NULL) {
            width = sdl_active_canvas->draw_buffer->draw_buffer_width;
            height = sdl_active_canvas->draw_buffer->draw_buffer_height;
            memcpy(sdl_active_canvas->draw_buffer->draw_buffer, sdl_ui_get_draw_buffer(), width * height);
            util_add_extension(&name, ffmpeg_drv->default_extension);
            if (screenshot_save("FFMPEG", name, sdl_active_canvas) < 0) {
                ui_error("Cannot save movie.");
            }
            lib_free(name);
            return sdl_menu_text_exit_ui;
        }
    }
    return NULL;
}

ui_menu_entry_t ffmpeg_menu[] = {
    { "Format",
      MENU_ENTRY_DYNAMIC_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)format_menu },
    { "Video codec",
      MENU_ENTRY_DYNAMIC_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)video_codec_menu },
    { "Audio codec",
      MENU_ENTRY_DYNAMIC_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)audio_codec_menu },
    { "Video bitrate",
      MENU_ENTRY_RESOURCE_INT,
      slider_FFMPEGVideoBitrate_callback,
      (ui_callback_data_t)"Set video bitrate" },
    { "Audio bitrate",
      MENU_ENTRY_RESOURCE_INT,
      slider_FFMPEGAudioBitrate_callback,
      (ui_callback_data_t)"Set audio bitrate" },
    { "Save movie",
      MENU_ENTRY_DIALOG,
      save_movie_callback,
      NULL },
    SDL_MENU_LIST_END
};


/** \brief  Regenerate the audio/video codec submenus
 *
 * \param[in]   current_format  FFMPEG driver name
 *
 * TODO:    Update code to use `ffmpegdrv_formatlist`
 */
static void update_codec_menus(const char *current_format)
{
    int i;
    gfxoutputdrv_format_t *format;
    gfxoutputdrv_codec_t *codec;

    int video_codec_id;
    int audio_codec_id;

    int codec_found;

    video_codec_menu[0].string = NULL;
    audio_codec_menu[0].string = NULL;

    if (ffmpegdrv_formatlist == NULL || ffmpegdrv_formatlist[0].name == NULL) {
#ifdef SDL_DEBUG
        fprintf(stderr, "%s: no driver found\n", __func__);
#endif
        return;
    }

#if 0
    /* Find currently selected format */
    format = ffmpeg_drv->formatlist;

    while (format) {
        if (!strcmp(format->name, current_format)) {
            break;
        } else {
            format++;
        }
    }
#else
    format = NULL;
    for (i = 0; ffmpegdrv_formatlist[i].name != NULL; i++) {
        if (strcmp(ffmpegdrv_formatlist[i].name, current_format) == 0) {
            /* got current format */
            format = &(ffmpegdrv_formatlist[i]);
            break;
        }
    }
#endif

    if (!format) {
#ifdef SDL_DEBUG
        fprintf(stderr, "%s: format %s not found\n", __func__, current_format ? current_format : "(NULL)");
#endif
        return;
    }

    /* Update video codec menu */
    codec = format->video_codecs;
    i = 0;

    if (codec == NULL) {
        video_codec_menu[0].string = NULL;
    } else {

        /* get the currently used video codec */
        resources_get_int("FFMPEGVideoCodec", &video_codec_id);

        codec_found = 0;
        while (codec && codec->name) {
            video_codec_menu[i].string = (char *)(codec->name);
            video_codec_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
            video_codec_menu[i].callback = radio_FFMPEGVideoCodec_callback;
            video_codec_menu[i].data = int_to_void_ptr(codec->id);
#ifdef SDL_DEBUG
            fprintf(stderr, "%s: video codec %i: %s (%i)\n", __func__, i, (codec->name) ? codec->name : "(NULL)", codec->id);
#endif
            if (codec-> id == video_codec_id) {
                /* old video codec is present in the new codecs */
                codec_found = 1;
            }

            codec++;
            i++;

            if (i == MAX_CODECS) {
#ifdef SDL_DEBUG
                fprintf(stderr, "%s: FIXME video codec %i > %i (MAX)\n", __func__, i, MAX_CODECS);
#endif
                break;
            }
        }
        video_codec_menu[i].string = NULL;

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
        audio_codec_menu[0].string = NULL;
    } else {

        /* get the currently selected audio codec */
        resources_get_int("FFMPEGAudioCodec", &audio_codec_id);
        codec_found = 0;
        while (codec && codec->name) {
            audio_codec_menu[i].string = (char *)(codec->name);
            audio_codec_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
            audio_codec_menu[i].callback = radio_FFMPEGAudioCodec_callback;
            audio_codec_menu[i].data = int_to_void_ptr(codec->id);
#ifdef SDL_DEBUG
            fprintf(stderr, "%s: audio codec %i: %s (%i)\n", __func__, i, (codec->name) ? codec->name : "(NULL)", codec->id);
#endif

            if (audio_codec_id == codec->id) {
                /*old audio codec is present in the new codecs */
                codec_found = 1;
            }

            codec++;
            i++;

            if (i == MAX_CODECS) {
#ifdef SDL_DEBUG
                fprintf(stderr, "%s: FIXME audio codec %i > %i (MAX)\n", __func__, i, MAX_CODECS);
#endif
                break;
            }
        }
        audio_codec_menu[i].string = NULL;

        /* is the old codec still valid for the new driver? */
        if (!codec_found) {
            /* no: default to the first codec in the new submenu */
            resources_set_int("FFMPEGAudioCodec", format->audio_codecs[0].id);
        }
    }

}

static UI_MENU_CALLBACK(custom_FFMPEGFormat_callback)
{
    if (activated) {
        resources_set_string("FFMPEGFormat", (char *)param);
        update_codec_menus((const char *)param);
    } else {
        const char *w;

        resources_get_string("FFMPEGFormat", &w);
#ifdef SDL_DEBUG
        fprintf(stderr, "%s: FFMPEGFormat = '%s'\n", __func__, w);
#endif
        if (!strcmp(w, (char *)param)) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

void sdl_menu_ffmpeg_init(void)
{
    int i;
    int k;
    const char *w;

    if (ffmpegdrv_formatlist == NULL || ffmpegdrv_formatlist[0].name == NULL) {
#ifdef SDL_DEBUG
        fprintf(stderr, "%s: no driver found\n", __func__);
#endif
        return;
    }


    k = 0;
    for (i = 0; ffmpegdrv_formatlist[i].name != NULL && i < MAX_FORMATS; i++) {
        char *name = ffmpegdrv_formatlist[i].name;
        if (ffmpegdrv_formatlist[i].audio_codecs != NULL &&
                ffmpegdrv_formatlist[i].video_codecs != NULL) {
            format_menu[k].string = name;
            format_menu[k].type = MENU_ENTRY_RESOURCE_RADIO;
            format_menu[k].callback = custom_FFMPEGFormat_callback;
            format_menu[k].data = (ui_callback_data_t)(name);
#ifdef SDL_DEBUG
            fprintf(stderr, "%s: format %i: %s selected\n",
                    __func__, i, name ? name : "(NULL)");
#endif
            k++;
#ifdef SDL_DEBUG
        } else {
            fprintf(stderr, "%s: format %i: %s not selected\n",
                    __func__, i, name ? name : "(NULL)");
#endif
        }
    }
    if (k == MAX_FORMATS) {
#ifdef SDL_DEBUG
        fprintf(stderr, "%s: FIXME format %i > %i (MAX)\n", __func__, i, MAX_FORMATS);
#endif
    }

    format_menu[k].string = NULL;

    resources_get_string("FFMPEGFormat", &w);
    update_codec_menus(w);
}

void sdl_menu_ffmpeg_shutdown(void)
{
    if (screenshot_is_recording()) {
        screenshot_stop_recording();
    }
}

#endif /* HAVE_FFMPEG */
