/*
 * uirs232user.c - Implementation of the RS232 userport settings dialog box.
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
#include <windows.h>
#include <tchar.h>

#include "intl.h"
#include "lib.h"
#include "res.h"
#include "resources.h"
#include "rs232.h"
#include "translate.h"
#include "uirs232user.h"
#include "winmain.h"
#include "uilib.h"

#define NUM_OF_BAUDRATES 6

static const int baudrates[NUM_OF_BAUDRATES] = {
    300,
    600,
    1200,
    2400,
    4800,
    9600
};

static void enable_rs232user_controls(HWND hwnd)
{
    int rs232user_enable;

    rs232user_enable = 0;

    if (IsDlgButtonChecked(hwnd, IDC_RS232USER_ENABLE) == BST_CHECKED) {
        rs232user_enable = 1;
    }

    EnableWindow(GetDlgItem(hwnd, IDC_RS232USER_DEVICE), rs232user_enable);
    EnableWindow(GetDlgItem(hwnd, IDC_RS232USER_BAUDRATE), rs232user_enable);
} 

static uilib_localize_dialog_param rs232user_dialog[] = {
    { 0, IDS_RS232USER_CAPTION, -1 },
    { IDC_RS232USER_ENABLE, IDS_RS232USER_ENABLE, 0 },
    { IDC_RS232USER_DEVICE_LABEL, IDS_RS232USER_DEVICE, 0 },
    { IDC_RS232USER_BAUDRATE_LABEL, IDS_RS232USER_BAUD, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group rs232user_leftgroup[] = {
    { IDC_RS232USER_DEVICE_LABEL, 0 },
    { IDC_RS232USER_BAUDRATE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group rs232user_rightgroup[] = {
    { IDC_RS232USER_DEVICE, 0 },
    { IDC_RS232USER_BAUDRATE, 0 },
    { 0, 0 }
};

static void init_rs232user_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    RECT rect;
    int res_value;
    int res_value_loop;
    int active_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, rs232user_dialog);
    uilib_get_group_extent(hwnd, rs232user_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, rs232user_leftgroup);
    uilib_move_group(hwnd, rs232user_rightgroup, xsize + 30);

    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xsize + 160, rect.bottom - rect.top, TRUE);

    resources_get_int("RsUserEnable", &res_value);
    CheckDlgButton(hwnd, IDC_RS232USER_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_RS232USER_DEVICE);
    for (res_value_loop = 0; res_value_loop < RS232_NUM_DEVICES; res_value_loop++) {
        TCHAR st[20];

        lib_sntprintf(st, 20, intl_translate_tcs(IDS_RS232_DEVICE_I), res_value_loop + 1);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("RsUserDev", &res_value);
    active_value = 0;
    for (res_value_loop = 0; res_value_loop < RS232_NUM_DEVICES; res_value_loop++) {
        if (res_value_loop == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_RS232USER_BAUDRATE);
    for (res_value_loop = 0; res_value_loop < NUM_OF_BAUDRATES; res_value_loop++) {
        TCHAR st[10];

        _itot(baudrates[res_value_loop], st, 10);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }

    resources_get_int("RsUserBaud", &res_value);
    active_value = 0;
    for (res_value_loop = 0; res_value_loop < NUM_OF_BAUDRATES; res_value_loop++) {
        if (baudrates[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    enable_rs232user_controls(hwnd);
}

static void end_rs232user_dialog(HWND hwnd)
{
    resources_set_int("RsUserEnable", (IsDlgButtonChecked(hwnd, IDC_RS232USER_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RsUserDev", (int)SendMessage(GetDlgItem(hwnd, IDC_RS232USER_DEVICE), CB_GETCURSEL, 0, 0));
    resources_set_int("RsUserBaud", baudrates[SendMessage(GetDlgItem(hwnd, IDC_RS232USER_BAUDRATE), CB_GETCURSEL, 0, 0)]);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_RS232USER_ENABLE:
                    enable_rs232user_controls(hwnd);
                    break;
                case IDOK:
                    end_rs232user_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_rs232user_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_rs232user_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_RS232USER_SETTINGS_DIALOG, hwnd, dialog_proc);
}
