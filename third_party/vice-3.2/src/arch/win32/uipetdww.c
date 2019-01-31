/*
 * uipetdww.c - Implementation of the PET DWW graphics board settings dialog box.
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
#include "uipetdww.h"
#include "winmain.h"

static void enable_petdww_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_PETDWW_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_PETDWW_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PETDWW_FILE), is_enabled);
}

static uilib_localize_dialog_param petdww_dialog[] = {
    { 0, IDS_PETDWW_CAPTION, -1 },
    { IDC_PETDWW_ENABLE, IDS_PETDWW_ENABLE, 0 },
    { IDC_PETDWW_FILE_LABEL, IDS_PETDWW_FILE, 0 },
    { IDC_PETDWW_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group petdww_group1[] = {
    { IDC_PETDWW_ENABLE, 1 },
    { 0, 0 }
};

static uilib_dialog_group petdww_leftgroup[] = {
    { IDC_PETDWW_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group petdww_rightgroup[] = {
    { IDC_PETDWW_BROWSE, 0 },
    { 0, 0 }
};

static void init_petdww_dialog(HWND hwnd)
{
    int res_value;
    const char *petdwwfile;
    TCHAR *st_petdwwfile;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, petdww_dialog);
    uilib_adjust_group_width(hwnd, petdww_group1);
    uilib_get_group_extent(hwnd, petdww_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, petdww_leftgroup);
    uilib_move_group(hwnd, petdww_rightgroup, xsize + 30);

    resources_get_int("PETDWW", &res_value);
    CheckDlgButton(hwnd, IDC_PETDWW_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_string("PETDWWfilename", &petdwwfile);
    st_petdwwfile = system_mbstowcs_alloc(petdwwfile);
    SetDlgItemText(hwnd, IDC_PETDWW_FILE, petdwwfile != NULL ? st_petdwwfile : TEXT(""));
    system_mbstowcs_free(st_petdwwfile);

    enable_petdww_controls(hwnd);
}

static void end_petdww_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("PETDWW", (IsDlgButtonChecked(hwnd, IDC_PETDWW_ENABLE) == BST_CHECKED ? 1 : 0 ));

    GetDlgItemText(hwnd, IDC_PETDWW_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("PETDWWfilename", s);
}

static void browse_petdww_file(HWND hwnd)
{
    uilib_select_browse(hwnd, translate_text(IDS_PETDWW_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_PETDWW_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_PETDWW_BROWSE:
                    browse_petdww_file(hwnd);
                    break;
                case IDC_PETDWW_ENABLE:
                    enable_petdww_controls(hwnd);
                    break;
                case IDOK:
                    end_petdww_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_petdww_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_petdww_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_PETDWW_SETTINGS_DIALOG, hwnd, dialog_proc);
}
