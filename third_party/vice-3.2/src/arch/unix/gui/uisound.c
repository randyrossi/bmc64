/*
 * uisound.c - Implementation of sound UI settings.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "lib.h"
#include "sound.h"
#include "uilib.h"
#include "uimenu.h"
#include "uisound.h"

UI_MENU_DEFINE_TOGGLE(Sound)
UI_MENU_DEFINE_STRING_RADIO(SoundDeviceName)
UI_MENU_DEFINE_RADIO(SoundSpeedAdjustment)
UI_MENU_DEFINE_RADIO(SoundSampleRate)
UI_MENU_DEFINE_RADIO(SoundBufferSize)
UI_MENU_DEFINE_RADIO(SoundFragmentSize)
UI_MENU_DEFINE_RADIO(SoundSuspendTime)
UI_MENU_DEFINE_RADIO(SoundOutput)

static UI_CALLBACK(set_sound_device_arg)
{
    uilib_select_string((char *)UI_MENU_CB_PARAM, _("Sound driver argument"), _("Argument:"));
}

ui_menu_entry_t set_sound_sample_rate_submenu[] = {
    { "8000Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSampleRate, (ui_callback_data_t)8000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "11025Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSampleRate, (ui_callback_data_t)11025, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "22050Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSampleRate, (ui_callback_data_t)22050, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "44100Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSampleRate, (ui_callback_data_t)44100, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "48000Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSampleRate, (ui_callback_data_t)48000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t set_sound_buffer_size_submenu[] = {
    { "0.35 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)350, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.25 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)250, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.15 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)150, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.10 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)100, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.075 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)75, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.05 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)50, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.04 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)40, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.03 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)30, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.025 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)25, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "0.02 s", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundBufferSize, (ui_callback_data_t)20, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t set_sound_fragment_size_submenu[] = {
    { N_("Very small"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundFragmentSize, (ui_callback_data_t)SOUND_FRAGMENT_VERY_SMALL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Small"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundFragmentSize, (ui_callback_data_t)SOUND_FRAGMENT_SMALL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Medium"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundFragmentSize, (ui_callback_data_t)SOUND_FRAGMENT_MEDIUM, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Large"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundFragmentSize, (ui_callback_data_t)SOUND_FRAGMENT_LARGE, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Very large"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundFragmentSize, (ui_callback_data_t)SOUND_FRAGMENT_VERY_LARGE, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t set_sound_suspend_time_submenu[] = {
    { N_("Keep going"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSuspendTime, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("1 s suspend"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSuspendTime, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("2 s suspend"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSuspendTime, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("5 s suspend"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSuspendTime, (ui_callback_data_t)5, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("10 s suspend"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSuspendTime, (ui_callback_data_t)10, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t set_sound_adjustment_submenu[] = {
    { N_("Flexible"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSpeedAdjustment, (ui_callback_data_t)SOUND_ADJUST_FLEXIBLE, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Adjusting"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSpeedAdjustment, (ui_callback_data_t)SOUND_ADJUST_ADJUSTING, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Exact"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundSpeedAdjustment, (ui_callback_data_t)SOUND_ADJUST_EXACT, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t set_sound_outmode_submenu[] = {
    { N_("System"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundOutput, (ui_callback_data_t)SOUND_OUTPUT_SYSTEM, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Mono"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundOutput, (ui_callback_data_t)SOUND_OUTPUT_MONO, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Stereo"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SoundOutput, (ui_callback_data_t)SOUND_OUTPUT_STEREO, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t sound_settings_submenu[] = {
    { N_("Enable sound playback"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Sound, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
      /* Do not change position as position 2 is hard coded. */
    { N_("Sound device name"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Sound driver argument"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_sound_device_arg, (ui_callback_data_t)"SoundDeviceArg", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Output mode"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sound_outmode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Sound synchronization"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sound_adjustment_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Sample rate"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sound_sample_rate_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Buffer size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sound_buffer_size_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Fragment size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sound_fragment_size_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Suspend time"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sound_suspend_time_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_sound_settings_menu[] = {
    { N_("Sound settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sound_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

void uisound_menu_create(void)
{
    unsigned int i, num;
    ui_menu_entry_t *devices_submenu;

    num = sound_device_num();

    if (num == 0) {
        return;
    }

    devices_submenu = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));

    for (i = 0; i < num ; i++) {
        devices_submenu[i].string = (ui_callback_data_t)sound_device_name(i);
        devices_submenu[i].type = UI_MENU_TYPE_TICK;
        devices_submenu[i].callback = (ui_callback_t)radio_SoundDeviceName;
        devices_submenu[i].callback_data = (ui_callback_data_t)sound_device_name(i);
    }

    sound_settings_submenu[2].sub_menu = devices_submenu;
}

void uisound_menu_shutdown(void)
{
    lib_free(sound_settings_submenu[2].sub_menu);
}
