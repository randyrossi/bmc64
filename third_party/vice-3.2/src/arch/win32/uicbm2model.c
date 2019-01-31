/*
 * uicbm2model.c - Implementation of the cbm2 model settings dialog box.
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
#include "uicbm2model.h"
#include "uilib.h"
#include "winmain.h"

#include "log.h"

static uilib_localize_dialog_param cbm2model_dialog_trans[] = {
    { IDC_CBM2MODEL_LABEL, IDS_CBM2MODEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group cbm2model_left_group[] = {
    { IDC_CBM2MODEL_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group cbm2model_right_group[] = {
    { IDC_CBM2MODEL_LIST, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_cbm2model_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xpos;
    RECT rect;

    EnableWindow(GetDlgItem(hwnd, IDC_CBM2MODEL_LIST), 1);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, cbm2model_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, cbm2model_left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, cbm2model_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, cbm2model_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, cbm2model_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_CBM2MODEL_LIST);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"610 PAL");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"610 NTSC");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"620 PAL");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"620 NTSC");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"620+ PAL");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"620+ NTSC");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"710 NTSC");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"720 NTSC");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"720+ NTSC");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(IDS_UNKNOWN));

    res_value = cbm2model_get();
    /* log_debug("cbm2model = %d\n", res_value); */
    if (res_value > CBM2MODEL_720PLUS_NTSC) {
        res_value = 9;  /* Unknown model */
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)(res_value - 2), 0);
}

static void end_cbm2model_dialog(HWND hwnd)
{
    int result = (int)SendMessage(GetDlgItem(hwnd, IDC_CBM2MODEL_LIST),
            CB_GETCURSEL, 0, 0);
    /* log_debug("cbm2model dialog result = %d\n", result); */
    cbm2model_set(result + 2);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_cbm2model_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_cbm2model_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_cbm2model_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_CBM2MODEL_SETTINGS_DIALOG, hwnd, dialog_proc);
}
