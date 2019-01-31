/*
 * uisoundexpander.c - Implementation of the sfx sound expander settings dialog box.
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

#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uilib.h"
#include "uisoundexpander.h"
#include "winmain.h"
#include "intl.h"

static void enable_soundexpander_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_SFX_SOUNDEXPANDER_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_SFX_SOUNDEXPANDER_TYPE), is_enabled);

    if (machine_class == VICE_MACHINE_VIC20) {
        EnableWindow(GetDlgItem(hwnd, IDC_SFX_SOUNDEXPANDER_IO_SWAP), is_enabled);
    }
}

static uilib_localize_dialog_param soundexpander_dialog[] = {
    { 0, IDS_SFX_SOUNDEXPANDER_CAPTION, -1 },
    { IDC_SFX_SOUNDEXPANDER_ENABLE, IDS_SFX_SOUNDEXPANDER_ENABLE, 0 },
    { IDC_SFX_SOUNDEXPANDER_TYPE_LABEL, IDS_SFX_SOUNDEXPANDER_TYPE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param soundexpander_mascuerade_dialog[] = {
    { IDC_SFX_SOUNDEXPANDER_IO_SWAP, IDS_IO_SWAP, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group soundexpander_leftgroup[] = {
    { IDC_SFX_SOUNDEXPANDER_TYPE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group soundexpander_left_total_group[] = {
    { IDC_SFX_SOUNDEXPANDER_ENABLE, 1 },
    { IDC_SFX_SOUNDEXPANDER_TYPE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group soundexpander_rightgroup[] = {
    { IDC_SFX_SOUNDEXPANDER_TYPE, 0 },
    { 0, 0 }
};

static void init_soundexpander_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, soundexpander_dialog);

    if (machine_class == VICE_MACHINE_VIC20) {
        uilib_localize_dialog(hwnd, soundexpander_mascuerade_dialog);
    }

    uilib_get_group_extent(hwnd, soundexpander_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, soundexpander_left_total_group);
    uilib_move_group(hwnd, soundexpander_rightgroup, xsize + 30);

    resources_get_int("SFXSoundExpander", &res_value);
    CheckDlgButton(hwnd, IDC_SFX_SOUNDEXPANDER_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    if (machine_class == VICE_MACHINE_VIC20) {
        resources_get_int("SFXSoundExpanderIOSwap", &res_value);
        CheckDlgButton(hwnd, IDC_SFX_SOUNDEXPANDER_IO_SWAP, res_value ? BST_CHECKED : BST_UNCHECKED);
    }

    temp_hwnd = GetDlgItem(hwnd, IDC_SFX_SOUNDEXPANDER_TYPE);

    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"3526");
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)"3812");
    resources_get_int("SFXSoundExpanderChip", &res_value);
    if (res_value == 3526) {
        SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)0, 0);
    } else {
        SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)1, 0);
    }
    enable_soundexpander_controls(hwnd);
}

static void end_soundexpander_dialog(HWND hwnd)
{
    resources_set_int("SFXSoundExpander", (IsDlgButtonChecked(hwnd, IDC_SFX_SOUNDEXPANDER_ENABLE) == BST_CHECKED ? 1 : 0 ));

    if (machine_class == VICE_MACHINE_VIC20) {
        resources_set_int("SFXSoundExpanderIOSwap", (IsDlgButtonChecked(hwnd, IDC_SFX_SOUNDEXPANDER_IO_SWAP) == BST_CHECKED ? 1 : 0 ));
    }

    resources_set_int("SFXSoundExpanderChip", (SendMessage(GetDlgItem(hwnd, IDC_SFX_SOUNDEXPANDER_TYPE), CB_GETCURSEL, 0, 0) == 0) ? 3526 : 3812);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_SFX_SOUNDEXPANDER_ENABLE:
                    enable_soundexpander_controls(hwnd);
                    break;
                case IDOK:
                    end_soundexpander_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_soundexpander_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_soundexpander_settings_dialog(HWND hwnd)
{
    if (machine_class == VICE_MACHINE_VIC20) {
        DialogBox(winmain_instance, (LPCTSTR)IDD_MASCUERADE_SFX_SOUNDEXPANDER_SETTINGS_DIALOG, hwnd, dialog_proc);
    } else {
        DialogBox(winmain_instance, (LPCTSTR)IDD_SFX_SOUNDEXPANDER_SETTINGS_DIALOG, hwnd, dialog_proc);
    }
}
