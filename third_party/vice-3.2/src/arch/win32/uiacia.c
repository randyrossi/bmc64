/*
 * uiacia.c - Implementation of the ACIA settings dialog box.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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
#include <windows.h>
#include <tchar.h>

#include "intl.h"
#include "lib.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "rs232.h"
#include "translate.h"
#include "uiacia.h"
#include "winmain.h"
#include "uilib.h"

static int c64_base_address[] = {
    0xde00,
    0xdf00,
    -1
};

static int c128_base_address[] = {
    0xd700,
    0xde00,
    0xdf00,
    -1
};

static int vic20_base_address[] = {
    0x9800,
    0x9c00,
    -1
};

static const int interrupt_names[] = {
    IDS_NONE,
    IDS_IRQ,
    IDS_NMI,
    0
};

static const int mode_names[] = {
    IDS_NORMAL,
    IDS_SWIFTLINK,
    IDS_TURBO232,
    0
};

static void enable_acia_controls(HWND hwnd)
{
    int enabled = 1;

    if (IsDlgButtonChecked(hwnd, IDC_ACIA_ENABLE) != BST_CHECKED) {
        enabled = 0;
    }

    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_LOCATION), enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_DEVICE), enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_INTERRUPT), enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_MODE), enabled);
}

static void enable_plus4_acia_controls(HWND hwnd)
{
    int enabled = 1;

    if (IsDlgButtonChecked(hwnd, IDC_ACIA_ENABLE) != BST_CHECKED) {
        enabled = 0;
    }

    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_INTERRUPT), 0);
    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_DEVICE), enabled);
}

static void enable_nonc64_acia_controls(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_DEVICE), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_ACIA_INTERRUPT), 0);
}

static uilib_localize_dialog_param acia_dialog_trans[] = {
    { 0, IDS_ACIA_CAPTION, -1 },
    { IDC_ACIA_ENABLE, IDS_ACIA_ENABLE, 0 },
    { IDC_ACIA_DEVICE_LABEL, IDS_ACIA_DEVICE, 0 },
    { IDC_ACIA_LOCATION_LABEL, IDS_ACIA_LOCATION, 0 },
    { IDC_ACIA_INTERRUPT_LABEL, IDS_ACIA_INTERRUPT, 0 },
    { IDC_ACIA_MODE_LABEL, IDS_ACIA_MODE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static void init_acia_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    unsigned int i;
    RECT rect;
    RECT child_rect;
    int min_width;
    int xpos;
    int xsize, ysize;
    int res_value_loop;
    int active_value;
    int *current_base_address;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, acia_dialog_trans);

    GetClientRect(hwnd, &rect);

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_ENABLE);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    xsize += 20;
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    min_width = child_rect.left + xsize + 10;

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    xpos = child_rect.left + xsize + 10;

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_LOCATION_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_INTERRUPT_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_MODE_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_LOCATION);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_INTERRUPT);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_MODE);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, min_width + 20, rect.bottom - rect.top, TRUE);

    resources_get_int("Acia1Enable", &res_value);
    CheckDlgButton(hwnd, IDC_ACIA_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("Acia1Dev", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE);
    for (i = 0; i < RS232_NUM_DEVICES; i++) {
        TCHAR st[20];
        lib_sntprintf(st, 20, intl_translate_tcs(IDS_RS232_DEVICE_I), i + 1);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("Acia1Irq", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_INTERRUPT);
    for (res_value_loop = 0; interrupt_names[res_value_loop];
        res_value_loop++) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(interrupt_names[res_value_loop]));
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    active_value = 0;
    resources_get_int("Acia1Base", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_LOCATION);
    switch (machine_class) {
        case VICE_MACHINE_C128:
            current_base_address = c128_base_address;
            break;
        case VICE_MACHINE_VIC20:
            current_base_address = vic20_base_address;
            break;
        default:
            current_base_address = c64_base_address;
            break;
    }

    for (res_value_loop = 0; current_base_address[res_value_loop] != -1; res_value_loop++) {
        TCHAR st[10];

        lib_sntprintf(st, 10, TEXT("$%X"), current_base_address[res_value_loop]);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    for (res_value_loop = 0; current_base_address[res_value_loop] != -1; res_value_loop++) {
        if (current_base_address[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_int("Acia1Mode", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_MODE);
    for (res_value_loop = 0; mode_names[res_value_loop]; res_value_loop++) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(mode_names[res_value_loop]));
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    enable_acia_controls(hwnd);
}

static uilib_localize_dialog_param plus4_acia_dialog_trans[] = {
    { 0, IDS_ACIA_CAPTION, -1 },
    { IDC_ACIA_ENABLE, IDS_ACIA_ENABLE, 0 },
    { IDC_ACIA_DEVICE_LABEL, IDS_ACIA_DEVICE, 0 },
    { IDC_ACIA_INTERRUPT_LABEL, IDS_ACIA_INTERRUPT, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static void init_plus4_acia_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    unsigned int i;
    RECT rect;
    RECT child_rect;
    int min_width;
    int xpos;
    int xsize, ysize;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, plus4_acia_dialog_trans);

    GetClientRect(hwnd, &rect);

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_ENABLE);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    xsize += 20;
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    min_width = child_rect.left + xsize + 10;

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    xpos = child_rect.left + xsize + 10;

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_INTERRUPT_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_INTERRUPT);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, min_width + 20, rect.bottom - rect.top, TRUE);

    resources_get_int("Acia1Enable", &res_value);
    CheckDlgButton(hwnd, IDC_ACIA_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("Acia1Dev", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE);
    for (i = 0; i < RS232_NUM_DEVICES; i++) {
        TCHAR st[20];
        lib_sntprintf(st, 20, intl_translate_tcs(IDS_RS232_DEVICE_I), i + 1);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    enable_plus4_acia_controls(hwnd);
}

static uilib_localize_dialog_param nonc64_acia_dialog_trans[] = {
    { 0, IDS_ACIA_CAPTION, -1 },
    { IDC_ACIA_DEVICE_LABEL, IDS_ACIA_DEVICE, 0 },
    { IDC_ACIA_INTERRUPT_LABEL, IDS_ACIA_INTERRUPT, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static void init_nonc64_acia_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    unsigned int i;
    RECT rect;
    RECT child_rect;
    int min_width = 0;
    int xpos;
    int xsize, ysize;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, nonc64_acia_dialog_trans);

    GetClientRect(hwnd, &rect);

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    xpos = child_rect.left + xsize + 10;

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_INTERRUPT_LABEL);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(temp_hwnd, &xsize, &ysize);
    MoveWindow(temp_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_INTERRUPT);
    GetClientRect(temp_hwnd, &child_rect);
    MapWindowPoints(temp_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(temp_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
    if (min_width < xpos + child_rect.right - child_rect.left) {
        min_width = xpos + child_rect.right - child_rect.left;
    }

    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, min_width + 20, rect.bottom - rect.top, TRUE);

    resources_get_int("Acia1Dev", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_ACIA_DEVICE);
    for (i = 0; i < RS232_NUM_DEVICES; i++) {
        TCHAR st[20];
        lib_sntprintf(st, 20, intl_translate_tcs(IDS_RS232_DEVICE_I), i + 1);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    enable_nonc64_acia_controls(hwnd);
}

static void end_acia_dialog(HWND hwnd)
{
    int base;
    int *current_base_address;

    resources_set_int("Acia1Enable", (IsDlgButtonChecked(hwnd, IDC_ACIA_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("Acia1Dev", (int)SendMessage(GetDlgItem(hwnd, IDC_ACIA_DEVICE), CB_GETCURSEL, 0, 0));
    resources_set_int("Acia1Irq", (int)SendMessage(GetDlgItem(hwnd, IDC_ACIA_INTERRUPT), CB_GETCURSEL, 0, 0));
    resources_set_int("Acia1Mode", (int)SendMessage(GetDlgItem(hwnd, IDC_ACIA_MODE), CB_GETCURSEL, 0, 0));

    base = (int)SendMessage(GetDlgItem(hwnd, IDC_ACIA_LOCATION), CB_GETCURSEL, 0, 0);
    switch (machine_class) {
        case VICE_MACHINE_C128:
            current_base_address = c128_base_address;
            break;
        case VICE_MACHINE_VIC20:
            current_base_address = vic20_base_address;
            break;
        default:
            current_base_address = c64_base_address;
            break;
    }
    resources_set_int("Acia1Base", current_base_address[base]);
}

static void end_plus4_acia_dialog(HWND hwnd)
{
    resources_set_int("Acia1Enable", (IsDlgButtonChecked(hwnd, IDC_ACIA_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("Acia1Dev", (int)SendMessage(GetDlgItem(hwnd, IDC_ACIA_DEVICE), CB_GETCURSEL, 0, 0));
}

static void end_nonc64_acia_dialog(HWND hwnd)
{
    resources_set_int("Acia1Dev", (int)SendMessage(GetDlgItem(hwnd, IDC_ACIA_DEVICE), CB_GETCURSEL, 0, 0));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_ACIA_ENABLE:
                    enable_acia_controls(hwnd);
                    break;
                case IDOK:
                    end_acia_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_acia_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK plus4_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_ACIA_ENABLE:
                    enable_plus4_acia_controls(hwnd);
                    break;
                case IDOK:
                    end_plus4_acia_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_plus4_acia_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK nonc64_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_nonc64_acia_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_nonc64_acia_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_acia_settings_dialog(HWND hwnd)
{
    switch (machine_class) {
        case VICE_MACHINE_C64:
        case VICE_MACHINE_C64SC:
        case VICE_MACHINE_SCPU64:
        case VICE_MACHINE_C128:
        case VICE_MACHINE_VIC20:
            DialogBox(winmain_instance, (LPCTSTR)IDD_ACIA_SETTINGS_DIALOG, hwnd, dialog_proc);
            break;
        case VICE_MACHINE_PLUS4:
            DialogBox(winmain_instance, (LPCTSTR)IDD_ACIA_SETTINGS_PLUS4_DIALOG, hwnd, plus4_dialog_proc);
            break;
        default:
            DialogBox(winmain_instance, (LPCTSTR)IDD_ACIA_SETTINGS_NONC64_DIALOG, hwnd, nonc64_dialog_proc);
            break;
    }
}
