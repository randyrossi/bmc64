/*
 * uic64model.c - Implementation of the c64(sc) model settings dialog box.
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

#include "c64model.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uic64model.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param c64model_dialog_trans[] = {
    { IDC_C64MODEL_LABEL, IDS_C64_MODEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group c64model_left_group[] = {
    { IDC_C64MODEL_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group c64model_right_group[] = {
    { IDC_C64MODEL_LIST, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static int c64_models[] = {
    IDS_C64_PAL,
    IDS_C64C_PAL,
    IDS_C64_OLD_PAL,
    IDS_C64_NTSC,
    IDS_C64C_NTSC,
    IDS_C64_OLD_NTSC,
    IDS_DREAN,
    IDS_C64_SX_PAL,
    IDS_C64_SX_NTSC,
    IDS_JAPANESE,
    IDS_C64_GS,
    IDS_PET64_PAL,
    IDS_PET64_NTSC,
    IDS_MAX_MACHINE,
    IDS_UNKNOWN,
    0
};

static int c64_models_values[] = {
    C64MODEL_C64_PAL,
    C64MODEL_C64C_PAL,
    C64MODEL_C64_OLD_PAL,
    C64MODEL_C64_NTSC,
    C64MODEL_C64C_NTSC,
    C64MODEL_C64_OLD_NTSC,
    C64MODEL_C64_PAL_N,
    C64MODEL_C64SX_PAL,
    C64MODEL_C64SX_NTSC,
    C64MODEL_C64_JAP,
    C64MODEL_C64_GS,
    C64MODEL_PET64_PAL,
    C64MODEL_PET64_NTSC,
    C64MODEL_ULTIMAX,
    C64MODEL_UNKNOWN
};

static void init_c64model_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value, index = 0;
    int xpos, i;
    RECT rect;

    EnableWindow(GetDlgItem(hwnd, IDC_C64MODEL_LIST), 1);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, c64model_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, c64model_left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, c64model_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, c64model_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, c64model_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_C64MODEL_LIST);
    res_value = c64model_get();
    for (i = 0; c64_models[i] != 0; i++) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(c64_models[i]));
        if (res_value == c64_models_values[i]) {
            index = i;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)index, 0);
}

static void end_c64model_dialog(HWND hwnd)
{
    c64model_set((int)c64_models_values[SendMessage(GetDlgItem(hwnd, IDC_C64MODEL_LIST), CB_GETCURSEL, 0, 0)]);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_c64model_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_c64model_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_c64model_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_C64MODEL_SETTINGS_DIALOG, hwnd, dialog_proc);
}
