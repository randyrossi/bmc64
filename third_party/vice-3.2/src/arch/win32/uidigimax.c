/*
 * uidigimax.c - Implementation of the DIGIMAX settings dialog box.
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
#include "uidigimax.h"
#include "uilib.h"
#include "winmain.h"
#include "intl.h"

static int ui_c64_digimax_base[] = {
    0xde00,
    0xde20,
    0xde40,
    0xde60,
    0xde80,
    0xdea0,
    0xdec0,
    0xdee0,
    0xdf00,
    0xdf20,
    0xdf40,
    0xdf60,
    0xdf80,
    0xdfa0,
    0xdfc0,
    0xdfe0,
    -1
};

static int ui_vic20_digimax_base[] = {
    0x9800,
    0x9820,
    0x9840,
    0x9860,
    0x9880,
    0x98a0,
    0x98c0,
    0x98e0,
    0x9c00,
    0x9c20,
    0x9c40,
    0x9c60,
    0x9c80,
    0x9ca0,
    0x9cc0,
    0x9ce0,
    -1
};

static void enable_digimax_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_DIGIMAX_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_DIGIMAX_BASE), is_enabled);
}

static uilib_localize_dialog_param digimax_dialog[] = {
    { 0, IDS_DIGIMAX_CAPTION, -1 },
    { IDC_DIGIMAX_ENABLE, IDS_DIGIMAX_ENABLE, 0 },
    { IDC_DIGIMAX_BASE_LABEL, IDS_DIGIMAX_BASE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group digimax_leftgroup[] = {
    { IDC_DIGIMAX_BASE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group digimax_left_total_group[] = {
    { IDC_DIGIMAX_ENABLE, 1 },
    { IDC_DIGIMAX_BASE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group digimax_rightgroup[] = {
    { IDC_DIGIMAX_BASE, 0 },
    { 0, 0 }
};

static void init_digimax_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int res_value_loop;
    int active_value;
    int xsize, ysize;
    int *ui_digimax_base = (machine_class == VICE_MACHINE_VIC20) ? ui_vic20_digimax_base : ui_c64_digimax_base;

    uilib_localize_dialog(hwnd, digimax_dialog);
    uilib_adjust_group_width(hwnd, digimax_left_total_group);
    uilib_get_group_extent(hwnd, digimax_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, digimax_leftgroup);
    uilib_move_group(hwnd, digimax_rightgroup, xsize + 30);

    resources_get_int("DIGIMAX", &res_value);
    CheckDlgButton(hwnd, IDC_DIGIMAX_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    temp_hwnd = GetDlgItem(hwnd, IDC_DIGIMAX_BASE);
    for (res_value_loop = 0; ui_digimax_base[res_value_loop] != -1; res_value_loop++) {
        TCHAR st[40];

        _stprintf(st, "$%X", ui_digimax_base[res_value_loop]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("DIGIMAXbase", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_digimax_base[res_value_loop] != -1; res_value_loop++) {
        if (ui_digimax_base[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    enable_digimax_controls(hwnd);
}

static void end_digimax_dialog(HWND hwnd)
{
    int *ui_digimax_base = (machine_class == VICE_MACHINE_VIC20) ? ui_vic20_digimax_base : ui_c64_digimax_base;

    resources_set_int("DIGIMAX", (IsDlgButtonChecked(hwnd, IDC_DIGIMAX_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("DIGIMAXbase", ui_digimax_base[SendMessage(GetDlgItem(hwnd, IDC_DIGIMAX_BASE), CB_GETCURSEL, 0, 0)]);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_DIGIMAX_ENABLE:
                    enable_digimax_controls(hwnd);
                    break;
                case IDOK:
                    end_digimax_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_digimax_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_digimax_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)IDD_DIGIMAX_SETTINGS_DIALOG, hwnd, dialog_proc);
}
