/*
 * uiide64.c - Implementation of the IDE64 settings dialog box.
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
#include <prsht.h>

#include "clockport.h"
#include "intl.h"
#include "lib.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "ui.h"
#include "uiide64.h"
#include "uilib.h"
#include "winlong.h"
#include "winmain.h"

static int ui_ide64_shortbus_digimax_base[] = {
    0xde40,
    0xde48,
    -1
};

#ifdef HAVE_RAWNET
static int ui_ide64_shortbus_etfe_base[] = {
    0xde00,
    0xde10,
    0xdf00,
    -1
};
#endif

static const int ui_ide64_version[] = {
    0,
    1,
    2,
    -1
};

static void enable_ide64_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_TOGGLE_IDE64_SIZEAUTODETECT) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_IDE64_CYLINDERS), !is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_IDE64_HEADS), !is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_IDE64_SECTORS), !is_enabled);
}

static void update_text(HWND hwnd)
{
    char *str;
    TCHAR *st;
    HWND ide64_hwnd;
    int cylinders_idx, heads_idx, sectors_idx, total;

    ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_CYLINDERS);
    cylinders_idx = (int)SendMessage(ide64_hwnd, CB_GETCURSEL, 0, 0);

    ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_HEADS);
    heads_idx = (int)SendMessage(ide64_hwnd, CB_GETCURSEL, 0, 0);

    ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_SECTORS);
    sectors_idx = (int)SendMessage(ide64_hwnd, CB_GETCURSEL, 0, 0);

    total = (cylinders_idx + 1) * (heads_idx + 1) * (sectors_idx + 1) / 2;

    str = lib_msprintf(translate_text(IDS_IDE64_TOTAL_SIZE), total);
    st = system_mbstowcs_alloc(str);
    SetDlgItemText(hwnd, IDC_IDE64_SIZE, st);
    system_mbstowcs_free(st);
    lib_free(str);
}

static uilib_localize_dialog_param ide64_shortbus_dialog[] = {
    { IDC_IDE64_SB_DIGIMAX, IDS_IDE64_SB_DIGIMAX, 0 },
    { IDC_IDE64_SB_DIGIMAX_ADDRESS_LABEL, IDS_IDE64_SB_DIGIMAX_ADDRESS, 0 },
#ifdef HAVE_RAWNET
    { IDC_IDE64_SB_ETFE, IDS_IDE64_SB_ETFE, 0 },
    { IDC_IDE64_SB_ETFE_ADDRESS_LABEL, IDS_IDE64_SB_ETFE_ADDRESS, 0 },
#endif
    { 0, 0, 0 }
};

static uilib_localize_dialog_param ide64_v4_dialog[] = {
    { IDC_IDE64_VERSION, IDS_IDE64_VERSION, 0 },
    { IDC_IDE64_USB_SERVER, IDS_IDE64_USB_SERVER, 0 },
    { IDC_IDE64_RTC_SAVE, IDS_IDE64_RTC_SAVE, 0 },
    { IDC_IDE64_USB_SERVER_BIND_LABEL, IDS_IDE64_USB_SERVER_BIND_LABEL, 0 },
    { IDC_IDE64_CLOCKPORT_DEVICE_LABEL, IDS_IDE64_CLOCKPORT_DEVICE_LABEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param ide64_dialog[] = {
    { IDC_IDE64_FILE_LABEL, IDS_IDE64_IMAGE, 0 },
    { IDC_IDE64_HDIMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDC_IDE64_GEOMETRY, IDS_IDE64_GEOMETRY, 0 },
    { IDC_TOGGLE_IDE64_SIZEAUTODETECT, IDS_IDE64_AUTODETECT, 0 },
    { IDC_IDE64_CYLINDERS_LABEL, IDS_IDE64_CYLINDERS, 0 },
    { IDC_IDE64_HEADS_LABEL, IDS_IDE64_HEADS, 0 },
    { IDC_IDE64_SECTORS_LABEL, IDS_IDE64_SECTORS, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param parent_dialog_trans[] = {
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group ide64_leftgroup[] = {
    { IDC_IDE64_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group ide64_rightgroup[] = {
    { IDC_IDE64_HDIMAGE_BROWSE, 0 },
    { 0, 0 }
};

static uilib_dialog_group ide64_sb_left_group[] = {
    { IDC_IDE64_SB_DIGIMAX_ADDRESS_LABEL, 0 },
#ifdef HAVE_RAWNET
    { IDC_IDE64_SB_ETFE_ADDRESS_LABEL, 0 },
#endif
    { 0, 0 }
};

static uilib_dialog_group ide64_sb_right_group[] = {
    { IDC_IDE64_SB_DIGIMAX_ADDRESS, 0 },
#ifdef HAVE_RAWNET
    { IDC_IDE64_SB_ETFE_ADDRESS, 0 },
#endif
    { 0, 0 }
};

static void init_ide64_shortbus_dialog(HWND hwnd)
{
    int res_value;
    HWND parent_hwnd;
    HWND temp_hwnd;
    int res_value_loop;
    int active_value;
    int xpos;

    parent_hwnd = GetParent(hwnd);

    /* translate the items */
    uilib_localize_dialog(hwnd, ide64_shortbus_dialog);

    /* translate the parent window items */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, ide64_sb_left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, ide64_sb_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, ide64_sb_right_group, xpos + 10);

    temp_hwnd = GetDlgItem(hwnd, IDC_IDE64_SB_DIGIMAX_ADDRESS);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("$DE40"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("$DE48"));
    resources_get_int("SBDIGIMAXBase", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_ide64_shortbus_digimax_base[res_value_loop] != -1; res_value_loop++) {
        if (ui_ide64_shortbus_digimax_base[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_int("SBDIGIMAX", &res_value);
    CheckDlgButton(hwnd, IDC_IDE64_SB_DIGIMAX, res_value ? BST_CHECKED : BST_UNCHECKED);

#ifdef HAVE_RAWNET
    temp_hwnd = GetDlgItem(hwnd, IDC_IDE64_SB_ETFE_ADDRESS);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("$DE00"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("$DE10"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("$DF00"));
    resources_get_int("SBETFEbase", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_ide64_shortbus_etfe_base[res_value_loop] != -1; res_value_loop++) {
        if (ui_ide64_shortbus_etfe_base[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_int("SBETFE", &res_value);
    CheckDlgButton(hwnd, IDC_IDE64_SB_ETFE, res_value ? BST_CHECKED : BST_UNCHECKED);
#endif
}

static int clockport_ids[CLOCKPORT_MAX_ENTRIES + 1];

static void init_ide64_v4_dialog(HWND hwnd)
{
    int res_value;
    int xtemp;
    const char *server_bind_address;
    TCHAR st[256];
    HWND parent_hwnd;
    HWND temp_hwnd;
    int res_value_loop;
    int active_value;
    int current_val = 0;
    TCHAR *st_clockport_device_name;

    parent_hwnd = GetParent(hwnd);

    /* translate the items */
    uilib_localize_dialog(hwnd, ide64_v4_dialog);

    /* get max x of label */
    uilib_get_element_max_x(hwnd, IDC_IDE64_USB_SERVER_BIND_LABEL, &xtemp);

    /* move the text input element */
    uilib_move_element(hwnd, IDC_ID64_USB_SERVER_BIND, xtemp + 10);

    /* resize rtc save element */
    uilib_adjust_element_width(hwnd, IDC_IDE64_RTC_SAVE);

    /* translate the parent window items */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    temp_hwnd = GetDlgItem(hwnd, IDC_IDE64_VERSION);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("V3"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("V4.1"));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("V4.2"));
    resources_get_int("IDE64version", &res_value);
    active_value = 0;
    for (res_value_loop = 0; ui_ide64_version[res_value_loop] != -1; res_value_loop++) {
        if (ui_ide64_version[res_value_loop] == res_value) {
            active_value = res_value_loop;
        }
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_int("IDE64USBServer", &res_value);
    CheckDlgButton(hwnd, IDC_IDE64_USB_SERVER, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("IDE64RTCSave", &res_value);
    CheckDlgButton(hwnd, IDC_IDE64_RTC_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_string("IDE64USBServerAddress", &server_bind_address);
    system_mbstowcs(st, server_bind_address, 256);
    SetDlgItemText(hwnd, IDC_ID64_USB_SERVER_BIND, st);

    resources_get_int("IDE64ClockPort", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_IDE64_CLOCKPORT_DEVICE);
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
}

static void init_ide64_dialog(HWND hwnd, int num)
{
    int res_value, index;
    TCHAR memb[20];
    HWND ide64_hwnd;
    int xsize, ysize;
    char tmp[256];
    const char *ide64file;
    TCHAR *st_ide64file;

    uilib_localize_dialog(hwnd, ide64_dialog);

    uilib_get_group_extent(hwnd, ide64_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, ide64_leftgroup);
    uilib_move_group(hwnd, ide64_rightgroup, xsize + 30);

    sprintf(tmp, "IDE64Image%d", num);
    resources_get_string(tmp, &ide64file);
    st_ide64file = system_mbstowcs_alloc(ide64file);
    SetDlgItemText(hwnd, IDC_IDE64_HDIMAGE_FILE, st_ide64file != NULL ? st_ide64file : TEXT(""));
    system_mbstowcs_free(st_ide64file);

    resources_get_int_sprintf("IDE64AutodetectSize%i", &res_value, num);
    CheckDlgButton(hwnd, IDC_TOGGLE_IDE64_SIZEAUTODETECT, res_value ? BST_CHECKED : BST_UNCHECKED);

    ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_CYLINDERS);
    for (index = 1; index <= 65535; index++) {
        _stprintf(memb, TEXT("%d"), index);
        SendMessage(ide64_hwnd, CB_ADDSTRING, 0, (LPARAM)memb);
    }
    resources_get_int_sprintf("IDE64Cylinders%i", &res_value, num);
    SendMessage(ide64_hwnd, CB_SETCURSEL, (WPARAM)(res_value - 1), 0);

    ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_HEADS);
    for (index = 1; index <= 16; index++) {
        _stprintf(memb, TEXT("%d"), index);
        SendMessage(ide64_hwnd, CB_ADDSTRING, 0, (LPARAM)memb);
    }
    resources_get_int_sprintf("IDE64Heads%i", &res_value, num);
    SendMessage(ide64_hwnd, CB_SETCURSEL, (WPARAM)(res_value - 1), 0);

    ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_SECTORS);
    for (index = 1; index <= 63; index++) {
        _stprintf(memb, TEXT("%d"), index);
        SendMessage(ide64_hwnd, CB_ADDSTRING, 0, (LPARAM)memb);
    }
    resources_get_int_sprintf("IDE64Sectors%i", &res_value, num);
    SendMessage(ide64_hwnd, CB_SETCURSEL, (WPARAM)(res_value - 1), 0);

    update_text(hwnd);
    enable_ide64_controls(hwnd);
}

static INT_PTR CALLBACK dialog_shortbus_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                resources_set_int("SBDIGIMAXBase", ui_ide64_shortbus_digimax_base[SendMessage(GetDlgItem(hwnd, IDC_IDE64_SB_DIGIMAX_ADDRESS), CB_GETCURSEL, 0, 0)]);
                resources_set_int("SBDIGIMAX", (IsDlgButtonChecked(hwnd, IDC_IDE64_SB_DIGIMAX) == BST_CHECKED ? 1 : 0));
#ifdef HAVE_RAWNET
                resources_set_int("SBETFEbase", ui_ide64_shortbus_digimax_base[SendMessage(GetDlgItem(hwnd, IDC_IDE64_SB_ETFE_ADDRESS), CB_GETCURSEL, 0, 0)]);
                resources_set_int("SBETFE", (IsDlgButtonChecked(hwnd, IDC_IDE64_SB_ETFE) == BST_CHECKED ? 1 : 0));
#endif
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_ide64_shortbus_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_IDE64_SB_DIGIMAX:
                case IDC_IDE64_SB_DIGIMAX_ADDRESS:
#ifdef HAVE_RAWNET
                case IDC_IDE64_SB_ETFE:
                case IDC_IDE64_SB_ETFE_ADDRESS:
#endif
                    break;
            }
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK dialog_v4_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    TCHAR st_text[MAX_PATH];
    char text[MAX_PATH];
    int type;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                resources_set_int("IDE64version", ui_ide64_version[SendMessage(GetDlgItem(hwnd, IDC_IDE64_VERSION), CB_GETCURSEL, 0, 0)]);
                resources_set_int("IDE64USBServer", (IsDlgButtonChecked(hwnd, IDC_IDE64_USB_SERVER) == BST_CHECKED ? 1 : 0));
                resources_set_int("IDE64RTCSave", (IsDlgButtonChecked(hwnd, IDC_IDE64_RTC_SAVE) == BST_CHECKED ? 1 : 0));
                GetDlgItemText(hwnd, IDC_ID64_USB_SERVER_BIND, st_text, MAX_PATH);
                system_wcstombs(text, st_text, MAX_PATH);
                resources_set_string("IDE64USBServerAddress", text);
                resources_set_int("IDE64ClockPort", clockport_ids[(int)SendMessage(GetDlgItem(hwnd, IDC_IDE64_CLOCKPORT_DEVICE), CB_GETCURSEL, 0, 0)]);
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_ide64_v4_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_IDE64_USB_SERVER:
                case IDC_IDE64_RTC_SAVE:
                case IDC_ID64_USB_SERVER_BIND:
                    break;
            }
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK dialog_proc(int num, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;
    char tmp[256];
    char s[MAX_PATH];
    TCHAR st[MAX_PATH];
    HWND ide64_hwnd;
    int res_value;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                sprintf(tmp, "IDE64Image%d", num);
                GetDlgItemText(hwnd, IDC_IDE64_HDIMAGE_FILE, st, MAX_PATH);
                system_wcstombs(s, st, MAX_PATH);
                resources_set_string(tmp, s);
                resources_set_int_sprintf("IDE64AutodetectSize%i", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_IDE64_SIZEAUTODETECT) == BST_CHECKED ? 1 : 0), num);
                ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_CYLINDERS);
                res_value = (int)SendMessage(ide64_hwnd, CB_GETCURSEL, 0, 0);
                resources_set_int_sprintf("IDE64Cylinders%i", res_value + 1, num);
                ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_HEADS);
                res_value = (int)SendMessage(ide64_hwnd, CB_GETCURSEL, 0, 0);
                resources_set_int_sprintf("IDE64Heads%i", res_value + 1, num);
                ide64_hwnd = GetDlgItem(hwnd, IDC_IDE64_SECTORS);
                res_value = (int)SendMessage(ide64_hwnd, CB_GETCURSEL, 0, 0);
                resources_set_int_sprintf("IDE64Sectors%i", res_value + 1, num);
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_ide64_dialog(hwnd, num);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_TOGGLE_IDE64_SIZEAUTODETECT:
                    enable_ide64_controls(hwnd);
                    break;
                case IDC_IDE64_CYLINDERS:
                case IDC_IDE64_HEADS:
                case IDC_IDE64_SECTORS:
                    update_text(hwnd);
                    break;
                case IDC_IDE64_HDIMAGE_BROWSE:
                    uilib_select_browse(hwnd, intl_translate_tcs(IDS_IDE64_SELECT_IMAGE), UILIB_FILTER_ALL | UILIB_FILTER_IDE64, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_IDE64_HDIMAGE_FILE);
                    break;
            }
            return TRUE;
    }
    return FALSE;
}

#define _CALLBACK(num)                                                                      \
static INT_PTR CALLBACK callback_##num(HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam) \
{                                                                                           \
    return dialog_proc(num, dialog, msg, wparam, lparam);                                   \
}

_CALLBACK(1)
_CALLBACK(2)
_CALLBACK(3)
_CALLBACK(4)

void uiide64_settings_dialog(HWND hwnd)
{
    PROPSHEETPAGE psp[6];
    PROPSHEETHEADER psh;
    int i;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[0].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_IDE64_SHORTBUS_SETTINGS_DIALOG);
    psp[0].pszIcon = NULL;
#else
    psp[0].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_IDE64_SHORTBUS_SETTINGS_DIALOG);
    psp[0].u2.pszIcon = NULL;
#endif
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[1].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_IDE64_V4_SETTINGS_DIALOG);
    psp[1].pszIcon = NULL;
#else
    psp[1].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_IDE64_V4_SETTINGS_DIALOG);
    psp[1].u2.pszIcon = NULL;
#endif
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    for (i = 2; i < 6; i++) {
        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
        psp[i].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
        psp[i].pszTemplate = MAKEINTRESOURCE(IDD_IDE64_SETTINGS_DIALOG);
        psp[i].pszIcon = NULL;
#else
        psp[i].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_IDE64_SETTINGS_DIALOG);
        psp[i].u2.pszIcon = NULL;
#endif
        psp[i].lParam = 0;
        psp[i].pfnCallback = NULL;
    }

    psp[0].pfnDlgProc = dialog_shortbus_proc;
    psp[0].pszTitle = intl_translate_tcs(IDS_IDE64_SHORTBUS_SETTINGS);
    psp[1].pfnDlgProc = dialog_v4_proc;
    psp[1].pszTitle = intl_translate_tcs(IDS_IDE64_V4_SETTINGS);
    psp[2].pfnDlgProc = callback_1;
    psp[2].pszTitle = intl_translate_tcs(IDS_IDE64_HD_IMAGE_1_SETTINGS);
    psp[3].pfnDlgProc = callback_2;
    psp[3].pszTitle = intl_translate_tcs(IDS_IDE64_HD_IMAGE_2_SETTINGS);
    psp[4].pfnDlgProc = callback_3;
    psp[4].pszTitle = intl_translate_tcs(IDS_IDE64_HD_IMAGE_3_SETTINGS);
    psp[5].pfnDlgProc = callback_4;
    psp[5].pszTitle = intl_translate_tcs(IDS_IDE64_HD_IMAGE_4_SETTINGS);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = winmain_instance;
    psh.pszCaption = intl_translate_tcs(IDS_IDE64_SETTINGS);
    psh.nPages = 6;
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

    system_psh_settings(&psh);
    PropertySheet(&psh);
}
