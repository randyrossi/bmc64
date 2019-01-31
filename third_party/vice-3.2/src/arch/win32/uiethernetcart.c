/*
 * uiethernetcart.c - Implementation of the Ethernet Cart settings dialog box.
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
#include "uilib.h"
#include "uiethernetcart.h"
#include "winmain.h"

static const int ui_ethernetcart_base64[] = {
    0xde00,
    0xde10,
    0xde20,
    0xde30,
    0xde40,
    0xde50,
    0xde60,
    0xde70,
    0xde80,
    0xde90,
    0xdea0,
    0xdeb0,
    0xdec0,
    0xded0,
    0xdee0,
    0xdef0,
    0xdf00,
    0xdf10,
    0xdf20,
    0xdf30,
    0xdf40,
    0xdf50,
    0xdf60,
    0xdf70,
    0xdf80,
    0xdf90,
    0xdfa0,
    0xdfb0,
    0xdfc0,
    0xdfd0,
    0xdfe0,
    0xdff0,
    0
};

static const int ui_ethernetcart_base20[] = {
    0x9800,
    0x9810,
    0x9820,
    0x9830,
    0x9840,
    0x9850,
    0x9860,
    0x9870,
    0x9880,
    0x9890,
    0x98a0,
    0x98b0,
    0x98c0,
    0x98d0,
    0x98e0,
    0x98f0,
    0x9c00,
    0x9c10,
    0x9c20,
    0x9c30,
    0x9c40,
    0x9c50,
    0x9c60,
    0x9c70,
    0x9c80,
    0x9c90,
    0x9ca0,
    0x9cb0,
    0x9cc0,
    0x9cd0,
    0x9ce0,
    0x9cf0,
    0
};

static void enable_ethernetcart_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_ETHERNETCART_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_ETHERNETCART_MODE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_ETHERNETCART_BASE), is_enabled);
}

static uilib_localize_dialog_param ethernetcart_dialog_trans[] = {
    { 0, IDS_ETHERNETCART_CAPTION, -1 },
    { IDC_ETHERNETCART_ENABLE, IDS_ETHERNETCART_ENABLE, 0 },
    { IDC_ETHERNETCART_MODE_LABEL, IDS_ETHERNETCART_MODE_LABEL, 0 },
    { IDC_ETHERNETCART_BASE_LABEL, IDS_ETHERNETCART_BASE_LABEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group ethernetcart_main_group[] = {
    { IDC_ETHERNETCART_ENABLE, 1 },
    { IDC_ETHERNETCART_MODE_LABEL, 0 },
    { IDC_ETHERNETCART_BASE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group ethernetcart_right_group[] = {
    { IDC_ETHERNETCART_ENABLE, 0 },
    { IDC_ETHERNETCART_MODE, 0 },
    { IDC_ETHERNETCART_BASE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_ethernetcart_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int res_value_loop;
    int active_value;
    int xpos;
    RECT rect;
    const int *ui_ethernetcart_base = (machine_class == VICE_MACHINE_VIC20) ? ui_ethernetcart_base20 : ui_ethernetcart_base64;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, ethernetcart_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, ethernetcart_main_group);

    /* get the max x of the main group */
    uilib_get_group_max_x(hwnd, ethernetcart_main_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, ethernetcart_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, ethernetcart_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("ETHERNETCART_ACTIVE", &res_value);
    CheckDlgButton(hwnd, IDC_ETHERNETCART_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    temp_hwnd = GetDlgItem(hwnd, IDC_ETHERNETCART_MODE);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("TFE"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RR-Net"));
    resources_get_int("ETHERNETCARTMode", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_ETHERNETCART_BASE);
    for (res_value_loop = 0; ui_ethernetcart_base[res_value_loop] != 0; res_value_loop++) {
        TCHAR st[10];

        _stprintf(st, "$%X", ui_ethernetcart_base[res_value_loop]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    resources_get_int("ETHERNETCARTBase", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_ethernetcart_base[res_value_loop] != 0; res_value_loop++) {
        if (ui_ethernetcart_base[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    enable_ethernetcart_controls(hwnd);
}

static void end_ethernetcart_dialog(HWND hwnd)
{
    const int *ui_ethernetcart_base = (machine_class == VICE_MACHINE_VIC20) ? ui_ethernetcart_base20 : ui_ethernetcart_base64;

    resources_set_int("ETHERNETCART_ACTIVE", (IsDlgButtonChecked(hwnd, IDC_ETHERNETCART_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("ETHERNETCARTMode", (int)SendMessage(GetDlgItem(hwnd, IDC_ETHERNETCART_MODE), CB_GETCURSEL, 0, 0));
    resources_set_int("ETHERNETCARTBase", ui_ethernetcart_base[SendMessage(GetDlgItem(hwnd, IDC_ETHERNETCART_BASE), CB_GETCURSEL, 0, 0)]);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_ETHERNETCART_ENABLE:
                    enable_ethernetcart_controls(hwnd);
                    break;
                case IDOK:
                    end_ethernetcart_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_ethernetcart_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_ethernetcart_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_ETHERNETCART_SETTINGS_DIALOG, hwnd, dialog_proc);
}
