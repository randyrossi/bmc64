/*
 * uisidcart.c - Implementation of the SID cartridge settings dialog box.
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

#include "catweaselmkiii.h"
#include "hardsid.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "sid.h"
#include "system.h"
#include "translate.h"
#include "uilib.h"
#include "uisidcart.h"
#include "winmain.h"

static TCHAR *native_primary_sid_text_address;
static TCHAR *native_secondary_sid_text_address;
static TCHAR *native_sid_clock;
static int native_primary_sid_int_address;
static int native_secondary_sid_int_address;

static sid_engine_model_t **ui_sid_engine_model_list;

/* Values currently selected in the UI
   Needed to enable/disable UI elements */
static int sel_engine, sel_cart_enabled;

static void enable_sidcart_hardsid_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = sel_cart_enabled && (sel_engine == SID_ENGINE_HARDSID) && (hardsid_available() > 0);

    EnableWindow(GetDlgItem(hwnd, IDC_SIDCART_HARDSID_MAIN_DEVICE), is_enabled);
}

static void enable_sidcart_controls(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDC_SIDCART_ENGINE_MODEL), sel_cart_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SIDCART_FILTERS), sel_cart_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SIDCART_ADDRESS), sel_cart_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SIDCART_CLOCK), sel_cart_enabled);
    if (machine_class == VICE_MACHINE_PLUS4) {
        EnableWindow(GetDlgItem(hwnd, IDC_DIGIBLASTER), sel_cart_enabled);
    }
    enable_sidcart_hardsid_controls(hwnd);
}

static uilib_localize_dialog_param sidcart_hardsid_dialog_trans[] = {
    { IDC_SIDCART_ENABLE, IDS_SIDCART_ENABLE, 0 },
    { IDC_SIDCART_ENGINE_MODEL_LABEL, IDS_SID_GENGROUP1, 0 },
    { IDC_SIDCART_FILTERS, IDS_SID_FILTERS, 0 },
    { IDC_SIDCART_ADDRESS_LABEL, IDS_SIDCART_ADDRESS_LABEL, 0 },
    { IDC_SIDCART_CLOCK_LABEL, IDS_SIDCART_CLOCK_LABEL, 0 },
    { IDC_SIDCART_HARDSID_DEVICE_SELECTION_LABEL, IDS_SIDCART_HARDSID_DEVICE_SELECTION_LABEL, 0 },
    { IDC_SIDCART_HARDSID_MAIN_DEVICE_LABEL, IDS_SIDCART_HARDSID_MAIN_DEVICE_LABEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param sidcart_normal_dialog_trans[] = {
    { IDC_SIDCART_ENABLE, IDS_SIDCART_ENABLE, 0 },
    { IDC_SIDCART_ENGINE_MODEL_LABEL, IDS_SID_GENGROUP1, 0 },
    { IDC_SIDCART_FILTERS, IDS_SID_FILTERS, 0 },
    { IDC_SIDCART_ADDRESS_LABEL, IDS_SIDCART_ADDRESS_LABEL, 0 },
    { IDC_SIDCART_CLOCK_LABEL, IDS_SIDCART_CLOCK_LABEL, 0 },
    { IDC_SIDCART_HARDSID_DEVICE_SELECTION_LABEL, IDS_SIDCART_HARDSID_DEVICE_SELECTION_LABEL, 0 },
    { IDC_SIDCART_HARDSID_MAIN_DEVICE_LABEL, IDS_SIDCART_HARDSID_MAIN_DEVICE_LABEL, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param sidcart_plus4_dialog_trans[] = {
    { IDC_DIGIBLASTER, IDS_DIGIBLASTER, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group sidcart_hardsid_main_group[] = {
    { IDC_SIDCART_ENABLE, 1 },
    { IDC_SIDCART_ENGINE_MODEL_LABEL, 0 },
    { IDC_SIDCART_FILTERS, 1 },
    { IDC_SIDCART_ADDRESS_LABEL, 0 },
    { IDC_SIDCART_CLOCK_LABEL, 0 },
    { IDC_SIDCART_HARDSID_DEVICE_SELECTION_LABEL, 0 },
    { IDC_SIDCART_HARDSID_MAIN_DEVICE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_normal_main_group[] = {
    { IDC_SIDCART_ENABLE, 1 },
    { IDC_SIDCART_ENGINE_MODEL_LABEL, 0 },
    { IDC_SIDCART_FILTERS, 1 },
    { IDC_SIDCART_ADDRESS_LABEL, 0 },
    { IDC_SIDCART_CLOCK_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_plus4_main_group[] = {
    { IDC_SIDCART_HARDSID_MAIN_DEVICE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_hardsid_left_group[] = {
    { IDC_SIDCART_ENGINE_MODEL_LABEL, 0 },
    { IDC_SIDCART_ADDRESS_LABEL, 0 },
    { IDC_SIDCART_CLOCK_LABEL, 0 },
    { IDC_SIDCART_HARDSID_MAIN_DEVICE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_normal_left_group[] = {
    { IDC_SIDCART_ENGINE_MODEL_LABEL, 0 },
    { IDC_SIDCART_ADDRESS_LABEL, 0 },
    { IDC_SIDCART_CLOCK_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_hardsid_right_group[] = {
    { IDC_SIDCART_ENGINE_MODEL, 0 },
    { IDC_SIDCART_ADDRESS, 0 },
    { IDC_SIDCART_CLOCK, 0 },
    { IDC_SIDCART_HARDSID_MAIN_DEVICE, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_normal_right_group[] = {
    { IDC_SIDCART_ENGINE_MODEL, 0 },
    { IDC_SIDCART_ADDRESS, 0 },
    { IDC_SIDCART_CLOCK, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_hardsid_window_group[] = {
    { IDC_SIDCART_ENABLE, 0 },
    { IDC_SIDCART_ENGINE_MODEL, 0 },
    { IDC_SIDCART_FILTERS, 0 },
    { IDC_SIDCART_ADDRESS, 0 },
    { IDC_SIDCART_CLOCK, 0 },
    { IDC_SIDCART_HARDSID_MAIN_DEVICE, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_normal_window_group[] = {
    { IDC_SIDCART_ENABLE, 0 },
    { IDC_SIDCART_ENGINE_MODEL, 0 },
    { IDC_SIDCART_FILTERS, 0 },
    { IDC_SIDCART_ADDRESS, 0 },
    { IDC_SIDCART_CLOCK, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_plus4_hardsid_window_group[] = {
    { IDC_SIDCART_ENABLE, 0 },
    { IDC_SIDCART_ENGINE_MODEL, 0 },
    { IDC_SIDCART_FILTERS, 0 },
    { IDC_SIDCART_ADDRESS, 0 },
    { IDC_SIDCART_CLOCK, 0 },
    { IDC_SIDCART_HARDSID_MAIN_DEVICE, 0 },
    { 0, 0 }
};

static uilib_dialog_group sidcart_plus4_normal_window_group[] = {
    { IDC_SIDCART_ENABLE, 0 },
    { IDC_SIDCART_ENGINE_MODEL, 0 },
    { IDC_SIDCART_FILTERS, 0 },
    { IDC_SIDCART_ADDRESS, 0 },
    { IDC_SIDCART_CLOCK, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_sidcart_dialog(HWND hwnd)
{
    HWND sid_hwnd;
    int res_value, i;
    int active_value;
    unsigned int available, device;
    int xpos;
    RECT rect;
    uilib_localize_dialog_param *sidcart_dialog_trans = (hardsid_available()) ? sidcart_hardsid_dialog_trans : sidcart_normal_dialog_trans;
    uilib_dialog_group *sidcart_main_group = (hardsid_available()) ? sidcart_hardsid_main_group : sidcart_normal_main_group;
    uilib_dialog_group *sidcart_left_group = (hardsid_available()) ? sidcart_hardsid_left_group : sidcart_normal_left_group;
    uilib_dialog_group *sidcart_right_group = (hardsid_available()) ? sidcart_hardsid_right_group : sidcart_normal_right_group;
    uilib_dialog_group *sidcart_window_group = (hardsid_available()) ? sidcart_hardsid_window_group : sidcart_normal_window_group;
    uilib_dialog_group *sidcart_plus4_window_group = (hardsid_available()) ? sidcart_plus4_hardsid_window_group : sidcart_plus4_normal_window_group;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, sidcart_dialog_trans);

    if (machine_class == VICE_MACHINE_PLUS4) {
        /* translate the plus4 extra item */
        uilib_localize_dialog(hwnd, sidcart_plus4_dialog_trans);
    }

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, sidcart_main_group);
  
    if (machine_class == VICE_MACHINE_PLUS4) {
        if (hardsid_available()) {
            /* adjust the size of the elements in the plus4 main group */
            uilib_adjust_group_width(hwnd, sidcart_plus4_main_group);
        }
    }

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, sidcart_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, sidcart_right_group, xpos + 10);

    /* get the max x of the left group */
    if (machine_class == VICE_MACHINE_PLUS4) {
        uilib_get_group_max_x(hwnd, sidcart_plus4_window_group, &xpos);
    } else {
        uilib_get_group_max_x(hwnd, sidcart_window_group, &xpos);
    }

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    switch (machine_class) {
        case VICE_MACHINE_PET:
            native_primary_sid_text_address = TEXT("$8F00");
            native_secondary_sid_text_address = TEXT("$E900");
            native_sid_clock = TEXT("PET");
            native_primary_sid_int_address = 0x8f00;
            native_secondary_sid_int_address = 0xe900;
            break;
        case VICE_MACHINE_PLUS4:
            native_primary_sid_text_address = TEXT("$FD40");
            native_secondary_sid_text_address = TEXT("$FE80");
            native_sid_clock = TEXT("PLUS4");
            native_primary_sid_int_address = 0xfd40;
            native_secondary_sid_int_address = 0xfe80;
            break;
        case VICE_MACHINE_VIC20:
            native_primary_sid_text_address = TEXT("$9800");
            native_secondary_sid_text_address = TEXT("$9C00");
            native_sid_clock = TEXT("VIC20");
            native_primary_sid_int_address = 0x9800;
            native_secondary_sid_int_address = 0x9c00;
            break;
    }

    resources_get_int("SidCart", &sel_cart_enabled);
    CheckDlgButton(hwnd, IDC_SIDCART_ENABLE, sel_cart_enabled ? BST_CHECKED : BST_UNCHECKED);

    ui_sid_engine_model_list = sid_get_engine_model_list();

    resources_get_int("SidModel", &res_value);
    resources_get_int("SidEngine", &sel_engine);
    res_value |= sel_engine << 8;
    sid_hwnd = GetDlgItem(hwnd, IDC_SIDCART_ENGINE_MODEL);

    active_value = 0;
    for (i = 0; ui_sid_engine_model_list[i]; i++) {
        TCHAR st[40];

        system_mbstowcs(st, ui_sid_engine_model_list[i]->name, 40);
        SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
        if (ui_sid_engine_model_list[i]->value == res_value) {
            active_value = i;
        }
    }
    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_int("SidFilters", &res_value);
    CheckDlgButton(hwnd, IDC_SIDCART_FILTERS, res_value ? BST_CHECKED : BST_UNCHECKED);

    sid_hwnd = GetDlgItem(hwnd, IDC_SIDCART_ADDRESS);

    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)native_primary_sid_text_address);
    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)native_secondary_sid_text_address);
    resources_get_int("SidAddress", &res_value);
    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)(res_value == native_secondary_sid_int_address), 0);

    sid_hwnd = GetDlgItem(hwnd, IDC_SIDCART_CLOCK);
    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("C64"));
    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)native_sid_clock);
    resources_get_int("SidClock", &res_value);
    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    if (machine_class == VICE_MACHINE_PLUS4) {
        resources_get_int("DIGIBLASTER", &res_value);
        CheckDlgButton(hwnd, IDC_DIGIBLASTER, res_value ? BST_CHECKED : BST_UNCHECKED);
    }

    available = hardsid_available();
    device = 0;

    if (available) {
        resources_get_int("SidHardSIDMain", &res_value);
        sid_hwnd = GetDlgItem(hwnd, IDC_SIDCART_HARDSID_MAIN_DEVICE);

        while (available > 0) {
            TCHAR item[10];

            _stprintf(item, TEXT("%d"), device++);
            SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)item);
            available--;
        }

        SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
    }

    enable_sidcart_controls(hwnd);
}

static void end_sidcart_dialog(HWND hwnd)
{
    int engine, model, temp;

    temp = ui_sid_engine_model_list[SendDlgItemMessage(hwnd, IDC_SIDCART_ENGINE_MODEL, CB_GETCURSEL, 0, 0)]->value;
    engine = temp >> 8;
    model = temp & 0xff;
    sid_set_engine_model(engine, model);
    resources_set_int("SidCart", (IsDlgButtonChecked(hwnd, IDC_SIDCART_ENABLE) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("SidFilters", (IsDlgButtonChecked(hwnd, IDC_SIDCART_FILTERS) == BST_CHECKED ? 1 : 0 ));
    resources_set_int("SidAddress", (SendMessage(GetDlgItem(hwnd, IDC_SIDCART_ADDRESS), CB_GETCURSEL, 0, 0) ? native_secondary_sid_int_address : native_primary_sid_int_address));
    resources_set_int("SidClock", (int)SendMessage(GetDlgItem(hwnd, IDC_SIDCART_CLOCK), CB_GETCURSEL, 0, 0));
    if (hardsid_available()) {
        resources_set_int("SidHardSIDMain", (int)SendMessage(GetDlgItem(hwnd, IDC_SIDCART_HARDSID_MAIN_DEVICE), CB_GETCURSEL, 0, 0));
    }

    if (machine_class == VICE_MACHINE_PLUS4) {
        resources_set_int("DIGIBLASTER", (IsDlgButtonChecked(hwnd, IDC_DIGIBLASTER) == BST_CHECKED ? 1 : 0 ));
    }
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int temp;

    switch (msg) {
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDC_SIDCART_ENGINE_MODEL:
                    temp = SendDlgItemMessage(hwnd, IDC_SIDCART_ENGINE_MODEL, CB_GETCURSEL, 0, 0);
                    sel_engine = (ui_sid_engine_model_list[temp]->value) >> 8;
                    enable_sidcart_hardsid_controls(hwnd);
                    break;
                case IDC_SIDCART_ENABLE:
                    sel_cart_enabled = (IsDlgButtonChecked(hwnd, IDC_SIDCART_ENABLE) == BST_CHECKED) ? 1 : 0;
                    enable_sidcart_controls(hwnd);
                    break;
                case IDOK:
                    end_sidcart_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_sidcart_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_sidcart_settings_dialog(HWND hwnd)
{
    if (machine_class == VICE_MACHINE_PLUS4) {
        if (hardsid_available()) {
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_SIDCARTPLUS4_HARDSID_SETTINGS_DIALOG, hwnd, dialog_proc);
        } else {
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_SIDCARTPLUS4_SETTINGS_DIALOG, hwnd, dialog_proc);
        }
    } else {
        if (hardsid_available()) {
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_SIDCART_HARDSID_SETTINGS_DIALOG, hwnd, dialog_proc);
        } else {
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_SIDCART_SETTINGS_DIALOG, hwnd, dialog_proc);
        }
    }
}
