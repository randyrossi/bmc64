/*
 * uimmc64.c - Implementation of the MMC64 settings dialog box.
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

#include "clockport.h"
#include "intl.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uimmc64.h"
#include "winmain.h"

static TCHAR *ui_mmc64_sdtype[] = {
    TEXT("Auto"),
    TEXT("MMC"),
    TEXT("SD"),
    TEXT("SDHC"),
    NULL
};

static int clockport_ids[CLOCKPORT_MAX_ENTRIES + 1];

static void enable_mmc64_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_MMC64_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_REVISION), is_enabled);

    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_FLASHJUMPER), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_BIOS_SAVE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_BIOS_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_BIOS_FILE), is_enabled);

    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_IMAGE_RO), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_IMAGE_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_IMAGE_FILE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_SDTYPE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MMC64_CLOCKPORT_DEVICE), is_enabled);
}

static uilib_localize_dialog_param mmc64_dialog_trans[] = {
    { IDC_MMC64_ENABLE, IDS_MMC64_ENABLE, 0 },
    { IDC_MMC64_REVISION_LABEL, IDS_MMC64_REVISION_LABEL, 0 },
    { IDC_MMC64_FLASHJUMPER, IDS_MMC64_FLASHJUMPER, 0 },
    { IDC_MMC64_BIOS_SAVE, IDS_MMC64_BIOS_SAVE, 0 },
    { IDC_MMC64_BIOS_FILE_LABEL, IDS_MMC64_BIOS_FILE_LABEL, 0 },
    { IDC_MMC64_BIOS_BROWSE, IDS_BROWSE, 0 },
    { IDC_MMC64_IMAGE_RO, IDS_MMC64_IMAGE_RO, 0 },
    { IDC_MMC64_IMAGE_FILE_LABEL, IDS_MMC64_IMAGE_FILE_LABEL, 0 },
    { IDC_MMC64_IMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDC_MMC64_SDTYPE_LABEL, IDS_MMC64_SDTYPE_LABEL, 0 },
    { IDC_MMC64_CLOCKPORT_DEVICE_LABEL, IDS_MMC64_CLOCKPORT_DEVICE_LABEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group mmc64_main_group[] = {
    { IDC_MMC64_ENABLE, 1 },
    { IDC_MMC64_REVISION_LABEL, 0 },
    { IDC_MMC64_FLASHJUMPER, 1 },
    { IDC_MMC64_BIOS_SAVE, 1 },
    { IDC_MMC64_BIOS_FILE_LABEL, 0 },
    { IDC_MMC64_IMAGE_RO, 1 },
    { IDC_MMC64_IMAGE_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group mmc64_left_group[] = {
    { IDC_MMC64_REVISION_LABEL, 0 },
    { IDC_MMC64_BIOS_FILE_LABEL, 0 },
    { IDC_MMC64_IMAGE_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group mmc64_right_group[] = {
    { IDC_MMC64_REVISION, 0 },
    { IDC_MMC64_BIOS_BROWSE, 0 },
    { IDC_MMC64_IMAGE_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group mmc64_window_group[] = {
    { IDC_MMC64_ENABLE, 0 },
    { IDC_MMC64_REVISION, 0 },
    { IDC_MMC64_FLASHJUMPER, 0 },
    { IDC_MMC64_BIOS_SAVE, 0 },
    { IDC_MMC64_BIOS_BROWSE, 0 },
    { IDC_MMC64_BIOS_FILE, 0 },
    { IDC_MMC64_IMAGE_RO, 0 },
    { IDC_MMC64_IMAGE_BROWSE, 0 },
    { IDC_MMC64_IMAGE_FILE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_mmc64_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int res_value_loop;
    int current_val = 0;
    const char *mmc64_image_file;
    TCHAR *st_mmc64_image_file;
    const char *mmc64_bios_file;
    TCHAR *st_mmc64_bios_file;
    TCHAR *st_clockport_device_name;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, mmc64_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, mmc64_main_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, mmc64_left_group, &xpos);

    /* move the right group to the correct location */
    uilib_move_group(hwnd, mmc64_right_group, xpos + 10);

    /* get the max x of the window group */
    uilib_get_group_max_x(hwnd, mmc64_window_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("MMC64", &res_value);
    CheckDlgButton(hwnd, IDC_MMC64_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    temp_hwnd = GetDlgItem(hwnd, IDC_MMC64_REVISION);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Rev A"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Rev B"));
    resources_get_int("MMC64_revision", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("MMC64_flashjumper", &res_value);
    CheckDlgButton(hwnd, IDC_MMC64_FLASHJUMPER, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("MMC64_bios_write", &res_value);
    CheckDlgButton(hwnd, IDC_MMC64_BIOS_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_string("MMC64BIOSfilename", &mmc64_bios_file);
    st_mmc64_bios_file = system_mbstowcs_alloc(mmc64_bios_file);
    SetDlgItemText(hwnd, IDC_MMC64_BIOS_FILE, mmc64_bios_file != NULL ? st_mmc64_bios_file : TEXT(""));
    system_mbstowcs_free(st_mmc64_bios_file);

    resources_get_int("MMC64_RO", &res_value);
    CheckDlgButton(hwnd, IDC_MMC64_IMAGE_RO, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_string("MMC64imagefilename", &mmc64_image_file);
    st_mmc64_image_file = system_mbstowcs_alloc(mmc64_image_file);
    SetDlgItemText(hwnd, IDC_MMC64_IMAGE_FILE, mmc64_image_file != NULL ? st_mmc64_image_file : TEXT(""));
    system_mbstowcs_free(st_mmc64_image_file);

    resources_get_int("MMC64_sd_type", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_MMC64_SDTYPE);
    for (res_value_loop = 0; ui_mmc64_sdtype[res_value_loop]; res_value_loop++) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_mmc64_sdtype[res_value_loop]);
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("MMC64ClockPort", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_MMC64_CLOCKPORT_DEVICE);
    for (res_value_loop = 0; clockport_supported_devices[res_value_loop].name; res_value_loop++) {
        st_clockport_device_name = system_mbstowcs_alloc(clockport_supported_devices[res_value_loop].name);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st_clockport_device_name);
        system_mbstowcs_free(st_clockport_device_name);
        clockport_ids[res_value_loop] = clockport_supported_devices[res_value_loop].id;
        if (clockport_ids[res_value_loop] == res_value) {
            current_val = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)current_val, 0);

    enable_mmc64_controls(hwnd);
}

static void end_mmc64_dialog(HWND hwnd)
{
    TCHAR st_name[MAX_PATH];
    char name[MAX_PATH];

    GetDlgItemText(hwnd, IDC_MMC64_BIOS_FILE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("MMC64BIOSfilename", name);

    GetDlgItemText(hwnd, IDC_MMC64_IMAGE_FILE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("MMC64imagefilename", name);
    resources_set_int("MMC64", (IsDlgButtonChecked(hwnd, IDC_MMC64_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMC64_flashjumper", (IsDlgButtonChecked(hwnd, IDC_MMC64_FLASHJUMPER) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMC64_bios_write", (IsDlgButtonChecked(hwnd, IDC_MMC64_BIOS_SAVE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMC64_RO", (IsDlgButtonChecked(hwnd, IDC_MMC64_IMAGE_RO) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("MMC64_revision",(int)SendMessage(GetDlgItem(hwnd, IDC_MMC64_REVISION), CB_GETCURSEL, 0, 0));
    resources_set_int("MMC64_sd_type", (int)SendMessage(GetDlgItem(hwnd, IDC_MMC64_SDTYPE), CB_GETCURSEL, 0, 0));
    resources_set_int("MMC64ClockPort", clockport_ids[(int)SendMessage(GetDlgItem(hwnd, IDC_MMC64_CLOCKPORT_DEVICE), CB_GETCURSEL, 0, 0)]);
}

static void browse_mmc64_bios_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_MMC64_BIOS_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_MMC64_BIOS_FILE);
}

static void browse_mmc64_image_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_MMC64_CARD_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_MMC64_IMAGE_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_MMC64_BIOS_BROWSE:
                    browse_mmc64_bios_file(hwnd);
                    break;
                case IDC_MMC64_IMAGE_BROWSE:
                    browse_mmc64_image_file(hwnd);
                    break;
                case IDC_MMC64_ENABLE:
                    enable_mmc64_controls(hwnd);
                    break;
                case IDOK:
                    end_mmc64_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_mmc64_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_mmc64_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_MMC64_SETTINGS_DIALOG, hwnd, dialog_proc);
}
