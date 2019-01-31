/*
 * uiram.c - Implementation of the RAM settings dialog box.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "lib.h"
#include "ram.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "uilib.h"
#include "uiram.h"
#include "winmain.h"

static int ui_ram_startvalue[] = {
    0, 255, -1
};

static int ui_ram_invertvalue[] = {
    0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, -1
};

static int orig_startvalue;
static int orig_valueinvert;
static int orig_patterninvert;

static int last_startvalue = -1;
static int last_valueinvert = -1;
static int last_patterninvert = -1;

static void update_preview(HWND hwnd)
{
    int curr_startvalue, curr_valueinvert, curr_patterninvert;
    char *s_win;
    TCHAR *st_win;

    resources_get_int("RAMInitStartValue", &curr_startvalue);
    resources_get_int("RAMInitValueInvert", &curr_valueinvert);
    resources_get_int("RAMInitPatternInvert", &curr_patterninvert);
    if ((curr_startvalue != last_startvalue) ||
        (curr_valueinvert != last_valueinvert) ||
        (curr_patterninvert != last_patterninvert)) {
        s_win = lib_malloc(65536 * 4);
        ram_init_print_pattern(s_win, 65536, "\r\n");
        st_win = system_mbstowcs_alloc(s_win);
        SetDlgItemText(hwnd, IDC_RAMINIT_PREVIEW, st_win);
        system_mbstowcs_free(st_win);
        lib_free(s_win);
        last_startvalue = curr_startvalue;
        last_valueinvert = curr_valueinvert;
        last_patterninvert = curr_patterninvert;
    }
}

static uilib_localize_dialog_param ram_dialog_trans[] = {
    { 0, IDS_RAM_CAPTION, -1 },
    { IDC_RAM_INIT_AT_POWERUP, IDS_RAM_INIT_AT_POWERUP, 0 },
    { IDC_RAM_VALUE_FIRST_BYTE, IDS_RAM_VALUE_FIRST_BYTE, 0 },
    { IDC_RAM_LENGTH_CONSTANT_VALUES, IDS_RAM_LENGTH_CONSTANT_VALUES, 0 },
    { IDC_RAM_LENGTH_CONSTANT_PATTERN, IDS_RAM_LENGTH_CONSTANT_PATTERN, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group ram_right_group[] = {
    { IDC_RAM_VALUE_FIRST_BYTE, 0 },
    { IDC_RAM_LENGTH_CONSTANT_VALUES, 0 },
    { IDC_RAM_LENGTH_CONSTANT_PATTERN, 0 },
    { 0, 0 }
};

static uilib_dialog_group ram_filling_group[] = {
    { IDC_RAM_INIT_AT_POWERUP, 0 },
    { IDC_RAM_VALUE_FIRST_BYTE, 0 },
    { IDC_RAM_LENGTH_CONSTANT_VALUES, 0 },
    { IDC_RAM_LENGTH_CONSTANT_PATTERN, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_ram_dialog(HWND hwnd)
{
    HWND temp_hwnd, temp_hwnd2;
    int i;
    LOGFONT logfont = { -12, -7, 0, 0, 400, 0, 0, 0, 0, 0, 0, DRAFT_QUALITY, FIXED_PITCH|FF_MODERN, TEXT("") };
    HFONT hfont = CreateFontIndirect(&logfont);
    int group_x;
    int size;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, ram_dialog_trans);

    /* get the max x of the group element */
    uilib_get_element_max_x(hwnd, IDC_RAM_INIT_AT_POWERUP, &group_x);

    /* get the size of the group element */
    uilib_get_element_size(hwnd, IDC_RAM_INIT_AT_POWERUP, &size);

    /* adjust the size of the right group elements */
    uilib_adjust_group_width(hwnd, ram_right_group);

    /* adjust the size of the group element */
    uilib_adjust_element_width(hwnd, IDC_RAM_INIT_AT_POWERUP);

    /* get the max x of the filling elements */
    uilib_get_group_max_x(hwnd, ram_filling_group, &xpos);

    if (xpos < group_x) {
        /* restore the size of the group element */
        uilib_set_element_width(hwnd, IDC_RAM_INIT_AT_POWERUP, size);
    } else {
        /* set the size of the group element */
        uilib_set_element_width(hwnd, IDC_RAM_INIT_AT_POWERUP, size + xpos - group_x + 10);

        /* set the size of the dialog window */
        GetWindowRect(hwnd, &rect);
        MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

        /* recenter the buttons in the newly resized dialog window */
        uilib_center_buttons(hwnd, move_buttons_group, 0);
    }

    if (hfont) {
        SendDlgItemMessage(hwnd, IDC_RAMINIT_PREVIEW, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE, 0));
    }

    resources_get_int("RAMInitStartValue", &orig_startvalue);
    temp_hwnd = GetDlgItem(hwnd,IDC_RAMINIT_STARTVALUE);

    for (i = 0; ui_ram_startvalue[i] >= 0; i++) {
        TCHAR s[16];

        _stprintf(s, TEXT("%d"), ui_ram_startvalue[i]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)s);
        if (orig_startvalue == ui_ram_startvalue[i]) {
            SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)i, 0);
        }
    }

    resources_get_int("RAMInitValueInvert", &orig_valueinvert);
    resources_get_int("RAMInitPatternInvert", &orig_patterninvert);
    temp_hwnd = GetDlgItem(hwnd, IDC_RAMINIT_VALUEINVERT);
    temp_hwnd2 = GetDlgItem(hwnd, IDC_RAMINIT_PATTERNINVERT);
    
    for (i = 0; ui_ram_invertvalue[i] >= 0; i++) {
        TCHAR s[16];

        _stprintf(s, TEXT("%d"), ui_ram_invertvalue[i]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)s);
        SendMessage(temp_hwnd2, CB_ADDSTRING, 0, (LPARAM)s);
        if (ui_ram_invertvalue[i] == orig_valueinvert) {
            SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)i, 0);
        }
        if (ui_ram_invertvalue[i] == orig_patterninvert) {
            SendMessage(temp_hwnd2, CB_SETCURSEL, (WPARAM)i, 0);
        }
    }

    update_preview(hwnd);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;
    int rv;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_RAMINIT_STARTVALUE:
                    rv = (int)SendMessage(GetDlgItem(hwnd, IDC_RAMINIT_STARTVALUE), CB_GETCURSEL, 0, 0);
                    resources_set_int("RAMInitStartValue", ui_ram_startvalue[rv]);
                    update_preview(hwnd);
                    break;
                case IDC_RAMINIT_VALUEINVERT:
                    rv = (int)SendMessage(GetDlgItem(hwnd, IDC_RAMINIT_VALUEINVERT), CB_GETCURSEL, 0, 0);
                    resources_set_int("RAMInitValueInvert", ui_ram_invertvalue[rv]);
                    update_preview(hwnd);
                    break;
                case IDC_RAMINIT_PATTERNINVERT:
                    rv = (int)SendMessage(GetDlgItem(hwnd, IDC_RAMINIT_PATTERNINVERT), CB_GETCURSEL, 0, 0);
                    resources_set_int("RAMInitPatternInvert", ui_ram_invertvalue[rv]);
                    update_preview(hwnd);
                    break;
                case IDOK:
                    EndDialog(hwnd, 0);
                    return TRUE;
                case IDCANCEL:
                    resources_set_int("RAMInitStartValue", orig_startvalue);
                    resources_set_int("RAMInitValueInvert", orig_valueinvert);
                    resources_set_int("RAMInitPatternInvert", orig_patterninvert);
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_ram_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_ram_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_RAM_SETTINGS_DIALOG), hwnd, dialog_proc);
}
