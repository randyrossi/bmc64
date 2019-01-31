/*
 * uivic.c - Implementation of VIC settings dialog box.
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
#include "translate.h"
#include "uilib.h"
#include "vic.h"
#include "winmain.h"

#include "uivic.h"


static uilib_localize_dialog_param vic_dialog[] = {
    { 0, IDS_VIC_CAPTION, -1 },
    { IDC_VIC_BORDERSGROUP, IDS_VIC_BORDERSGROUP, 0 },
    { IDC_TOGGLE_VIC_NORMALBORDERS, IDS_VIC_NORMALBORDERS, 0 },
    { IDC_TOGGLE_VIC_FULLBORDERS, IDS_VIC_FULLBORDERS, 0 },
    { IDC_TOGGLE_VIC_DEBUGBORDERS, IDS_VIC_DEBUGBORDERS, 0 },
    { IDC_TOGGLE_VIC_NOBORDERS, IDS_VIC_NOBORDERS, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static void init_vic_dialog(HWND hwnd)
{
    int n;

    uilib_localize_dialog(hwnd, vic_dialog);

    resources_get_int("VICBorderMode", &n);
    switch (n) {
        default:
        case VIC_NORMAL_BORDERS:
            n = IDC_TOGGLE_VIC_NORMALBORDERS;
            break;
        case VIC_FULL_BORDERS:
            n = IDC_TOGGLE_VIC_FULLBORDERS;
            break;
        case VIC_DEBUG_BORDERS:
            n = IDC_TOGGLE_VIC_DEBUGBORDERS;
            break;
        case VIC_NO_BORDERS:
            n = IDC_TOGGLE_VIC_NOBORDERS;
            break;
    }
    CheckRadioButton(hwnd, IDC_TOGGLE_VIC_NORMALBORDERS, IDC_TOGGLE_VIC_NOBORDERS, n);
}

static void end_vic_dialog(HWND hwnd)
{
    resources_set_int("VICBorderMode", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_VIC_DEBUGBORDERS) == BST_CHECKED ? VIC_DEBUG_BORDERS : 
                      IsDlgButtonChecked(hwnd, IDC_TOGGLE_VIC_FULLBORDERS) == BST_CHECKED ? VIC_FULL_BORDERS :
                      IsDlgButtonChecked(hwnd, IDC_TOGGLE_VIC_NORMALBORDERS) == BST_CHECKED ? VIC_NORMAL_BORDERS : VIC_NO_BORDERS));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_vic_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_TOGGLE_VIC_NORMALBORDERS:
                case IDC_TOGGLE_VIC_FULLBORDERS:
                case IDC_TOGGLE_VIC_DEBUGBORDERS:
                case IDC_TOGGLE_VIC_NOBORDERS:
                    break;
                case IDOK:
                    end_vic_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return TRUE;
    }
    return FALSE;
}

void ui_vic_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_VIC_DIALOG), hwnd, dialog_proc);
}
