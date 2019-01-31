/*
 * uisound.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include "resources.h"
#include "sound.h"
#include "tui.h"
#include "tuimenu.h"
#include "uisound.h"

tui_menu_t ui_sound_buffer_size_submenu;
tui_menu_t ui_sound_sample_rate_submenu;
tui_menu_t ui_sound_submenu;

TUI_MENU_DEFINE_TOGGLE(Sound)
TUI_MENU_DEFINE_TOGGLE(DriveSoundEmulation)

static TUI_MENU_CALLBACK(sound_sample_rate_submenu_callback)
{
    static char s[256];
    int value;

    resources_get_int("SoundSampleRate", &value);
    sprintf(s, "%d Hz", value);
    return s;
}

TUI_MENU_DEFINE_RADIO(SoundSampleRate)

TUI_MENU_DEFINE_RADIO(SoundBufferSize)

static TUI_MENU_CALLBACK(sound_buffer_size_submenu_callback)
{
    static char s[256];
    int value;

    resources_get_int("SoundBufferSize", &value);
    sprintf(s, "%d msec", value);
    return s;
}

TUI_MENU_DEFINE_RADIO(SoundSpeedAdjustment)

static TUI_MENU_CALLBACK(sound_synchronization_submenu_callback)
{
    int value;

    resources_get_int("SoundSpeedAdjustment", &value);

    switch (value) {
        case SOUND_ADJUST_FLEXIBLE:
            return "Flexible";
        case SOUND_ADJUST_ADJUSTING:
            return "Adjusting";
        case SOUND_ADJUST_EXACT:
            return "Exact";
        default:
            return "Unknown";
    }
}

TUI_MENU_DEFINE_RADIO(SoundFragmentSize)

static TUI_MENU_CALLBACK(sound_fragment_size_submenu_callback)
{
    int value;

    resources_get_int("SoundFragmentSize", &value);

    switch (value) {
        case SOUND_FRAGMENT_VERY_SMALL:
            return "Very small";
        case SOUND_FRAGMENT_SMALL:
            return "Small";
        case SOUND_FRAGMENT_MEDIUM:
            return "Medium";
        case SOUND_FRAGMENT_LARGE:
            return "Large";
        case SOUND_FRAGMENT_VERY_LARGE:
            return "Very large";
        default:
            return "Unknown";
    }
}

static tui_menu_item_def_t sample_rate_submenu[] = {
    { "_0: 8000 Hz",
      "Set sampling rate to 8000 Hz",
      radio_SoundSampleRate_callback, (void *)8000, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_1: 11025 Hz",
      "Set sampling rate to 11025 Hz",
      radio_SoundSampleRate_callback, (void *)11025, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2: 22050 Hz",
      "Set sampling rate to 22050 Hz",
      radio_SoundSampleRate_callback, (void *)22050, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3: 44100 Hz",
      "Set sampling rate to 44100 Hz",
      radio_SoundSampleRate_callback, (void *)44100, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t sound_buffer_size_submenu[] = {
    { "_1: 50 msec",
      "Set sound buffer size to 50 msec",
      radio_SoundBufferSize_callback, (void *)50, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2: 100 msec",
      "Set sound buffer size to 100 msec",
      radio_SoundBufferSize_callback, (void *)100, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3: 150 msec",
      "Set sound buffer size to 150 msec",
      radio_SoundBufferSize_callback, (void *)150, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_4: 200 msec",
      "Set sound buffer size to 200 msec",
      radio_SoundBufferSize_callback, (void *)200, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_5: 250 msec",
      "Set sound buffer size to 250 msec",
      radio_SoundBufferSize_callback, (void *)250, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_6: 300 msec",
      "Set sound buffer size to 300 msec",
      radio_SoundBufferSize_callback, (void *)300, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_7: 350 msec",
      "Set sound buffer size to 350 msec",
      radio_SoundBufferSize_callback, (void *)350, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t sound_synchronization_submenu[] = {
    { "_Flexible",
      "Slightly adapt sound playback speed to the speed of the emulator",
      radio_SoundSpeedAdjustment_callback, (void *)SOUND_ADJUST_FLEXIBLE, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Adjusting",
      "Fully adapt the playback speed to the emulator, avoiding clicks when it's slower",
      radio_SoundSpeedAdjustment_callback, (void *)SOUND_ADJUST_ADJUSTING, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Exact",
      "Don't adapt sound playback: make the emulator finetune its speed to the playback",
      radio_SoundSpeedAdjustment_callback, (void *)SOUND_ADJUST_EXACT, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t sound_fragment_size_submenu[] = {
    { "_Very small",
      "Use a very small fragment size",
      radio_SoundFragmentSize_callback, (void *)SOUND_FRAGMENT_VERY_SMALL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Small",
      "Use a small fragment size",
      radio_SoundFragmentSize_callback, (void *)SOUND_FRAGMENT_SMALL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Medium",
      "Use a medium fragment size",
      radio_SoundFragmentSize_callback, (void *)SOUND_FRAGMENT_MEDIUM, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Large",
      "Use a large fragment size",
      radio_SoundFragmentSize_callback, (void *)SOUND_FRAGMENT_LARGE, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "V_ery large",
      "Use a very large fragment size",
      radio_SoundFragmentSize_callback, (void *)SOUND_FRAGMENT_VERY_LARGE, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(ui_sound_set_volume_callback)
{
    if (been_activated) {
        int current_volume, value;
        char buf[10];

        resources_get_int("SoundVolume", &current_volume);
        sprintf(buf, "%d", current_volume);

        if (tui_input_string("Volume", "Enter volume to use (0-100):", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 100) {
                value = 100;
            } else if (value < 0) {
                value = 0;
            }
            resources_set_int("SoundVolume", value);
            tui_message("Volume set to : %d", value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_sound_set_drive_volume_callback)
{
    if (been_activated) {
        int current_volume, value;
        char buf[10];

        resources_get_int("DriveSoundEmulationVolume", &current_volume);
        sprintf(buf, "%d", current_volume);

        if (tui_input_string("DriveSoundEmulationVolume", "Enter drive sound volume to use (0-4000):", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 4000) {
                value = 4000;
            } else if (value < 0) {
                value = 0;
            }
            resources_set_int("DriveSoundEmulationVolume", value);
            tui_message("Drive sound volume set to : %d", value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static tui_menu_item_def_t sound_submenu[] = {
    { "Sound _Playback:",
      "Enable sound output",
      toggle_Sound_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Sound _Volume",
      "Set the volume to use",
      ui_sound_set_volume_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Sample Frequency:",
      "Choose sound output sampling rate",
      sound_sample_rate_submenu_callback, NULL, 10,
      TUI_MENU_BEH_CONTINUE, sample_rate_submenu, "Sample rate" },
    { "Sound _Buffer Size:",
      "Specify playback latency",
      sound_buffer_size_submenu_callback, NULL, 10,
      TUI_MENU_BEH_CONTINUE, sound_buffer_size_submenu, "Latency" },
    { "Sound _Fragment Size:",
      "Specify fragment size",
      sound_fragment_size_submenu_callback, NULL, 10,
      TUI_MENU_BEH_CONTINUE, sound_fragment_size_submenu, "Fragment Size" },
    { "S_ynchronization Method:",
      "Specify method used to synchronize the sound playback with the emulator",
      sound_synchronization_submenu_callback, NULL, 9,
      TUI_MENU_BEH_CONTINUE, sound_synchronization_submenu, "Synchronization" },
    { "--" },
    { "Drive sound emulation:",
      "Enable drive sound emulation",
      toggle_DriveSoundEmulation_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive sound volume",
      "Set the volume to use",
      ui_sound_set_drive_volume_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uisound_init(struct tui_menu *parent_submenu)
{
    ui_sound_submenu = tui_menu_create("Audio Settings", 1);

    tui_menu_add(ui_sound_submenu, sound_submenu);
    tui_menu_add_submenu(parent_submenu, "_Sound Settings...",
                         "Sampling rate, sound output, soundcard settings",
                         ui_sound_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
