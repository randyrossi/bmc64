/*
 * uic64burstmod.c - Implementation of the C64 drive burst modification settings dialog box.
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
#include "uic64burstmod.h"
#include "uilib.h"
#include "winmain.h"

static void enable_c64burstmod_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_C64BURSTMOD_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_C64BURSTMOD_CIA), is_enabled);
}

static uilib_localize_dialog_param c64burstmod_dialog[] = {
    { 0, IDS_C64BURSTMOD_CAPTION, -1 },
    { IDC_C64BURSTMOD_ENABLE, IDS_C64BURSTMOD_ENABLE, 0 },
    { IDC_C64BURSTMOD_CIA_LABEL, IDS_C64BURSTMOD_CIA, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group c64burstmod_leftgroup[] = {
    { IDC_C64BURSTMOD_CIA_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group c64burstmod_left_total_group[] = {
    { IDC_C64BURSTMOD_ENABLE, 1 },
    { IDC_C64BURSTMOD_CIA_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group c64burstmod_rightgroup[] = {
    { IDC_C64BURSTMOD_CIA, 0 },
    { 0, 0 }
};

static void init_c64burstmod_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, c64burstmod_dialog);
    uilib_adjust_group_width(hwnd, c64burstmod_left_total_group);
    uilib_get_group_extent(hwnd, c64burstmod_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, c64burstmod_leftgroup);
    uilib_move_group(hwnd, c64burstmod_rightgroup, xsize + 30);

    resources_get_int("BurstMod", &res_value);
    CheckDlgButton(hwnd, IDC_C64BURSTMOD_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    temp_hwnd = GetDlgItem(hwnd, IDC_C64BURSTMOD_CIA);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"CIA-1");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"CIA-2");

    if (res_value) {
        SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value - 1, 0);
    } else {
        SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)0, 0);
    }

    enable_c64burstmod_controls(hwnd);
}

static void end_c64burstmod_dialog(HWND hwnd)
{
    int is_enabled = (IsDlgButtonChecked(hwnd, IDC_C64BURSTMOD_ENABLE) == BST_CHECKED ? 1 : 0 );

    if (is_enabled) {
        resources_set_int("BurstMod", (int)(SendMessage(GetDlgItem(hwnd, IDC_C64BURSTMOD_CIA), CB_GETCURSEL, 0, 0) + 1));
    } else {
        resources_set_int("BurstMod", 0);
    }
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_C64BURSTMOD_ENABLE:
                    enable_c64burstmod_controls(hwnd);
                    break;
                case IDOK:
                    end_c64burstmod_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_c64burstmod_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_c64burstmod_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)IDD_C64BURSTMOD_SETTINGS_DIALOG, hwnd, dialog_proc);
}
