/*
 * uiethernet.c - Implementation of the ethernet settings dialog box.
 *
 * Written by
 *  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Greg King <greg.king5@verizon.net>
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

#ifdef HAVE_RAWNET

#include <windows.h>

#include "intl.h"
#include "lib.h"
#include "rawnet.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "uiethernet.h"
#include "uilib.h"
#include "util.h"
#include "winmain.h"

static BOOL get_ethernetname(int number, char **ppname, char **ppdescription)
{
    if (rawnet_enumadapter_open()) {
        char *pname = NULL;
        char *pdescription = NULL;

        while (number--) {
            if (!rawnet_enumadapter(&pname, &pdescription)) {
                break;
            }

            lib_free(pdescription);
            lib_free(pname);
        }

        if (rawnet_enumadapter(&pname, &pdescription)) {
            *ppname = pname;
            *ppdescription = pdescription;
            rawnet_enumadapter_close();
            return TRUE;
        }

        rawnet_enumadapter_close();
    }
    return FALSE;
}

static void show_interface_selection(HWND hwnd)
{
    TCHAR *st_name;
    TCHAR *st_description;
    char *pname = NULL;
    char *pdescription = NULL;
    int number = (int)SendMessage(GetDlgItem(hwnd, IDC_ETHERNET_SETTINGS_INTERFACE), CB_GETCURSEL, 0, 0);

    if (get_ethernetname(number, &pname, &pdescription)) {
        st_name = system_mbstowcs_alloc(pname);
        st_description = system_mbstowcs_alloc(pdescription);
        SetWindowText(GetDlgItem(hwnd, IDC_ETHERNET_SETTINGS_INTERFACE_NAME), st_name);
        SetWindowText(GetDlgItem(hwnd, IDC_ETHERNET_SETTINGS_INTERFACE_DESC), st_description);
        system_mbstowcs_free(st_description);
        system_mbstowcs_free(st_name);
        lib_free(pdescription);
        lib_free(pname);
    }
}

static uilib_localize_dialog_param ethernet_dialog[] = {
    { 0, IDS_ETHERNET_CAPTION, -1 },
    { IDC_ETHERNET_SETTINGS_INTERFACE_T, IDS_ETHERNET_INTERFACE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group ethernet_leftgroup[] = {
    { IDC_ETHERNET_SETTINGS_INTERFACE_T, 0 },
    { 0, 0 }
};

static uilib_dialog_group ethernet_rightgroup[] = {
    { IDC_ETHERNET_SETTINGS_INTERFACE, 0 },
    { 0, 0 }
};

static void init_ethernet_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int cnt;
    char *combined;
    TCHAR *st_combined;
    int ysize, xsize = 0;
    char *pname = NULL;
    char *pdescription = NULL;
    const char *interface_name = NULL;

    uilib_localize_dialog(hwnd, ethernet_dialog);
    uilib_get_group_extent(hwnd, ethernet_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, ethernet_leftgroup);
    uilib_move_group(hwnd, ethernet_rightgroup, xsize + 30);

    if (rawnet_enumadapter_open()) {
        resources_get_string("ETHERNET_INTERFACE", &interface_name);
        temp_hwnd = GetDlgItem(hwnd, IDC_ETHERNET_SETTINGS_INTERFACE);

        for (cnt = 0; rawnet_enumadapter(&pname, &pdescription); cnt++) {
            combined = util_concat(pdescription, " (", pname, ")", NULL);
            st_combined = system_mbstowcs_alloc(combined);
            SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st_combined);
            system_mbstowcs_free(st_combined);

            if (strcmp(combined, interface_name) == 0) {
                SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)cnt, 0);
            }

            lib_free(combined);
            lib_free(pdescription);
            lib_free(pname);
        }

        rawnet_enumadapter_close();
    }

    /* Show the current interface. */
    show_interface_selection(hwnd);
}

static void save_ethernet_dialog(HWND hwnd)
{
    TCHAR st_buffer[256];
    char buffer[256];

    GetDlgItemText(hwnd, IDC_ETHERNET_SETTINGS_INTERFACE, st_buffer, 256);
    system_wcstombs(buffer, st_buffer, 256);
    resources_set_string("ETHERNET_INTERFACE", buffer);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDOK:
                    save_ethernet_dialog(hwnd);
                    /* FALL THROUGH */
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
                case IDC_ETHERNET_SETTINGS_INTERFACE:
                    show_interface_selection(hwnd);
                    break;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_ethernet_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_ethernet_settings_dialog(HWND hwnd)
{
    int disabled = 0;

    resources_get_int("ETHERNET_DISABLED", &disabled);
    if (disabled) {
        MessageBox(hwnd, intl_translate_tcs(IDS_ETHERNET_PROBLEM), intl_translate_tcs(IDS_ETHERNET_SUPPORT), MB_ICONINFORMATION | MB_OK);
    } else {
        DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_ETHERNET_SETTINGS_DIALOG, hwnd, dialog_proc);
    }
}

#endif // #ifdef HAVE_RAWNET
