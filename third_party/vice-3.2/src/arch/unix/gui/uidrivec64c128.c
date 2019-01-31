/*
 * uidrivec64c128.c
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
#include "uidrivec64c128.h"
#include "uidriveiec.h"
#include "uimenu.h"
#include "uiromset.h"

UI_MENU_DEFINE_RADIO(Drive8ParallelCable)
UI_MENU_DEFINE_RADIO(Drive9ParallelCable)
UI_MENU_DEFINE_RADIO(Drive10ParallelCable)
UI_MENU_DEFINE_RADIO(Drive11ParallelCable)
UI_MENU_DEFINE_TOGGLE_COND(Drive8ProfDOS, Drive8Type, drive_check_profdos)
UI_MENU_DEFINE_TOGGLE_COND(Drive9ProfDOS, Drive9Type, drive_check_profdos)
UI_MENU_DEFINE_TOGGLE_COND(Drive10ProfDOS, Drive10Type, drive_check_profdos)
UI_MENU_DEFINE_TOGGLE_COND(Drive11ProfDOS, Drive11Type, drive_check_profdos)
UI_MENU_DEFINE_TOGGLE_COND(Drive8SuperCard, Drive8Type, drive_check_supercard)
UI_MENU_DEFINE_TOGGLE_COND(Drive9SuperCard, Drive9Type, drive_check_supercard)
UI_MENU_DEFINE_TOGGLE_COND(Drive10SuperCard, Drive10Type, drive_check_supercard)
UI_MENU_DEFINE_TOGGLE_COND(Drive11SuperCard, Drive11Type, drive_check_supercard)
UI_MENU_DEFINE_TOGGLE_COND(Drive8StarDos, Drive8Type, drive_check_stardos)
UI_MENU_DEFINE_TOGGLE_COND(Drive9StarDos, Drive9Type, drive_check_stardos)
UI_MENU_DEFINE_TOGGLE_COND(Drive10StarDos, Drive10Type, drive_check_stardos)
UI_MENU_DEFINE_TOGGLE_COND(Drive11StarDos, Drive11Type, drive_check_stardos)

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
static ui_menu_entry_t set_drive##x##_parallel_cable_submenu[] = {                               \
    { N_("None"), UI_MENU_TYPE_TICK,                                                             \
      (ui_callback_t)radio_Drive##y##ParallelCable, (ui_callback_data_t)DRIVE_PC_NONE, NULL,     \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                 \
    { N_("Standard Userport"), UI_MENU_TYPE_TICK,                                                \
      (ui_callback_t)radio_Drive##y##ParallelCable,(ui_callback_data_t)DRIVE_PC_STANDARD, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                 \
    { N_("Dolphin DOS 3"), UI_MENU_TYPE_TICK,                                                    \
      (ui_callback_t)radio_Drive##y##ParallelCable, (ui_callback_data_t)DRIVE_PC_DD3, NULL,      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                 \
    { N_("Formel 64"), UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##ParallelCable, (ui_callback_data_t)DRIVE_PC_FORMEL64, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                 \
    UI_MENU_ENTRY_LIST_END                                                                       \
}

SET_DRIVE_PAR_CABLE_MENU(0, 8);
SET_DRIVE_PAR_CABLE_MENU(1, 9);
SET_DRIVE_PAR_CABLE_MENU(2, 10);
SET_DRIVE_PAR_CABLE_MENU(3, 11);

#define UIDRIVEC64C128_DRIVE_EXPANSION_MENU(x, y)                                                          \
ui_menu_entry_t uidrivec64c128_drive##x##_expansion_submenu[] = {                                          \
    { N_("Parallel cable"), UI_MENU_TYPE_NORMAL,                                                           \
      (ui_callback_t)parallel_cable_control, (ui_callback_data_t)x, set_drive##x##_parallel_cable_submenu, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                           \
    UI_MENU_ENTRY_SEPERATOR,                                                                               \
    { "", UI_MENU_TYPE_NONE,                                                                               \
      NULL, NULL, uidriveiec_drive##x##_ram_expansion_submenu,                                             \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                           \
    UI_MENU_ENTRY_SEPERATOR,                                                                               \
    { N_("Enable Professional DOS"), UI_MENU_TYPE_TICK,                                                    \
      (ui_callback_t)toggle_Drive##y##ProfDOS, NULL, NULL,                                                 \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                           \
    { N_("Enable SuperCard+"), UI_MENU_TYPE_TICK,                                                          \
      (ui_callback_t)toggle_Drive##y##SuperCard, NULL, NULL,                                               \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                           \
    { N_("Enable StarDOS"), UI_MENU_TYPE_TICK,                                                             \
      (ui_callback_t)toggle_Drive##y##StarDos, NULL, NULL,                                                 \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                           \
    UI_MENU_ENTRY_LIST_END                                                                                 \
}

UIDRIVEC64C128_DRIVE_EXPANSION_MENU(0, 8);
UIDRIVEC64C128_DRIVE_EXPANSION_MENU(1, 9);
UIDRIVEC64C128_DRIVE_EXPANSION_MENU(2, 10);
UIDRIVEC64C128_DRIVE_EXPANSION_MENU(3, 11);

ui_menu_entry_t uidrivec64c128_expansion_romset_submenu[] = {
    { N_("Load new Professional DOS 1571 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DriveProfDOS1571Name", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new SuperCard+ ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DriveSuperCardName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new StarDOS ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DriveStarDosName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
