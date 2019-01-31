/*
 * uidqbb.c - Implementation of the Double Quick Brown Box settings dialog box.
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
#include "uidqbb.h"
#include "winmain.h"

static void enable_dqbb_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_DQBB_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_DQBB_WRITE_ENABLE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_DQBB_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_DQBB_FILE), is_enabled);
}

static uilib_localize_dialog_param dqbb_dialog_trans[] = {
    { 0, IDS_DQBB_CAPTION, -1 },
    { IDC_DQBB_ENABLE, IDS_DQBB_ENABLE, 0 },
    { IDC_DQBB_WRITE_ENABLE, IDS_DQBB_WRITE_ENABLE, 0 },
    { IDC_DQBB_FILE_LABEL, IDS_DQBB_FILE_LABEL, 0 },
    { IDC_DQBB_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group dqbb_main_group[] = {
    { IDC_DQBB_ENABLE, 1 },
    { IDC_DQBB_WRITE_ENABLE, 1 },
    { IDC_DQBB_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group dqbb_right_group[] = {
    { IDC_DQBB_ENABLE, 0 },
    { IDC_DQBB_WRITE_ENABLE, 0 },
    { IDC_DQBB_FILE, 0 },
    { IDC_DQBB_BROWSE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_dqbb_dialog(HWND hwnd)
{
    int res_value;
    const char *dqbbfile;
    TCHAR *st_dqbbfile;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, dqbb_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, dqbb_main_group);

    /* get the max x of the file label element */
    uilib_get_element_max_x(hwnd, IDC_DQBB_FILE_LABEL, &xpos);

    /* move the browse button to the correct location */
    uilib_move_element(hwnd, IDC_DQBB_BROWSE, xpos + 10);

    /* get the max of the right group */
    uilib_get_group_max_x(hwnd, dqbb_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("DQBB", &res_value);
    CheckDlgButton(hwnd, IDC_DQBB_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("DQBBImageWrite", &res_value);
    CheckDlgButton(hwnd, IDC_DQBB_WRITE_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_string("DQBBfilename", &dqbbfile);
    st_dqbbfile = system_mbstowcs_alloc(dqbbfile);
    SetDlgItemText(hwnd, IDC_DQBB_FILE, dqbbfile != NULL ? st_dqbbfile : TEXT(""));
    system_mbstowcs_free(st_dqbbfile);

    enable_dqbb_controls(hwnd);
}

static void end_dqbb_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("DQBB", (IsDlgButtonChecked(hwnd, IDC_DQBB_ENABLE) == BST_CHECKED ? 1 : 0 ));

    resources_set_int("DQBBImageWrite", (IsDlgButtonChecked(hwnd, IDC_DQBB_WRITE_ENABLE) == BST_CHECKED ? 1 : 0 ));

    GetDlgItemText(hwnd, IDC_DQBB_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("DQBBfilename", s);
}

static void browse_dqbb_file(HWND hwnd)
{
    uilib_select_browse(hwnd, translate_text(IDS_SELECT_FILE_DQBB), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_DQBB_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
                switch (command) {
                    case IDC_DQBB_BROWSE:
                        browse_dqbb_file(hwnd);
                        break;
                    case IDC_DQBB_ENABLE:
                        enable_dqbb_controls(hwnd);
                        break;
                    case IDOK:
                        end_dqbb_dialog(hwnd);
                    case IDCANCEL:
                        EndDialog(hwnd, 0);
                        return TRUE;
                }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_dqbb_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_dqbb_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_DQBB_SETTINGS_DIALOG, hwnd, dialog_proc);
}
