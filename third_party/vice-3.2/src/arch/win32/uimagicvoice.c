/*
 * uimagicvoice.c - Implementation of the Magic Voice cartridge settings dialog box.
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
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uimagicvoice.h"
#include "winmain.h"

static void enable_magicvoice_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_MAGICVOICE_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_MAGICVOICE_BROWSE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_MAGICVOICE_FILE), is_enabled);
}

static uilib_localize_dialog_param magicvoice_dialog[] = {
    { 0, IDS_MAGICVOICE_CAPTION, -1 },
    { IDC_MAGICVOICE_ENABLE, IDS_MAGICVOICE_ENABLE, 0 },
    { IDC_MAGICVOICE_FILE_LABEL, IDS_MAGICVOICE_FILE, 0 },
    { IDC_MAGICVOICE_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group magicvoice_group1[] = {
    { IDC_MAGICVOICE_ENABLE, 1 },
    { 0, 0 }
};

static uilib_dialog_group magicvoice_leftgroup[] = {
    { IDC_MAGICVOICE_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group magicvoice_rightgroup[] = {
    { IDC_MAGICVOICE_BROWSE, 0 },
    { 0, 0 }
};

static void init_magicvoice_dialog(HWND hwnd)
{
    int res_value;
    const char *magicvoicefile;
    TCHAR *st_magicvoicefile;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, magicvoice_dialog);
    uilib_adjust_group_width(hwnd, magicvoice_group1);
    uilib_get_group_extent(hwnd, magicvoice_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, magicvoice_leftgroup);
    uilib_move_group(hwnd, magicvoice_rightgroup, xsize + 30);

    resources_get_int("MagicVoiceCartridgeEnabled", &res_value);
    CheckDlgButton(hwnd, IDC_MAGICVOICE_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_string("MagicVoiceImage", &magicvoicefile);
    st_magicvoicefile = system_mbstowcs_alloc(magicvoicefile);
    SetDlgItemText(hwnd, IDC_MAGICVOICE_FILE, magicvoicefile != NULL ? st_magicvoicefile : TEXT(""));
    system_mbstowcs_free(st_magicvoicefile);

    enable_magicvoice_controls(hwnd);
}

static void end_magicvoice_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    GetDlgItemText(hwnd, IDC_MAGICVOICE_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("MagicVoiceImage", s);

    resources_set_int("MagicVoiceCartridgeEnabled", (IsDlgButtonChecked(hwnd, IDC_MAGICVOICE_ENABLE) == BST_CHECKED ? 1 : 0 ));
}

static void browse_magicvoice_file(HWND hwnd)
{
    uilib_select_browse(hwnd, translate_text(IDS_MAGICVOICE_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_MAGICVOICE_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_MAGICVOICE_BROWSE:
                    browse_magicvoice_file(hwnd);
                    break;
                case IDC_MAGICVOICE_ENABLE:
                    enable_magicvoice_controls(hwnd);
                    break;
                case IDOK:
                    end_magicvoice_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_magicvoice_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_magicvoice_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_MAGICVOICE_SETTINGS_DIALOG, hwnd, dialog_proc);
}
