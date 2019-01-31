/*
 * uiautostart.c - Implementation of the autostart settings dialog box.
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
#include "uiautostart.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param autostart_dialog[] = {
    { 0, IDS_AUTOSTART_CAPTION, -1 },
    { IDC_AUTOSTART_WARP, IDS_AUTOSTART_WARP, 0 },
    { IDC_AUTOSTART_USE_COLON_WITH_RUN, IDS_AUTOSTART_COLON_RUN, 0 },
    { IDC_AUTOSTART_LOAD_TO_BASIC, IDS_AUTOSTART_LOAD_TO_BASIC, 0 },
    { IDC_AUTOSTART_RANDOM_DELAY, IDS_AUTOSTART_RANDOM_DELAY, 0 },
    { IDC_AUTOSTART_HANDLE_TRUE_DRIVE_EMULATION, IDS_AUTOSTART_HANDLE_TRUE_DRIVE_EMULATION, 0 },
    { IDC_AUTOSTART_DELAY, IDS_AUTOSTART_DELAY, 0 },
    { IDC_AUTOSTART_PRG_MODE_LABEL, IDS_AUTOSTART_PRG_MODE, 0 },
    { IDC_AUTOSTART_DISK_IMAGE_FILE_LABEL, IDS_AUTOSTART_PRG_DISK_IMAGE, 0 },
    { IDC_AUTOSTART_DISK_IMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group autostart_leftgroup1[] = {
    { IDC_AUTOSTART_WARP, 1 },
    { IDC_AUTOSTART_LOAD_TO_BASIC, 1 },
    { 0, 0 }
};

static uilib_dialog_group autostart_rightgroup1[] = {
    { IDC_AUTOSTART_USE_COLON_WITH_RUN, 1 },
    { IDC_AUTOSTART_RANDOM_DELAY, 1 },
    { 0, 0 }
};

static uilib_dialog_group autostart_leftgroup2[] = {
    { IDC_AUTOSTART_PRG_MODE_LABEL, 0 },
    { IDC_AUTOSTART_DISK_IMAGE_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group autostart_rightgroup2[] = {
    { IDC_AUTOSTART_PRG_MODE, 0 },
    { IDC_AUTOSTART_DISK_IMAGE_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group autostart_rightgroup3[] = {
    { IDC_AUTOSTART_USE_COLON_WITH_RUN, 0 },
    { IDC_AUTOSTART_RANDOM_DELAY, 0 },
    { IDC_AUTOSTART_PRG_MODE, 0 },
    { IDC_AUTOSTART_DISK_IMAGE_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group autostart_leftgroup4[] = {
    { IDC_AUTOSTART_DELAY, 0 },
    { 0, 0 }
};

static uilib_dialog_group autostart_rightgroup4[] = {
    { IDC_AUTOSTART_DELAY_VALUE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_autostart_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *autostartfile;
    TCHAR *st_autostartfile;
    int xsize, ysize;
    int xsize2;
    RECT rect;

    uilib_localize_dialog(hwnd, autostart_dialog);
    uilib_get_group_extent(hwnd, autostart_leftgroup1, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, autostart_leftgroup1);
    uilib_move_and_adjust_group_width(hwnd, autostart_rightgroup1, xsize + 30);
    uilib_get_group_extent(hwnd, autostart_leftgroup2, &xsize2, &ysize);
    uilib_adjust_group_width(hwnd, autostart_leftgroup2);
    uilib_move_group(hwnd, autostart_rightgroup2, xsize2 + 30);

    /* adjust width of left group 4 */
    uilib_adjust_group_width(hwnd, autostart_leftgroup4);

    /* get the max x of the left group 4 elements */
    uilib_get_group_max_x(hwnd, autostart_leftgroup4, &xsize);

    /* move right group 4 elements */
    uilib_move_group(hwnd, autostart_rightgroup4, xsize + 10);

    /* get the max x of the rightgroup3 elements */
    uilib_get_group_max_x(hwnd, autostart_rightgroup3, &xsize);

    /* resize the text fill-in box */
    uilib_move_and_set_element_width(hwnd, IDC_AUTOSTART_DISK_IMAGE_FILE, 9, xsize - 9);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xsize + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("AutostartWarp", &res_value);
    CheckDlgButton(hwnd, IDC_AUTOSTART_WARP, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("AutostartRunWithColon", &res_value);
    CheckDlgButton(hwnd, IDC_AUTOSTART_USE_COLON_WITH_RUN, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("AutostartBasicLoad", &res_value);
    CheckDlgButton(hwnd, IDC_AUTOSTART_LOAD_TO_BASIC, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("AutostartDelay", &res_value);
    SetDlgItemInt(hwnd, IDC_AUTOSTART_DELAY_VALUE, res_value, TRUE);

    resources_get_int("AutostartDelayRandom", &res_value);
    CheckDlgButton(hwnd, IDC_AUTOSTART_RANDOM_DELAY, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("AutostartHandleTrueDriveEmulation", &res_value);
    CheckDlgButton(hwnd, IDC_AUTOSTART_HANDLE_TRUE_DRIVE_EMULATION, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_AUTOSTART_PRG_MODE);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_AUTOSTART_VIRTUAL_FS));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_AUTOSTART_INJECT));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_AUTOSTART_DISK));
    resources_get_int("AutostartPrgMode", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_string("AutostartPrgDiskImage", &autostartfile);
    st_autostartfile = system_mbstowcs_alloc(autostartfile);
    SetDlgItemText(hwnd, IDC_AUTOSTART_DISK_IMAGE_FILE, autostartfile != NULL ? st_autostartfile : TEXT(""));
    system_mbstowcs_free(st_autostartfile);
}

static void end_autostart_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("AutostartWarp", (IsDlgButtonChecked(hwnd, IDC_AUTOSTART_WARP) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("AutostartRunWithColon", (IsDlgButtonChecked(hwnd, IDC_AUTOSTART_USE_COLON_WITH_RUN) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("AutostartBasicLoad", (IsDlgButtonChecked(hwnd, IDC_AUTOSTART_LOAD_TO_BASIC) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("AutostartDelay", GetDlgItemInt(hwnd, IDC_AUTOSTART_DELAY_VALUE, NULL, TRUE));
    resources_set_int("AutostartDelayRandom", (IsDlgButtonChecked(hwnd, IDC_AUTOSTART_RANDOM_DELAY) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("AutostartHandleTrueDriveEmulation", (IsDlgButtonChecked(hwnd, IDC_AUTOSTART_HANDLE_TRUE_DRIVE_EMULATION) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("AutostartPrgMode", (int)SendMessage(GetDlgItem(hwnd, IDC_AUTOSTART_PRG_MODE), CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_AUTOSTART_DISK_IMAGE_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("AutostartPrgDiskImage", s);
}

static void browse_autostart_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_AUTOSTART_PRG_DISK_IMAGE_SELECT_FILE),
                        UILIB_FILTER_ALL | UILIB_FILTER_DISK,
                        UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_AUTOSTART_DISK_IMAGE_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_AUTOSTART_DISK_IMAGE_BROWSE:
                    browse_autostart_file(hwnd);
                    break;
                case IDOK:
                    end_autostart_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_autostart_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_autostart_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_AUTOSTART_SETTINGS_DIALOG, hwnd, dialog_proc);
}
