/*
 * uimidi.c
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Andreas Boose <viceteam@t-online.de>
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

#ifdef HAVE_MIDI

#include <stdio.h>

#include "uilib.h"
#include "uimenu.h"
#include "uimidi.h"

UI_MENU_DEFINE_TOGGLE(MIDIEnable)
UI_MENU_DEFINE_RADIO(MIDIMode)

#ifdef USE_ALSA
UI_MENU_DEFINE_RADIO(MIDIDriver)
#endif

static UI_CALLBACK(set_midi_in_name)
{
    uilib_select_dev((char *)UI_MENU_CB_PARAM, _("MIDI-In device"), UILIB_FILTER_MIDI);
}

static UI_CALLBACK(set_midi_out_name)
{
    uilib_select_dev((char *)UI_MENU_CB_PARAM, _("MIDI-Out device"), UILIB_FILTER_MIDI);
}

#ifdef USE_ALSA
static ui_menu_entry_t midi_driver_submenu[] = {
    { "OSS", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MIDIDriver, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "ALSA", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MIDIDriver, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif

static ui_menu_entry_t midi_mode_submenu[] = {
    { "Sequential", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MIDIMode, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Passport/Syntech", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MIDIMode, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "DATEL/Siel/JMS", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MIDIMode, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Namesoft", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MIDIMode, (ui_callback_data_t)3, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Maplin", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MIDIMode, (ui_callback_data_t)4, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t midi_c64_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MIDIEnable, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, midi_mode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_ALSA
    { N_("Driver"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, midi_driver_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("MIDI-In device"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_midi_in_name, (ui_callback_data_t)"MIDIInDev", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("MIDI-Out device"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_midi_out_name, (ui_callback_data_t)"MIDIOutDev", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t midi_vic20_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MIDIEnable, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_ALSA
    { N_("Driver"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, midi_driver_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("MIDI-In device"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_midi_in_name, (ui_callback_data_t)"MIDIInDev", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("MIDI-Out device"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_midi_out_name, (ui_callback_data_t)"MIDIOutDev", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

#endif /* HAVE_MIDI */
