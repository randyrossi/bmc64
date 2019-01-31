/*
 * petui.c - PET-specific user interface.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include <windows.h>

#include "debug.h"
#include "petui.h"
#include "res.h"
#include "translate.h"
#include "ui.h"
#include "uiacia.h"
#include "uicpclockf83.h"
#include "uidrivepetcbm2.h"
#include "uiiocollisions.h"
#include "uijoyport.h"
#include "uijoystick.h"
#include "uikeyboard.h"
#include "uikeymap.h"
#include "uilib.h"
#include "uimouse.h"
#include "uipetcolour.h"
#include "uipetdww.h"
#include "uipetmodel.h"
#include "uipetreu.h"
#include "uipetset.h"
#include "uirom.h"
#include "uisampler.h"
#include "uisidcart.h"
#include "uitapelog.h"
#include "uiuserportrtc58321a.h"
#include "uiuserportrtcds1307.h"
#include "uivideo.h"
#include "videoarch.h"
#include "winmain.h"

static const ui_menu_toggle_t pet_ui_menu_toggles[] = {
    { "UserportDAC", IDM_TOGGLE_PET_USERPORT_DAC },
    { "PETHRE", IDM_TOGGLE_PETHRE },
    { "Mouse", IDM_MOUSE },
    { "UserportDAC", IDM_TOGGLE_PET_USERPORT_DAC },
    { "Datasette", IDM_TOGGLE_DATASETTE },
    { "TapeSenseDongle", IDM_TOGGLE_TAPE_SENSE_DONGLE },
    { "DTLBasicDongle", IDM_TOGGLE_DTL_BASIC_DONGLE },
    { NULL, 0 }
};

static const uirom_settings_t uirom_settings[] = {
    { UIROM_TYPE_MAIN, TEXT("Kernal"), "KernalName",
      IDC_PETROM_KERNAL_FILE, IDC_PETROM_KERNAL_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("Basic"), "BasicName",
      IDC_PETROM_BASIC_FILE, IDC_PETROM_BASIC_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("Character"), "ChargenName",
      IDC_PETROM_CHARGEN_FILE, IDC_PETROM_CHARGEN_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("Editor"), "EditorName",
      IDC_PETROM_EDITOR_FILE, IDC_PETROM_EDITOR_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("ROM9"), "RomModule9Name",
      IDC_PETROM_ROM9_FILE, IDC_PETROM_ROM9_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("ROMA"), "RomModuleAName",
      IDC_PETROM_ROMA_FILE, IDC_PETROM_ROMA_BROWSE },
    { UIROM_TYPE_MAIN, TEXT("ROMB"), "RomModuleBName",
      IDC_PETROM_ROMB_FILE, IDC_PETROM_ROMB_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("2031"), "DosName2031",
      IDC_DRIVEROM_2031_FILE, IDC_DRIVEROM_2031_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("2040"), "DosName2040",
      IDC_DRIVEROM_2040_FILE, IDC_DRIVEROM_2040_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("3040"), "DosName3040",
      IDC_DRIVEROM_3040_FILE, IDC_DRIVEROM_3040_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("4040"), "DosName4040",
      IDC_DRIVEROM_4040_FILE, IDC_DRIVEROM_4040_BROWSE },
    { UIROM_TYPE_DRIVE, TEXT("1001"), "DosName1001",
      IDC_DRIVEROM_1001_FILE, IDC_DRIVEROM_1001_BROWSE },
    { 0, NULL, NULL, 0, 0 }
};

ui_menu_translation_table_t petui_menu_translation_table[] = {
    { IDM_EXIT, IDS_MI_EXIT },
    { IDM_ABOUT, IDS_MI_ABOUT },
    { IDM_HELP, IDS_MP_HELP },
    { IDM_PAUSE, IDS_MI_PAUSE },
    { IDM_SINGLE_FRAME_ADVANCE, IDS_MI_SINGLE_FRAME_ADVANCE },
    { IDM_EDIT_COPY, IDS_MI_EDIT_COPY },
    { IDM_EDIT_PASTE, IDS_MI_EDIT_PASTE },
    { IDM_AUTOSTART, IDS_MI_AUTOSTART },
    { IDM_RESET_HARD, IDS_MI_RESET_HARD },
    { IDM_RESET_SOFT, IDS_MI_RESET_SOFT },
    { IDM_RESET_DRIVE8, IDS_MI_DRIVE8 },
    { IDM_RESET_DRIVE9, IDS_MI_DRIVE9 },
    { IDM_RESET_DRIVE10, IDS_MI_DRIVE10 },
    { IDM_RESET_DRIVE11, IDS_MI_DRIVE11 },
    { IDM_ATTACH_8, IDS_MI_DRIVE8 },
    { IDM_ATTACH_9, IDS_MI_DRIVE9 },
    { IDM_ATTACH_10, IDS_MI_DRIVE10 },
    { IDM_ATTACH_11, IDS_MI_DRIVE11 },
    { IDM_DETACH_8, IDS_MI_DRIVE8 },
    { IDM_DETACH_9, IDS_MI_DRIVE9 },
    { IDM_DETACH_10, IDS_MI_DRIVE10 },
    { IDM_DETACH_11, IDS_MI_DRIVE11 },
    { IDM_ATTACH_TAPE, IDS_MI_ATTACH_TAPE },
    { IDM_DETACH_TAPE, IDS_MI_DETACH_TAPE },
    { IDM_DETACH_ALL, IDS_MI_DETACH_ALL },
    { IDM_TOGGLE_SOUND, IDS_MI_TOGGLE_SOUND },
    { IDM_TOGGLE_CRTCDOUBLESIZE, IDS_MI_TOGGLE_DOUBLESIZE },
    { IDM_TOGGLE_CRTCDOUBLESCAN, IDS_MI_TOGGLE_DOUBLESCAN },
    { IDM_TOGGLE_CRTCDOUBLEVERTICAL, IDS_MI_TOGGLE_DOUBLEVERTICAL },
    { IDM_TOGGLE_DRIVE_TRUE_EMULATION, IDS_MI_DRIVE_TRUE_EMULATION },
    { IDM_TOGGLE_DRIVE_SOUND_EMULATION, IDS_MI_DRIVE_SOUND_EMULATION },
    { IDM_TOGGLE_CRTCVIDEOCACHE, IDS_MI_TOGGLE_VIDEOCACHE },
    { IDM_DRIVE_SETTINGS, IDS_MI_DRIVE_SETTINGS },
    { IDM_FLIP_ADD, IDS_MI_FLIP_ADD },
    { IDM_FLIP_REMOVE, IDS_MI_FLIP_REMOVE },
    { IDM_FLIP_NEXT, IDS_MI_FLIP_NEXT },
    { IDM_FLIP_PREVIOUS, IDS_MI_FLIP_PREVIOUS },
    { IDM_FLIP_LOAD, IDS_MI_FLIP_LOAD },
    { IDM_FLIP_SAVE, IDS_MI_FLIP_SAVE },
    { IDM_DATASETTE_CONTROL_STOP, IDS_MI_DATASETTE_STOP },
    { IDM_DATASETTE_CONTROL_START, IDS_MI_DATASETTE_START },
    { IDM_DATASETTE_CONTROL_FORWARD, IDS_MI_DATASETTE_FORWARD },
    { IDM_DATASETTE_CONTROL_REWIND, IDS_MI_DATASETTE_REWIND },
    { IDM_DATASETTE_CONTROL_RECORD, IDS_MI_DATASETTE_RECORD },
    { IDM_DATASETTE_CONTROL_RESET, IDS_MI_DATASETTE_RESET },
    { IDM_DATASETTE_RESET_COUNTER, IDS_MI_DATASETTE_RESET_COUNTER },
    { IDM_MONITOR, IDS_MI_MONITOR },
#ifdef DEBUG
    { IDM_DEBUG_MODE_NORMAL, IDS_MI_DEBUG_MODE_NORMAL },
    { IDM_DEBUG_MODE_SMALL, IDS_MI_DEBUG_MODE_SMALL },
    { IDM_DEBUG_MODE_HISTORY, IDS_MI_DEBUG_MODE_HISTORY },
    { IDM_DEBUG_MODE_AUTOPLAY, IDS_MI_DEBUG_MODE_AUTOPLAY },
    { IDM_TOGGLE_MAINCPU_TRACE, IDS_MI_TOGGLE_MAINCPU_TRACE },
    { IDM_TOGGLE_DRIVE0CPU_TRACE, IDS_MI_TOGGLE_DRIVE0CPU_TRACE },
    { IDM_TOGGLE_DRIVE1CPU_TRACE, IDS_MI_TOGGLE_DRIVE1CPU_TRACE },
#endif
    { IDM_SNAPSHOT_LOAD, IDS_MI_SNAPSHOT_LOAD },
    { IDM_SNAPSHOT_SAVE, IDS_MI_SNAPSHOT_SAVE },
    { IDM_LOADQUICK, IDS_MI_LOADQUICK },
    { IDM_SAVEQUICK, IDS_MI_SAVEQUICK },
    { IDM_EVENT_TOGGLE_RECORD, IDS_MI_EVENT_TOGGLE_RECORD },
    { IDM_EVENT_TOGGLE_PLAYBACK, IDS_MI_EVENT_TOGGLE_PLAYBACK },
    { IDM_EVENT_SETMILESTONE, IDS_MI_EVENT_SETMILESTONE },
    { IDM_EVENT_RESETMILESTONE, IDS_MI_EVENT_RESETMILESTONE },
    { IDM_EVENT_START_MODE_SAVE, IDS_MI_EVENT_START_MODE_SAVE },
    { IDM_EVENT_START_MODE_LOAD, IDS_MI_EVENT_START_MODE_LOAD },
    { IDM_EVENT_START_MODE_RESET, IDS_MI_EVENT_START_MODE_RESET },
    { IDM_EVENT_START_MODE_PLAYBACK, IDS_MI_EVENT_START_MODE_PLAYBCK },
    { IDM_EVENT_DIRECTORY, IDS_MI_EVENT_DIRECTORY },
    { IDM_JAM_ACTION_ASK, IDS_MI_JAM_ACTION_ASK },
    { IDM_JAM_ACTION_CONTINUE, IDS_MI_JAM_ACTION_CONTINUE },
    { IDM_JAM_ACTION_START_MONITOR, IDS_MI_JAM_ACTION_START_MONITOR },
    { IDM_JAM_ACTION_RESET, IDS_MI_JAM_ACTION_RESET },
    { IDM_JAM_ACTION_HARD_RESET, IDS_MI_JAM_ACTION_HARD_RESET },
    { IDM_JAM_ACTION_QUIT_EMULATOR, IDS_MI_JAM_ACTION_QUIT_EMULATOR },
    { IDM_MEDIAFILE, IDS_MI_MEDIAFILE },
    { IDM_SOUND_RECORD_START, IDS_MI_SOUND_RECORD_START },
    { IDM_SOUND_RECORD_STOP, IDS_MI_SOUND_RECORD_STOP },
    { IDM_REFRESH_RATE_AUTO, IDS_MI_REFRESH_RATE_AUTO },
    { IDM_MAXIMUM_SPEED_NO_LIMIT, IDS_MI_MAXIMUM_SPEED_NO_LIMIT },
    { IDM_MAXIMUM_SPEED_CUSTOM, IDS_MI_MAXIMUM_SPEED_CUSTOM },
    { IDM_TOGGLE_WARP_MODE, IDS_MI_TOGGLE_WARP_MODE },
    { IDM_TOGGLE_ALWAYSONTOP, IDS_MI_TOGGLE_ALWAYSONTOP },
    { IDM_TOGGLE_CPU_AFFINITY, IDS_MI_TOGGLE_CPU_AFFINITY },
    { IDM_SWAP_EXTRA_JOYSTICK, IDS_MI_SWAP_EXTRA_JOYSTICK },
    { IDM_ALLOW_JOY_OPPOSITE_TOGGLE, IDS_MI_ALLOW_JOY_OPPOSITE },
    { IDM_JOYKEYS_TOGGLE, IDS_MI_JOYKEYS_TOGGLE },
    { IDM_TOGGLE_VIRTUAL_DEVICES, IDS_MI_TOGGLE_VIRTUAL_DEVICES },
    { IDM_PETMODEL_SETTINGS, IDS_MI_PETMODEL_SETTINGS },
    { IDM_MOUSE, IDS_MI_MOUSE },
    { IDM_AUTOSTART_SETTINGS, IDS_MI_AUTOSTART_SETTINGS },
    { IDM_VIDEO_SETTINGS, IDS_MI_VIDEO_SETTINGS },
    { IDM_DEVICEMANAGER, IDS_MI_DEVICEMANAGER },
    { IDM_JOYPORT_SETTINGS, IDS_MI_JOYPORT_SETTINGS },
    { IDM_EXTRA_JOY_SETTINGS, IDS_MI_USERPORT_JOY_SETTINGS },
    { IDM_KEYBOARD_SETTINGS, IDS_MI_KEYBOARD_SETTINGS },
    { IDM_MOUSE_SETTINGS, IDS_MI_MOUSE_SETTINGS },
    { IDM_SOUND_SETTINGS, IDS_MI_SOUND_SETTINGS },
    { IDM_SAMPLER_SETTINGS, IDS_MI_SAMPLER_SETTINGS },
    { IDM_ROM_SETTINGS, IDS_MI_ROM_SETTINGS },
    { IDM_RAM_SETTINGS, IDS_MI_RAM_SETTINGS },
    { IDM_DATASETTE_SETTINGS, IDS_MI_DATASETTE_SETTINGS },
    { IDM_RS232_SETTINGS, IDS_MI_RS232_SETTINGS },
    { IDM_ACIA_SETTINGS, IDS_MI_ACIA_SETTINGS },
    { IDM_SETTINGS_SAVE_FILE, IDS_MI_SETTINGS_SAVE_FILE },
    { IDM_SETTINGS_LOAD_FILE, IDS_MI_SETTINGS_LOAD_FILE },
    { IDM_SETTINGS_SAVE, IDS_MI_SETTINGS_SAVE },
    { IDM_SETTINGS_LOAD, IDS_MI_SETTINGS_LOAD },
    { IDM_SETTINGS_DEFAULT, IDS_MI_SETTINGS_DEFAULT },
    { IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT, IDS_MI_SAVE_SETTINGS_ON_EXIT },
    { IDM_TOGGLE_CONFIRM_ON_EXIT, IDS_MI_CONFIRM_ON_EXIT },
    { IDM_TOGGLE_PET_USERPORT_DAC, IDS_MI_TOGGLE_PET_USERPORT_DAC },
    { IDM_LANG_EN, IDS_MI_LANG_EN },
    { IDM_LANG_DA, IDS_MI_LANG_DA },
    { IDM_LANG_DE, IDS_MI_LANG_DE },
    { IDM_LANG_ES, IDS_MI_LANG_ES },
    { IDM_LANG_FR, IDS_MI_LANG_FR },
    { IDM_LANG_HU, IDS_MI_LANG_HU },
    { IDM_LANG_IT, IDS_MI_LANG_IT },
    { IDM_LANG_KO, IDS_MI_LANG_KO },
    { IDM_LANG_NL, IDS_MI_LANG_NL },
    { IDM_LANG_PL, IDS_MI_LANG_PL },
    { IDM_LANG_RU, IDS_MI_LANG_RU },
    { IDM_LANG_SV, IDS_MI_LANG_SV },
    { IDM_LANG_TR, IDS_MI_LANG_TR },
    { IDM_CMDLINE, IDS_MI_CMDLINE },
    { IDM_FEATURES, IDS_MI_FEATURES },
    { IDM_CONTRIBUTORS, IDS_MI_CONTRIBUTORS },
    { IDM_LICENSE, IDS_MI_LICENSE },
    { IDM_WARRANTY, IDS_MI_WARRANTY },
#ifdef HAVE_D3D9_H
    { IDM_TOGGLE_FULLSCREEN, IDS_MI_TOGGLE_FULLSCREEN },
#endif
    { IDM_PET_SETTINGS, IDS_MI_PET_SETTINGS },
    { IDM_PETCOLOUR_SETTINGS, IDS_MI_PETCOLOUR_SETTINGS },
    { IDM_PETDWW_SETTINGS, IDS_MI_PETDWW_SETTINGS },
    { IDM_PETREU_SETTINGS, IDS_MI_PETREU_SETTINGS },
    { IDM_SIDCART_SETTINGS, IDS_MI_SIDCART_SETTINGS },
    { IDM_NETWORK_SETTINGS, IDS_MI_NETWORK_SETTINGS },
    { IDM_USERPORT_RTC_58321A_SETTINGS, IDS_MI_USERPORT_RTC_58321A_SETTINGS },
    { IDM_USERPORT_RTC_DS1307_SETTINGS, IDS_MI_USERPORT_RTC_DS1307_SETTINGS },
    { IDM_TOGGLE_PET_USERPORT_DAC, IDS_MI_TOGGLE_PET_USERPORT_DAC },
    { IDM_TAPELOG_SETTINGS, IDS_MI_TAPELOG_SETTINGS },
    { IDM_CP_CLOCK_F83_SETTINGS, IDS_MI_CP_CLOCK_F83_SETTINGS },
    { IDM_TOGGLE_DATASETTE, IDS_MI_TOGGLE_DATASETTE },
    { IDM_TOGGLE_TAPE_SENSE_DONGLE, IDS_MI_TOGGLE_TAPE_SENSE_DONGLE },
    { IDM_TOGGLE_DTL_BASIC_DONGLE, IDS_MI_TOGGLE_DTL_BASIC_DONGLE },
    { IDM_IO_COLLISION_HANDLING, IDS_MI_IO_COLLISION_HANDLING },
    { 0, 0 }
};

ui_popup_translation_table_t petui_popup_translation_table[] = {
    { 1, IDS_MP_FILE, NULL },
    { 2, IDS_MP_ATTACH_DISK_IMAGE, NULL },
    { 2, IDS_MP_DETACH_DISK_IMAGE, NULL },
    { 2, IDS_MP_FLIP_LIST, NULL },
    { 2, IDS_MP_DATASETTE_CONTROL, NULL },
    { 2, IDS_MP_RESET, NULL },
    { 2, IDS_MP_DEFAULT_CPU_JAM_ACTION, NULL },
#ifdef DEBUG
    { 2, IDS_MP_DEBUG, NULL },
    { 3, IDS_MP_MODE, NULL },
#endif
    { 1, IDS_MP_EDIT, NULL },
    { 1, IDS_MP_SNAPSHOT, NULL },
    { 2, IDS_MP_RECORDING_START_MODE, NULL },
/*    { 1, IDS_MP_OPTIONS, NULL },*/
    { 1, IDS_MP_SETTINGS, NULL },
    { 2, IDS_MP_REFRESH_RATE, NULL },
    { 2, IDS_MP_MAXIMUM_SPEED, NULL },
    { 2, IDS_MP_SOUND_SETTINGS, NULL },
    { 2, IDS_MP_DRIVE_SETTINGS, NULL },
    { 2, IDS_MP_JOYSTICK_SETTINGS, NULL },
    { 2, IDS_MP_MOUSE_SETTINGS, NULL },
    { 2, IDS_MP_DRIVE_SYNC_FACTOR, NULL },
    { 2, IDS_MP_CARTRIDGE_IO_SETTINGS, NULL },
    { 3, IDS_MP_USERPORT_DEVICES, NULL },
    { 3, IDS_MP_TAPEPORT_DEVICES, NULL },
    { 2, IDS_MP_RS232_SETTINGS, NULL },
    { 1, IDS_MP_LANGUAGE, NULL },
    { 1, IDS_MP_HELP, NULL },
    { 0, 0, NULL }
};

static uilib_localize_dialog_param pet_main_trans[] = {
    { IDC_KERNAL, IDS_KERNAL, 0 },
    { IDC_PETROM_KERNAL_BROWSE, IDS_BROWSE, 0 },
    { IDC_BASIC, IDS_BASIC, 0 },
    { IDC_PETROM_BASIC_BROWSE, IDS_BROWSE, 0 },
    { IDC_CHARACTER, IDS_CHARACTER, 0 },
    { IDC_PETROM_CHARGEN_BROWSE, IDS_BROWSE, 0 },
    { IDC_EDITOR, IDS_EDITOR, 0 },
    { IDC_PETROM_EDITOR_BROWSE, IDS_BROWSE, 0 },
    { IDC_ROM_9, IDS_ROM_9, 0 },
    { IDC_PETROM_ROM9_BROWSE, IDS_BROWSE, 0 },
    { IDC_ROM_A, IDS_ROM_A, 0 },
    { IDC_PETROM_ROMA_BROWSE, IDS_BROWSE, 0 },
    { IDC_ROM_B, IDS_ROM_B, 0 },
    { IDC_PETROM_ROMB_BROWSE, IDS_BROWSE, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param pet_drive_trans[] = {
    { IDC_DRIVEROM_2031_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_2040_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_3040_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_4040_BROWSE, IDS_BROWSE, 0 },
    { IDC_DRIVEROM_1001_BROWSE, IDS_BROWSE, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group pet_main_left_group[] = {
    { IDC_KERNAL, 0 },
    { IDC_BASIC, 0 },
    { IDC_CHARACTER, 0 },
    { IDC_EDITOR, 0 },
    { IDC_ROM_9, 0 },
    { IDC_ROM_A, 0 },
    { IDC_ROM_B, 0 },
    { 0, 0 }
};

static uilib_dialog_group pet_main_middle_group[] = {
    { IDC_PETROM_KERNAL_FILE, 0 },
    { IDC_PETROM_BASIC_FILE, 0 },
    { IDC_PETROM_CHARGEN_FILE, 0 },
    { IDC_PETROM_EDITOR_FILE, 0 },
    { IDC_PETROM_ROM9_FILE, 0 },
    { IDC_PETROM_ROMA_FILE, 0 },
    { IDC_PETROM_ROMB_FILE, 0 },
    { 0, 0 }
};

static uilib_dialog_group pet_main_right_group[] = {
    { IDC_PETROM_KERNAL_BROWSE, 0 },
    { IDC_PETROM_BASIC_BROWSE, 0 },
    { IDC_PETROM_CHARGEN_BROWSE, 0 },
    { IDC_PETROM_EDITOR_BROWSE, 0 },
    { IDC_PETROM_ROM9_BROWSE, 0 },
    { IDC_PETROM_ROMA_BROWSE, 0 },
    { IDC_PETROM_ROMB_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group pet_drive_left_group[] = {
    { IDC_2031, 0 },
    { IDC_2040, 0 },
    { IDC_3040, 0 },
    { IDC_4040, 0 },
    { IDC_1001, 0 },
    { 0, 0 }
};

static uilib_dialog_group pet_drive_middle_group[] = {
    { IDC_DRIVEROM_2031_FILE, 0 },
    { IDC_DRIVEROM_2040_FILE, 0 },
    { IDC_DRIVEROM_3040_FILE, 0 },
    { IDC_DRIVEROM_4040_FILE, 0 },
    { IDC_DRIVEROM_1001_FILE, 0 },
    { 0, 0 }
};

static uilib_dialog_group pet_drive_right_group[] = {
    { IDC_DRIVEROM_2031_BROWSE, 0 },
    { IDC_DRIVEROM_2040_BROWSE, 0 },
    { IDC_DRIVEROM_3040_BROWSE, 0 },
    { IDC_DRIVEROM_4040_BROWSE, 0 },
    { IDC_DRIVEROM_1001_BROWSE, 0 },
    { 0, 0 }
};

static generic_trans_table_t pet_generic_trans[] = {
    { IDC_2031, TEXT("2031") },
    { IDC_2040, TEXT("2040") },
    { IDC_3040, TEXT("3040") },
    { IDC_4040, TEXT("4040") },
    { IDC_1001, TEXT("1001") },
    { 0, NULL }
};

static void pet_ui_specific(WPARAM wparam, HWND hwnd)
{
    switch (wparam) {
        case IDM_PETMODEL_SETTINGS:
            ui_petmodel_settings_dialog(hwnd);
            break;
        case IDM_PET_SETTINGS:
            ui_pet_settings_dialog(hwnd);
            break;
        case IDM_PETCOLOUR_SETTINGS:
            ui_petcolour_settings_dialog(hwnd);
            break;
        case IDM_PETDWW_SETTINGS:
            ui_petdww_settings_dialog(hwnd);
            break;
        case IDM_PETREU_SETTINGS:
            ui_petreu_settings_dialog(hwnd);
            break;
        case IDM_SIDCART_SETTINGS:
            ui_sidcart_settings_dialog(hwnd);
            break;
        case IDM_ROM_SETTINGS:
            uirom_settings_dialog(hwnd, IDD_PETROM_SETTINGS_DIALOG, IDD_PETDRIVEROM_SETTINGS_DIALOG,
                                  uirom_settings,
                                  pet_main_trans, pet_drive_trans, pet_generic_trans,
                                  pet_main_left_group, pet_main_middle_group, pet_main_right_group,
                                  pet_drive_left_group, pet_drive_middle_group, pet_drive_right_group);
            break;
        case IDM_VIDEO_SETTINGS:
            ui_video_settings_dialog(hwnd, UI_VIDEO_CHIP_CRTC, UI_VIDEO_CHIP_NONE);
            break;
        case IDM_DRIVE_SETTINGS:
            uidrivepetcbm2_settings_dialog(hwnd);
            break;
        case IDM_ACIA_SETTINGS:
            ui_acia_settings_dialog(hwnd);
            break;
        case IDM_JOYPORT_SETTINGS:
            ui_joyport_settings_dialog(hwnd, 0, 0, 1, 1, 0);
            break;
        case IDM_EXTRA_JOY_SETTINGS:
            ui_extra_joystick_settings_dialog(hwnd);
            break;
        case IDM_KEYBOARD_SETTINGS:
            ui_keymap_settings_dialog(hwnd);
            break;
        case IDM_MOUSE_SETTINGS:
            ui_mouse_settings_dialog(hwnd, 0);
            break;
        case IDM_SAMPLER_SETTINGS:
            ui_sampler_settings_dialog(hwnd);
            break;
        case IDM_USERPORT_RTC_58321A_SETTINGS:
            ui_userport_rtc_58321a_settings_dialog(hwnd);
            break;
        case IDM_USERPORT_RTC_DS1307_SETTINGS:
            ui_userport_rtc_ds1307_settings_dialog(hwnd);
            break;
        case IDM_TAPELOG_SETTINGS:
            ui_tapelog_settings_dialog(hwnd);
            break;
        case IDM_CP_CLOCK_F83_SETTINGS:
            ui_cp_clock_f83_settings_dialog(hwnd);
            break;
        case IDM_IO_COLLISION_HANDLING:
            ui_iocollision_settings_dialog(hwnd);
            break;
    }
}

int petui_init(void)
{
    ui_register_machine_specific(pet_ui_specific);
    ui_register_menu_toggles(pet_ui_menu_toggles);
    ui_register_translation_tables(petui_menu_translation_table, petui_popup_translation_table);
    return 0;
}

void petui_shutdown(void)
{
}
