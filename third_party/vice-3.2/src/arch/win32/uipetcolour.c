/*
 * uipetcolour.c - Implementation of the PET Colour board settings dialog box.
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
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

#include "intl.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uiapi.h"
#include "uilib.h"
#include "uipetcolour.h"
#include "winmain.h"

static uilib_localize_dialog_param petcolour_dialog[] = {
    { 0, IDS_PETCOLOUR_CAPTION, -1 },
    { IDC_PETCOLOUR_TYPE_LABEL, IDS_PETCOLOUR_TYPE, 0 },
    { IDC_PETCOLOUR_BG, IDS_PETCOLOUR_BACKGROUND, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group petcolour_leftgroup[] = {
    { IDC_PETCOLOUR_TYPE_LABEL, 0 },
    { IDC_PETCOLOUR_BG, 0 },
    { 0, 0 }
};

static uilib_dialog_group petcolour_rightgroup[] = {
    { IDC_PETCOLOUR_TYPE, 0 },
    { IDC_PETCOLOUR_BG_VALUE, 0 },
    { 0, 0 }
};

static void init_petcolour_dialog(HWND hwnd)
{
    int res_value;
    int xsize, ysize;
    HWND temp_hwnd;
    TCHAR st[10];

    uilib_localize_dialog(hwnd, petcolour_dialog);
    uilib_adjust_group_width(hwnd, petcolour_leftgroup);
    uilib_get_group_extent(hwnd, petcolour_leftgroup, &xsize, &ysize);
    uilib_move_group(hwnd, petcolour_rightgroup, xsize + 30);

    resources_get_int("PETColour", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_PETCOLOUR_TYPE);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_PETCOLOUR_OFF));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RGBI"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_PETCOLOUR_ANALOG));
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("PETColourBG", &res_value);
    _itot(res_value, st, 10);
    SetDlgItemText(hwnd, IDC_PETCOLOUR_BG_VALUE, st);
}

static void end_petcolour_dialog(HWND hwnd)
{
    TCHAR st[4];
    int temp_val, res_val;

    resources_set_int("PETColour", (int)SendDlgItemMessage(hwnd, IDC_PETCOLOUR_TYPE, CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_PETCOLOUR_BG_VALUE, st, 4);
    temp_val = _ttoi(st);
    if (temp_val < 0) {
        res_val = 0;
    } else if (temp_val > 255) {
        res_val = 255;
    } else {
        res_val = temp_val;
    }
    
    if (temp_val != res_val) {
        ui_error(translate_text(IDS_VAL_D_FOR_S_OUT_RANGE_USE_D), temp_val, translate_text(IDS_PETCOLOUR_BACKGROUND), res_val);
    }
    resources_set_int("PETColourBG", res_val);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_petcolour_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_petcolour_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_petcolour_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_PETCOLOUR_SETTINGS_DIALOG, hwnd, dialog_proc);
}
