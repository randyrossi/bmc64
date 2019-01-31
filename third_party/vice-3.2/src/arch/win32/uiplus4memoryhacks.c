/*
 * uiplus4memoryhacks.c - Implementation of the plus4 memory expansion hacks (CSORY/HANNES 256K/1024K/4096K) settings dialog box.
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
#include "uiplus4memoryhacks.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param plus4_memory_hacks_dialog[] = {
    { 0, IDS_PLUS4_MEMORY_HACKS_CAPTION, -1 },
    { IDC_PLUS4_MEMORY_HACKS_DEVICE_LABEL, IDS_PLUS4_MEMORY_HACKS_DEVICE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group plus4_memory_hacks_leftgroup[] = {
    { IDC_PLUS4_MEMORY_HACKS_DEVICE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group plus4_memory_hacks_rightgroup[] = {
    { IDC_PLUS4_MEMORY_HACKS_DEVICE, 0 },
    { 0, 0 }
};

static void init_plus4_memory_hacks_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, plus4_memory_hacks_dialog);
    uilib_adjust_group_width(hwnd, plus4_memory_hacks_leftgroup);
    uilib_get_group_extent(hwnd, plus4_memory_hacks_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, plus4_memory_hacks_leftgroup);
    uilib_move_group(hwnd, plus4_memory_hacks_rightgroup, xsize + 30);

    temp_hwnd = GetDlgItem(hwnd, IDC_PLUS4_MEMORY_HACKS_DEVICE);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_NONE));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("CSORY 256K"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("HANNES 256K"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("HANNES 1024K"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("HANNES 4096K"));

    resources_get_int("MemoryHack", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
}

static void end_plus4_memory_hacks_dialog(HWND hwnd)
{
    resources_set_int("MemoryHack", SendMessage(GetDlgItem(hwnd, IDC_PLUS4_MEMORY_HACKS_DEVICE), CB_GETCURSEL, 0, 0));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_plus4_memory_hacks_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_plus4_memory_hacks_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_plus4_memory_hacks_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_PLUS4_MEMORY_HACKS_SETTINGS_DIALOG, hwnd, dialog_proc);
}
