/*
 * uited.c - Implementation of TED settings dialog box.
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

#include <windows.h>

#include "res.h"
#include "resources.h"
#include "system.h"
#include "ted.h"
#include "translate.h"
#include "uilib.h"
#include "winmain.h"

#include "uited.h"


static uilib_localize_dialog_param ted_dialog[] = {
    { 0, IDS_TED_CAPTION, -1 },
    { IDC_TED_BORDERSGROUP, IDS_TED_BORDERSGROUP, 0 },
    { IDC_TOGGLE_TED_NORMALBORDERS, IDS_TED_NORMALBORDERS, 0 },
    { IDC_TOGGLE_TED_FULLBORDERS, IDS_TED_FULLBORDERS, 0 },
    { IDC_TOGGLE_TED_DEBUGBORDERS, IDS_TED_DEBUGBORDERS, 0 },
    { IDC_TOGGLE_TED_NOBORDERS, IDS_TED_NOBORDERS, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static void init_ted_dialog(HWND hwnd)
{
    int n;

    uilib_localize_dialog(hwnd, ted_dialog);

    resources_get_int("TEDBorderMode", &n);
    switch (n) {
        default:
        case TED_NORMAL_BORDERS:
            n = IDC_TOGGLE_TED_NORMALBORDERS;
            break;
        case TED_FULL_BORDERS:
            n = IDC_TOGGLE_TED_FULLBORDERS;
            break;
        case TED_DEBUG_BORDERS:
            n = IDC_TOGGLE_TED_DEBUGBORDERS;
            break;
        case TED_NO_BORDERS:
            n = IDC_TOGGLE_TED_NOBORDERS;
            break;
    }
    CheckRadioButton(hwnd, IDC_TOGGLE_TED_NORMALBORDERS, IDC_TOGGLE_TED_NOBORDERS, n);
}

static void end_ted_dialog(HWND hwnd)
{
    resources_set_int("TEDBorderMode", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_TED_DEBUGBORDERS) == BST_CHECKED ? TED_DEBUG_BORDERS : 
                      IsDlgButtonChecked(hwnd, IDC_TOGGLE_TED_FULLBORDERS) == BST_CHECKED ? TED_FULL_BORDERS :
                      IsDlgButtonChecked(hwnd, IDC_TOGGLE_TED_NORMALBORDERS) == BST_CHECKED ? TED_NORMAL_BORDERS : TED_NO_BORDERS));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_ted_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_TOGGLE_TED_NORMALBORDERS:
                case IDC_TOGGLE_TED_FULLBORDERS:
                case IDC_TOGGLE_TED_DEBUGBORDERS:
                case IDC_TOGGLE_TED_NOBORDERS:
                    break;
                case IDOK:
                    end_ted_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return TRUE;
    }
    return FALSE;
}

void ui_ted_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_TED_DIALOG), hwnd, dialog_proc);
}
