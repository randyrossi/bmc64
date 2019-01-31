/*
 * uisampler.c - Implementation of the Sampler settings dialog box.
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
#include "sampler.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uiapi.h"
#include "uilib.h"
#include "uisampler.h"
#include "winmain.h"

static uilib_localize_dialog_param sampler_dialog_trans[] = {
    { 0, IDS_SAMPLER_CAPTION, -1 },
    { IDC_SAMPLER_DEVICE_LABEL, IDS_SAMPLER_DEVICE_LABEL, 0 },
    { IDC_SAMPLER_GAIN, IDS_SAMPLER_GAIN, 0 },
    { IDC_SAMPLER_FILE_LABEL, IDS_SAMPLER_FILE_LABEL, 0 },
    { IDC_SAMPLER_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group sampler_main_group[] = {
    { IDC_SAMPLER_DEVICE_LABEL, 0 },
    { IDC_SAMPLER_GAIN, 0 },
    { IDC_SAMPLER_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group sampler_right_group[] = {
    { IDC_SAMPLER_DEVICE, 0 },
    { IDC_SAMPLER_GAIN_VALUE, 0 },
    { IDC_SAMPLER_BROWSE, 0 },
    { IDC_SAMPLER_FILE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_sampler_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *samplerfile;
    TCHAR *st_samplerfile;
    TCHAR st[10];
    int xpos;
    RECT rect;
    sampler_device_t *devices = sampler_get_devices();
    int i;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, sampler_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, sampler_main_group);

    /* get the max x of the main group */
    uilib_get_group_max_x(hwnd, sampler_main_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, sampler_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, sampler_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_SAMPLER_DEVICE);
    for (i = 0; devices[i].name; ++i) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)devices[i].name);
    }
    resources_get_int("SamplerDevice", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("SamplerGain", &res_value);
    _stprintf(st, TEXT("%d"), res_value);
    SetDlgItemText(hwnd, IDC_SAMPLER_GAIN_VALUE, st);

    resources_get_string("SampleName", &samplerfile);
    st_samplerfile = system_mbstowcs_alloc(samplerfile);
    SetDlgItemText(hwnd, IDC_SAMPLER_FILE, samplerfile != NULL ? st_samplerfile : TEXT(""));
    system_mbstowcs_free(st_samplerfile);
}

static void end_sampler_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];
    int temp_val;
    int res_val;

    resources_set_int("SamplerDevice", (int)SendMessage(GetDlgItem(hwnd, IDC_SAMPLER_DEVICE), CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_SAMPLER_GAIN_VALUE, st, 4);
    temp_val = _ttoi(st);
    if (temp_val < 1) {
        res_val = 1;
    } else if (temp_val > 200) {
        res_val = 200;
    } else {
        res_val = temp_val;
    }
    
    if (temp_val != res_val) {
        ui_error(translate_text(IDS_VAL_D_FOR_S_OUT_RANGE_USE_D), temp_val, translate_text(IDS_SAMPLER_GAIN), res_val);
    }
    resources_set_int("SamplerGain", res_val);

    GetDlgItemText(hwnd, IDC_SAMPLER_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("SampleName", s);
}

static void browse_sampler_file(HWND hwnd)
{
    uilib_select_browse(hwnd, translate_text(IDS_SELECT_FILE_SAMPLER), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_SAMPLER_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_SAMPLER_BROWSE:
                    browse_sampler_file(hwnd);
                    break;
                case IDOK:
                    end_sampler_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_sampler_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_sampler_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_SAMPLER_SETTINGS_DIALOG, hwnd, dialog_proc);
}
