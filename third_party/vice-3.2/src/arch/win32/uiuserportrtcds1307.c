/*
 * uiuserportrtcds1307.c - Implementation of the Userport RTC (DS1307) settings dialog box.
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

#include "intl.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uilib.h"
#include "uiuserportrtcds1307.h"
#include "winmain.h"

static void enable_userport_rtc_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_USERPORT_RTC_DS1307_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_USERPORT_RTC_DS1307_SAVE), is_enabled);
}

static uilib_localize_dialog_param userport_rtc_dialog[] = {
    { 0, IDS_USERPORT_RTC_DS1307_CAPTION, -1 },
    { IDC_USERPORT_RTC_DS1307_ENABLE, IDS_USERPORT_RTC_DS1307_ENABLE, 0 },
    { IDC_USERPORT_RTC_DS1307_SAVE, IDS_USERPORT_RTC_DS1307_SAVE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group userport_rtc_left_total_group[] = {
    { IDC_USERPORT_RTC_DS1307_ENABLE, 1 },
    { IDC_USERPORT_RTC_DS1307_SAVE, 1 },
    { 0, 0 }
};

static void init_userport_rtc_dialog(HWND hwnd)
{
    int res_value;

    uilib_localize_dialog(hwnd, userport_rtc_dialog);
    uilib_adjust_group_width(hwnd, userport_rtc_left_total_group);

    resources_get_int("UserportRTCDS1307", &res_value);
    CheckDlgButton(hwnd, IDC_USERPORT_RTC_DS1307_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("UserportRTCDS1307Save", &res_value);
    CheckDlgButton(hwnd, IDC_USERPORT_RTC_DS1307_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);

    enable_userport_rtc_controls(hwnd);
}

static void end_userport_rtc_dialog(HWND hwnd)
{

    resources_set_int("UserportRTCDS1307", (IsDlgButtonChecked(hwnd, IDC_USERPORT_RTC_DS1307_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("UserportRTCDS1307Save", (IsDlgButtonChecked(hwnd, IDC_USERPORT_RTC_DS1307_SAVE) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_USERPORT_RTC_DS1307_ENABLE:
                    enable_userport_rtc_controls(hwnd);
                    break;
                case IDOK:
                    end_userport_rtc_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_userport_rtc_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_userport_rtc_ds1307_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)IDD_USERPORT_RTC_DS1307_SETTINGS_DIALOG, hwnd, dialog_proc);
}
