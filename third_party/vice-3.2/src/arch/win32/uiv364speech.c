/*
 * uiv364speech.c - Implementation of the V364 Speech settings dialog box.
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
#include "uiv364speech.h"
#include "winmain.h"

static void enable_v364speech_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_V364SPEECH_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_V364SPEECH_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_V364SPEECH_FILE), is_enabled);
}

static uilib_localize_dialog_param v364speech_dialog[] = {
    { 0, IDS_V364SPEECH_CAPTION, -1 },
    { IDC_V364SPEECH_ENABLE, IDS_V364SPEECH_ENABLE, 0 },
    { IDC_V364SPEECH_FILE_LABEL, IDS_V364SPEECH_FILE, 0 },
    { IDC_V364SPEECH_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group v364speech_group1[] = {
    { IDC_V364SPEECH_ENABLE, 1 },
    { 0, 0 }
};

static uilib_dialog_group v364speech_leftgroup[] = {
    { IDC_V364SPEECH_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group v364speech_rightgroup[] = {
    { IDC_V364SPEECH_BROWSE, 0 },
    { 0, 0 }
};

static void init_v364speech_dialog(HWND hwnd)
{
    int res_value;
    const char *v364speechfile;
    TCHAR *st_v364speechfile;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, v364speech_dialog);
    uilib_adjust_group_width(hwnd, v364speech_group1);
    uilib_get_group_extent(hwnd, v364speech_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, v364speech_leftgroup);
    uilib_move_group(hwnd, v364speech_rightgroup, xsize + 30);

    resources_get_int("SpeechEnabled", &res_value);
    CheckDlgButton(hwnd, IDC_V364SPEECH_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_string("SpeechImage", &v364speechfile);
    st_v364speechfile = system_mbstowcs_alloc(v364speechfile);
    SetDlgItemText(hwnd, IDC_V364SPEECH_FILE, v364speechfile != NULL ? st_v364speechfile : TEXT(""));
    system_mbstowcs_free(st_v364speechfile);

    enable_v364speech_controls(hwnd);
}

static void end_v364speech_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("SpeechEnabled", (IsDlgButtonChecked(hwnd, IDC_V364SPEECH_ENABLE) == BST_CHECKED ? 1 : 0 ));

    GetDlgItemText(hwnd, IDC_V364SPEECH_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("SpeechImage", s);
}

static void browse_v364speech_file(HWND hwnd)
{
    uilib_select_browse(hwnd, translate_text(IDS_V364SPEECH_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_V364SPEECH_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_V364SPEECH_BROWSE:
                    browse_v364speech_file(hwnd);
                    break;
                case IDC_V364SPEECH_ENABLE:
                    enable_v364speech_controls(hwnd);
                    break;
                case IDOK:
                    end_v364speech_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_v364speech_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_v364speech_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_V364SPEECH_SETTINGS_DIALOG, hwnd, dialog_proc);
}
