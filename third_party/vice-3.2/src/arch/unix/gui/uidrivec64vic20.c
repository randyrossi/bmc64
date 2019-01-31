/*
 * uidrivec64vic20.c
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
#include "uidrive.h"
#include "uidrivec64vic20.h"
#include "uidriveiec.h"
#include "uimenu.h"
#include "uiromset.h"

#define UIDRIVEC64VIC20_SET_DRIVE_TYPE_MENU(x, y)                                       \
ui_menu_entry_t uidrivec64vic20_set_drive##x##_type_submenu[] = {                       \
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
    { "2031", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_2031, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "2040", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_2040, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "3040", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_3040, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "4040", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_4040, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1001", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1001, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "8050", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_8050, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "8250", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_8250, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    UI_MENU_ENTRY_LIST_END                                                              \
}

#define UIDRIVEC64VIC20_SET_DRIVE_TYPE_MENU_SHORT(x, y)                                 \
ui_menu_entry_t uidrivec64vic20_set_drive##x##_type_submenu[] = {                       \
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
    { "2031", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_2031, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    { "1001", UI_MENU_TYPE_TICK,                                                        \
      (ui_callback_t)radio_Drive##y##Type, (ui_callback_data_t)DRIVE_TYPE_1001, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                        \
    UI_MENU_ENTRY_LIST_END                                                              \
}

UIDRIVEC64VIC20_SET_DRIVE_TYPE_MENU(0, 8);
UIDRIVEC64VIC20_SET_DRIVE_TYPE_MENU_SHORT(1, 9);
UIDRIVEC64VIC20_SET_DRIVE_TYPE_MENU(2, 10);
UIDRIVEC64VIC20_SET_DRIVE_TYPE_MENU_SHORT(3, 11);

/* ------------------------------------------------------------------------- */

ui_menu_entry_t ui_drivec64vic20_romset_submenu[] = {
    { N_("Load new 1540 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1540", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1541 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1541", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1541-II ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1541ii", NULL,
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
    { N_("Load new 2031 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName2031", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 2040 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName2040", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 3040 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName3040", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 4040 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName4040", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 1001 ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"DosName1001", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
