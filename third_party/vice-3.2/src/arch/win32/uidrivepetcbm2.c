/*
 * uidrivepetcbm2.c - Implementation of the drive settings dialog box.
 *
 * Written by
 *  Tibor Biczo <viceteam@t-online.de>
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
#include <prsht.h>

#include "drive.h"
#include "intl.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uidrivepetcbm2.h"
#include "uilib.h"
#include "winlong.h"
#include "winmain.h"

static uilib_localize_dialog_param drive_dialog_trans[] = {
    { IDC_DRIVE_TYPE, IDS_DRIVE_TYPE, 0 },
    { IDC_SELECT_DRIVE_TYPE_NONE, IDS_SELECTNONE, 0 },
    { IDC_40_TRACK_HANDLING, IDS_40_TRACK_HANDLING, 0 },
    { IDC_SELECT_DRIVE_EXTEND_NEVER, IDS_SELECT_DRIVE_EXTEND_NEVER, 0 },
    { IDC_SELECT_DRIVE_EXTEND_ASK, IDS_SELECT_DRIVE_EXTEND_ASK, 0 },
    { IDC_SELECT_DRIVE_EXTEND_ACCESS, IDS_SELECT_DRIVE_EXTEND_ACCESS, 0 },
    { IDC_DRIVE_RPM_GROUP, IDS_DRIVE_RPM_GROUP, 0 },
    { IDC_DRIVE_RPM, IDS_DRIVE_RPM, 0 },
    { IDC_DRIVE_WOBBLE, IDS_DRIVE_WOBBLE, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param parent_dialog_trans[] = {
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group drive_main_group[] = {
    { IDC_DRIVE_TYPE, 1 },
    { IDC_SELECT_DRIVE_TYPE_NONE, 1 },
    { IDC_40_TRACK_HANDLING, 1 },
    { IDC_SELECT_DRIVE_EXTEND_NEVER, 1 },
    { IDC_SELECT_DRIVE_EXTEND_ASK, 1 },
    { IDC_SELECT_DRIVE_EXTEND_ACCESS, 1 },
    { 0, 0 }
};

static uilib_dialog_group drive_left_group[] = {
    { IDC_DRIVE_TYPE, 0 },
    { IDC_SELECT_DRIVE_TYPE_2031, 0 },
    { IDC_SELECT_DRIVE_TYPE_2040, 0 },
    { IDC_SELECT_DRIVE_TYPE_3040, 0 },
    { IDC_SELECT_DRIVE_TYPE_4040, 0 },
    { IDC_SELECT_DRIVE_TYPE_1001, 0 },
    { IDC_SELECT_DRIVE_TYPE_8050, 0 },
    { IDC_SELECT_DRIVE_TYPE_8250, 0 },
    { IDC_SELECT_DRIVE_TYPE_NONE, 0 },
    { 0, 0 }
};

static uilib_dialog_group drive_middle_group[] = {
    { IDC_40_TRACK_HANDLING, 0 },
    { IDC_SELECT_DRIVE_EXTEND_NEVER, 0 },
    { IDC_SELECT_DRIVE_EXTEND_ASK, 0 },
    { IDC_SELECT_DRIVE_EXTEND_ACCESS, 0 },
    { IDC_DRIVE_RPM_VALUE, 0 },
    { IDC_DRIVE_WOBBLE_VALUE, 0 },
    { 0, 0 }
};

static uilib_dialog_group drive_middle_move_group[] = {
    { IDC_SELECT_DRIVE_EXTEND_NEVER, 0 },
    { IDC_SELECT_DRIVE_EXTEND_ASK, 0 },
    { IDC_SELECT_DRIVE_EXTEND_ACCESS, 0 },
    { 0, 0 }
};

static uilib_dialog_group drive_rpm_right_group[] = {
    { IDC_DRIVE_RPM_VALUE, 0 },
    { IDC_DRIVE_WOBBLE_VALUE, 0 },
    { 0, 0 }
};

static uilib_dialog_group drive_rpm_left_group[] = {
    { IDC_DRIVE_RPM, 0 },
    { IDC_DRIVE_WOBBLE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static generic_trans_table_t generic_items[] = {
    { IDC_SELECT_DRIVE_TYPE_2031, TEXT("2031") },
    { IDC_SELECT_DRIVE_TYPE_2040, TEXT("2040") },
    { IDC_SELECT_DRIVE_TYPE_3040, TEXT("3040") },
    { IDC_SELECT_DRIVE_TYPE_4040, TEXT("4040") },
    { IDC_SELECT_DRIVE_TYPE_1001, TEXT("1001") },
    { IDC_SELECT_DRIVE_TYPE_8050, TEXT("8050") },
    { IDC_SELECT_DRIVE_TYPE_8250, TEXT("8250") },
    { 0, NULL }
};

static void enable_controls_for_drive_settings(HWND hwnd, int type)
{
    int drive_type = 0;
    int xpos;
    int xstart;
    int n;
    HWND parent_hwnd;
    HWND element;

    parent_hwnd = GetParent(hwnd);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, drive_dialog_trans);

    /* translate the parent window items */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    /* translate the generic items */
    for (n = 0; generic_items[n].text != NULL; n++) {
        element = GetDlgItem(hwnd, generic_items[n].idm);
        SetWindowText(element, generic_items[n].text);
    }

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, drive_main_group);

    /* get the max x of the elements in the left group */
    uilib_get_group_max_x(hwnd, drive_left_group, &xpos);

    /* get the min x of the none element of the left group */
    uilib_get_element_min_x(hwnd, IDC_SELECT_DRIVE_TYPE_NONE, &xstart);

    /* resize and move the left group element to the correct position */
    uilib_move_and_set_element_width(hwnd, IDC_DRIVE_TYPE, xstart - 10, xpos - xstart + 20);

    /* get the max x of the left group element */
    uilib_get_element_max_x(hwnd, IDC_DRIVE_TYPE, &xpos);
    
    /* move the middle group elements to the correct position */
    uilib_move_group(hwnd, drive_middle_move_group, xpos + 20);
    uilib_move_group(hwnd, drive_rpm_left_group, xpos + 20);
    uilib_move_element(hwnd, IDC_40_TRACK_HANDLING, xpos + 10);
    uilib_move_element(hwnd, IDS_DRIVE_RPM_GROUP, xpos + 10);

    xstart = xpos + 20;

    /* get the max x of the rpm left group */
    uilib_get_group_max_x(hwnd, drive_rpm_left_group, &xpos);

    /* move the right rpm group elements to the correct position */
    uilib_move_group(hwnd, drive_rpm_right_group, xpos + 10);

    /* get the max x of the middle group */
    uilib_get_group_max_x(hwnd, drive_middle_group, &xpos);
    
    /* resize and move the middle group boxes to the correct position */
    uilib_move_and_set_element_width(hwnd, IDC_40_TRACK_HANDLING, xstart - 10, xpos - xstart + 20);
    uilib_move_and_set_element_width(hwnd, IDC_DRIVE_RPM_GROUP, xstart - 10, xpos - xstart + 20);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(parent_hwnd, move_buttons_group, 0);

    switch (type) {
        case IDC_SELECT_DRIVE_TYPE_2031:
            drive_type = DRIVE_TYPE_2031;
            break;
        case IDC_SELECT_DRIVE_TYPE_2040:
            drive_type = DRIVE_TYPE_2040;
            break;
        case IDC_SELECT_DRIVE_TYPE_3040:
            drive_type = DRIVE_TYPE_3040;
            break;
        case IDC_SELECT_DRIVE_TYPE_4040:
            drive_type = DRIVE_TYPE_4040;
            break;
        case IDC_SELECT_DRIVE_TYPE_1001:
            drive_type = DRIVE_TYPE_1001;
            break;
        case IDC_SELECT_DRIVE_TYPE_8050:
            drive_type = DRIVE_TYPE_8050;
            break;
        case IDC_SELECT_DRIVE_TYPE_8250:
            drive_type = DRIVE_TYPE_8250;
            break;
    }

    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_EXTEND_NEVER), drive_check_extend_policy(drive_type));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_EXTEND_ASK), drive_check_extend_policy(drive_type));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_EXTEND_ACCESS), drive_check_extend_policy(drive_type));
}

static int dialog_drive_type[4];
static int dialog_drive_extend[4];

static void init_dialog(HWND hwnd, int num)
{
    int drive_type, drive_extend_image_policy, n = 0;

    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_TYPE_2031), drive_check_type(DRIVE_TYPE_2031, num - 8));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_TYPE_2040), drive_check_type(DRIVE_TYPE_2040, num - 8));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_TYPE_3040), drive_check_type(DRIVE_TYPE_3040, num - 8));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_TYPE_4040), drive_check_type(DRIVE_TYPE_4040, num - 8));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_TYPE_1001), drive_check_type(DRIVE_TYPE_1001, num - 8));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_TYPE_8050), drive_check_type(DRIVE_TYPE_8050, num - 8));
    EnableWindow(GetDlgItem(hwnd, IDC_SELECT_DRIVE_TYPE_8250), drive_check_type(DRIVE_TYPE_8250, num - 8));

    resources_get_int_sprintf("Drive%dType", &drive_type, num);
    resources_get_int_sprintf("Drive%dExtendImagePolicy", &drive_extend_image_policy, num);

    dialog_drive_type[num - 8] = drive_type;
    switch (drive_type) {
        case DRIVE_TYPE_NONE:
            n = IDC_SELECT_DRIVE_TYPE_NONE;
            break;
        case DRIVE_TYPE_2031:
            n = IDC_SELECT_DRIVE_TYPE_2031;
            break;
        case DRIVE_TYPE_2040:
            n = IDC_SELECT_DRIVE_TYPE_2040;
            break;
        case DRIVE_TYPE_3040:
            n = IDC_SELECT_DRIVE_TYPE_3040;
            break;
        case DRIVE_TYPE_4040:
            n = IDC_SELECT_DRIVE_TYPE_4040;
            break;
        case DRIVE_TYPE_1001:
            n = IDC_SELECT_DRIVE_TYPE_1001;
            break;
        case DRIVE_TYPE_8050:
            n = IDC_SELECT_DRIVE_TYPE_8050;
            break;
        case DRIVE_TYPE_8250:
            n = IDC_SELECT_DRIVE_TYPE_8250;
            break;
    }

    CheckRadioButton(hwnd, IDC_SELECT_DRIVE_TYPE_2031, IDC_SELECT_DRIVE_TYPE_NONE, n);

    enable_controls_for_drive_settings(hwnd, n);

    dialog_drive_extend[num - 8] = drive_extend_image_policy;
    switch (drive_extend_image_policy) {
        case DRIVE_EXTEND_NEVER:
            n = IDC_SELECT_DRIVE_EXTEND_NEVER;
            break;
        case DRIVE_EXTEND_ASK:
            n = IDC_SELECT_DRIVE_EXTEND_ASK;
            break;
        case DRIVE_EXTEND_ACCESS:
            n = IDC_SELECT_DRIVE_EXTEND_ACCESS;
            break;
    }

    CheckRadioButton(hwnd, IDC_SELECT_DRIVE_EXTEND_NEVER, IDC_SELECT_DRIVE_EXTEND_ACCESS, n);

    resources_get_int_sprintf("Drive%dRPM", &n, num);
    SetDlgItemInt(hwnd, IDC_DRIVE_RPM_VALUE, n, TRUE);

    resources_get_int_sprintf("Drive%dWobble", &n, num);
    SetDlgItemInt(hwnd, IDC_DRIVE_WOBBLE_VALUE, n, TRUE);
}

static BOOL CALLBACK dialog_proc(int num, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;
    int rpm;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR *)lparam)->code == (UINT)PSN_APPLY) {
                resources_set_int_sprintf("Drive%dType", dialog_drive_type[num - 8], num);
                resources_set_int_sprintf("Drive%dExtendImagePolicy", dialog_drive_extend[num - 8], num);

                rpm = GetDlgItemInt(hwnd, IDC_DRIVE_RPM_VALUE, NULL, TRUE);
                resources_set_int_sprintf("Drive%dRPM", rpm, num);

                rpm = GetDlgItemInt(hwnd, IDC_DRIVE_WOBBLE_VALUE, NULL, TRUE);
                resources_set_int_sprintf("Drive%dWobble", rpm, num);

                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_dialog(hwnd, num);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_SELECT_DRIVE_TYPE_NONE:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_NONE;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_TYPE_2031:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_2031;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_TYPE_2040:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_2040;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_TYPE_3040:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_3040;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_TYPE_4040:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_4040;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_TYPE_1001:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_1001;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_TYPE_8050:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_8050;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_TYPE_8250:
                    dialog_drive_type[num - 8] = DRIVE_TYPE_8250;
                    enable_controls_for_drive_settings(hwnd, LOWORD(wparam));
                    break;
                case IDC_SELECT_DRIVE_EXTEND_NEVER:
                    dialog_drive_extend[num - 8] = DRIVE_EXTEND_NEVER;
                    break;
                case IDC_SELECT_DRIVE_EXTEND_ASK:
                    dialog_drive_extend[num - 8] = DRIVE_EXTEND_ASK;
                    break;
                case IDC_SELECT_DRIVE_EXTEND_ACCESS:
                    dialog_drive_extend[num - 8] = DRIVE_EXTEND_ACCESS;
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

_CALLBACK(8)
_CALLBACK(9)

void uidrivepetcbm2_settings_dialog(HWND hwnd)
{
    PROPSHEETPAGE psp[2];
    PROPSHEETHEADER psh;
    int i;

    for (i = 0; i < 2; i++) {
        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
        psp[i].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
        psp[i].pszTemplate = MAKEINTRESOURCE(IDD_DRIVE_SETTINGS_DIALOG_PETCBM2);
        psp[i].pszIcon = NULL;
#else
        psp[i].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_DRIVE_SETTINGS_DIALOG_PETCBM2);
        psp[i].u2.pszIcon = NULL;
#endif
        psp[i].lParam = 0;
        psp[i].pfnCallback = NULL;
    }

    psp[0].pfnDlgProc = callback_8;
    psp[0].pszTitle = intl_translate_tcs(IDS_DRIVE_8);
    psp[1].pfnDlgProc = callback_9;
    psp[1].pszTitle = intl_translate_tcs(IDS_DRIVE_9);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = winmain_instance;
    psh.pszCaption = intl_translate_tcs(IDS_DRIVE_SETTINGS);
    psh.nPages = 2;
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
