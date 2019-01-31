/*
 * uivicii.c - Implementation of VIC-II settings dialog box.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@mail.matav.hu>
 *  Gunnar Ruthenberg <Krill.Plush@gmail.com>
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

#include "intl.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "uilib.h"
#include "vicii.h"
#include "winmain.h"

#include "uivicii.h"


static uilib_localize_dialog_param vicii_dialog[] = {
    { 0, IDS_VICII_CAPTION, -1 },
    { IDC_VICII_BORDERSGROUP, IDS_VICII_BORDERSGROUP, 0 },
    { IDC_TOGGLE_VICII_NORMALBORDERS, IDS_VICII_NORMALBORDERS, 0 },
    { IDC_TOGGLE_VICII_FULLBORDERS, IDS_VICII_FULLBORDERS, 0 },
    { IDC_TOGGLE_VICII_DEBUGBORDERS, IDS_VICII_DEBUGBORDERS, 0 },
    { IDC_TOGGLE_VICII_NOBORDERS, IDS_VICII_NOBORDERS, 0 },
    { IDC_VICII_SPRITEGROUP, IDS_VICII_SPRITEGROUP, 0 },
    { IDC_TOGGLE_VICII_SSC, IDS_VICII_SPRITECOLL, 0 },
    { IDC_TOGGLE_VICII_SBC, IDS_VICII_BACKCOLL, 0 },
    { IDC_C64VICII_LABEL, IDS_VICII_MODEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param viciisc_dialog[] = {
    { 0, IDS_VICII_CAPTION, -1 },
    { IDC_VICII_BORDERSGROUP, IDS_VICII_BORDERSGROUP, 0 },
    { IDC_TOGGLE_VICII_NORMALBORDERS, IDS_VICII_NORMALBORDERS, 0 },
    { IDC_TOGGLE_VICII_FULLBORDERS, IDS_VICII_FULLBORDERS, 0 },
    { IDC_TOGGLE_VICII_DEBUGBORDERS, IDS_VICII_DEBUGBORDERS, 0 },
    { IDC_TOGGLE_VICII_NOBORDERS, IDS_VICII_NOBORDERS, 0 },
    { IDC_VICII_SPRITEGROUP, IDS_VICII_SPRITEGROUP, 0 },
    { IDC_TOGGLE_VICII_SSC, IDS_VICII_SPRITECOLL, 0 },
    { IDC_TOGGLE_VICII_SBC, IDS_VICII_BACKCOLL, 0 },
    { IDC_TOGGLE_VICII_VSP_BUG, IDS_VICII_VSP_BUG, 0 },
    { IDC_C64VICII_LABEL, IDS_VICII_MODEL, 0 },
    { IDC_C64GLUELOGIC_LABEL, IDS_GLUE_LOGIC, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static const int ui_c64vicii[] = {
    IDS_6569_PAL,
    IDS_8565_PAL,
    IDS_6569R1_OLD_PAL,
    IDS_6567_NTSC,
    IDS_8562_NTSC,
    IDS_6567R56A_OLD_NTSC,
    IDS_6572_PAL_N,
    0
};

static const int ui_c64vicii_values[] = {
    VICII_MODEL_6569,
    VICII_MODEL_8565,
    VICII_MODEL_6569R1,
    VICII_MODEL_6567,
    VICII_MODEL_8562,
    VICII_MODEL_6567R56A,
    VICII_MODEL_6572,
    -1
};

static const TCHAR *ui_c64video_standard[] = {
    TEXT("PAL-G"),
    TEXT("NTSC-M"),
    TEXT("Old NTSC-M"),
    TEXT("PAL-N"),
    0
};

static const int ui_c64video_standard_values[] = {
    MACHINE_SYNC_PAL,
    MACHINE_SYNC_NTSC,
    MACHINE_SYNC_NTSCOLD,
    MACHINE_SYNC_PALN,
    0
};

static const int ui_c64gluelogic[] = {
    IDS_DISCRETE,
    IDS_CUSTOM_IC,
    0
};

static const int ui_c64gluelogic_values[] = {
    0,
    1,
    -1
};

static void init_vicii_dialog(HWND hwnd)
{
    HWND sub_hwnd;
    int i, n;
    int res_value, index = 0;

    if (machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64) {
        uilib_localize_dialog(hwnd, viciisc_dialog);
    } else {
        uilib_localize_dialog(hwnd, vicii_dialog);
    }

    sub_hwnd = GetDlgItem(hwnd, IDC_C64VICII_LIST);
    if (machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64) {
        resources_get_int("VICIIModel", &res_value);
        for (i = 0; ui_c64vicii[i] != 0; i++) {
            SendMessage(sub_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(ui_c64vicii[i]));
            if (ui_c64vicii_values[i] == res_value) {
                index = i;
            }
        }
    } else {
        resources_get_int("MachineVideoStandard", &res_value);
        for (i = 0; ui_c64video_standard[i] != 0; i++) {
            SendMessage(sub_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_c64video_standard[i]);
            if (ui_c64video_standard_values[i] == res_value) {
                index = i;
            }
        }
    }
    SendMessage(sub_hwnd, CB_SETCURSEL, (WPARAM)index, 0);

    if (machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64) {
        resources_get_int("GlueLogic", &res_value);
        sub_hwnd = GetDlgItem(hwnd, IDC_C64GLUELOGIC_LIST);
        for (i = 0; ui_c64gluelogic[i] != 0; i++) {
            SendMessage(sub_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(ui_c64gluelogic[i]));
            if (ui_c64gluelogic_values[i] == res_value) {
                index = i;
            }
        }
        SendMessage(sub_hwnd, CB_SETCURSEL, (WPARAM)index, 0);
    }

    resources_get_int("VICIIBorderMode", &n);
    switch (n) {
        default:
        case VICII_NORMAL_BORDERS:
            n = IDC_TOGGLE_VICII_NORMALBORDERS;
            break;
        case VICII_FULL_BORDERS:
            n = IDC_TOGGLE_VICII_FULLBORDERS;
            break;
        case VICII_DEBUG_BORDERS:
            n = IDC_TOGGLE_VICII_DEBUGBORDERS;
            break;
        case VICII_NO_BORDERS:
            n = IDC_TOGGLE_VICII_NOBORDERS;
            break;
    }
    CheckRadioButton(hwnd, IDC_TOGGLE_VICII_NORMALBORDERS, IDC_TOGGLE_VICII_NOBORDERS, n);

    resources_get_int("VICIICheckSsColl", &n);
    CheckDlgButton(hwnd, IDC_TOGGLE_VICII_SSC, n ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("VICIICheckSbColl", &n);
    CheckDlgButton(hwnd, IDC_TOGGLE_VICII_SBC, n ? BST_CHECKED : BST_UNCHECKED);

    if (machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64) {
        resources_get_int("VICIIVSPBug", &n);
        CheckDlgButton(hwnd, IDC_TOGGLE_VICII_VSP_BUG, n ? BST_CHECKED : BST_UNCHECKED);
    }
}

static void end_vicii_dialog(HWND hwnd)
{
    resources_set_int("VICIIBorderMode", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_VICII_DEBUGBORDERS) == BST_CHECKED ? VICII_DEBUG_BORDERS : 
                      IsDlgButtonChecked(hwnd, IDC_TOGGLE_VICII_FULLBORDERS) == BST_CHECKED ? VICII_FULL_BORDERS :
                      IsDlgButtonChecked(hwnd, IDC_TOGGLE_VICII_NORMALBORDERS) == BST_CHECKED ? VICII_NORMAL_BORDERS : VICII_NO_BORDERS));

    resources_set_int("VICIICheckSsColl", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_VICII_SSC) == BST_CHECKED ? 1 : 0 ));

    resources_set_int("VICIICheckSbColl", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_VICII_SBC) == BST_CHECKED ? 1 : 0 ));

    if (machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64) {
        resources_set_int("VICIIModel", ui_c64vicii_values[(int)SendMessage(GetDlgItem(hwnd, IDC_C64VICII_LIST), CB_GETCURSEL, 0, 0)]);
        resources_set_int("VICIIVSPBug", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_VICII_VSP_BUG) == BST_CHECKED ? 1 : 0 ));
        resources_set_int("GlueLogic", ui_c64gluelogic_values[(int)SendMessage(GetDlgItem(hwnd, IDC_C64GLUELOGIC_LIST), CB_GETCURSEL, 0, 0)]);
    } else {
        resources_set_int("MachineVideoStandard", ui_c64video_standard_values[(int)SendMessage(GetDlgItem(hwnd, IDC_C64VICII_LIST), CB_GETCURSEL, 0, 0)]);
    }
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_vicii_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_TOGGLE_VICII_NORMALBORDERS:
                case IDC_TOGGLE_VICII_FULLBORDERS:
                case IDC_TOGGLE_VICII_DEBUGBORDERS:
                case IDC_TOGGLE_VICII_NOBORDERS:
                case IDC_TOGGLE_VICII_SSC:
                case IDC_TOGGLE_VICII_SBC:
                case IDC_C64VICII_LIST:
                case IDC_C64GLUELOGIC_LIST:
                    break;
                case IDOK:
                    end_vicii_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return TRUE;
    }
    return FALSE;
}

void ui_vicii_settings_dialog(HWND hwnd)
{
    if (machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64) {
        DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_VICIISC_DIALOG), hwnd, dialog_proc);
    } else {
        DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_VICII_DIALOG), hwnd, dialog_proc);
    }
}
