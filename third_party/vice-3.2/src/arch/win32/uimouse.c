/*
 * uimouse.c - Implementation of the mouse settings dialog box.
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

#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uimouse.h"
#include "winmain.h"

static int smart = 0;

static void enable_mouse_controls(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDC_SMART_MOUSE_RTC_SAVE), 1);
}

static uilib_localize_dialog_param mouse_dialog_trans[] = {
    { IDC_MOUSE_SENSITIVITY_LABEL, IDS_MOUSE_SENSITIVITY_LABEL, 0 },
    { IDC_SMART_MOUSE_RTC_SAVE, IDS_SMART_MOUSE_RTC_SAVE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param mouse_no_smart_dialog_trans[] = {
    { IDC_MOUSE_SENSITIVITY_LABEL, IDS_MOUSE_SENSITIVITY_LABEL, 0 },
    { IDC_SMART_MOUSE_RTC_SAVE, IDS_SMART_MOUSE_RTC_SAVE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group mouse_left_group[] = {
    { IDC_MOUSE_SENSITIVITY_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group mouse_right_group[] = {
    { IDC_MOUSE_SENSITIVITY, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_mouse_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    if (smart) {
        uilib_localize_dialog(hwnd, mouse_dialog_trans);
    } else {
        uilib_localize_dialog(hwnd, mouse_no_smart_dialog_trans);
    }

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, mouse_left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, mouse_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, mouse_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, mouse_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_MOUSE_SENSITIVITY);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"5");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"10");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"15");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"20");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"25");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"30");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"35");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"40");
    resources_get_int("MouseSensitivity", &res_value);
    res_value = (int)(res_value / 5) * 5;
    if (res_value < 5) {
        res_value = 5;
    }
    if (res_value > 40) {
        res_value = 40;
    }   
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)(res_value / 5) - 1, 0);

    if (smart) {
        resources_get_int("SmartMouseRTCSave", &res_value);
        CheckDlgButton(hwnd, IDC_SMART_MOUSE_RTC_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);
        enable_mouse_controls(hwnd);
    }
}

static void end_mouse_dialog(HWND hwnd)
{
    resources_set_int("MouseSensitivity",((int)SendMessage(GetDlgItem(hwnd, IDC_MOUSE_SENSITIVITY), CB_GETCURSEL, 0, 0) + 1) * 5);
    if (smart) {
        resources_set_int("SmartMouseRTCSave", (IsDlgButtonChecked(hwnd, IDC_SMART_MOUSE_RTC_SAVE) == BST_CHECKED ? 1 : 0 ));
    }
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_mouse_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_mouse_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_mouse_settings_dialog(HWND hwnd, int smartmouse)
{
    smart = smartmouse;

    if (smart) {
        DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_MOUSE_SETTINGS_DIALOG, hwnd, dialog_proc);
    } else {
        DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_MOUSE_SETTINGS_NO_SMART_DIALOG, hwnd, dialog_proc);
    }
}
