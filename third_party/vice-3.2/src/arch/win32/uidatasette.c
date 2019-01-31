/*
 * uidatasette.c - Implementation of the datasette settings dialog box.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@arcormail.de>
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

#include <tchar.h>
#include <windows.h>

#include "datasette.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uidatasette.h"
#include "uilib.h"
#include "winmain.h"

static const int ui_datasette_zero_gap_delay[] = {
    1000,
    2000,
    5000,
    10000,
    20000,
    50000,
    100000
};

static uilib_localize_dialog_param datasette_dialog[] = {
    { 0, IDS_DATASETTE_CAPTION, -1 },
    { IDC_DATASETTE_RESET_WITH_CPU, IDS_DATASETTE_RESET_WITH_CPU, 0 },
    { IDC_DATASETTE_DELAY_TRIGGER, IDS_DATASETTE_DELAY_TRIGGER, 0 },
    { IDC_DATASETTE_DELAY_AT_ZERO, IDS_DATASETTE_DELAY_AT_ZERO, 0 },
    { IDC_DATASETTE_TAPE_WOBBLE_LABEL, IDS_DATASETTE_TAPE_WOBBLE, 0 },
    { IDC_DATASETTE_MOTOR_GROUP, IDS_DATASETTE_MOTOR_GROUP, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group datasette_main_group[] = {
    { IDC_DATASETTE_RESET_WITH_CPU, 0 },
    { IDC_DATASETTE_MOTOR_GROUP, 0 },
    { 0, 0 }
};

static uilib_dialog_group datasette_sub_group[] = {
    { IDC_DATASETTE_DELAY_TRIGGER, 0 },
    { IDC_DATASETTE_DELAY_AT_ZERO, 0 },
    { 0, 0 }
};

static uilib_dialog_group datasette_size_group[] = {
    { IDC_DATASETTE_DELAY_TRIGGER, 0 },
    { IDC_DATASETTE_DELAY_AT_ZERO, 0 },
    { IDC_DATASETTE_TAPE_WOBBLE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_datasette_dialog(HWND hwnd)
{
    TCHAR st[256];
    HWND snd_hwnd;
    int res_value;
    int res_value_loop;
    int active_value;
    int xpos_max;
    int group_max;
    int wobble;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, datasette_dialog);

    /* adjust the size of the elements in the datasette_sub_group */
    uilib_adjust_group_width(hwnd, datasette_sub_group);

    /* adjust the size of the 'tape wobble' label element */
    uilib_adjust_element_width(hwnd, IDC_DATASETTE_TAPE_WOBBLE_LABEL);

    /* get the max x of the 'tape wobble' label element */
    uilib_get_element_max_x(hwnd, IDC_DATASETTE_TAPE_WOBBLE_LABEL, &xpos_max);

    /* move the 'tape wobble' input element */
    uilib_move_element(hwnd, IDC_DATASETTE_TAPE_WOBBLE, xpos_max + 10);

    /* adjust the size of the  'reset with cpu' element */
    uilib_adjust_element_width(hwnd, IDC_DATASETTE_RESET_WITH_CPU);

    /* adjust the size of the motor group element */
    uilib_adjust_element_width(hwnd, IDC_DATASETTE_MOTOR_GROUP);

    /* get the max x of the datasette_size_group */
    uilib_get_group_max_x(hwnd, datasette_size_group, &xpos_max);

    /* get the max x of the motor group element */
    uilib_get_element_max_x(hwnd, IDC_DATASETTE_MOTOR_GROUP, &group_max);

    /* calculate the max x of the motor group element and the
       datasette_sub_group elements */
    if (group_max < xpos_max + 10) {
        group_max = xpos_max + 10;
    }

    /* set the width of the motor group element to 'surround' the
       datasette_sub_group elements */
    uilib_set_element_width(hwnd, IDC_DATASETTE_MOTOR_GROUP, group_max);

    /* get the max x of the datasette_main_group elements */
    uilib_get_group_max_x(hwnd, datasette_main_group, &xpos_max);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos_max + 10, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("DatasetteTapeWobble", &wobble);
    _stprintf(st, TEXT("%d"), wobble);
    SetDlgItemText(hwnd, IDC_DATASETTE_TAPE_WOBBLE, st);

    resources_get_int("DatasetteResetWithCPU", &res_value);
    CheckDlgButton(hwnd, IDC_DATASETTE_RESET_WITH_CPU, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    snd_hwnd = GetDlgItem(hwnd, IDC_DATASETTE_ZERO_GAP_DELAY);
    for (res_value_loop = 0; res_value_loop < 7; res_value_loop++) {
        TCHAR st[10];

        _stprintf(st, TEXT("%d"), ui_datasette_zero_gap_delay[res_value_loop]);
        SendMessage(snd_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("DatasetteZeroGapDelay", &res_value);
    active_value = 4; /* default */
    for (res_value_loop = 0; res_value_loop < 7; res_value_loop++) {
        if (ui_datasette_zero_gap_delay[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(snd_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    snd_hwnd = GetDlgItem(hwnd, IDC_DATASETTE_SPEED_TUNING);
    for (res_value_loop = 0; res_value_loop < 8; res_value_loop++) {
        TCHAR st[10];
        _stprintf(st, TEXT("%d"), res_value_loop);
        SendMessage(snd_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("DatasetteSpeedTuning", &res_value);
    SendMessage(snd_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
}

static void end_datasette_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    int wobble;

    resources_set_int("DatasetteResetWithCPU", (IsDlgButtonChecked(hwnd, IDC_DATASETTE_RESET_WITH_CPU) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("DatasetteSpeedTuning", (int)SendDlgItemMessage(hwnd, IDC_DATASETTE_SPEED_TUNING, CB_GETCURSEL, 0, 0));
    resources_set_int("DatasetteZeroGapDelay", ui_datasette_zero_gap_delay[SendDlgItemMessage(hwnd, IDC_DATASETTE_ZERO_GAP_DELAY, CB_GETCURSEL, 0, 0)]);

    GetDlgItemText(hwnd, IDC_DATASETTE_TAPE_WOBBLE, st, MAX_PATH);
    wobble = atoi(st);
    resources_set_int("DatasetteTapeWobble", wobble);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command=LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_datasette_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_datasette_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


static void uidatasette_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_DATASETTE_SETTINGS_DIALOG), hwnd, dialog_proc);
}

void uidatasette_command(HWND hwnd, WPARAM wparam)
{
    switch (wparam) {
        case IDM_DATASETTE_SETTINGS:
            uidatasette_settings_dialog(hwnd);
            break;
        case IDM_DATASETTE_CONTROL_STOP:
            datasette_control(DATASETTE_CONTROL_STOP);
            break;
        case IDM_DATASETTE_CONTROL_START:
            datasette_control(DATASETTE_CONTROL_START);
            break;
        case IDM_DATASETTE_CONTROL_FORWARD:
            datasette_control(DATASETTE_CONTROL_FORWARD);
            break;
        case IDM_DATASETTE_CONTROL_REWIND:
            datasette_control(DATASETTE_CONTROL_REWIND);
            break;
        case IDM_DATASETTE_CONTROL_RECORD:
            datasette_control(DATASETTE_CONTROL_RECORD);
            break;
        case IDM_DATASETTE_CONTROL_RESET:
            datasette_control(DATASETTE_CONTROL_RESET);
            break;
        case IDM_DATASETTE_RESET_COUNTER:
            datasette_control(DATASETTE_CONTROL_RESET_COUNTER);
            break;
    }
}
