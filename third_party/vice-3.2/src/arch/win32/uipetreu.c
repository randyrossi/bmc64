/*
 * uipetreu.c - Implementation of the PET RAM and Expansion Unit settings
 *              dialog box.
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
#include "uipetreu.h"
#include "winmain.h"

#define NUM_OF_PETREU_SIZE 4

static const int ui_petreu_size[NUM_OF_PETREU_SIZE] = {
    128,
    512,
    1024,
    2048
};

static void enable_petreu_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_PETREU_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_PETREU_SIZE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PETREU_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PETREU_FILE), is_enabled);
}

static uilib_localize_dialog_param petreu_dialog[] = {
    { 0, IDS_PETREU_CAPTION, -1 },
    { IDC_PETREU_ENABLE, IDS_PETREU_ENABLE, 0 },
    { IDC_PETREU_SIZE_LABEL, IDS_PETREU_SIZE, 0 },
    { IDC_PETREU_FILE_LABEL, IDS_PETREU_FILE, 0 },
    { IDC_PETREU_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group petreu_leftgroup[] = {
    { IDC_PETREU_SIZE_LABEL, 0 },
    { IDC_PETREU_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group petreu_left_total_group[] = {
    { IDC_PETREU_ENABLE, 1 },
    { IDC_PETREU_SIZE_LABEL, 0 },
    { IDC_PETREU_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group petreu_rightgroup[] = {
    { IDC_PETREU_SIZE, 0 },
    { IDC_PETREU_BROWSE, 0 },
    { 0, 0 }
};

static void init_petreu_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *petreufile;
    TCHAR *st_petreufile;
    int res_value_loop;
    int active_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, petreu_dialog);
    uilib_adjust_group_width(hwnd, petreu_left_total_group);
    uilib_get_group_extent(hwnd, petreu_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, petreu_leftgroup);
    uilib_move_group(hwnd, petreu_rightgroup, xsize + 30);

    resources_get_int("PETREU", &res_value);
    CheckDlgButton(hwnd, IDC_PETREU_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_PETREU_SIZE);
    for (res_value_loop = 0; res_value_loop < NUM_OF_PETREU_SIZE; res_value_loop++) {
        TCHAR st[10];

        _itot(ui_petreu_size[res_value_loop], st, 10);
        _tcscat(st, translate_text(IDS_SPACE_KB));
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("PETREUsize", &res_value);
    active_value = 0;
    for (res_value_loop = 0; res_value_loop < NUM_OF_PETREU_SIZE; res_value_loop++) {
        if (ui_petreu_size[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_string("PETREUfilename", &petreufile);
    st_petreufile = system_mbstowcs_alloc(petreufile);
    SetDlgItemText(hwnd, IDC_PETREU_FILE, petreufile != NULL ? st_petreufile : TEXT(""));
    system_mbstowcs_free(st_petreufile);

    enable_petreu_controls(hwnd);
}

static void end_petreu_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("PETREU", (IsDlgButtonChecked(hwnd, IDC_PETREU_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("PETREUsize", ui_petreu_size[SendMessage(GetDlgItem(hwnd, IDC_PETREU_SIZE), CB_GETCURSEL, 0, 0)]);

    GetDlgItemText(hwnd, IDC_PETREU_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("PETREUfilename", s);
}

static void browse_petreu_file(HWND hwnd)
{
    uilib_select_browse(hwnd, translate_text(IDS_PETREU_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_PETREU_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_PETREU_BROWSE:
                    browse_petreu_file(hwnd);
                    break;
                case IDC_PETREU_ENABLE:
                    enable_petreu_controls(hwnd);
                    break;
                case IDOK:
                    end_petreu_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_petreu_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_petreu_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_PETREU_SETTINGS_DIALOG, hwnd, dialog_proc);
}
