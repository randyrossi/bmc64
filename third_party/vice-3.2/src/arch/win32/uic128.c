/*
 * uic128.c - Implementation of the C128 settings dialog box.
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
#include <string.h>
#include <tchar.h>
#include <windows.h>

#include "intl.h"
#include "lib.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uic128.h"
#include "uilib.h"
#include "winmain.h"

static const int ui_machine[] = {
    IDS_INTERNATIONAL,
    IDS_FINNISH,
    IDS_FRENCH,
    IDS_GERMAN,
    IDS_ITALIAN,
    IDS_NORWEGIAN,
    IDS_SWEDISH,
    IDS_SWISS,
    0
};

static void enable_machine_controls(HWND hwnd)
{
}

static uilib_localize_dialog_param machine_dialog_trans[] = {
    { IDC_MACHINE_TYPE, IDS_MACHINE_TYPE, 0 },
    { 0, 0, 0 }
};

static void init_machine_dialog(HWND hwnd)
{
    HWND machine_hwnd;
    int res_value;
    int res_value_loop;
    int xpos;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, machine_dialog_trans);

    /* adjust the size of the  machine type element */
    uilib_adjust_element_width(hwnd, IDC_MACHINE_TYPE);

    /* get the max x of the machine type element */
    uilib_get_element_max_x(hwnd, IDC_MACHINE_TYPE, &xpos);

    /* move the machine type indicator element */
    uilib_move_element(hwnd, IDC_C128_MACHINE_TYPE, xpos + 10);

    resources_get_int("MachineType", &res_value);
    machine_hwnd = GetDlgItem(hwnd, IDC_C128_MACHINE_TYPE);
    for (res_value_loop = 0; ui_machine[res_value_loop] != 0; res_value_loop++) {
        SendMessage(machine_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(ui_machine[res_value_loop]));
    }
    SendMessage(machine_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    enable_machine_controls(hwnd);
}

static uilib_localize_dialog_param functionrom_dialog_trans[] = {
    { IDC_C128_FUNCTIONROM_INTERNAL_TYPE_LABEL, IDS_INT_FUNCTION_ROM_TYPE, 0 },
    { IDC_INTERNAL_FUNCTION_ROM, IDS_INT_FUNCTION_ROM, 0 },
    { IDC_FILE_NAME_1, IDS_FILE_NAME, 0 },
    { IDC_FILE_NAME_2, IDS_FILE_NAME, 0 },
    { IDC_C128_FUNCTIONROM_INTERNAL_BROWSE, IDS_BROWSE, 0 },
    { IDC_C128_FUNCTIONROM_EXTERNAL_TYPE_LABEL, IDS_EXT_FUNCTION_ROM_TYPE, 0 },
    { IDC_C128_FUNCTIONROM_EXTERNAL_BROWSE, IDS_BROWSE, 0 },
    { IDC_C128_FUNCTIONROM_INTERNAL_RTC_SAVE, IDS_C128_FUNCTIONROM_INTERNAL_RTC_SAVE, 0 },
    { IDC_C128_FUNCTIONROM_EXTERNAL_RTC_SAVE, IDS_C128_FUNCTIONROM_EXTERNAL_RTC_SAVE, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group enable_group[] = {
    { IDC_C128_FUNCTIONROM_INTERNAL_TYPE_LABEL, 0 },
    { IDC_C128_FUNCTIONROM_EXTERNAL_TYPE_LABEL, 0 },
    { IDC_C128_FUNCTIONROM_INTERNAL_RTC_SAVE, 1 },
    { IDC_C128_FUNCTIONROM_EXTERNAL_RTC_SAVE, 1 },
    { 0, 0 }
};

static uilib_dialog_group file_name_group[] = {
    { IDC_FILE_NAME_1, 0 },
    { IDC_FILE_NAME_2, 0 },
    { 0, 0 }
};

static uilib_dialog_group file_name_indicator_group[] = {
    { IDC_C128_FUNCTIONROM_INTERNAL_NAME, 0 },
    { IDC_C128_FUNCTIONROM_EXTERNAL_NAME, 0 },
    { 0, 0 }
};

static void init_functionrom_dialog(HWND hwnd)
{
    int res_value;
    const char *romfile;
    TCHAR *st_romfile;
    int xstart;
    int xpos;
    int size;
    HWND temp_hwnd;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, functionrom_dialog_trans);

    /* adjust the size of the enable group elements */
    uilib_adjust_group_width(hwnd, enable_group);

    /* adjust the size of the file name group elements */
    uilib_adjust_group_width(hwnd, file_name_group);

    /* get the max x of the file name indicator group */
    uilib_get_group_max_x(hwnd, file_name_indicator_group, &xstart);

    /* get the size of the file name indicator group elements */
    uilib_get_group_width(hwnd, file_name_indicator_group, &size);

    /* get the max x of the file name group */
    uilib_get_group_max_x(hwnd, file_name_group, &xpos);

    /* move the file name indicator group to the correct position */
    uilib_move_group(hwnd, file_name_indicator_group, xpos + 10);

    /* get the max x of the file name indicator group */
    uilib_get_group_max_x(hwnd, file_name_indicator_group, &xpos);

    /* set the size of the file name indicator group */
    uilib_set_element_width(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_NAME, size - (xpos - xstart));
    uilib_set_element_width(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_NAME, size - (xpos - xstart));

    /* get the size of the internal function rom type label */
    uilib_get_element_max_x(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_TYPE_LABEL, &xpos);

    /* move the internal function rom type box */
    uilib_move_element(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_TYPE, xpos + 10);

    /* move the external function rom type box */
    uilib_move_element(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_TYPE, xpos + 10);

    resources_get_string("InternalFunctionName", &romfile);
    st_romfile = system_mbstowcs_alloc(romfile);
    SetDlgItemText(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_NAME, st_romfile != NULL ? st_romfile : TEXT(""));
    system_mbstowcs_free(st_romfile);

    resources_get_string("ExternalFunctionName", &romfile);
    st_romfile = system_mbstowcs_alloc(romfile);
    SetDlgItemText(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_NAME, st_romfile != NULL ? st_romfile : TEXT(""));
    system_mbstowcs_free(st_romfile);

    temp_hwnd = GetDlgItem(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_TYPE);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_NONE));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("ROM"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RAM"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RAM+RTC"));
    resources_get_int("InternalFunctionROM", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_TYPE);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(IDS_NONE));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("ROM"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RAM"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RAM+RTC"));
    resources_get_int("ExternalFunctionROM", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("InternalFunctionROMRTCSave", &res_value);
    CheckDlgButton(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_RTC_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);
    resources_get_int("ExternalFunctionROMRTCSave", &res_value);
    CheckDlgButton(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_RTC_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);
}

static uilib_localize_dialog_param rambanks_dialog_trans[] = {
    { IDC_C128_FULL_BANKS, IDS_C128_FULL_BANKS, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group rambanks_group[] = {
    { IDC_C128_FULL_BANKS, 1 },
    { 0, 0 }
};

static void init_rambanks_dialog(HWND hwnd)
{
    int res_value;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, rambanks_dialog_trans);

    /* adjust the size of the elements in the datasette_sub_group */
    uilib_adjust_group_width(hwnd, rambanks_group);

    resources_get_int("C128FullBanks", &res_value);
    CheckDlgButton(hwnd, IDC_C128_FULL_BANKS, res_value ? BST_CHECKED : BST_UNCHECKED);
}

static void end_machine_dialog(HWND hwnd)
{
    resources_set_int("MachineType", (int)SendMessage(GetDlgItem(hwnd, IDC_C128_MACHINE_TYPE), CB_GETCURSEL, 0, 0));
}

static INT_PTR CALLBACK machine_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lparam)->code) {
                case PSN_KILLACTIVE:
                    end_machine_dialog(hwnd);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_machine_dialog(hwnd);
            return TRUE;
    }

    return FALSE;
}

static void end_functionrom_dialog(HWND hwnd)
{
    char name[MAX_PATH];
    TCHAR st_name[MAX_PATH];

    resources_set_int("InternalFunctionROM", SendMessage(GetDlgItem(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_TYPE), CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_NAME, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("InternalFunctionName", name);

    resources_set_int("ExternalFunctionROM", SendMessage(GetDlgItem(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_TYPE), CB_GETCURSEL, 0, 0));

    GetDlgItemText(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_NAME, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("ExternalFunctionName", name);

    resources_set_int("InternalFunctionROMRTCSave", (IsDlgButtonChecked(hwnd, IDC_C128_FUNCTIONROM_INTERNAL_RTC_SAVE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("ExternalFunctionROMRTCSave", (IsDlgButtonChecked(hwnd, IDC_C128_FUNCTIONROM_EXTERNAL_RTC_SAVE) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK functionrom_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_C128_FUNCTIONROM_INTERNAL_BROWSE:
                    uilib_select_browse(hwnd, intl_translate_tcs(IDS_SELECT_INT_FUNCTION_ROM), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_LOAD, IDC_C128_FUNCTIONROM_INTERNAL_NAME);
                    break;
                case IDC_C128_FUNCTIONROM_EXTERNAL_BROWSE:
                    uilib_select_browse(hwnd, intl_translate_tcs(IDS_SELECT_EXT_FUNCTION_ROM), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_LOAD, IDC_C128_FUNCTIONROM_EXTERNAL_NAME);
                    break;
            }
            return FALSE;
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lparam)->code) {
                case PSN_KILLACTIVE:
                    end_functionrom_dialog(hwnd);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_functionrom_dialog(hwnd);
            return TRUE;
    }

    return FALSE;
}

static void end_rambanks_dialog(HWND hwnd)
{
    resources_set_int("C128FullBanks", (IsDlgButtonChecked(hwnd, IDC_C128_FULL_BANKS) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK rambanks_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_COMMAND:
            return FALSE;
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lparam)->code) {
                case PSN_KILLACTIVE:
                    end_rambanks_dialog(hwnd);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_rambanks_dialog(hwnd);
            return TRUE;
    }

    return FALSE;
}

void ui_c128_dialog(HWND hwnd)
{
    PROPSHEETPAGE psp[3];
    PROPSHEETHEADER psh;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[0].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_C128_MACHINE_SETTINGS_DIALOG);
    psp[0].pszIcon = NULL;
#else
    psp[0].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_C128_MACHINE_SETTINGS_DIALOG);
    psp[0].u2.pszIcon = NULL;
#endif
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[1].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_C128_FUNCTIONROM_SETTINGS_DIALOG);
    psp[1].pszIcon = NULL;
#else
    psp[1].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_C128_FUNCTIONROM_SETTINGS_DIALOG);
    psp[1].u2.pszIcon = NULL;
#endif
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[2].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_C128_RAM_BANKS_DIALOG);
    psp[2].pszIcon = NULL;
#else
    psp[2].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_C128_RAM_BANKS_DIALOG);
    psp[2].u2.pszIcon = NULL;
#endif
    psp[2].lParam = 0;
    psp[2].pfnCallback = NULL;

    psp[0].pfnDlgProc = machine_dialog_proc;
    psp[0].pszTitle = intl_translate_tcs(IDS_MACHINE_TYPE);
    psp[1].pfnDlgProc = functionrom_dialog_proc;
    psp[1].pszTitle = intl_translate_tcs(IDS_FUNCTION_ROM);
    psp[2].pfnDlgProc = rambanks_dialog_proc;
    psp[2].pszTitle = intl_translate_tcs(IDS_RAM_BANKS);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = winmain_instance;
    psh.pszCaption = intl_translate_tcs(IDS_C128_SETTINGS);
    psh.nPages = 3;
#ifdef _ANONYMOUS_UNION
    psh.pszIcon = NULL;
    psh.nStartPage = 0;
    psh.ppsp = psp;
#else
    psh.DUMMYUNIONNAME.pszIcon = NULL;
    psh.u2.nStartPage = 0;
    psh.u3.ppsp = psp;
#endif
    psh.pfnCallback = NULL;

    PropertySheet(&psh);
}
