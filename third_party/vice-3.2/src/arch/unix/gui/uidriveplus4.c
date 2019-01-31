/*
 * uidriveplus4.c
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

#include "drive.h"
#include "resources.h"
#include "uiapi.h"
#include "uidrive.h"
#include "uidriveiec.h"
#include "uidriveplus4.h"
#include "uimenu.h"
#include "uiperipheral.h"
#include "uiperipheraliec.h"
#include "uiromset.h"

UI_MENU_DEFINE_TOGGLE(DriveTrueEmulation)
UI_MENU_DEFINE_TOGGLE(DriveSoundEmulation)
UI_MENU_DEFINE_RADIO(Drive8ParallelCable)
UI_MENU_DEFINE_RADIO(Drive9ParallelCable)
UI_MENU_DEFINE_RADIO(Drive10ParallelCable)
UI_MENU_DEFINE_RADIO(Drive11ParallelCable)

static UI_CALLBACK(parallel_cable_control)
{
    if (!CHECK_MENUS) {
        ui_update_menus();
    } else {
        int type;

        resources_get_int_sprintf("Drive%iType", &type, vice_ptr_to_int(UI_MENU_CB_PARAM) + 8);

        if (drive_check_parallel_cable(type)) {
            ui_menu_set_sensitive(w, 1);
        } else {
            ui_menu_set_sensitive(w, 0);
        }
    }
}

#define SET_DRIVE_PAR_CABLE_MENU(x, y)                                                           \
ui_menu_entry_t set_drive##x##_parallel_cable_submenu[] = {                                      \
    { N_("None"), UI_MENU_TYPE_TICK,                                                             \
      (ui_callback_t)radio_Drive##y##ParallelCable, (ui_callback_data_t)DRIVE_PC_NONE, NULL,     \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                 \
    { N_("Standard Userport"), UI_MENU_TYPE_TICK,                                                \
      (ui_callback_t)radio_Drive##y##ParallelCable, (ui_callback_data_t)DRIVE_PC_STANDARD, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                 \
    UI_MENU_ENTRY_LIST_END                                                                       \
}

SET_DRIVE_PAR_CABLE_MENU(0, 8);
SET_DRIVE_PAR_CABLE_MENU(1, 9);
SET_DRIVE_PAR_CABLE_MENU(2, 10);
SET_DRIVE_PAR_CABLE_MENU(3, 11);

#define UIDRIVEPLUS4_DRIVE_EXPANSION_MENU(x)                                                               \
static ui_menu_entry_t uidriveplus4_drive##x##_expansion_submenu[] = {                                     \
    { N_("Parallel cable"), UI_MENU_TYPE_TICK,                                                             \
      (ui_callback_t)parallel_cable_control, (ui_callback_data_t)x, set_drive##x##_parallel_cable_submenu, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                           \
    UI_MENU_ENTRY_SEPERATOR,                                                                               \
    { "", UI_MENU_TYPE_NONE,                                                                               \
      NULL, NULL, uidriveiec_drive##x##_ram_expansion_submenu,                                             \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                           \
    UI_MENU_ENTRY_LIST_END                                                                                 \
}

UIDRIVEPLUS4_DRIVE_EXPANSION_MENU(0);
UIDRIVEPLUS4_DRIVE_EXPANSION_MENU(1);
UIDRIVEPLUS4_DRIVE_EXPANSION_MENU(2);
UIDRIVEPLUS4_DRIVE_EXPANSION_MENU(3);

#define SET_DRIVE_TYPE_MENU(x, y)                                                       \
static ui_menu_entry_t set_drive##x##_type_submenu[] = {                                \
    { N_("None"), UI_MENU_TYPE_TICK,                                                    \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_NONE, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1540", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1540, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1541", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1541, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1541-II", UI_MENU_TYPE_TICK,                                                     \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1541II, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1551", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1551, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1570", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1570, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1571", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1571, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1581", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1581, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "2000", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_2000, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "4000", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_4000, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    UI_MENU_ENTRY_LIST_END                                                              \
}

SET_DRIVE_TYPE_MENU(0, 8);
SET_DRIVE_TYPE_MENU(1, 9);
SET_DRIVE_TYPE_MENU(2, 10);
SET_DRIVE_TYPE_MENU(3, 11);

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t driveplus4_settings_submenu[] = {
    { N_("True drive emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DriveTrueEmulation, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive sound emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DriveSoundEmulation, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #8 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_drive0_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_plus4_settings_drive8_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #8 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)0, uidriveplus4_drive0_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #8 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)0, set_drive0_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #8 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)0, set_drive0_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #9 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_drive1_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_plus4_settings_drive9_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #9 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)1, uidriveplus4_drive1_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #9 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)1, set_drive1_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #9 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)1, set_drive1_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #10 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_drive2_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_plus4_settings_drive10_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #10 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)2, uidriveplus4_drive2_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #10 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)2, set_drive2_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #10 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)2, set_drive2_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive #11 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_drive3_type_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, peripheraliec_plus4_settings_drive11_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #11 expansion"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_expansion_control, (ui_callback_data_t)3, uidriveplus4_drive3_expansion_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #11 40-track image support"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidrive_extend_policy_control, (ui_callback_data_t)3, set_drive3_extend_image_policy_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive #11 idle method"), UI_MENU_TYPE_BL_SUB,
      (ui_callback_t)uidriveiec_idle_method_control, (ui_callback_data_t)3, set_drive3_idle_method_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_RAWDRIVE
    UI_MENU_ENTRY_SEPERATOR,
    { N_("RAW Block Device Name"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uiperipheral_set_rawdevice_name, (ui_callback_data_t)"RawDriveDriver", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_driveplus4_settings_menu[] = {
    { N_("Drive settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, driveplus4_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

ui_menu_entry_t ui_driveplus4_romset_submenu[] = {
    { N_("Load new 1540 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1540", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1541 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1541", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1541-II ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1541ii", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1551 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1551", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1570 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1570", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1571 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1571", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1581 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1581", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 2000 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName2000", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 4000 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName4000", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
