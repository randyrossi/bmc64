/*
 * uicbm5x0model.c - Implementation of the cbm5x0 model settings dialog box.
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

#include "cbm2model.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uicbm5x0model.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param cbm5x0model_dialog_trans[] = {
    { IDC_CBM2MODEL_LABEL, IDS_CBM5X0MODEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group cbm5x0model_left_group[] = {
    { IDC_CBM2MODEL_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm5x0model_right_group[] = {
    { IDC_CBM2MODEL_LIST, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_cbm5x0model_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xpos;
    RECT rect;

    EnableWindow(GetDlgItem(hwnd, IDC_CBM2MODEL_LIST), 1);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, cbm5x0model_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, cbm5x0model_left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, cbm5x0model_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, cbm5x0model_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, cbm5x0model_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_CBM2MODEL_LIST);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"510 PAL");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"510 NTSC");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(IDS_UNKNOWN));

    res_value = cbm2model_get();
    if (res_value > 2) {
        res_value = 2;
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
}

static void end_cbm5x0model_dialog(HWND hwnd)
{
    cbm2model_set((int)SendMessage(GetDlgItem(hwnd, IDC_CBM2MODEL_LIST), CB_GETCURSEL, 0, 0));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_cbm5x0model_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_cbm5x0model_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_cbm5x0model_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_CBM2MODEL_SETTINGS_DIALOG, hwnd, dialog_proc);
}
