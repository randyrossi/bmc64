/*
 * uicpclockf83.c - Implementation of the CP Clock F83 (tape port PCF8583 RTC) settings dialog box.
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
#include "uicpclockf83.h"
#include "uilib.h"
#include "winmain.h"

static void enable_cp_clock_f83_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_CP_CLOCK_F83_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_CP_CLOCK_F83_SAVE), is_enabled);
}

static uilib_localize_dialog_param cp_clock_f83_dialog[] = {
    { 0, IDS_CP_CLOCK_F83_CAPTION, -1 },
    { IDC_CP_CLOCK_F83_ENABLE, IDS_CP_CLOCK_F83_ENABLE, 0 },
    { IDC_CP_CLOCK_F83_SAVE, IDS_CP_CLOCK_F83_SAVE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group cp_clock_f83_left_total_group[] = {
    { IDC_CP_CLOCK_F83_ENABLE, 1 },
    { IDC_CP_CLOCK_F83_SAVE, 1 },
    { 0, 0 }
};

static void init_cp_clock_f83_dialog(HWND hwnd)
{
    int res_value;

    uilib_localize_dialog(hwnd, cp_clock_f83_dialog);
    uilib_adjust_group_width(hwnd, cp_clock_f83_left_total_group);

    resources_get_int("CPClockF83", &res_value);
    CheckDlgButton(hwnd, IDC_CP_CLOCK_F83_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("CPClockF83Save", &res_value);
    CheckDlgButton(hwnd, IDC_CP_CLOCK_F83_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);

    enable_cp_clock_f83_controls(hwnd);
}

static void end_cp_clock_f83_dialog(HWND hwnd)
{

    resources_set_int("CPClockF83", (IsDlgButtonChecked(hwnd, IDC_CP_CLOCK_F83_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("CPClockF83Save", (IsDlgButtonChecked(hwnd, IDC_CP_CLOCK_F83_SAVE) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_CP_CLOCK_F83_ENABLE:
                    enable_cp_clock_f83_controls(hwnd);
                    break;
                case IDOK:
                    end_cp_clock_f83_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_cp_clock_f83_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_cp_clock_f83_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)IDD_CP_CLOCK_F83_SETTINGS_DIALOG, hwnd, dialog_proc);
}
