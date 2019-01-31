/*
 * uitapecart.c - Implementation of the tapecart settings dialog box.
 *
 * Written by
 *  Michael C. Martin <mcmartin@gmail.com>
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
#include "tapecart.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uitapecart.h"
#include "winmain.h"

#if 0
static int (*tapecart_flush_func)(void) = NULL;
#endif

static void enable_tapecart_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_TAPECART_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_TAPECART_SAVE_ON_CHANGE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_TAPECART_OPTIMIZE_ON_CHANGE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_TAPECART_LOG_LEVEL), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_TAPECART_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_TAPECART_FILE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_TAPECART_FLUSH), is_enabled);
}

static uilib_localize_dialog_param tapecart_dialog[] = {
    { 0, IDS_TAPECART_CAPTION, -1 },
    { IDC_TAPECART_ENABLE, IDS_TAPECART_ENABLE, 0 },
    { IDC_TAPECART_SAVE_ON_CHANGE, IDS_TAPECART_SAVE_ON_CHANGE, 0 },
    { IDC_TAPECART_OPTIMIZE_ON_CHANGE, IDS_TAPECART_OPTIMIZE_ON_CHANGE, 0 },
    { IDC_TAPECART_LOG_LEVEL_LABEL, IDS_TAPECART_LOG_LEVEL, 0 },
    { IDC_TAPECART_FILE_LABEL, IDS_TAPECART_FILE, 0 },
    { IDC_TAPECART_FLUSH, IDS_TAPECART_FLUSH, 0 },
    { IDC_TAPECART_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group tapecart_leftgroup[] = {
    { IDC_TAPECART_LOG_LEVEL_LABEL, 0 },
    { IDC_TAPECART_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group tapecart_left_total_group[] = {
    { IDC_TAPECART_ENABLE, 1 },
    { IDC_TAPECART_LOG_LEVEL_LABEL, 0 },
    { IDC_TAPECART_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group tapecart_rightgroup[] = {
    { IDC_TAPECART_LOG_LEVEL, 0 },
    { IDC_TAPECART_BROWSE, 0 },
    { 0, 0 }
};

static void init_tapecart_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *tapecartfile;
    TCHAR *st_tapecartfile;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, tapecart_dialog);
    uilib_adjust_group_width(hwnd, tapecart_left_total_group);
    uilib_get_group_extent(hwnd, tapecart_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, tapecart_leftgroup);
    uilib_move_group(hwnd, tapecart_rightgroup, xsize + 30);

    resources_get_int("TapecartEnabled", &res_value);
    CheckDlgButton(hwnd, IDC_TAPECART_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    resources_get_int("TapecartUpdateTCRT", &res_value);
    CheckDlgButton(hwnd, IDC_TAPECART_SAVE_ON_CHANGE, res_value ? BST_CHECKED : BST_UNCHECKED);
    resources_get_int("TapecartOptimizeTCRT", &res_value);
    CheckDlgButton(hwnd, IDC_TAPECART_OPTIMIZE_ON_CHANGE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_TAPECART_LOG_LEVEL);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_TAPECART_LOG_LEVEL_0));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_TAPECART_LOG_LEVEL_1));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_TAPECART_LOG_LEVEL_2));
    resources_get_int("TapecartLogLevel", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_string("TapecartTCRTFilename", &tapecartfile);
    st_tapecartfile = system_mbstowcs_alloc(tapecartfile);
    SetDlgItemText(hwnd, IDC_TAPECART_FILE, tapecartfile != NULL ? st_tapecartfile : TEXT(""));
    system_mbstowcs_free(st_tapecartfile);
    enable_tapecart_controls(hwnd);
}

static void end_tapecart_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("TapecartEnabled",
                      (IsDlgButtonChecked(hwnd, IDC_TAPECART_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("TapecartUpdateTCRT",
                      (IsDlgButtonChecked(hwnd, IDC_TAPECART_SAVE_ON_CHANGE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("TapecartOptimizeTCRT",
                      (IsDlgButtonChecked(hwnd, IDC_TAPECART_OPTIMIZE_ON_CHANGE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("TapecartLogLevel",
                      (int)SendMessage(GetDlgItem(hwnd, IDC_TAPECART_LOG_LEVEL), CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_TAPECART_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("TapecartTCRTFilename", s);
}

static void browse_tapecart_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_TAPECART_SELECT_FILE),
                        UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_TAPECART_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_TAPECART_BROWSE:
                    browse_tapecart_file(hwnd);
                    break;
                case IDC_TAPECART_ENABLE:
                    enable_tapecart_controls(hwnd);
                    break;
                case IDC_TAPECART_FLUSH:
#if 1
                    tapecart_flush_tcrt();
#else
                    if (tapecart_flush_func) {
                        tapecart_flush_func();
                    } else {
                        MessageBox(NULL, _T("Tapecart cannot be flushed on this platform"), _T("Tapecart Error"), MB_ICONERROR | MB_OK);
                    }
#endif
                    break;
                case IDOK:
                    end_tapecart_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_tapecart_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_tapecart_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_TAPECART_SETTINGS_DIALOG, hwnd, dialog_proc);
}

#if 0
/** \brief  Set the tapecart flush function
 *
 * This is probably not required to work around vsid not linking against
 * tapecart, because this file should only get linked into x64, x64sc, and x128.
 *
 * \param[in]   func    tapecart flush function
 */
void tapeport_devices_widget_set_tapecart_flush_func(int (*func)(void))
{
    tapecart_flush_func = func;
}
#endif
