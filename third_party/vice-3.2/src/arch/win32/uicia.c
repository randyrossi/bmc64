/*
 * uicia.c - Implementation of the CIA settings dialog box.
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
#include "uicia.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param cia1_dialog_trans[] = {
    { IDC_C64CIA1_LABEL, IDS_CIA1_MODEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param cia2_dialog_trans[] = {
    { IDC_C64CIA1_LABEL, IDS_CIA1_MODEL, 0 },
    { IDC_C64CIA2_LABEL, IDS_CIA2_MODEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group cia1_left_group[] = {
    { IDC_C64CIA1_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group cia2_left_group[] = {
    { IDC_C64CIA1_LABEL, 0 },
    { IDC_C64CIA2_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group cia1_right_group[] = {
    { IDC_C64CIA1_LIST, 0 },
    { 0, 0 }
};

static uilib_dialog_group cia2_right_group[] = {
    { IDC_C64CIA1_LIST, 0 },
    { IDC_C64CIA2_LIST, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static int amount = 0;

static void init_cia_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xpos;
    RECT rect;
    uilib_dialog_group *left_group = (amount == 2) ? cia2_left_group : cia1_left_group;
    uilib_dialog_group *right_group = (amount == 2) ? cia2_right_group : cia1_right_group;
    uilib_localize_dialog_param *dialog_trans = (amount == 2) ? cia2_dialog_trans : cia1_dialog_trans ;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("CIA1Model", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_C64CIA1_LIST);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_6526_OLD));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_6526A_NEW));
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    if (amount == 2) {
        resources_get_int("CIA2Model", &res_value);
        temp_hwnd = GetDlgItem(hwnd, IDC_C64CIA2_LIST);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_6526_OLD));
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_6526A_NEW));
        SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
    }
}

static void end_cia_dialog(HWND hwnd)
{
    resources_set_int("CIA1Model", (int)SendMessage(GetDlgItem(hwnd, IDC_C64CIA1_LIST), CB_GETCURSEL, 0, 0));

    if (amount == 2) {
        resources_set_int("CIA2Model", (int)SendMessage(GetDlgItem(hwnd, IDC_C64CIA2_LIST), CB_GETCURSEL, 0, 0));
    }
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_cia_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_cia_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_cia_settings_dialog(HWND hwnd, int cia_amount)
{
    amount = cia_amount;

    if (amount == 2) {
        DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_CIA2_SETTINGS_DIALOG, hwnd, dialog_proc);
    } else {
        DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_CIA1_SETTINGS_DIALOG, hwnd, dialog_proc);
    }
}
