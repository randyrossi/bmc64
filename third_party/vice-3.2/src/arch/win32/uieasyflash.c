/*
 * uieasyflash.c - Implementation of the EasyFlash settings dialog box.
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
#include "intl.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uiapi.h"
#include "uieasyflash.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param easyflash_dialog_trans[] = {
    { 0, IDS_EASYFLASH_CAPTION, -1 },
    { IDC_EASYFLASH_JUMPER, IDS_EASYFLASH_JUMPER, 0 },
    { IDC_EASYFLASH_CRT_OPTIMIZE, IDS_EASYFLASH_CRT_OPTIMIZE, 0 },
    { IDC_EASYFLASH_AUTOSAVE, IDS_EASYFLASH_AUTOSAVE, 0 },
    { IDC_EASYFLASH_SAVE_NOW, IDS_EASYFLASH_SAVE_NOW, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group easyflash_group[] = {
    { IDC_EASYFLASH_JUMPER, 1 },
    { IDC_EASYFLASH_AUTOSAVE, 1 },
    { IDC_EASYFLASH_SAVE_NOW, 1 },
    { IDC_EASYFLASH_CRT_OPTIMIZE, 1 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_easyflash_dialog(HWND hwnd)
{
    int res_value;
    int xpos;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, easyflash_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, easyflash_group);

    /* get the max x of the main group */
    uilib_get_group_max_x(hwnd, easyflash_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 10, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    resources_get_int("EasyFlashJumper", &res_value);
    CheckDlgButton(hwnd, IDC_EASYFLASH_JUMPER, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("EasyFlashWriteCRT", &res_value);
    CheckDlgButton(hwnd, IDC_EASYFLASH_AUTOSAVE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("EasyFlashOptimizeCRT", &res_value);
    CheckDlgButton(hwnd, IDC_EASYFLASH_CRT_OPTIMIZE, res_value ? BST_CHECKED : BST_UNCHECKED);
}

static void end_easyflash_dialog(HWND hwnd)
{
    resources_set_int("EasyFlashJumper", (IsDlgButtonChecked(hwnd, IDC_EASYFLASH_JUMPER) == BST_CHECKED ? 1 : 0));
    resources_set_int("EasyFlashWriteCRT", (IsDlgButtonChecked(hwnd, IDC_EASYFLASH_AUTOSAVE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("EasyFlashOptimizeCRT", (IsDlgButtonChecked(hwnd, IDC_EASYFLASH_CRT_OPTIMIZE) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;
    int result;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_EASYFLASH_SAVE_NOW:
                    result = cartridge_flush_image(CARTRIDGE_EASYFLASH);
                    if (result == -2) {
                        ui_error(translate_text(IDS_NO_EASYFLASH_CART_INSERTED));
                    }
                    if (result == -1) {
                        ui_error(translate_text(IDS_ERROR_SAVING_EASYFLASH_CRT));
                    }
                    break;
                case IDOK:
                    end_easyflash_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_easyflash_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_easyflash_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_EASYFLASH_SETTINGS_DIALOG, hwnd, dialog_proc);
}
