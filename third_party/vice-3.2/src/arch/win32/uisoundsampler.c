/*
 * uisoundsampler.c - Implementation of the sfx sound sampler settings dialog box.
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

#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uilib.h"
#include "uisoundsampler.h"
#include "winmain.h"
#include "intl.h"

static void enable_soundsampler_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (IsDlgButtonChecked(hwnd, IDC_SFX_SOUNDSAMPLER_ENABLE) == BST_CHECKED) ? 1 : 0;

    EnableWindow(GetDlgItem(hwnd, IDC_SFX_SOUNDSAMPLER_IO_SWAP), is_enabled);
}

static uilib_localize_dialog_param soundsampler_dialog[] = {
    { 0, IDS_SFX_SOUNDSAMPLER_CAPTION, -1 },
    { IDC_SFX_SOUNDSAMPLER_ENABLE, IDS_SFX_SOUNDSAMPLER_ENABLE, 0 },
    { IDC_SFX_SOUNDSAMPLER_IO_SWAP, IDS_IO_SWAP, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static void init_soundsampler_dialog(HWND hwnd)
{
    int res_value;

    uilib_localize_dialog(hwnd, soundsampler_dialog);

    resources_get_int("SFXSoundSampler", &res_value);
    CheckDlgButton(hwnd, IDC_SFX_SOUNDSAMPLER_ENABLE, res_value ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("SFXSoundSamplerIOSwap", &res_value);
    CheckDlgButton(hwnd, IDC_SFX_SOUNDSAMPLER_IO_SWAP, res_value ? BST_CHECKED : BST_UNCHECKED);

    enable_soundsampler_controls(hwnd);
}

static void end_soundsampler_dialog(HWND hwnd)
{
    resources_set_int("SFXSoundSampler", (IsDlgButtonChecked(hwnd, IDC_SFX_SOUNDSAMPLER_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("SFXSoundSamplerIOSwap", (IsDlgButtonChecked(hwnd, IDC_SFX_SOUNDSAMPLER_IO_SWAP) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_SFX_SOUNDSAMPLER_ENABLE:
                    enable_soundsampler_controls(hwnd);
                    break;
                case IDOK:
                    end_soundsampler_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_soundsampler_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_soundsampler_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)IDD_SFX_SOUNDSAMPLER_SETTINGS_DIALOG, hwnd, dialog_proc);
}
