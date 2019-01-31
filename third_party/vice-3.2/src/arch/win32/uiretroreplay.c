/*
 * uiretroreplay.c - Implementation of the Retro Replay settings dialog box.
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
#include <string.h>
#include <windows.h>
#include <tchar.h>

#include "cartridge.h"
#include "clockport.h"
#include "intl.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uiretroreplay.h"
#include "winmain.h"

static int clockport_ids[CLOCKPORT_MAX_ENTRIES + 1];

static uilib_localize_dialog_param rr_dialog_trans[] = {
    { IDC_RR_FLASH_JUMPER, IDS_RR_FLASH_JUMPER, 0 },
    { IDC_RR_BANK_JUMPER, IDS_RR_BANK_JUMPER, 0 },
    { IDC_RR_REVISION_LABEL, IDS_RR_REVISION_LABEL, 0 },
    { IDC_RR_SAVE_WHEN_CHANGED, IDS_RR_BIOS_SAVE, 0 },
    { IDC_RR_CLOCKPORT_DEVICE_LABEL, IDS_RR_CLOCKPORT_DEVICE_LABEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group rr_main_group[] = {
    { IDC_RR_FLASH_JUMPER, 1 },
    { IDC_RR_BANK_JUMPER, 1 },
    { IDC_RR_SAVE_WHEN_CHANGED, 1 },
    { 0, 0 }
};

static uilib_dialog_group rr_left_group[] = {
    { IDC_RR_REVISION_LABEL, 0 },
    { IDC_RR_CLOCKPORT_DEVICE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group rr_right_group[] = {
    { IDC_RR_REVISION, 0 },
    { IDC_RR_CLOCKPORT_DEVICE, 0 },
    { 0, 0 }
};

static uilib_dialog_group rr_window_group[] = {
    { IDC_RR_FLASH_JUMPER, 0 },
    { IDC_RR_BANK_JUMPER, 0 },
    { IDC_RR_REVISION, 0 },
    { IDC_RR_SAVE_WHEN_CHANGED, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_rr_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int res_value_loop;
    int xpos;
    int current_val = 0;
    RECT rect;
    TCHAR *st_clockport_device_name;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, rr_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, rr_main_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, rr_left_group, &xpos);

    /* move the right group to the correct location */
    uilib_move_group(hwnd, rr_right_group, xpos + 10);

    /* get the max x of the window group */
    uilib_get_group_max_x(hwnd, rr_window_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("RRFlashJumper", &res_value);
    CheckDlgButton(hwnd, IDC_RR_FLASH_JUMPER, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("RRBankJumper", &res_value);
    CheckDlgButton(hwnd, IDC_RR_BANK_JUMPER, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("RRBiosWrite", &res_value);
    CheckDlgButton(hwnd, IDC_RR_SAVE_WHEN_CHANGED, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_RR_REVISION);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT(CARTRIDGE_NAME_RETRO_REPLAY));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT(CARTRIDGE_NAME_NORDIC_REPLAY));
    resources_get_int("RRrevision", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("RRClockPort", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_RR_CLOCKPORT_DEVICE);
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

static void end_rr_dialog(HWND hwnd)
{
    resources_set_int("RRFlashJumper", (IsDlgButtonChecked(hwnd, IDC_RR_FLASH_JUMPER) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RRBankJumper", (IsDlgButtonChecked(hwnd, IDC_RR_BANK_JUMPER) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RRBiosWrite", (IsDlgButtonChecked(hwnd, IDC_RR_SAVE_WHEN_CHANGED) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RRrevision",(int)SendMessage(GetDlgItem(hwnd, IDC_RR_REVISION), CB_GETCURSEL, 0, 0));
    resources_set_int("RRClockPort", clockport_ids[(int)SendMessage(GetDlgItem(hwnd, IDC_RR_CLOCKPORT_DEVICE), CB_GETCURSEL, 0, 0)]);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_rr_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_rr_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_rr_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_RR_SETTINGS_DIALOG, hwnd, dialog_proc);
}
