/*
 * uiscpu64.c - Implementation of the scpu64 settings dialog box.
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

#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uilib.h"
#include "uisoundexpander.h"
#include "winmain.h"
#include "intl.h"

#include "uiscpu64.h"


static uilib_localize_dialog_param scpu64_dialog[] = {
    { 0, IDS_SCPU64_CAPTION, -1 },
    { IDC_SCPU64_SIMM_SIZE_LABEL, IDS_SCPU64_SIMM_SIZE, 0 },
    { IDC_SCPU64_JIFFY_SWITCH_ENABLE, IDS_SCPU64_JIFFY_SWITCH_ENABLE, 0 },
    { IDC_SCPU64_SPEED_SWITCH_ENABLE, IDS_SCPU64_SPEED_SWITCH_ENABLE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group scpu64_leftgroup[] = {
    { IDC_SCPU64_SIMM_SIZE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group scpu64_left_total_group[] = {
    { IDC_SCPU64_JIFFY_SWITCH_ENABLE, 1 },
    { IDC_SCPU64_SPEED_SWITCH_ENABLE, 1 },
    { IDC_SCPU64_SIMM_SIZE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group scpu64_rightgroup[] = {
    { IDC_SCPU64_SIMM_SIZE, 0 },
    { 0, 0 }
};

static void init_scpu64_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, scpu64_dialog);

    uilib_get_group_extent(hwnd, scpu64_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, scpu64_left_total_group);
    uilib_move_group(hwnd, scpu64_rightgroup, xsize + 30);

    resources_get_int("JiffySwitch", &res_value);
    CheckDlgButton(hwnd, IDC_SCPU64_JIFFY_SWITCH_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("SpeedSwitch", &res_value);
    CheckDlgButton(hwnd, IDC_SCPU64_SPEED_SWITCH_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_SCPU64_SIMM_SIZE);

    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"0 MB");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"1 MB");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"4 MB");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"8 MB");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"16 MB");
    resources_get_int("SIMMSize", &res_value);
    switch (res_value) {
        case 0:
            SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)0, 0);
            break;
        case 1:
            SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)1, 0);
            break;
        case 4:
            SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)2, 0);
            break;
        case 8:
            SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)3, 0);
            break;
        case 16:
            SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)4, 0);
            break;
    }
}

static void end_scpu64_dialog(HWND hwnd)
{
    int val;
    int size = 0;

    resources_set_int("JiffySwitch", (IsDlgButtonChecked(hwnd, IDC_SCPU64_JIFFY_SWITCH_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("SpeedSwitch", (IsDlgButtonChecked(hwnd, IDC_SCPU64_SPEED_SWITCH_ENABLE) == BST_CHECKED ? 1 : 0 ));

    val = SendMessage(GetDlgItem(hwnd, IDC_SCPU64_SIMM_SIZE), CB_GETCURSEL, 0, 0);

    switch (val) {
        case 0:
            size = 0;
            break;
        case 1:
            size = 1;
            break;
        case 2:
            size = 4;
            break;
        case 3:
            size = 8;
            break;
        case 4:
            size = 16;
            break;
    }
    resources_set_int("SIMMSize", size);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_scpu64_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_scpu64_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_scpu64_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)IDD_SCPU64_SETTINGS_DIALOG, hwnd, dialog_proc);
}
