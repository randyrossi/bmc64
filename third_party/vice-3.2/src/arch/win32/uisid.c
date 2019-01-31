/*
 * uisid.c - Implementation of the C64 and C128 SID settings dialog box.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Andreas Boose <viceteam@t-online.de>
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
#include <tchar.h>
#include <windows.h>
#include <prsht.h>

#include "catweaselmkiii.h"
#include "hardsid.h"
#include "intl.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "sid.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uisid.h"
#include "winmain.h"
#include "uilib.h"

static sid_engine_model_t **ui_sid_engine_model_list;

static const int ui_sid_samplemethod[] = {
    IDS_FAST,
    IDS_INTERPOLATING,
    IDS_RESAMPLING,
    IDS_FAST_RESAMPLING,
    0
};

static const int *ui_sid_baseaddress;

/* Values currently selected in the UI
   Needed to enable/disable UI elements */
static int sel_engine, sel_extra_sids;

static void enable_general_sid_controls(HWND hwnd)
{
    EnableWindow(GetDlgItem(hwnd, IDC_SID_STEREOADDRESS), sel_extra_sids >= 1);
    EnableWindow(GetDlgItem(hwnd, IDC_SID_TRIPLEADDRESS), sel_extra_sids >= 2);
    EnableWindow(GetDlgItem(hwnd, IDC_SID_QUADADDRESS), sel_extra_sids >= 3);
}

static void enable_resid_sid_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = sel_engine == SID_ENGINE_RESID;

    EnableWindow(GetDlgItem(hwnd, IDC_SID_RESID_SAMPLING), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SID_RESID_PASSBAND_VALUE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SID_RESID_GAIN_VALUE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SID_RESID_FILTER_BIAS_VALUE), is_enabled);
}

static void enable_hardsid_sid_controls(HWND hwnd)
{
    int is_enabled;

    is_enabled = (sel_engine == SID_ENGINE_HARDSID) && (hardsid_available() > 0);

    EnableWindow(GetDlgItem(hwnd, IDC_SID_HARDSID_LEFT_ENGINE), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_SID_HARDSID_RIGHT_ENGINE), is_enabled && (sel_extra_sids >= 1));
}

static void CreateAndGetSidAddress(HWND sid_hwnd, int mode, int cur_value, char *resource)
{
    /* mode=0: Create - cur_value = value of resource */
    /* mode=1: Get - cur_value = index of selected item */
    TCHAR st[12];
    int adr, ladr, hi, index = -1;
    const int *hadr = ui_sid_baseaddress;

    for (hi = 0; hadr[hi] >= 0; hi++) {
        for (ladr = (hi > 0 ? 0x0 : 0x20); ladr < 0x100; ladr += 0x20) {
            index++;
            adr = hadr[hi] * 0x100 + ladr;

            if (mode == 0) {
                _stprintf(st, TEXT("$%04X"), adr);
                SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
                if (adr == cur_value) {
                    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)index, 0);
                }
            } else {
                if (index == cur_value) {
                    resources_set_int(resource, adr);
                    return;
                }
            }
        }
    }
}

static uilib_localize_dialog_param general_sid_dialog_trans[] = {
    { IDC_SID_GENGROUP1, IDS_SID_GENGROUP1, 0 },
    { IDC_SID_GENGROUP3, IDS_SID_GENGROUP3, 0 },
    { IDC_SID_EXTRA_AMOUNT_LABEL, IDS_SID_EXTRA_AMOUNT_LABEL, 0 },
    { IDC_SID_STEREOADDRESS_LABEL, IDS_SID_STEREOADDRESS_LABEL, 0 },
    { IDC_SID_TRIPLEADDRESS_LABEL, IDS_SID_TRIPLEADDRESS_LABEL, 0 },
    { IDC_SID_QUADADDRESS_LABEL, IDS_SID_QUADADDRESS_LABEL, 0 },
    { IDC_SID_FILTERS, IDS_SID_FILTERS, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param parent_dialog_trans[] = {
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static void init_general_sid_dialog(HWND hwnd)
{
    HWND sid_hwnd, parent_hwnd;
    int res_value, i;
    int active_value;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, general_sid_dialog_trans);

    parent_hwnd = GetParent(hwnd);

    /* translate the parent window items */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    /* Setup status */
    resources_get_int("SidFilters", &res_value);
    CheckDlgButton(hwnd, IDC_SID_FILTERS, res_value ? BST_CHECKED : BST_UNCHECKED);
    
    resources_get_int("SidStereo", &sel_extra_sids);
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_EXTRA_AMOUNT);
    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("0"));
    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("1"));
    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("2"));
    SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)TEXT("3"));
    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)sel_extra_sids, 0);

    resources_get_int("SidStereoAddressStart", &res_value);
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_STEREOADDRESS);
    CreateAndGetSidAddress(sid_hwnd, 0, res_value, "SidStereoAddressStart");

    resources_get_int("SidTripleAddressStart", &res_value);
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_TRIPLEADDRESS);
    CreateAndGetSidAddress(sid_hwnd, 0, res_value, "SidTripleAddressStart");

    resources_get_int("SidQuadAddressStart", &res_value);
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_QUADADDRESS);
    CreateAndGetSidAddress(sid_hwnd, 0, res_value, "SidQuadAddressStart");

    ui_sid_engine_model_list = sid_get_engine_model_list();

    resources_get_int("SidModel", &res_value);
    resources_get_int("SidEngine", &sel_engine);
    res_value |= sel_engine << 8;
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_ENGINE_MODEL);

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

    enable_general_sid_controls(hwnd);
}

union rect_point_s {
    RECT rect;
    POINT point;
};
typedef union rect_point_s rect_point_u;

static void resize_general_sid_dialog(HWND hwnd)
{
    int xsize, ysize;
    HWND child_hwnd;
    RECT rect;
    rect_point_u child_rect;
    int new_xpos = 0;
    int xpos;

    GetClientRect(hwnd, &rect);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_GENGROUP1);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    MoveWindow(child_hwnd, child_rect.rect.left, child_rect.rect.top, rect.right - 2 * child_rect.rect.left, child_rect.rect.bottom - child_rect.rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_EXTRA_AMOUNT_LABEL);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.rect.left, child_rect.rect.top, xsize + 20, child_rect.rect.bottom - child_rect.rect.top, TRUE);
    new_xpos = child_rect.rect.left + xsize + 20 + 10;

    child_hwnd = GetDlgItem(hwnd, IDC_SID_STEREOADDRESS_LABEL);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.rect.left, child_rect.rect.top, xsize + 20, child_rect.rect.bottom - child_rect.rect.top, TRUE);
    xpos = child_rect.rect.left + xsize + 20 + 10;

    if (xpos > new_xpos) {
        new_xpos = xpos;
    }

    child_hwnd = GetDlgItem(hwnd, IDC_SID_TRIPLEADDRESS_LABEL);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.rect.left, child_rect.rect.top, xsize + 20, child_rect.rect.bottom - child_rect.rect.top, TRUE);
    xpos = child_rect.rect.left + xsize + 20 + 10;

    if (xpos > new_xpos) {
        new_xpos = xpos;
    }

    child_hwnd = GetDlgItem(hwnd, IDC_SID_QUADADDRESS_LABEL);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.rect.left, child_rect.rect.top, xsize + 20, child_rect.rect.bottom - child_rect.rect.top, TRUE);
    xpos = child_rect.rect.left + xsize + 20 + 10;

    if (xpos > new_xpos) {
        new_xpos = xpos;
    }

    child_hwnd = GetDlgItem(hwnd, IDC_SID_EXTRA_AMOUNT);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    MoveWindow(child_hwnd, new_xpos, child_rect.rect.top, child_rect.rect.right - child_rect.rect.left, child_rect.rect.bottom - child_rect.rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_STEREOADDRESS);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    MoveWindow(child_hwnd, new_xpos, child_rect.rect.top, child_rect.rect.right - child_rect.rect.left, child_rect.rect.bottom - child_rect.rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_TRIPLEADDRESS);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    MoveWindow(child_hwnd, new_xpos, child_rect.rect.top, child_rect.rect.right - child_rect.rect.left, child_rect.rect.bottom - child_rect.rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_QUADADDRESS);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    MoveWindow(child_hwnd, new_xpos, child_rect.rect.top, child_rect.rect.right - child_rect.rect.left, child_rect.rect.bottom - child_rect.rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_GENGROUP3);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    MoveWindow(child_hwnd, child_rect.rect.left, child_rect.rect.top, rect.right - 2 * child_rect.rect.left, child_rect.rect.bottom - child_rect.rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_FILTERS);
    GetClientRect(child_hwnd, &child_rect.rect);
    MapWindowPoints(child_hwnd, hwnd, &child_rect.point, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.rect.left, child_rect.rect.top, xsize + 20, child_rect.rect.bottom - child_rect.rect.top, TRUE);
}

static uilib_localize_dialog_param resid_sid_dialog_trans[] = {
    { IDC_SID_RESID_GROUP, IDS_SID_RESID_GROUP, 0 },
    { IDC_SID_RESID_SAMPLE, IDS_SID_RESID_SAMPLE, 0 },
    { IDC_SID_RESID_PASSBAND, IDS_SID_RESID_PASSBAND, 0 },
    { IDC_SID_RESID_GAIN, IDS_SID_RESID_GAIN, 0 },
    { IDC_SID_RESID_FILTER_BIAS, IDS_SID_RESID_FILTER_BIAS, 0 },
    { 0, 0, 0 }
};

static void init_resid_sid_dialog(HWND hwnd)
{
    HWND sid_hwnd;
#if defined(HAVE_RESID) || defined(HAVE_RESID_DTV)
    int res_value, i;
    TCHAR st[10];
#endif

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, resid_sid_dialog_trans);

#if defined(HAVE_RESID) || defined(HAVE_RESID_DTV)
    resources_get_int("SidResidSampling", &res_value);
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_SAMPLING);
    for (i = 0; ui_sid_samplemethod[i]; i++) {
        SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(ui_sid_samplemethod[i]));
    }
    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("SidResidPassband", &res_value);
    _stprintf(st, TEXT("%d"), res_value);
    SetDlgItemText(hwnd, IDC_SID_RESID_PASSBAND_VALUE, st);

    resources_get_int("SidResidGain", &res_value);
    _stprintf(st, TEXT("%d"), res_value);
    SetDlgItemText(hwnd, IDC_SID_RESID_GAIN_VALUE, st);

    resources_get_int("SidResidFilterBias", &res_value);
    _stprintf(st, TEXT("%d"), res_value);
    SetDlgItemText(hwnd, IDC_SID_RESID_FILTER_BIAS_VALUE, st);
#endif
    enable_resid_sid_controls(hwnd);
}

static void resize_resid_sid_dialog(HWND hwnd)
{
    int xsize, ysize;
    HWND child_hwnd;
    RECT rect;
    RECT child_rect;
    int xpos;

    GetClientRect(hwnd, &rect);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_GROUP);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, rect.right - 2 * child_rect.left, rect.bottom - 2 * child_rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_SAMPLE);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    xpos = child_rect.left + xsize + 10;

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_PASSBAND);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_GAIN);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_FILTER_BIAS);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_SAMPLING);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_PASSBAND_VALUE);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_GAIN_VALUE);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_RESID_FILTER_BIAS_VALUE);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
}

static uilib_localize_dialog_param hardsid_sid_dialog_trans[] = {
    { IDC_HARDSID_GROUP, IDS_HARDSID_GROUP, 0 },
    { IDC_HARDSID_LEFT_LABEL, IDS_HARDSID_LEFT_LABEL, 0 },
    { IDC_HARDSID_RIGHT_LABEL, IDS_HARDSID_RIGHT_LABEL, 0 },
    { 0, 0, 0 }
};

static void init_hardsid_sid_dialog(HWND hwnd)
{
    HWND sid_hwnd;
    int res_value;
    unsigned int available, device;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, hardsid_sid_dialog_trans);

    available = hardsid_available();
    device = 0;

    resources_get_int("SidHardSIDMain", &res_value);
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_HARDSID_LEFT_ENGINE);

    while (available > 0) {
        TCHAR item[10];

        _stprintf(item, TEXT("%d"), device++);
        SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)item);
        available--;
    }

    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    available = hardsid_available();
    device = 0;

    resources_get_int("SidHardSIDRight", &res_value);
    sid_hwnd = GetDlgItem(hwnd, IDC_SID_HARDSID_RIGHT_ENGINE);

    while (available > 0) {
        TCHAR item[10];

        _stprintf(item, TEXT("%d"), device++);
        SendMessage(sid_hwnd, CB_ADDSTRING, 0, (LPARAM)item);
        available--;
    }

    SendMessage(sid_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    enable_hardsid_sid_controls(hwnd);
}

static void resize_hardsid_sid_dialog(HWND hwnd)
{
    int xsize, ysize;
    HWND child_hwnd;
    RECT rect;
    RECT child_rect;
    int xpos;

    GetClientRect(hwnd, &rect);

    child_hwnd = GetDlgItem(hwnd, IDC_HARDSID_GROUP);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, rect.right - 2 * child_rect.left, rect.bottom - 2 * child_rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_HARDSID_LEFT_LABEL);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    xpos = child_rect.left + xsize + 10;

    child_hwnd = GetDlgItem(hwnd, IDC_HARDSID_RIGHT_LABEL);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    uilib_get_general_window_extents(child_hwnd, &xsize, &ysize);
    MoveWindow(child_hwnd, child_rect.left, child_rect.top, xsize, child_rect.bottom - child_rect.top, TRUE);
    if (xpos < child_rect.left + xsize + 10) {
        xpos = child_rect.left + xsize + 10;
    }

    child_hwnd = GetDlgItem(hwnd, IDC_SID_HARDSID_LEFT_ENGINE);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);

    child_hwnd = GetDlgItem(hwnd, IDC_SID_HARDSID_RIGHT_ENGINE);
    GetClientRect(child_hwnd, &child_rect);
    MapWindowPoints(child_hwnd, hwnd, (POINT*)&child_rect, 2);
    MoveWindow(child_hwnd, xpos, child_rect.top, child_rect.right - child_rect.left, child_rect.bottom - child_rect.top, TRUE);
}

static void end_general_dialog(HWND hwnd)
{
    int engine, model, temp;
    HWND sid_hwnd;

    resources_set_int("SidFilters", (IsDlgButtonChecked(hwnd, IDC_SID_FILTERS) == BST_CHECKED ? 1 : 0));

    sid_hwnd = GetDlgItem(hwnd, IDC_SID_EXTRA_AMOUNT);
    resources_set_int("SidStereo", (int)SendMessage(sid_hwnd, CB_GETCURSEL, 0, 0));

    sid_hwnd = GetDlgItem(hwnd, IDC_SID_STEREOADDRESS);
    CreateAndGetSidAddress(sid_hwnd, 1, (int)SendMessage(sid_hwnd, CB_GETCURSEL, 0, 0), "SidStereoAddressStart");

    sid_hwnd = GetDlgItem(hwnd, IDC_SID_TRIPLEADDRESS);
    CreateAndGetSidAddress(sid_hwnd, 1, (int)SendMessage(sid_hwnd, CB_GETCURSEL, 0, 0), "SidTripleAddressStart");

    sid_hwnd = GetDlgItem(hwnd, IDC_SID_QUADADDRESS);
    CreateAndGetSidAddress(sid_hwnd, 1, (int)SendMessage(sid_hwnd, CB_GETCURSEL, 0, 0), "SidQuadAddressStart");

    temp = ui_sid_engine_model_list[SendDlgItemMessage(hwnd, IDC_SID_ENGINE_MODEL, CB_GETCURSEL, 0, 0)]->value;
    engine = temp >> 8;
    model = temp & 0xff;
    sid_set_engine_model(engine, model);
}

static INT_PTR CALLBACK general_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int temp;

    switch (msg) {
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDC_SID_ENGINE_MODEL:
                    temp = SendDlgItemMessage(hwnd, IDC_SID_ENGINE_MODEL, CB_GETCURSEL, 0, 0);
                    sel_engine = (ui_sid_engine_model_list[temp]->value) >> 8;
                    break;
                case IDC_SID_EXTRA_AMOUNT:
                    sel_extra_sids = SendDlgItemMessage(hwnd, IDC_SID_EXTRA_AMOUNT, CB_GETCURSEL, 0, 0);
                    enable_general_sid_controls(hwnd);
                    break;
            }
            return FALSE;
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lparam)->code) {
                case PSN_APPLY:
                    end_general_dialog(hwnd);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_general_sid_dialog(hwnd);
            return TRUE;
        case WM_SIZE:
            resize_general_sid_dialog(hwnd);
            break;
    }
    return FALSE;
}

static void end_resid_dialog(HWND hwnd)
{
#if defined(HAVE_RESID) || defined(HAVE_RESID_DTV)
    TCHAR st[10];
    int temp_val, res_val;

    res_val = (int)SendDlgItemMessage(hwnd, IDC_SID_RESID_SAMPLING, CB_GETCURSEL, 0, 0);
    resources_set_int("SidResidSampling", res_val);

    GetDlgItemText(hwnd, IDC_SID_RESID_PASSBAND_VALUE, st, 4);
    temp_val = _ttoi(st);
    if (temp_val < 0) {
        res_val = 0;
    } else if (temp_val > 90) {
        res_val = 90;
    } else {
        res_val = temp_val;
    }
    
    if (temp_val != res_val) {
        ui_error(translate_text(IDS_VAL_D_FOR_S_OUT_RANGE_USE_D), temp_val, translate_text(IDS_SID_RESID_PASSBAND), res_val);
    }
    resources_set_int("SidResidPassband", res_val);

    GetDlgItemText(hwnd, IDC_SID_RESID_GAIN_VALUE, st, 4);
    temp_val = _ttoi(st);
    if (temp_val < 90) {
        res_val = 90;
    } else if (temp_val > 100) {
        res_val = 100;
    } else {
        res_val = temp_val;
    }
    
    if (temp_val != res_val) {
        ui_error(translate_text(IDS_VAL_D_FOR_S_OUT_RANGE_USE_D), temp_val, translate_text(IDS_SID_RESID_GAIN), res_val);
    }
    resources_set_int("SidResidGain", res_val);

    GetDlgItemText(hwnd, IDC_SID_RESID_FILTER_BIAS_VALUE, st, 6);
    temp_val = _ttoi(st);
    if (temp_val < -5000) {
        res_val = -5000;
    } else if (temp_val > 5000) {
        res_val = 5000;
    } else {
        res_val = temp_val;
    }

    if (temp_val != res_val) {
        ui_error(translate_text(IDS_VAL_D_FOR_S_OUT_RANGE_USE_D), temp_val, translate_text(IDS_SID_RESID_FILTER_BIAS), res_val);
    }
    resources_set_int("SidResidFilterBias", res_val);
#endif
}

static INT_PTR CALLBACK resid_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lparam)->code) {
                case PSN_SETACTIVE:
                    enable_resid_sid_controls(hwnd);
                    return TRUE;
                case PSN_APPLY:
                    end_resid_dialog(hwnd);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_resid_sid_dialog(hwnd);
            return TRUE;
        case WM_SIZE:
            resize_resid_sid_dialog(hwnd);
            break;
    }
    return FALSE;
}

static void end_hardsid_dialog(HWND hwnd)
{
    resources_set_int("SidHardSIDMain", (int)SendMessage(GetDlgItem(hwnd, IDC_SID_HARDSID_LEFT_ENGINE), CB_GETCURSEL, 0, 0));
    resources_set_int("SidHardSIDRight", (int)SendMessage(GetDlgItem(hwnd, IDC_SID_HARDSID_RIGHT_ENGINE), CB_GETCURSEL, 0, 0));
}

static INT_PTR CALLBACK hardsid_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lparam)->code) {
                case PSN_SETACTIVE:
                    enable_hardsid_sid_controls(hwnd);
                    return TRUE;
                case PSN_APPLY:
                    end_hardsid_dialog(hwnd);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_hardsid_sid_dialog(hwnd);
            return TRUE;
        case WM_SIZE:
            resize_hardsid_sid_dialog(hwnd);
            break;
    }
    return FALSE;
}

void ui_sid_settings_dialog(HWND hwnd, const int *stereo_baseaddress)
{
    PROPSHEETPAGE psp[3];
    PROPSHEETHEADER psh;

    ui_sid_baseaddress = stereo_baseaddress;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[0].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_SID_GENERAL_SETTINGS_DIALOG);
    psp[0].pszIcon = NULL;
#else
    psp[0].u1.pszTemplate = MAKEINTRESOURCE(IDD_SID_GENERAL_SETTINGS_DIALOG);
    psp[0].u2.pszIcon = NULL;
#endif
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[1].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_SID_RESID_SETTINGS_DIALOG);
    psp[1].pszIcon = NULL;
#else
    psp[1].u1.pszTemplate = MAKEINTRESOURCE(IDD_SID_RESID_SETTINGS_DIALOG);
    psp[1].u2.pszIcon = NULL;
#endif
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    if (hardsid_available()) {
        psp[2].dwSize = sizeof(PROPSHEETPAGE);
        psp[2].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
        psp[2].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
        psp[2].pszTemplate = MAKEINTRESOURCE(translate_res(IDD_SID_HARDSID_SETTINGS_DIALOG));
        psp[2].pszIcon = NULL;
#else
        psp[2].u1.pszTemplate = MAKEINTRESOURCE(translate_res(IDD_SID_HARDSID_SETTINGS_DIALOG));
        psp[2].u2.pszIcon = NULL;
#endif
        psp[2].lParam = 0;
        psp[2].pfnCallback = NULL;
    }

    psp[0].pfnDlgProc = general_dialog_proc;
    psp[0].pszTitle = intl_translate_tcs(IDS_GENERAL);
    psp[1].pfnDlgProc = resid_dialog_proc;
    psp[1].pszTitle = TEXT("ReSID");
    if (hardsid_available()) {
        psp[2].pfnDlgProc = hardsid_dialog_proc;
        psp[2].pszTitle = TEXT("HardSID");
    }

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = winmain_instance;
    psh.pszCaption = intl_translate_tcs(IDS_SID_SETTINGS);
    if (hardsid_available()) {
        psh.nPages = 3;
    } else {
        psh.nPages = 2;
    }
#ifdef _ANONYMOUS_UNION
    psh.pszIcon = NULL;
    psh.nStartPage = 0;
    psh.ppsp = psp;
#else
    psh.u1.pszIcon = NULL;
    psh.u2.nStartPage = 0;
    psh.u3.ppsp = psp;
#endif
    psh.pfnCallback = NULL;

    PropertySheet(&psh);
}
