/*
 * menu_sound.c - Implementation of the sound settings menu for the SDL UI.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "menu_common.h"
#include "menu_sound.h"
#include "resources.h"
#include "sound.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "util.h"

UI_MENU_DEFINE_TOGGLE(Sound)
UI_MENU_DEFINE_RADIO(SoundSampleRate)
UI_MENU_DEFINE_RADIO(SoundFragmentSize)
UI_MENU_DEFINE_RADIO(SoundSpeedAdjustment)
UI_MENU_DEFINE_RADIO(SoundDeviceName)
UI_MENU_DEFINE_RADIO(SoundOutput)

static UI_MENU_CALLBACK(custom_volume_callback)
{
    static char buf[20];
    int previous, new_value;

    resources_get_int("SoundVolume", &previous);

    if (activated) {
        new_value = sdl_ui_slider_input_dialog("Select volume", previous, 0, 100);
        if (new_value != previous) {
            resources_set_int("SoundVolume", new_value);
        }
    } else {
        sprintf(buf, "%i%%", previous);
        return buf;
    }
    return NULL;
}

static UI_MENU_CALLBACK(custom_buffer_size_callback)
{
    static char buf[20];
    char *value = NULL;
    int previous, new_value;

    resources_get_int("SoundBufferSize", &previous);

    if (activated) {
        sprintf(buf, "%i", previous);
        value = sdl_ui_text_input_dialog("Enter buffer size in msec", buf);
        if (value) {
            new_value = strtol(value, NULL, 0);
            if (new_value != previous) {
                resources_set_int("SoundBufferSize", new_value);
            }
            lib_free(value);
        }
    } else {
        sprintf(buf, "%i msec", previous);
        return buf;
    }
    return NULL;
}

static UI_MENU_CALLBACK(custom_frequency_callback)
{
    static char buf[20];
    char *value = NULL;
    int previous, new_value;

    resources_get_int("SoundSampleRate", &previous);

    if (activated) {
        sprintf(buf, "%i", previous);
        value = sdl_ui_text_input_dialog("Enter frequency in Hz", buf);
        if (value) {
            new_value = strtol(value, NULL, 0);
            if (new_value != previous) {
                resources_set_int("SoundSampleRate", new_value);
            }
            lib_free(value);
        }
    } else {
        if (previous != 22050 && previous != 44100 && previous != 48000) {
            sprintf(buf, "%i Hz", previous);
            return buf;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(start_recording_callback)
{
    char *parameter = (char *)param;

    if (activated) {
        resources_set_string("SoundRecordDeviceName", "");
        if (parameter != NULL) {
            char *name = NULL;

            name = sdl_ui_file_selection_dialog("Choose audio file to record to", FILEREQ_MODE_CHOOSE_FILE);
            if (name != NULL) {
                util_add_extension(&name, parameter);
                resources_set_string("SoundRecordDeviceArg", name);
                resources_set_string("SoundRecordDeviceName", parameter);
                lib_free(name);
            }
        }
    } else {
        if (parameter != NULL) {
            const char *w;

            resources_get_string("SoundRecordDeviceName", &w);
            if (!strcmp(w, parameter)) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

static ui_menu_entry_t sound_output_driver_menu[] = {
#ifdef HAVE_DEVICES_AHI_H
    { "AHI",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"ahi" },
#endif
#ifdef USE_AIX_AUDIO
    { "AIX",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"aix" },
#endif
#if defined(__MSDOS__) && !defined(USE_MIDAS_SOUND)
    { "Allegro",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"allegro" },
#endif
#ifdef USE_ALSA
    { "ALSA",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"alsa" },
#endif
#ifdef USE_ARTS
    { "aRts",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"arts" },
#endif
#ifdef __BEOS__
    { "BeOS",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"beos" },
#endif
#ifdef USE_COREAUDIO
    { "Core Audio",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"coreaudio" },
#endif
#ifdef __OS2__
    { "OS/2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"dart" },
#endif
    { "Dummy",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"dummy" },
#if defined(WIN32) && defined(USE_DXSOUND)
    { "DirectX",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"dx" },
#endif
#ifdef HAVE_SYS_AUDIO_H
    { "HPUX",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"hpux" },
#endif
#if defined(__MSDOS__) && defined(USE_MIDAS_SOUND)
    { "Midas",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"midas" },
#endif
#ifdef USE_PULSE
    { "PulseAudio",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"pulse" },
#endif
#ifdef USE_SDL_AUDIO
    { "SDL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"sdl" },
#endif
#ifdef USE_DMEDIA
    { "SGI",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"sgi" },
#endif
#if defined(HAVE_SYS_AUDIOIO_H) && !defined(__NetBSD__) && !defined(__OpenBSD__)
    { "Sun audio",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"sun" },
#endif
#if defined(HAVE_SYS_AUDIOIO_H) && defined(__NetBSD__) && !defined(__OpenBSD__)
    { "NetBSD",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"netbsd" },
#endif
#if defined(USE_OSS) && !defined(__FreeBSD__) && !defined(__bsdi__)
    { "OSS",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"uss" },
#endif
#ifdef WIN32
    { "WMM",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundDeviceName_callback,
      (ui_callback_data_t)"wmm" },
#endif
    SDL_MENU_LIST_END
};

static ui_menu_entry_t sound_output_mode_menu[] = {
    { "System",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundOutput_callback,
      (ui_callback_data_t)SOUND_OUTPUT_SYSTEM },
    { "Mono",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundOutput_callback,
      (ui_callback_data_t)SOUND_OUTPUT_MONO },
    { "Stereo",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundOutput_callback,
      (ui_callback_data_t)SOUND_OUTPUT_STEREO },
    SDL_MENU_LIST_END
};

static ui_menu_entry_t fragment_size_menu[] = {
    { "Very small",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundFragmentSize_callback,
      (ui_callback_data_t)SOUND_FRAGMENT_VERY_SMALL },
    { "Small",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundFragmentSize_callback,
      (ui_callback_data_t)SOUND_FRAGMENT_SMALL },
    { "Medium",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundFragmentSize_callback,
      (ui_callback_data_t)SOUND_FRAGMENT_MEDIUM },
    { "Large",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundFragmentSize_callback,
      (ui_callback_data_t)SOUND_FRAGMENT_LARGE },
    { "Very large",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundFragmentSize_callback,
      (ui_callback_data_t)SOUND_FRAGMENT_VERY_LARGE },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t sound_record_menu[] = {
    { "Start recording AIFF audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"aiff" },
    { "Start recording IFF audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"iff" },
#ifdef USE_LAMEMP3
    { "Start recording MP3 audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"mp3" },
#endif
#ifdef USE_FLAC
    { "Start recording FLAC audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"flac" },
#endif
#ifdef USE_VORBIS
    { "Start recording ogg/vorbis audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"ogg" },
#endif
    { "Start recording VOC audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"voc" },
    { "Start recording WAV audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"wav" },
    SDL_MENU_ITEM_SEPARATOR,
    { "Start recording RAW audio file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"fs" },
    SDL_MENU_ITEM_SEPARATOR,
    { "Start recording sound dump file",
      MENU_ENTRY_DIALOG,
      start_recording_callback,
      (ui_callback_data_t)"dump" },
    SDL_MENU_ITEM_SEPARATOR,
    { "Stop recording",
      MENU_ENTRY_OTHER,
      start_recording_callback,
      NULL },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t sound_output_menu[] = {
    { "Sound",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Sound_callback,
      NULL },
    { "Volume",
      MENU_ENTRY_DIALOG,
      custom_volume_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Output driver",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)sound_output_driver_menu },
    { "Output Mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)sound_output_mode_menu },
    { "Buffer size",
      MENU_ENTRY_DIALOG,
      custom_buffer_size_callback,
      NULL },
    { "Fragment size",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)fragment_size_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Frequency"),
    { "22050 Hz",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundSampleRate_callback,
      (ui_callback_data_t)22050 },
    { "44100 Hz",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundSampleRate_callback,
      (ui_callback_data_t)44100 },
    { "48000 Hz",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundSampleRate_callback,
      (ui_callback_data_t)48000 },
    { "Custom frequency",
      MENU_ENTRY_DIALOG,
      custom_frequency_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Synchronization method"),
    { "Flexible",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundSpeedAdjustment_callback,
      (ui_callback_data_t)SOUND_ADJUST_FLEXIBLE },
    { "Adjusting",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundSpeedAdjustment_callback,
      (ui_callback_data_t)SOUND_ADJUST_ADJUSTING },
    { "Exact",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SoundSpeedAdjustment_callback,
      (ui_callback_data_t)SOUND_ADJUST_EXACT },

    SDL_MENU_LIST_END
};
