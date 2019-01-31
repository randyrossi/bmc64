/*
 * uimmcreplay.c - Implementation of the MMC Replay settings dialog box.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
#include <string.h>
#include <windows.h>
#include <tchar.h>

#include "clockport.h"
#include "intl.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uimmcreplay.h"
#include "winmain.h"

static TCHAR *ui_mmcreplay_sdtype[] = {
    TEXT("Auto"),
    TEXT("MMC"),
    TEXT("SD"),
    TEXT("SDHC"),
    NULL
};

static int clockport_ids[CLOCKPORT_MAX_ENTRIES + 1];

static uilib_localize_dialog_param mmcreplay_dialog_trans[] = {
    { 0, IDS_MMCREPLAY_CAPTION, -1 },
    { IDC_MMCREPLAY_CARDIMAGE_LABEL, IDS_MMCREPLAY_CARDIMAGE_LABEL, 0 },
    { IDC_MMCREPLAY_CARDIMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDC_MMCREPLAY_CARDRW, IDS_MMCREPLAY_READ_WRITE, 0 },
    { IDC_MMCREPLAY_EEPROMIMAGE_LABEL, IDS_MMCREPLAY_EEPROMIMAGE_LABEL, 0 },
    { IDC_MMCREPLAY_EEPROMIMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDC_MMCREPLAY_EEPROMRW, IDS_MMCREPLAY_READ_WRITE, 0 },
    { IDC_MMCREPLAY_RESCUEMODE, IDS_MMCREPLAY_RESCUEMODE, 0 },
    { IDC_MMCREPLAY_SDTYPE_LABEL, IDS_MMCREPLAY_SDTYPE_LABEL, 0 },
    { IDC_MMCREPLAY_CLOCKPORT_DEVICE_LABEL, IDS_MMCREPLAY_CLOCKPORT_DEVICE_LABEL, 0 },
    { IDC_MMCREPLAY_WRITE_ENABLE, IDS_MMCREPLAY_WRITE_ENABLE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group mmcreplay_main_group[] = {
    { IDC_MMCREPLAY_CARDIMAGE_LABEL, 0 },
    { IDC_MMCREPLAY_CARDIMAGE_BROWSE, 1 },
    { IDC_MMCREPLAY_CARDRW, 1 },
    { IDC_MMCREPLAY_EEPROMIMAGE_LABEL, 0 },
    { IDC_MMCREPLAY_EEPROMIMAGE_BROWSE, 1 },
    { IDC_MMCREPLAY_EEPROMRW, 1 },
    { IDC_MMCREPLAY_WRITE_ENABLE, 1 },
    { IDC_MMCREPLAY_RESCUEMODE, 1 },
    { IDC_MMCREPLAY_SDTYPE_LABEL, 0 },
    { IDC_MMCREPLAY_CLOCKPORT_DEVICE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group mmcreplay_top_left_group[] = {
    { IDC_MMCREPLAY_CARDIMAGE_LABEL, 0 },
    { IDC_MMCREPLAY_EEPROMIMAGE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group mmcreplay_top_middle_group[] = {
    { IDC_MMCREPLAY_CARDIMAGE_BROWSE, 1 },
    { IDC_MMCREPLAY_EEPROMIMAGE_BROWSE, 1 },
    { 0, 0 }
};

static uilib_dialog_group mmcreplay_top_right_group[] = {
    { IDC_MMCREPLAY_CARDRW, 1 },
    { IDC_MMCREPLAY_EEPROMRW, 1 },
    { 0, 0 }
};

static uilib_dialog_group mmcreplay_right_group[] = {
    { IDC_MMCREPLAY_CARDRW, 1 },
    { IDC_MMCREPLAY_EEPROMRW, 1 },
    { IDC_MMCREPLAY_WRITE_ENABLE, 1 },
    { IDC_MMCREPLAY_SDTYPE, 0 },
    { IDC_MMCREPLAY_CLOCKPORT_DEVICE, 0 },
    { 0, 0 }
};

static uilib_dialog_group mmcreplay_bottom_middle_group[] = {
    { IDC_MMCREPLAY_SDTYPE_LABEL, 0 },
    { IDC_MMCREPLAY_CLOCKPORT_DEVICE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group mmcreplay_bottom_right_group[] = {
    { IDC_MMCREPLAY_SDTYPE, 0 },
    { IDC_MMCREPLAY_CLOCKPORT_DEVICE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_mmcreplay_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int res_value_loop;
    int current_val = 0;
    int xpos;
    const char *mmcreplay_cardimage_file;
    TCHAR *st_mmcreplay_cardimage_file;
    const char *mmcreplay_eeprom_file;
    TCHAR *st_mmcreplay_eeprom_file;
    TCHAR *st_clockport_device_name;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, mmcreplay_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, mmcreplay_main_group);

    /* get the max x of the top left group */
    uilib_get_group_max_x(hwnd, mmcreplay_top_left_group, &xpos);

    /* move the top middle group to the correct location */
    uilib_move_group(hwnd, mmcreplay_top_middle_group, xpos + 10);

    /* get the max x of the top middle group */
    uilib_get_group_max_x(hwnd, mmcreplay_top_middle_group, &xpos);

    /* move the top right group to the correct location */
    uilib_move_group(hwnd, mmcreplay_top_right_group, xpos + 10);

    /* get the max x of the rescue mode element */
    uilib_get_element_max_x(hwnd, IDC_MMCREPLAY_RESCUEMODE, &xpos);

    /* move the bottom middle group to the correct position */
    uilib_move_group(hwnd, mmcreplay_bottom_middle_group, xpos + 10);

    /* get the max x of the bottom middle group */
    uilib_get_group_max_x(hwnd, mmcreplay_bottom_middle_group, &xpos);

    /* move the bottom right group to the correct position */
    uilib_move_group(hwnd, mmcreplay_bottom_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, mmcreplay_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_string("MMCRCardImage", &mmcreplay_cardimage_file);
    st_mmcreplay_cardimage_file = system_mbstowcs_alloc(mmcreplay_cardimage_file);
    SetDlgItemText(hwnd, IDC_MMCREPLAY_CARDIMAGE, mmcreplay_cardimage_file != NULL ? st_mmcreplay_cardimage_file : TEXT(""));
    system_mbstowcs_free(st_mmcreplay_cardimage_file);

    resources_get_int("MMCRCardRW", &res_value);
    CheckDlgButton(hwnd, IDC_MMCREPLAY_CARDRW, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("MMCRImageWrite", &res_value);
    CheckDlgButton(hwnd, IDC_MMCREPLAY_WRITE_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_string("MMCREEPROMImage", &mmcreplay_eeprom_file);
    st_mmcreplay_eeprom_file = system_mbstowcs_alloc(mmcreplay_eeprom_file);
    SetDlgItemText(hwnd, IDC_MMCREPLAY_EEPROMIMAGE, mmcreplay_eeprom_file != NULL ? st_mmcreplay_eeprom_file : TEXT(""));
    system_mbstowcs_free(st_mmcreplay_eeprom_file);

    resources_get_int("MMCREEPROMRW", &res_value);
    CheckDlgButton(hwnd, IDC_MMCREPLAY_EEPROMRW, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("MMCRRescueMode", &res_value);
    CheckDlgButton(hwnd, IDC_MMCREPLAY_RESCUEMODE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("MMCRSDType", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_MMCREPLAY_SDTYPE);
    for (res_value_loop = 0; ui_mmcreplay_sdtype[res_value_loop]; res_value_loop++) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_mmcreplay_sdtype[res_value_loop]);
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("MMCRClockPort", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_MMCREPLAY_CLOCKPORT_DEVICE);
    for (res_value_loop = 0; clockport_supported_devices[res_value_loop].name; res_value_loop++) {
        st_clockport_device_name = system_mbstowcs_alloc(clockport_supported_devices[res_value_loop].name);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st_clockport_device_name);
        system_mbstowcs_free(st_clockport_device_name);
        clockport_ids[res_value_loop] = clockport_supported_devices[res_value_loop].id;
        if (clockport_ids[res_value_loop] == res_value) {
            current_val = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)current_val, 0);
}

static void end_mmcreplay_dialog(HWND hwnd)
{
    TCHAR st_name[MAX_PATH];
    char name[MAX_PATH];

    GetDlgItemText(hwnd, IDC_MMCREPLAY_CARDIMAGE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("MMCRCardImage", name);

    GetDlgItemText(hwnd, IDC_MMCREPLAY_EEPROMIMAGE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("MMCREEPROMImage", name);

    resources_set_int("MMCRCardRW", (IsDlgButtonChecked(hwnd, IDC_MMCREPLAY_CARDRW) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMCRImageWrite", (IsDlgButtonChecked(hwnd, IDC_MMCREPLAY_WRITE_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMCREEPROMRW", (IsDlgButtonChecked(hwnd, IDC_MMCREPLAY_EEPROMRW) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMCRRescueMode", (IsDlgButtonChecked(hwnd, IDC_MMCREPLAY_RESCUEMODE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMCRSDType", (int)SendMessage(GetDlgItem(hwnd, IDC_MMCREPLAY_SDTYPE), CB_GETCURSEL, 0, 0));
    resources_set_int("MMCRClockPort", clockport_ids[(int)SendMessage(GetDlgItem(hwnd, IDC_MMCREPLAY_CLOCKPORT_DEVICE), CB_GETCURSEL, 0, 0)]);
}

static void browse_mmcreplay_cardimage_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_MMCREPLAY_CARD_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_MMCREPLAY_CARDIMAGE);
}

static void browse_mmcreplay_eeprom_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_MMCREPLAY_EEPROM_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_MMCREPLAY_EEPROMIMAGE);
}


static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_MMCREPLAY_CARDIMAGE_BROWSE:
                    browse_mmcreplay_cardimage_file(hwnd);
                    break;
                case IDC_MMCREPLAY_EEPROMIMAGE_BROWSE:
                    browse_mmcreplay_eeprom_file(hwnd);
                    break;
                case IDOK:
                    end_mmcreplay_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_mmcreplay_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_mmcreplay_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_MMCREPLAY_SETTINGS_DIALOG, hwnd, dialog_proc);
}
