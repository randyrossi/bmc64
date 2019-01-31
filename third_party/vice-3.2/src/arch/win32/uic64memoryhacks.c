/*
 * uic64memoryhacks.c - Implementation of the C64 memory expansion hacks (C64 256K/+60K/+256K) settings dialog box.
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
#include "lib.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uic64memoryhacks.h"
#include "uilib.h"
#include "winmain.h"

#define NUM_OF_C64_256K_BASE 4

static const int ui_c64_256k_base[NUM_OF_C64_256K_BASE] = {
    0xde00,
    0xde80,
    0xdf00,
    0xdf80
};

#define NUM_OF_PLUS60K_BASE 2

static const int ui_plus60k_base[NUM_OF_PLUS60K_BASE] = {
    0xd040,
    0xd100
};

static uilib_localize_dialog_param c64_memory_hacks_dialog[] = {
    { 0, IDS_C64_MEMORY_HACKS_CAPTION, -1 },
    { IDC_C64_MEMORY_HACKS_DEVICE_LABEL, IDS_C64_MEMORY_HACKS_DEVICE, 0 },
    { IDC_C64_256K_BASE_LABEL, IDS_C64_256K_BASE, 0 },
    { IDC_C64_256K_FILE_LABEL, IDS_C64_256K_FILE, 0 },
    { IDC_C64_256K_BROWSE, IDS_BROWSE, 0 },
    { IDC_PLUS60K_BASE_LABEL, IDS_PLUS60K_BASE, 0 },
    { IDC_PLUS60K_FILE_LABEL, IDS_PLUS60K_FILE, 0 },
    { IDC_PLUS60K_BROWSE, IDS_BROWSE, 0 },
    { IDC_PLUS256K_FILE_LABEL, IDS_PLUS256K_FILE, 0 },
    { IDC_PLUS256K_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group c64_memory_hacks_leftgroup[] = {
    { IDC_C64_MEMORY_HACKS_DEVICE_LABEL, 0 },
    { IDC_C64_256K_BASE_LABEL, 0 },
    { IDC_C64_256K_FILE_LABEL, 0 },
    { IDC_PLUS60K_BASE_LABEL, 0 },
    { IDC_PLUS60K_FILE_LABEL, 0 },
    { IDC_PLUS256K_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group c64_memory_hacks_rightgroup[] = {
    { IDC_C64_MEMORY_HACKS_DEVICE, 0 },
    { IDC_C64_256K_BASE, 0 },
    { IDC_C64_256K_BROWSE, 0 },
    { IDC_PLUS60K_BASE, 0 },
    { IDC_PLUS60K_BROWSE, 0 },
    { IDC_PLUS256K_BROWSE, 0 },
    { 0, 0 }
};

static void init_c64_memory_hacks_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *file;
    TCHAR *st_file;
    int res_value_loop;
    int active_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, c64_memory_hacks_dialog);
    uilib_adjust_group_width(hwnd, c64_memory_hacks_leftgroup);
    uilib_get_group_extent(hwnd, c64_memory_hacks_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, c64_memory_hacks_leftgroup);
    uilib_move_group(hwnd, c64_memory_hacks_rightgroup, xsize + 30);

    temp_hwnd = GetDlgItem(hwnd, IDC_C64_MEMORY_HACKS_DEVICE);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_NONE));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("C64 256K"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("+60K"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("+256K"));

    resources_get_int("MemoryHack", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
    
    temp_hwnd = GetDlgItem(hwnd, IDC_C64_256K_BASE);
    for (res_value_loop = 0; res_value_loop < NUM_OF_C64_256K_BASE; res_value_loop++) {
        TCHAR st[10];

        lib_sntprintf(st, 10, TEXT("$%X"), ui_c64_256k_base[res_value_loop]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("C64_256Kbase", &res_value);
    active_value = 0;
    for (res_value_loop = 0; res_value_loop < NUM_OF_C64_256K_BASE; res_value_loop++) {
        if (ui_c64_256k_base[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_string("C64_256Kfilename", &file);
    st_file = system_mbstowcs_alloc(file);
    SetDlgItemText(hwnd, IDC_C64_256K_FILE, file != NULL ? st_file : TEXT(""));
    system_mbstowcs_free(st_file);

    temp_hwnd = GetDlgItem(hwnd, IDC_PLUS60K_BASE);
    for (res_value_loop = 0; res_value_loop < NUM_OF_PLUS60K_BASE; res_value_loop++) {
        TCHAR st[10];

        lib_sntprintf(st, 10, TEXT("$%X"), ui_plus60k_base[res_value_loop]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("PLUS60Kbase", &res_value);
    active_value = 0;
    for (res_value_loop = 0; res_value_loop < NUM_OF_PLUS60K_BASE; res_value_loop++) {
        if (ui_plus60k_base[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_string("PLUS60Kfilename", &file);
    st_file = system_mbstowcs_alloc(file);
    SetDlgItemText(hwnd, IDC_PLUS60K_FILE, file != NULL ? st_file : TEXT(""));
    system_mbstowcs_free(st_file);

    resources_get_string("PLUS256Kfilename", &file);
    st_file = system_mbstowcs_alloc(file);
    SetDlgItemText(hwnd, IDC_PLUS256K_FILE, file != NULL ? st_file : TEXT(""));
    system_mbstowcs_free(st_file);
}

static void end_c64_memory_hacks_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("MemoryHack", SendMessage(GetDlgItem(hwnd, IDC_C64_MEMORY_HACKS_DEVICE), CB_GETCURSEL, 0, 0));

    resources_set_int("C64_256Kbase", ui_c64_256k_base[SendMessage(GetDlgItem(hwnd, IDC_C64_256K_BASE), CB_GETCURSEL, 0, 0)]);

    GetDlgItemText(hwnd, IDC_C64_256K_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("C64_256Kfilename", s);

    resources_set_int("PLUS60Kbase", ui_plus60k_base[SendMessage(GetDlgItem(hwnd, IDC_PLUS60K_BASE), CB_GETCURSEL, 0, 0)]);

    GetDlgItemText(hwnd, IDC_PLUS60K_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("PLUS60Kfilename", s);

    GetDlgItemText(hwnd, IDC_PLUS256K_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("PLUS256Kfilename", s);
}

static void browse_c64_256k_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_C64_256K_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_C64_256K_FILE);
}

static void browse_plus60k_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_PLUS60K_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_PLUS60K_FILE);
}

static void browse_plus256k_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_PLUS256K_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_PLUS256K_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_C64_256K_BROWSE:
                    browse_c64_256k_file(hwnd);
                    break;
                case IDC_PLUS60K_BROWSE:
                    browse_plus60k_file(hwnd);
                    break;
                case IDC_PLUS256K_BROWSE:
                    browse_plus256k_file(hwnd);
                    break;
                case IDOK:
                    end_c64_memory_hacks_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_c64_memory_hacks_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_c64_memory_hacks_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_C64_MEMORY_HACKS_SETTINGS_DIALOG, hwnd, dialog_proc);
}
