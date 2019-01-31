/*
 * uitapelog.c - Implementation of the tape log device settings dialog box.
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
#include "uitapelog.h"
#include "winmain.h"

static void enable_tapelog_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_TAPELOG_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_TAPELOG_DESTINATION), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_TAPELOG_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_TAPELOG_FILE), is_enabled);
}

static uilib_localize_dialog_param tapelog_dialog[] = {
    { 0, IDS_TAPELOG_CAPTION, -1 },
    { IDC_TAPELOG_ENABLE, IDS_TAPELOG_ENABLE, 0 },
    { IDC_TAPELOG_DESTINATION_LABEL, IDS_TAPELOG_DESTINATION, 0 },
    { IDC_TAPELOG_FILE_LABEL, IDS_TAPELOG_FILE, 0 },
    { IDC_TAPELOG_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group tapelog_leftgroup[] = {
    { IDC_TAPELOG_DESTINATION_LABEL, 0 },
    { IDC_TAPELOG_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group tapelog_left_total_group[] = {
    { IDC_TAPELOG_ENABLE, 1 },
    { IDC_TAPELOG_DESTINATION_LABEL, 0 },
    { IDC_TAPELOG_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group tapelog_rightgroup[] = {
    { IDC_TAPELOG_DESTINATION, 0 },
    { IDC_TAPELOG_BROWSE, 0 },
    { 0, 0 }
};

static void init_tapelog_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *tapelogfile;
    TCHAR *st_tapelogfile;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, tapelog_dialog);
    uilib_adjust_group_width(hwnd, tapelog_left_total_group);
    uilib_get_group_extent(hwnd, tapelog_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, tapelog_leftgroup);
    uilib_move_group(hwnd, tapelog_rightgroup, xsize + 30);

    resources_get_int("TapeLog", &res_value);
    CheckDlgButton(hwnd, IDC_TAPELOG_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_TAPELOG_DESTINATION);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(IDS_TAPELOG_LOG_TO_EMULATION_LOGFILE));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(IDS_TAPELOG_LOG_TO_USER_SPECIFIED_FILE));
    resources_get_int("TapeLogDestination", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_string("TapeLogfilename", &tapelogfile);
    st_tapelogfile = system_mbstowcs_alloc(tapelogfile);
    SetDlgItemText(hwnd, IDC_TAPELOG_FILE, tapelogfile != NULL ? st_tapelogfile : TEXT(""));
    system_mbstowcs_free(st_tapelogfile);

    enable_tapelog_controls(hwnd);
}

static void end_tapelog_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("TapeLog", (IsDlgButtonChecked(hwnd, IDC_TAPELOG_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("TapeLogDestination", (int)SendMessage(GetDlgItem(hwnd, IDC_TAPELOG_DESTINATION), CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_TAPELOG_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("TapeLogfilename", s);
}

static void browse_tapelog_file(HWND hwnd)
{
    uilib_select_browse(hwnd, translate_text(IDS_TAPELOG_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_TAPELOG_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_TAPELOG_BROWSE:
                    browse_tapelog_file(hwnd);
                    break;
                case IDC_TAPELOG_ENABLE:
                    enable_tapelog_controls(hwnd);
                    break;
                case IDOK:
                    end_tapelog_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_tapelog_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_tapelog_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_TAPELOG_SETTINGS_DIALOG, hwnd, dialog_proc);
}
