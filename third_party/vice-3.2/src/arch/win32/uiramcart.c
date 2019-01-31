/*
 * uiramcart.c - Implementation of the RAMCART settings dialog box.
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
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uiramcart.h"
#include "winmain.h"

static const int ui_ramcart_size[] = {
    64,
    128,
    0
};

static void enable_ramcart_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_RAMCART_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_RAMCART_WRITE_ENABLE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_RAMCART_RO), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_RAMCART_SIZE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_RAMCART_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_RAMCART_FILE), is_enabled);
}

static uilib_localize_dialog_param ramcart_dialog[] = {
    { 0, IDS_RAMCART_CAPTION, -1 },
    { IDC_RAMCART_ENABLE, IDS_RAMCART_ENABLE, 0 },
    { IDC_RAMCART_RO, IDS_RAMCART_READONLY, 0 },
    { IDC_RAMCART_WRITE_ENABLE, IDS_RAMCART_WRITE_ENABLE, 0 },
    { IDC_RAMCART_SIZE_LABEL, IDS_RAMCART_SIZE, 0 },
    { IDC_RAMCART_FILE_LABEL, IDS_RAMCART_FILE, 0 },
    { IDC_RAMCART_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group ramcart_maingroup[] = {
    { IDC_RAMCART_WRITE_ENABLE, 1 },
    { 0, 0 }
};

static uilib_dialog_group ramcart_leftgroup1[] = {
    { IDC_RAMCART_ENABLE, 1 },
    { 0, 0 }
};

static uilib_dialog_group ramcart_rightgroup1[] = {
    { IDC_RAMCART_RO, 1 },
    { 0, 0 }
};

static uilib_dialog_group ramcart_leftgroup2[] = {
    { IDC_RAMCART_SIZE_LABEL, 0 },
    { IDC_RAMCART_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group ramcart_rightgroup2[] = {
    { IDC_RAMCART_SIZE, 0 },
    { IDC_RAMCART_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group ramcart_total_group[] = {
    { IDC_RAMCART_ENABLE, 0 },
    { IDC_RAMCART_RO, 0 },
    { IDC_RAMCART_WRITE_ENABLE, 0 },
    { IDC_RAMCART_SIZE_LABEL, 0 },
    { IDC_RAMCART_FILE_LABEL, 0 },
    { IDC_RAMCART_BROWSE, 0 },
    { IDOK, 0 },
    { IDCANCEL, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_ramcart_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *ramcartfile;
    TCHAR *st_ramcartfile;
    int res_value_loop;
    int active_value;
    int xsize, ysize;
    int xsize2;
    RECT rect;

    uilib_localize_dialog(hwnd, ramcart_dialog);
    uilib_get_group_extent(hwnd, ramcart_leftgroup1, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, ramcart_maingroup);
    uilib_adjust_group_width(hwnd, ramcart_leftgroup1);
    uilib_move_and_adjust_group_width(hwnd, ramcart_rightgroup1, xsize + 30);
    uilib_get_group_extent(hwnd, ramcart_leftgroup2, &xsize2, &ysize);
    uilib_adjust_group_width(hwnd, ramcart_leftgroup2);
    uilib_move_group(hwnd, ramcart_rightgroup2, xsize2 + 30);
    uilib_get_group_max_x(hwnd, ramcart_total_group, &xsize);
    
    /* set the size of the dialog window */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xsize + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("RAMCART", &res_value);
    CheckDlgButton(hwnd, IDC_RAMCART_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("RAMCARTImageWrite", &res_value);
    CheckDlgButton(hwnd, IDC_RAMCART_WRITE_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("RAMCART_RO", &res_value);
    CheckDlgButton(hwnd, IDC_RAMCART_RO, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    temp_hwnd = GetDlgItem(hwnd, IDC_RAMCART_SIZE);
    for (res_value_loop = 0; ui_ramcart_size[res_value_loop] != 0; res_value_loop++) {
        TCHAR st[10];

        _itot(ui_ramcart_size[res_value_loop], st, 10);
        _tcscat(st, intl_translate_tcs(IDS_SPACE_KB));
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("RAMCARTsize", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_ramcart_size[res_value_loop] != 0; res_value_loop++) {
        if (ui_ramcart_size[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_string("RAMCARTfilename", &ramcartfile);
    st_ramcartfile = system_mbstowcs_alloc(ramcartfile);
    SetDlgItemText(hwnd, IDC_RAMCART_FILE, ramcartfile != NULL ? st_ramcartfile : TEXT(""));
    system_mbstowcs_free(st_ramcartfile);

    enable_ramcart_controls(hwnd);
}

static void end_ramcart_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("RAMCART", (IsDlgButtonChecked(hwnd, IDC_RAMCART_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RAMCARTImageWrite", (IsDlgButtonChecked(hwnd, IDC_RAMCART_WRITE_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RAMCART_RO", (IsDlgButtonChecked(hwnd, IDC_RAMCART_RO) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RAMCARTsize", ui_ramcart_size[SendMessage(GetDlgItem(hwnd, IDC_RAMCART_SIZE), CB_GETCURSEL, 0, 0)]);

    GetDlgItemText(hwnd, IDC_RAMCART_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("RAMCARTfilename", s);
}

static void browse_ramcart_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_RAMCART_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_RAMCART_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_RAMCART_BROWSE:
                    browse_ramcart_file(hwnd);
                    break;
                case IDC_RAMCART_ENABLE:
                    enable_ramcart_controls(hwnd);
                    break;
                case IDOK:
                    end_ramcart_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_ramcart_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_ramcart_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_RAMCART_SETTINGS_DIALOG, hwnd, dialog_proc);
}
