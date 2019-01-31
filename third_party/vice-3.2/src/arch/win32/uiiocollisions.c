/*
 * uiiocollisions.c - Implementation of the I/O collision settings dialog box.
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
#include "uiiocollisions.h"
#include "uilib.h"
#include "winmain.h"

static uilib_localize_dialog_param iocollisions_dialog[] = {
    { 0, IDS_IO_COLLISION_CAPTION, -1 },
    { IDC_IO_COLLISION_METHOD_LABEL, IDS_IO_COLLISION_METHOD, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group iocollisions_leftgroup[] = {
    { IDC_IO_COLLISION_METHOD_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group iocollisions_rightgroup[] = {
    { IDC_IO_COLLISION_METHOD, 0 },
    { 0, 0 }
};

static void init_iocollisions_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    int xsize, ysize;

    uilib_localize_dialog(hwnd, iocollisions_dialog);
    uilib_adjust_group_width(hwnd, iocollisions_leftgroup);
    uilib_get_group_extent(hwnd, iocollisions_leftgroup, &xsize, &ysize);
    uilib_adjust_group_width(hwnd, iocollisions_leftgroup);
    uilib_move_group(hwnd, iocollisions_rightgroup, xsize + 30);

    temp_hwnd = GetDlgItem(hwnd, IDC_IO_COLLISION_METHOD);
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(IDS_IO_COLLISION_DETACH_ALL));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(IDS_IO_COLLISION_DETACH_LAST));
    SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)translate_text(IDS_IO_COLLISION_AND_WIRES));

    resources_get_int("IOCollisionHandling", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
}

static void end_iocollisions_dialog(HWND hwnd)
{
    resources_set_int("IOCollisionHandling", SendMessage(GetDlgItem(hwnd, IDC_IO_COLLISION_METHOD), CB_GETCURSEL, 0, 0));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_iocollisions_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_iocollisions_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_iocollision_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_IO_COLLISION_SETTINGS_DIALOG, hwnd, dialog_proc);
}
