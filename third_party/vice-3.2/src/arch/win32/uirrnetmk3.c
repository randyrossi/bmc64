/*
 * uirrnetmk3.c - Implementation of the RRNET MK3 settings dialog box.
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

#include "cartridge.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uirrnetmk3.h"
#include "winmain.h"

static uilib_localize_dialog_param rrnetmk3_dialog_trans[] = {
    { IDC_RRNETMK3_FLASH_JUMPER, IDS_RRNETMK3_FLASH_JUMPER, 0 },
    { IDC_RRNETMK3_SAVE_WHEN_CHANGED, IDS_RRNETMK3_BIOS_SAVE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group rrnetmk3_main_group[] = {
    { IDC_RRNETMK3_FLASH_JUMPER, 1 },
    { IDC_RRNETMK3_SAVE_WHEN_CHANGED, 1 },
    { 0, 0 }
};

static uilib_dialog_group rrnetmk3_window_group[] = {
    { IDC_RRNETMK3_FLASH_JUMPER, 0 },
    { IDC_RRNETMK3_SAVE_WHEN_CHANGED, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_rrnetmk3_dialog(HWND hwnd)
{
    int res_value;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, rrnetmk3_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, rrnetmk3_main_group);

    /* get the max x of the window group */
    uilib_get_group_max_x(hwnd, rrnetmk3_window_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("RRNETMK3_flashjumper", &res_value);
    CheckDlgButton(hwnd, IDC_RRNETMK3_FLASH_JUMPER, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("RRNETMK3_bios_write", &res_value);
    CheckDlgButton(hwnd, IDC_RRNETMK3_SAVE_WHEN_CHANGED, res_value ? BST_CHECKED : BST_UNCHECKED);
}

static void end_rrnetmk3_dialog(HWND hwnd)
{
    resources_set_int("RRNETMK3_flashjumper", (IsDlgButtonChecked(hwnd, IDC_RRNETMK3_FLASH_JUMPER) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("RRNETMK3_bios_write", (IsDlgButtonChecked(hwnd, IDC_RRNETMK3_SAVE_WHEN_CHANGED) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_rrnetmk3_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_rrnetmk3_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_rrnetmk3_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_RRNETMK3_SETTINGS_DIALOG, hwnd, dialog_proc);
}
