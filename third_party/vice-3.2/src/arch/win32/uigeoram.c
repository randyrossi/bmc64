/*
 * uigeoram.c - Implementation of the GEORAM settings dialog box.
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
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uigeoram.h"
#include "winmain.h"

static const int ui_georam_size[] = {
    64,
    128,
    256,
    512,
    1024,
    2048,
    4096,
    0
};

static void enable_georam_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_GEORAM_ENABLE) == BST_CHECKED) ? 1 : 0;

    if (machine_class == VICE_MACHINE_VIC20) {
        EnableWindow(GetDlgItem(hwnd, IDC_GEORAM_IO_SWAP), is_enabled);
    }
    EnableWindow(GetDlgItem(hwnd, IDC_GEORAM_WRITE_ENABLE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_GEORAM_SIZE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_GEORAM_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_GEORAM_FILE), is_enabled);
}

static uilib_localize_dialog_param georam_dialog_trans[] = {
    { 0, IDS_GEORAM_CAPTION, -1 },
    { IDC_GEORAM_ENABLE, IDS_GEORAM_ENABLE, 0 },
    { IDC_GEORAM_SIZE_LABEL, IDS_GEORAM_SIZE_LABEL, 0 },
    { IDC_GEORAM_WRITE_ENABLE, IDS_GEORAM_WRITE_ENABLE, 0 },
    { IDC_GEORAM_FILE_LABEL, IDS_GEORAM_FILE_LABEL, 0 },
    { IDC_GEORAM_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param georam_io_swap_dialog_trans[] = {
    { IDC_GEORAM_IO_SWAP, IDS_IO_SWAP, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group georam_main_group[] = {
    { IDC_GEORAM_ENABLE, 1 },
    { IDC_GEORAM_WRITE_ENABLE, 1 },
    { IDC_GEORAM_SIZE_LABEL, 0 },
    { IDC_GEORAM_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group georam_mascuerade_main_group[] = {
    { IDC_GEORAM_ENABLE, 1 },
    { IDC_GEORAM_IO_SWAP, 1 },
    { IDC_GEORAM_WRITE_ENABLE, 1 },
    { IDC_GEORAM_SIZE_LABEL, 0 },
    { IDC_GEORAM_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group georam_right_group[] = {
    { IDC_GEORAM_ENABLE, 0 },
    { IDC_GEORAM_WRITE_ENABLE, 0 },
    { IDC_GEORAM_SIZE, 0 },
    { IDC_GEORAM_BROWSE, 0 },
    { IDC_GEORAM_FILE, 0 },
    { 0, 0 }
};

static uilib_dialog_group georam_mascuerade_right_group[] = {
    { IDC_GEORAM_ENABLE, 0 },
    { IDC_GEORAM_IO_SWAP, 0 },
    { IDC_GEORAM_WRITE_ENABLE, 0 },
    { IDC_GEORAM_SIZE, 0 },
    { IDC_GEORAM_BROWSE, 0 },
    { IDC_GEORAM_FILE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_georam_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *georamfile;
    TCHAR *st_georamfile;
    int res_value_loop;
    int active_value;
    int xpos;
    RECT rect;
    uilib_dialog_group *main_group = (machine_class == VICE_MACHINE_VIC20) ? georam_mascuerade_main_group : georam_main_group;
    uilib_dialog_group *right_group = (machine_class == VICE_MACHINE_VIC20) ? georam_mascuerade_right_group : georam_right_group;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, georam_dialog_trans);

    if (machine_class == VICE_MACHINE_VIC20) {
        /* translate masC=uerade specific item */
        uilib_localize_dialog(hwnd, georam_io_swap_dialog_trans);
    }

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, main_group);

    /* get the max x of the georam size label element */
    uilib_get_element_max_x(hwnd, IDC_GEORAM_SIZE_LABEL, &xpos);

    /* move the georam size indicator element to the correct position */
    uilib_move_element(hwnd, IDC_GEORAM_SIZE, xpos + 10);

    /* get the max x of the file name label element */
    uilib_get_element_max_x(hwnd, IDC_GEORAM_FILE_LABEL, &xpos);

    /* move the browse button to the correct position */
    uilib_move_element(hwnd, IDC_GEORAM_BROWSE, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("GEORAM", &res_value);
    CheckDlgButton(hwnd, IDC_GEORAM_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    if (machine_class == VICE_MACHINE_VIC20) {
        resources_get_int("GEORAMIOSwap", &res_value);
        CheckDlgButton(hwnd, IDC_GEORAM_IO_SWAP, res_value ? BST_CHECKED : BST_UNCHECKED);
    }

    resources_get_int("GEORAMImageWrite", &res_value);
    CheckDlgButton(hwnd, IDC_GEORAM_WRITE_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_GEORAM_SIZE);
    for (res_value_loop = 0; ui_georam_size[res_value_loop] != 0; res_value_loop++) {
        TCHAR st[10];

        _itot(ui_georam_size[res_value_loop], st, 10);
        _tcscat(st, intl_translate_tcs(IDS_SPACE_KB));
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("GEORAMsize", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_georam_size[res_value_loop] != 0; res_value_loop++) {
        if (ui_georam_size[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_string("GEORAMfilename", &georamfile);
    st_georamfile = system_mbstowcs_alloc(georamfile);
    SetDlgItemText(hwnd, IDC_GEORAM_FILE, georamfile != NULL ? st_georamfile : TEXT(""));
    system_mbstowcs_free(st_georamfile);

    enable_georam_controls(hwnd);
}

static void end_georam_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("GEORAM", (IsDlgButtonChecked(hwnd, IDC_GEORAM_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("GEORAMImageWrite", (IsDlgButtonChecked(hwnd, IDC_GEORAM_WRITE_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("GEORAMsize", ui_georam_size[SendMessage(GetDlgItem(hwnd, IDC_GEORAM_SIZE), CB_GETCURSEL, 0, 0)]);

    if (machine_class == VICE_MACHINE_VIC20) {
        resources_set_int("GEORAMIOSwap", (IsDlgButtonChecked(hwnd, IDC_GEORAM_IO_SWAP) == BST_CHECKED ? 1 : 0 ));
    }

    GetDlgItemText(hwnd, IDC_GEORAM_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("GEORAMfilename", s);
}

static void browse_georam_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_SELECT_FILE_GEORAM), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_GEORAM_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_GEORAM_BROWSE:
                    browse_georam_file(hwnd);
                    break;
                case IDC_GEORAM_ENABLE:
                    enable_georam_controls(hwnd);
                    break;
                case IDOK:
                    end_georam_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_georam_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_georam_settings_dialog(HWND hwnd)
{
    if (machine_class == VICE_MACHINE_VIC20) {
        DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_GEORAM_MASCUERADE_SETTINGS_DIALOG, hwnd, dialog_proc);
    } else {
        DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_GEORAM_SETTINGS_DIALOG, hwnd, dialog_proc);
    }
}
