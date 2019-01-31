/*
 * uic64dtv.c - Implementation of the C64DTV specific settings dialog box.
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

#include "c64dtvflash.h"
#include "intl.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "uilib.h"
#include "uic64dtv.h"
#include "winmain.h"

static void enable_c64dtv_controls(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDC_DTV_REVISION), 1);
}

static void enable_c64dtv_attach_flash_controls(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDC_C64DTV_ROM_WRITE_ENABLE), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_C64DTV_ROM_IMAGE_BROWSE), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_C64DTV_ROM_IMAGE_FILE), 1);
}

static void enable_c64dtv_create_flash_controls(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDC_C64DTV_ROM_COPY_C64), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_C64DTV_ROM_IMAGE_BROWSE), 1);
    EnableWindow(GetDlgItem(hwnd, IDC_C64DTV_ROM_IMAGE_FILE), 1);
}

static uilib_localize_dialog_param c64dtv_dialog_trans[] = {
    { 0, IDS_C64DTV_SETTINGS_CAPTION, -1 },
    { IDC_DTV_REVISION_LABEL, IDS_DTV_REVISION_LABEL, 0 },
    { IDC_ENABLE_HUMMER_ADC, IDS_ENABLE_HUMMER_ADC, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group c64dtv_settings_main_group[] = {
    { IDC_DTV_REVISION_LABEL, 0 },
    { IDC_ENABLE_HUMMER_ADC, 1 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_c64dtv_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, c64dtv_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, c64dtv_settings_main_group);

    /* get the max x of the main group */
    uilib_get_group_max_x(hwnd, c64dtv_settings_main_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 10, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_DTV_REVISION);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("DTV2"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("DTV3"));
    resources_get_int("DtvRevision", &res_value);
    res_value -= 2;
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("HummerADC", &res_value);
    CheckDlgButton(hwnd, IDC_ENABLE_HUMMER_ADC, res_value ? BST_CHECKED : BST_UNCHECKED);

    enable_c64dtv_controls(hwnd);
}

static uilib_localize_dialog_param c64dtv_attach_flash_dialog_trans[] = {
    { 0, IDS_ATTACH_FLASH_CAPTION, -1 },
    { IDC_DTV_ROM, IDS_DTV_ROM, 0 },
    { IDC_C64DTV_ROM_IMAGE_FILE_LABEL, IDS_C64DTV_ROM_IMAGE_FILE_LABEL, 0 },
    { IDC_C64DTV_ROM_IMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDC_C64DTV_ROM_WRITE_ENABLE, IDS_C64DTV_ROM_WRITE_ENABLE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group c64dtv_attach_flash_main_group[] = {
    { IDC_C64DTV_ROM_IMAGE_FILE_LABEL, 0 },
    { IDC_C64DTV_ROM_WRITE_ENABLE, 1 },
    { 0, 0 }
};

static uilib_dialog_group c64dtv_attach_flash_right_group[] = {
    { IDC_C64DTV_ROM_IMAGE_BROWSE, 0 },
    { IDC_C64DTV_ROM_IMAGE_FILE, 0 },
    { IDC_C64DTV_ROM_WRITE_ENABLE, 0 },
    { 0, 0 }
};

static void init_c64dtv_attach_flash_dialog(HWND hwnd)
{
    int res_value;
    const char *c64dtv_bios_file;
    TCHAR *st_c64dtv_bios_file;
    int xpos;
    int xstart;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, c64dtv_attach_flash_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, c64dtv_attach_flash_main_group);

    /* get the max x of the image file name element */
    uilib_get_element_max_x(hwnd, IDC_C64DTV_ROM_IMAGE_FILE_LABEL, &xpos);

    /* move the browse button to the correct location */
    uilib_move_element(hwnd, IDC_C64DTV_ROM_IMAGE_BROWSE, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, c64dtv_attach_flash_right_group, &xpos);

    /* get the min x of the file name element */
    uilib_get_element_min_x(hwnd, IDC_C64DTV_ROM_IMAGE_FILE_LABEL, &xstart);

    /* move and resize the group element to surround all elements */
    uilib_move_and_set_element_width(hwnd, IDC_DTV_ROM, xstart - 10 , xpos - xstart + 20);

    /* get the max x of the group element */
    uilib_get_element_max_x(hwnd, IDC_DTV_ROM, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 10, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_string("c64dtvromfilename", &c64dtv_bios_file);
    st_c64dtv_bios_file = system_mbstowcs_alloc(c64dtv_bios_file);
    SetDlgItemText(hwnd, IDC_C64DTV_ROM_IMAGE_FILE, c64dtv_bios_file != NULL ? st_c64dtv_bios_file : TEXT(""));
    system_mbstowcs_free(st_c64dtv_bios_file);

    resources_get_int("c64dtvromrw", &res_value);
    CheckDlgButton(hwnd, IDC_C64DTV_ROM_WRITE_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    enable_c64dtv_attach_flash_controls(hwnd);
}

static uilib_localize_dialog_param c64dtv_create_flash_dialog_trans[] = {
    { 0, IDS_CREATE_FLASH_CAPTION, -1 },
    { IDC_DTV_ROM_CREATION, IDS_DTV_ROM_CREATION, 0 },
    { IDC_C64DTV_ROM_IMAGE_FILE_LABEL, IDS_C64DTV_ROM_IMAGE_FILE_LABEL, 0 },
    { IDC_C64DTV_ROM_IMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDC_C64DTV_ROM_COPY_C64, IDS_C64DTV_ROM_COPY_C64, 0 },
    { IDOK, IDS_CREATE, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group c64dtv_create_flash_main_group[] = {
    { IDC_C64DTV_ROM_IMAGE_FILE_LABEL, 0 },
    { IDC_C64DTV_ROM_COPY_C64, 1 },
    { IDOK, 1 },
    { 0, 0 }
};

static uilib_dialog_group c64dtv_create_flash_right_group[] = {
    { IDC_C64DTV_ROM_IMAGE_BROWSE, 0 },
    { IDC_C64DTV_ROM_COPY_C64, 0 },
    { IDC_C64DTV_ROM_IMAGE_FILE, 0 },
    { 0, 0 }
};

static void init_c64dtv_create_flash_dialog(HWND hwnd)
{
    int xpos;
    int xstart;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, c64dtv_create_flash_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, c64dtv_create_flash_main_group);

    /* get the max x of the image file name element */
    uilib_get_element_max_x(hwnd, IDC_C64DTV_ROM_IMAGE_FILE_LABEL, &xpos);

    /* move the browse button to the correct location */
    uilib_move_element(hwnd, IDC_C64DTV_ROM_IMAGE_BROWSE, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, c64dtv_create_flash_right_group, &xpos);

    /* get the min x of the file name element */
    uilib_get_element_min_x(hwnd, IDC_C64DTV_ROM_IMAGE_FILE_LABEL, &xstart);

    /* move and resize the group element to surround all elements */
    uilib_move_and_set_element_width(hwnd, IDC_DTV_ROM_CREATION, xstart - 10 , xpos - xstart + 20);

    /* get the max x of the group element */
    uilib_get_element_max_x(hwnd, IDC_DTV_ROM_CREATION, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 10, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    SetDlgItemText(hwnd, IDC_C64DTV_ROM_IMAGE_FILE, TEXT(""));

    CheckDlgButton(hwnd, IDC_C64DTV_ROM_COPY_C64, BST_CHECKED);

    enable_c64dtv_create_flash_controls(hwnd);
}

static void end_c64dtv_dialog(HWND hwnd)
{
    resources_set_int("DtvRevision",(int)SendMessage(GetDlgItem(hwnd, IDC_DTV_REVISION), CB_GETCURSEL, 0, 0) + 2);

    resources_set_int("HummerADC", (IsDlgButtonChecked(hwnd, IDC_ENABLE_HUMMER_ADC) == BST_CHECKED ? 1 : 0 ));
}

static void end_c64dtv_attach_flash_dialog(HWND hwnd)
{
    TCHAR st_name[MAX_PATH];
    char name[MAX_PATH];

    GetDlgItemText(hwnd, IDC_C64DTV_ROM_IMAGE_FILE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("c64dtvromfilename", name);

    resources_set_int("c64dtvromrw", (IsDlgButtonChecked(hwnd, IDC_C64DTV_ROM_WRITE_ENABLE) == BST_CHECKED ? 1 : 0 ));
}

static void end_c64dtv_create_flash_dialog(HWND hwnd)
{
    TCHAR st_name[MAX_PATH];
    char name[MAX_PATH];

    GetDlgItemText(hwnd, IDC_C64DTV_ROM_IMAGE_FILE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    c64dtvflash_create_blank_image(name, (IsDlgButtonChecked(hwnd, IDC_C64DTV_ROM_COPY_C64) == BST_CHECKED ? 1 : 0 ));
}

static void browse_c64dtv_bios_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_C64DTV_ROM_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_C64DTV_ROM_IMAGE_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_c64dtv_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_c64dtv_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK dialog_attach_flash_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_C64DTV_ROM_IMAGE_BROWSE:
                    browse_c64dtv_bios_file(hwnd);
                    break;
                case IDOK:
                    end_c64dtv_attach_flash_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_c64dtv_attach_flash_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK dialog_create_flash_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_C64DTV_ROM_IMAGE_BROWSE:
                    browse_c64dtv_bios_file(hwnd);
                    break;
                case IDOK:
                    end_c64dtv_create_flash_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_c64dtv_create_flash_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_c64dtv_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_C64DTV_SETTINGS_DIALOG, hwnd, dialog_proc);
}

void ui_c64dtv_attach_flash_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_C64DTV_ATTACH_FLASH_IMAGE_DIALOG, hwnd, dialog_attach_flash_proc);
}

void ui_c64dtv_create_flash_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_C64DTV_CREATE_FLASH_IMAGE_DIALOG, hwnd, dialog_create_flash_proc);
}
