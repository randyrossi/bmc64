/*
 * uids12c887rtc.c - Implementation of the DS12C887 RTC settings dialog box.
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
#include "uids12c887rtc.h"
#include "uilib.h"
#include "winmain.h"
#include "intl.h"

static int ui_c64_ds12c887rtc_base[] = {
    0xd500,
    0xd600,
    0xd700,
    0xde00,
    0xdf00,
    -1
};

static int ui_c128_ds12c887rtc_base[] = {
    0xd700,
    0xde00,
    0xdf00,
    -1
};

static int ui_vic20_ds12c887rtc_base[] = {
    0x9800,
    0x9c00,
    -1
};

static void enable_ds12c887rtc_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_DS12C887RTC_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_DS12C887RTC_RUNMODE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_DS12C887RTC_BASE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_DS12C887RTC_SAVE), is_enabled);
}

static uilib_localize_dialog_param ds12c887rtc_dialog[] = {
    { 0, IDS_DS12C887RTC_CAPTION, -1 },
    { IDC_DS12C887RTC_ENABLE, IDS_DS12C887RTC_ENABLE, 0 },
    { IDC_DS12C887RTC_RUNMODE, IDS_DS12C887RTC_RUNMODE, 0 },
    { IDC_DS12C887RTC_SAVE, IDS_DS12C887RTC_SAVE, 0 },
    { IDC_DS12C887RTC_BASE_LABEL, IDS_DS12C887RTC_BASE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group ds12c887rtc_leftgroup[] = {
    { IDC_DS12C887RTC_BASE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group ds12c887rtc_left_total_group[] = {
    { IDC_DS12C887RTC_ENABLE, 1 },
    { IDC_DS12C887RTC_RUNMODE, 1 },
    { IDC_DS12C887RTC_SAVE, 1 },
    { IDC_DS12C887RTC_BASE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group ds12c887rtc_rightgroup[] = {
    { IDC_DS12C887RTC_BASE, 0 },
    { 0, 0 }
};

static void init_ds12c887rtc_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int res_value_loop;
    int active_value;
    int xsize, ysize;
    int *ui_ds12c887rtc_base;

    if (machine_class == VICE_MACHINE_VIC20) {
        ui_ds12c887rtc_base = ui_vic20_ds12c887rtc_base;
    } else if (machine_class == VICE_MACHINE_C128) {
        ui_ds12c887rtc_base = ui_c128_ds12c887rtc_base;
    } else {
        ui_ds12c887rtc_base = ui_c64_ds12c887rtc_base;
    }

    uilib_localize_dialog(hwnd, ds12c887rtc_dialog);
    uilib_adjust_group_width(hwnd, ds12c887rtc_left_total_group);
    uilib_get_group_extent(hwnd, ds12c887rtc_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, ds12c887rtc_leftgroup);
    uilib_move_group(hwnd, ds12c887rtc_rightgroup, xsize + 30);

    resources_get_int("DS12C887RTC", &res_value);
    CheckDlgButton(hwnd, IDC_DS12C887RTC_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("DS12C887RTCRunMode", &res_value);
    CheckDlgButton(hwnd, IDC_DS12C887RTC_RUNMODE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("DS12C887RTCSave", &res_value);
    CheckDlgButton(hwnd, IDC_DS12C887RTC_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_DS12C887RTC_BASE);
    for (res_value_loop = 0; ui_ds12c887rtc_base[res_value_loop] != -1; res_value_loop++) {
        TCHAR st[40];

        _stprintf(st, "$%X", ui_ds12c887rtc_base[res_value_loop]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("DS12C887RTCbase", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_ds12c887rtc_base[res_value_loop] != -1; res_value_loop++) {
        if (ui_ds12c887rtc_base[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    enable_ds12c887rtc_controls(hwnd);
}

static void end_ds12c887rtc_dialog(HWND hwnd)
{
    int *ui_ds12c887rtc_base;

    if (machine_class == VICE_MACHINE_VIC20) {
        ui_ds12c887rtc_base = ui_vic20_ds12c887rtc_base;
    } else if (machine_class == VICE_MACHINE_C128) {
        ui_ds12c887rtc_base = ui_c128_ds12c887rtc_base;
    } else {
        ui_ds12c887rtc_base = ui_c64_ds12c887rtc_base;
    }

    resources_set_int("DS12C887RTC", (IsDlgButtonChecked(hwnd, IDC_DS12C887RTC_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("DS12C887RTCRunMode", (IsDlgButtonChecked(hwnd, IDC_DS12C887RTC_RUNMODE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("DS12C887RTCSave", (IsDlgButtonChecked(hwnd, IDC_DS12C887RTC_SAVE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("DS12C887RTCbase", ui_ds12c887rtc_base[SendMessage(GetDlgItem(hwnd, IDC_DS12C887RTC_BASE), CB_GETCURSEL, 0, 0)]);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_DS12C887RTC_ENABLE:
                    enable_ds12c887rtc_controls(hwnd);
                    break;
                case IDOK:
                    end_ds12c887rtc_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_ds12c887rtc_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_ds12c887rtc_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)IDD_DS12C887RTC_SETTINGS_DIALOG, hwnd, dialog_proc);
}
