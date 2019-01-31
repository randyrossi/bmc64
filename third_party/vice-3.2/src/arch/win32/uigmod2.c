/*
 * uigmod2.c - Implementation of the GMod2 settings dialog box.
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
#include "uigmod2.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param gmod2_dialog_trans[] = {
    { 0, IDS_GMOD2_CAPTION, -1 },
    { IDC_GMOD2_EEPROMIMAGE_LABEL, IDS_GMOD2_EEPROMIMAGE_LABEL, 0 },
    { IDC_GMOD2_EEPROMIMAGE_BROWSE, IDS_BROWSE, 0 },
    { IDC_GMOD2_EEPROMRW, IDS_GMOD2_EEPROM_READ_WRITE, 0 },
    { IDC_GMOD2_FLASHRW, IDS_GMOD2_FLASH_READ_WRITE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group gmod2_main_group[] = {
    { IDC_GMOD2_EEPROMIMAGE_LABEL, 0 },
    { IDC_GMOD2_EEPROMIMAGE_BROWSE, 1 },
    { IDC_GMOD2_EEPROMRW, 1 },
    { IDC_GMOD2_FLASHRW, 1 },
    { 0, 0 }
};

static uilib_dialog_group gmod2_top_left_group[] = {
    { IDC_GMOD2_EEPROMIMAGE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group gmod2_top_middle_group[] = {
    { IDC_GMOD2_EEPROMIMAGE_BROWSE, 1 },
    { 0, 0 }
};

static uilib_dialog_group gmod2_top_right_group[] = {
    { IDC_GMOD2_FLASHRW, 1 },
    { IDC_GMOD2_EEPROMRW, 1 },
    { 0, 0 }
};

static uilib_dialog_group gmod2_right_group[] = {
    { IDC_GMOD2_FLASHRW, 1 },
    { IDC_GMOD2_EEPROMRW, 1 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_gmod2_dialog(HWND hwnd)
{
    int res_value;
    int xpos;
    const char *gmod2_eeprom_file;
    TCHAR *st_gmod2_eeprom_file;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, gmod2_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, gmod2_main_group);

    /* get the max x of the top left group */
    uilib_get_group_max_x(hwnd, gmod2_top_left_group, &xpos);

    /* move the top middle group to the correct location */
    uilib_move_group(hwnd, gmod2_top_middle_group, xpos + 10);

    /* get the max x of the top middle group */
    uilib_get_group_max_x(hwnd, gmod2_top_middle_group, &xpos);

    /* move the top right group to the correct location */
    uilib_move_group(hwnd, gmod2_top_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, gmod2_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("GMod2FlashWrite", &res_value);
    CheckDlgButton(hwnd, IDC_GMOD2_FLASHRW, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_string("GMod2EEPROMImage", &gmod2_eeprom_file);
    st_gmod2_eeprom_file = system_mbstowcs_alloc(gmod2_eeprom_file);
    SetDlgItemText(hwnd, IDC_GMOD2_EEPROMIMAGE, gmod2_eeprom_file != NULL ? st_gmod2_eeprom_file : TEXT(""));
    system_mbstowcs_free(st_gmod2_eeprom_file);

    resources_get_int("GMOD2EEPROMRW", &res_value);
    CheckDlgButton(hwnd, IDC_GMOD2_EEPROMRW, res_value ? BST_CHECKED : BST_UNCHECKED);
}

static void end_gmod2_dialog(HWND hwnd)
{
    TCHAR st_name[MAX_PATH];
    char name[MAX_PATH];

    GetDlgItemText(hwnd, IDC_GMOD2_EEPROMIMAGE, st_name, MAX_PATH);
    system_wcstombs(name, st_name, MAX_PATH);
    resources_set_string("GMod2EEPROMImage", name);

    resources_set_int("GMod2FlashWrite", (IsDlgButtonChecked(hwnd, IDC_GMOD2_FLASHRW) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("GMOD2EEPROMRW", (IsDlgButtonChecked(hwnd, IDC_GMOD2_EEPROMRW) == BST_CHECKED ? 1 : 0 ));
}

static void browse_gmod2_eeprom_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_GMOD2_EEPROM_SELECT_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_GMOD2_EEPROMIMAGE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_GMOD2_EEPROMIMAGE_BROWSE:
                    browse_gmod2_eeprom_file(hwnd);
                    break;
                case IDOK:
                    end_gmod2_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_gmod2_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_gmod2_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_GMOD2_SETTINGS_DIALOG, hwnd, dialog_proc);
}
