/*
 * uiperipheralieee.c
 *
 * Written by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attach.h"
#include "lib.h"
#include "resources.h"
#include "uidrive.h"
#include "uimenu.h"
#include "uiperipheral.h"
#include "vsync.h"

UI_MENU_DEFINE_TOGGLE(FSDevice8ConvertP00)
UI_MENU_DEFINE_TOGGLE(FSDevice9ConvertP00)
UI_MENU_DEFINE_TOGGLE(FSDevice10ConvertP00)
UI_MENU_DEFINE_TOGGLE(FSDevice11ConvertP00)
UI_MENU_DEFINE_TOGGLE(FSDevice8SaveP00)
UI_MENU_DEFINE_TOGGLE(FSDevice9SaveP00)
UI_MENU_DEFINE_TOGGLE(FSDevice10SaveP00)
UI_MENU_DEFINE_TOGGLE(FSDevice11SaveP00)
UI_MENU_DEFINE_TOGGLE(FSDevice8HideCBMFiles)
UI_MENU_DEFINE_TOGGLE(FSDevice9HideCBMFiles)
UI_MENU_DEFINE_TOGGLE(FSDevice10HideCBMFiles)
UI_MENU_DEFINE_TOGGLE(FSDevice11HideCBMFiles)
UI_MENU_DEFINE_TOGGLE(AttachDevice8Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice9Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice10Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice11Readonly)

#define FSDEVICE_DRIVE_MENU(x)                                                         \
static ui_menu_entry_t fsdevice_drive##x##_submenu[] = {                               \
    { N_("Read only access"), UI_MENU_TYPE_TICK,                                       \
      (ui_callback_t)toggle_AttachDevice##x##Readonly, NULL, NULL,                     \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                       \
    UI_MENU_ENTRY_SEPERATOR,                                                           \
    { N_("File system directory"), UI_MENU_TYPE_DOTS,                                  \
      (ui_callback_t)uiperipheral_set_fsdevice_directory, (ui_callback_data_t)x, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                       \
    { N_("Convert P00 file names"), UI_MENU_TYPE_TICK,                                 \
     (ui_callback_t)toggle_FSDevice##x##ConvertP00, NULL, NULL,                        \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                       \
    { N_("Create P00 files on save"), UI_MENU_TYPE_TICK,                               \
     (ui_callback_t)toggle_FSDevice##x##SaveP00, NULL, NULL,                           \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                       \
    { N_("Hide non-P00 files"), UI_MENU_TYPE_TICK,                                     \
     (ui_callback_t)toggle_FSDevice##x##HideCBMFiles, NULL, NULL,                      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                       \
    UI_MENU_ENTRY_SEPERATOR,                                                           \
    { N_("Drive RPM"), UI_MENU_TYPE_DOTS,                                              \
      (ui_callback_t)drive_rpm_settings, (ui_callback_data_t)x, NULL,                  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                       \
    { N_("Drive wobble"), UI_MENU_TYPE_DOTS,                                           \
      (ui_callback_t)drive_wobble_settings, (ui_callback_data_t)x, NULL,               \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                       \
    UI_MENU_ENTRY_LIST_END                                                             \
}

FSDEVICE_DRIVE_MENU(8);
FSDEVICE_DRIVE_MENU(9);
FSDEVICE_DRIVE_MENU(10);
FSDEVICE_DRIVE_MENU(11);

#define PERIPHERALIEEE_SETTINGS_DRIVE_MENU(x)                    \
ui_menu_entry_t peripheralieee_settings_drive##x##_submenu[] = { \
    { N_("Drive #"#x" device type"), UI_MENU_TYPE_NORMAL,        \
      NULL, NULL, uiperipheral_set_device##x##_type_submenu,     \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                 \
    { N_("Drive #"#x" options"), UI_MENU_TYPE_NORMAL,            \
      NULL, NULL, fsdevice_drive##x##_submenu,                   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                 \
    UI_MENU_ENTRY_LIST_END                                       \
}

PERIPHERALIEEE_SETTINGS_DRIVE_MENU(8);
PERIPHERALIEEE_SETTINGS_DRIVE_MENU(9);
PERIPHERALIEEE_SETTINGS_DRIVE_MENU(10);
PERIPHERALIEEE_SETTINGS_DRIVE_MENU(11);
