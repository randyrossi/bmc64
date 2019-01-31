/*
 * uiexpert.c - Implementation of the Expert cartridge settings dialog box.
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
#include "uiexpert.h"
#include "uilib.h"
#include "winmain.h"

static const int ui_expert_mode[] = {
    IDS_OFF,
    IDS_PRG,
    IDS_ON,
    0
};

static void enable_expert_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_EXPERT_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_EXPERT_WRITE_ENABLE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_EXPERT_MODE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_EXPERT_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_EXPERT_FILE), is_enabled);
}

static uilib_localize_dialog_param expert_dialog_trans[] = {
    { 0, IDS_EXPERT_CAPTION, -1 },
    { IDC_EXPERT_ENABLE, IDS_EXPERT_ENABLE, 0 },
    { IDC_EXPERT_MODE_LABEL, IDS_EXPERT_MODE_LABEL, 0 },
    { IDC_EXPERT_WRITE_ENABLE, IDS_EXPERT_WRITE_ENABLE, 0 },
    { IDC_EXPERT_FILE_LABEL, IDS_EXPERT_FILE_LABEL, 0 },
    { IDC_EXPERT_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group expert_main_group[] = {
    { IDC_EXPERT_ENABLE, 1 },
    { IDC_EXPERT_WRITE_ENABLE, 1 },
    { IDC_EXPERT_MODE_LABEL, 0 },
    { IDC_EXPERT_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group expert_right_group[] = {
    { IDC_EXPERT_ENABLE, 0 },
    { IDC_EXPERT_WRITE_ENABLE, 0 },
    { IDC_EXPERT_MODE, 0 },
    { IDC_EXPERT_BROWSE, 0 },
    { IDC_EXPERT_FILE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_expert_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *expertfile;
    TCHAR *st_expertfile;
    int res_value_loop;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, expert_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, expert_main_group);

    /* get the max x of the expert mode label element */
    uilib_get_element_max_x(hwnd, IDC_EXPERT_MODE_LABEL, &xpos);

    /* move the expert mode indicator element to the correct position */
    uilib_move_element(hwnd, IDC_EXPERT_MODE, xpos + 10);

    /* get the max x of the file name label element */
    uilib_get_element_max_x(hwnd, IDC_EXPERT_FILE_LABEL, &xpos);

    /* move the browse button to the correct position */
    uilib_move_element(hwnd, IDC_EXPERT_BROWSE, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, expert_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("ExpertCartridgeEnabled", &res_value);
    CheckDlgButton(hwnd, IDC_EXPERT_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("ExpertImageWrite", &res_value);
    CheckDlgButton(hwnd, IDC_EXPERT_WRITE_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_EXPERT_MODE);
    for (res_value_loop = 0; ui_expert_mode[res_value_loop] != 0; res_value_loop++) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(ui_expert_mode[res_value_loop]));
    }
    resources_get_int("ExpertCartridgeMode", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_string("Expertfilename", &expertfile);
    st_expertfile = system_mbstowcs_alloc(expertfile);
    SetDlgItemText(hwnd, IDC_EXPERT_FILE, expertfile != NULL ? st_expertfile : TEXT(""));
    system_mbstowcs_free(st_expertfile);

    enable_expert_controls(hwnd);
}

static void end_expert_dialog(HWND hwnd)
{
    TCHAR st_name[MAX_PATH];
    char name[MAX_PATH];

    resources_set_int("ExpertCartridgeEnabled", (IsDlgButtonChecked(hwnd, IDC_EXPERT_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("ExpertImageWrite", (IsDlgButtonChecked(hwnd, IDC_EXPERT_WRITE_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("ExpertCartridgeMode", (int)SendMessage(GetDlgItem(hwnd, IDC_EXPERT_MODE), CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_EXPERT_FILE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("Expertfilename", name);
}

static void browse_expert_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_SELECT_FILE_EXPERT), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_EXPERT_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_EXPERT_BROWSE:
                    browse_expert_file(hwnd);
                    break;
                case IDC_EXPERT_ENABLE:
                    enable_expert_controls(hwnd);
                    break;
                case IDOK:
                    end_expert_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_expert_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_expert_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_EXPERT_SETTINGS_DIALOG, hwnd, dialog_proc);
}
