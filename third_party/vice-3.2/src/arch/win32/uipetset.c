/*
 * uipetset.c - Implementation of PET settings dialog box.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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
#include <prsht.h>

#include "intl.h"
#include "pets.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uilib.h"
#include "uipetset.h"
#include "winlong.h"
#include "winmain.h"

static uilib_localize_dialog_param mem_dialog_trans[] = {
    { IDC_PET_MEMORY, IDS_MEMORY, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param parent_dialog_trans[] = {
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group mem_main_group[] = {
    { IDC_PET_MEMORY, 1 },
    { 0, 0 }
};

static uilib_dialog_group mem_left_group[] = {
    { IDC_PET_MEMORY, 0 },
    { IDC_SELECT_PET_MEM4K, 0 },
    { IDC_SELECT_PET_MEM8K, 0 },
    { IDC_SELECT_PET_MEM16K, 0 },
    { IDC_SELECT_PET_MEM32K, 0 },
    { IDC_SELECT_PET_MEM96K, 0 },
    { IDC_SELECT_PET_MEM128K, 0 },
    { 0, 0 }
};

static uilib_dialog_group mem_left_move_group[] = {
    { IDC_SELECT_PET_MEM4K, 0 },
    { IDC_SELECT_PET_MEM8K, 0 },
    { IDC_SELECT_PET_MEM16K, 0 },
    { IDC_SELECT_PET_MEM32K, 0 },
    { IDC_SELECT_PET_MEM96K, 0 },
    { IDC_SELECT_PET_MEM128K, 0 },
    { 0, 0 }
};

static generic_trans_table_t generic_items[] = {
    { IDC_SELECT_PET_MEM4K,   TEXT("4KB")   },
    { IDC_SELECT_PET_MEM8K,   TEXT("8KB")   },
    { IDC_SELECT_PET_MEM16K,  TEXT("16KB")  },
    { IDC_SELECT_PET_MEM32K,  TEXT("32KB")  },
    { IDC_SELECT_PET_MEM96K,  TEXT("96KB")  },
    { IDC_SELECT_PET_MEM128K, TEXT("128KB") },
    { 0, NULL }
};

static void init_mem_dialog(HWND hwnd)
{
    int n, res;
    HWND parent_hwnd;
    HWND element;
    int xstart, xpos;

    parent_hwnd = GetParent(hwnd);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, mem_dialog_trans);

    /* translate all dialog items of the parent */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    /* translate all generic items */
    for (n = 0; generic_items[n].text != NULL; n++) {
        element = GetDlgItem(hwnd, generic_items[n].idm);
        SetWindowText(element, generic_items[n].text);
    }

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, mem_main_group);

    /* get the min x of the left move group */
    uilib_get_group_min_x(hwnd, mem_left_move_group, &xstart);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, mem_left_group, &xpos);

    /* resize and move the group box to the correct position */
    uilib_move_and_set_element_width(hwnd, IDC_PET_MEMORY, xstart - 10, xpos - xstart + 20);

    resources_get_int("RamSize", &res);
    switch (res) {
        case 4:
            n = IDC_SELECT_PET_MEM4K;
            break;
        case 8:
            n = IDC_SELECT_PET_MEM8K;
            break;
        case 16:
            n = IDC_SELECT_PET_MEM16K;
            break;
        case 32:
            n = IDC_SELECT_PET_MEM32K;
            break;
        case 96:
            n = IDC_SELECT_PET_MEM96K;
            break;
        case 128:
            n = IDC_SELECT_PET_MEM128K;
            break;
    }
    CheckRadioButton(hwnd, IDC_SELECT_PET_MEM4K, IDC_SELECT_PET_MEM128K, n);
}

static uilib_localize_dialog_param io_dialog_trans[] = {
    { IDC_PET_VIDEO, IDS_PET_VIDEO, 0 },
    { IDC_SELECT_PET_VIDEO_AUTO, IDS_SELECT_PET_VIDEO_AUTO, 0 },
    { IDC_SELECT_PET_VIDEO_40, IDS_SELECT_PET_VIDEO_40, 0 },
    { IDC_SELECT_PET_VIDEO_80, IDS_SELECT_PET_VIDEO_80, 0 },
    { IDC_PET_IO_SIZE, IDS_PET_IO_SIZE, 0 },
    { IDC_SELECT_PET_IO256, IDS_SELECT_PET_IO256, 0 },
    { IDC_SELECT_PET_IO2K, IDS_SELECT_PET_IO2K, 0 },
    { IDC_TOGGLE_PET_CRTC, IDS_TOGGLE_PET_CRTC, 0 },
    { IDC_PET_KEYBOARD, IDS_PET_KEYBOARD, 0 },
    { IDC_SELECT_PET_KEYB_GRAPHICS, IDS_SELECT_PET_KEYB_GRAPHICS, 0 },
    { IDC_SELECT_PET_KEYB_BUSINESS, IDS_SELECT_PET_KEYB_BUSINESS, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group io_main_group[] = {
    { IDC_PET_VIDEO, 1 },
    { IDC_SELECT_PET_VIDEO_AUTO, 1 },
    { IDC_SELECT_PET_VIDEO_40, 1 },
    { IDC_SELECT_PET_VIDEO_80, 1 },
    { IDC_PET_IO_SIZE, 1 },
    { IDC_SELECT_PET_IO256, 1 },
    { IDC_SELECT_PET_IO2K, 1 },
    { IDC_PET_CRTC, 1 },
    { IDC_TOGGLE_PET_CRTC, 1 },
    { IDC_PET_KEYBOARD, 1 },
    { IDC_SELECT_PET_KEYB_GRAPHICS, 1 },
    { IDC_SELECT_PET_KEYB_BUSINESS, 1 },
    { 0, 0 }
};

static uilib_dialog_group io_left_group[] = {
    { IDC_PET_VIDEO, 0 },
    { IDC_SELECT_PET_VIDEO_AUTO, 0 },
    { IDC_SELECT_PET_VIDEO_40, 0 },
    { IDC_SELECT_PET_VIDEO_80, 0 },
    { IDC_PET_IO_SIZE, 0 },
    { IDC_SELECT_PET_IO256, 0 },
    { IDC_SELECT_PET_IO2K, 0 },
    { 0, 0 }
};

static uilib_dialog_group io_right_move_group[] = {
    { IDC_TOGGLE_PET_CRTC, 0 },
    { IDC_SELECT_PET_KEYB_GRAPHICS, 0 },
    { IDC_SELECT_PET_KEYB_BUSINESS, 0 },
    { 0, 0 }
};

static uilib_dialog_group io_right_group[] = {
    { IDC_PET_CRTC, 0 },
    { IDC_TOGGLE_PET_CRTC, 0 },
    { IDC_PET_KEYBOARD, 0 },
    { IDC_SELECT_PET_KEYB_GRAPHICS, 0 },
    { IDC_SELECT_PET_KEYB_BUSINESS, 0 },
    { 0, 0 }
};

static generic_trans_table_t generic_items2[] = {
    { IDC_PET_CRTC, TEXT("&CRTC") },
    { 0, NULL }
};

static void init_io_dialog(HWND hwnd)
{
    int n, res;
    HWND parent_hwnd;
    HWND element;
    int xpos;
    int xstart;

    parent_hwnd = GetParent(hwnd);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, io_dialog_trans);

    /* translate all dialog items of the parent */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    /* translate all generic items */
    for (n = 0; generic_items2[n].text != NULL; n++) {
        element = GetDlgItem(hwnd, generic_items2[n].idm);
        SetWindowText(element, generic_items2[n].text);
    }

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, io_main_group);

    /* get the min x of the video auto element */
    uilib_get_element_min_x(hwnd, IDC_SELECT_PET_VIDEO_AUTO, &xstart);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, io_left_group, &xpos);

    /* resize and move the left group boxes to the correct positions */
    uilib_move_and_set_element_width(hwnd, IDC_PET_VIDEO, xstart - 10, xpos - xstart + 20);
    uilib_move_and_set_element_width(hwnd, IDC_PET_IO_SIZE, xstart - 10, xpos - xstart + 20);

    /* get the max x of the video group element */
    uilib_get_element_max_x(hwnd, IDC_PET_VIDEO, &xpos);

    /* move the right move group to the correct position */
    uilib_move_group(hwnd, io_right_move_group, xpos + 20);

    /* move the right group boxes to the correct position */
    uilib_move_element(hwnd, IDC_PET_CRTC, xpos + 10);
    uilib_move_element(hwnd, IDC_PET_KEYBOARD, xpos + 10);

    xstart = xpos + 20;

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, io_right_group, &xpos);

    /* resize and move the right group boxes to the correct positions */
    uilib_move_and_set_element_width(hwnd, IDC_PET_CRTC, xstart - 10, xpos - xstart + 20);
    uilib_move_and_set_element_width(hwnd, IDC_PET_KEYBOARD, xstart - 10, xpos - xstart + 20);

    resources_get_int("Crtc", &n);
    CheckDlgButton(hwnd, IDC_TOGGLE_PET_CRTC,
                   n ? BST_CHECKED : BST_UNCHECKED);

    resources_get_int("VideoSize", &res);
    switch (res) {
        case 0:
            n = IDC_SELECT_PET_VIDEO_AUTO;
            break;
        case 40:
            n = IDC_SELECT_PET_VIDEO_40;
            break;
        case 80:
            n = IDC_SELECT_PET_VIDEO_80;
            break;
    }
    CheckRadioButton(hwnd, IDC_SELECT_PET_VIDEO_AUTO, IDC_SELECT_PET_VIDEO_80, n);

    resources_get_int("IOSize", &res);
    switch (res) {
        case 0x100:
            n = IDC_SELECT_PET_IO256;
            break;
        case 0x800:
            n = IDC_SELECT_PET_IO2K;
            break;
    }
    CheckRadioButton(hwnd, IDC_SELECT_PET_IO2K, IDC_SELECT_PET_IO256, n);

    resources_get_int("KeymapIndex", &res);
    switch (res) {
        case 2:
            n = IDC_SELECT_PET_KEYB_GRAPHICS;
            break;
        case 0:
            n = IDC_SELECT_PET_KEYB_BUSINESS;
            break;
    }
    CheckRadioButton(hwnd, IDC_SELECT_PET_KEYB_GRAPHICS, IDC_SELECT_PET_KEYB_BUSINESS, n);
}

static uilib_localize_dialog_param superpet_io_dialog_trans[] = {
    { IDC_PET_IO_SETTINGS, IDS_PET_IO_SETTINGS, 0 },
    { IDC_TOGGLE_PET_SUPER_IO_ENABLE, IDS_TOGGLE_PET_SUPER_IO_ENABLE, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group superpet_io_main_group[] = {
    { IDC_PET_IO_SETTINGS, 1 },
    { IDC_TOGGLE_PET_SUPER_IO_ENABLE, 1 },
    { 0, 0 }
};

static void init_superpet_io_dialog(HWND hwnd)
{
    int n;
    int xstart;
    int xpos;
    HWND parent_hwnd;

    parent_hwnd = GetParent(hwnd);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, superpet_io_dialog_trans);

    /* translate all dialog items of the parent */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, superpet_io_main_group);

    /* get the min x of the io enable element */
    uilib_get_element_min_x(hwnd, IDC_TOGGLE_PET_SUPER_IO_ENABLE, &xstart);

    /* get the max x of the main group */
    uilib_get_group_max_x(hwnd, superpet_io_main_group, &xpos);

    /* resize and move the group box to the correct position */
    uilib_move_and_set_element_width(hwnd, IDC_PET_IO_SETTINGS, xstart - 10, xpos - xstart + 20);

    resources_get_int("SuperPET", &n);
    CheckDlgButton(hwnd, IDC_TOGGLE_PET_SUPER_IO_ENABLE, n ? BST_CHECKED : BST_UNCHECKED);
}

static uilib_localize_dialog_param pet8296_dialog_trans[] = {
    { IDC_PET_RAM_SETTINGS, IDS_PET_RAM_SETTINGS, 0 },
    { IDC_TOGGLE_PET_8296_RAM9, IDS_TOGGLE_PET_8296_RAM9, 0 },
    { IDC_TOGGLE_PET_8296_RAMA, IDS_TOGGLE_PET_8296_RAMA, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group pet8296_main_group[] = {
    { IDC_PET_RAM_SETTINGS, 1 },
    { IDC_TOGGLE_PET_8296_RAM9, 1 },
    { IDC_TOGGLE_PET_8296_RAMA, 1 },
    { 0, 0 }
};

static void init_pet8296_dialog(HWND hwnd)
{
    int n;
    int xstart;
    int xpos;
    HWND parent_hwnd;

    parent_hwnd = GetParent(hwnd);

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, pet8296_dialog_trans);

    /* translate all dialog items of the parent */
    uilib_localize_dialog(parent_hwnd, parent_dialog_trans);

    /* adjust the size of the elements in the main group */
    uilib_adjust_group_width(hwnd, pet8296_main_group);

    /* get the min x of the ram 9 element */
    uilib_get_element_min_x(hwnd, IDC_TOGGLE_PET_8296_RAM9, &xstart);

    /* get the max x of the main group */
    uilib_get_group_max_x(hwnd, pet8296_main_group, &xpos);

    /* resize and move the group box to the correct position */
    uilib_move_and_set_element_width(hwnd, IDC_PET_RAM_SETTINGS, xstart - 10, xpos - xstart + 20);

    resources_get_int("Ram9", &n);
    CheckDlgButton(hwnd, IDC_TOGGLE_PET_8296_RAM9, n ? BST_CHECKED : BST_UNCHECKED);
    resources_get_int("RamA", &n);
    CheckDlgButton(hwnd, IDC_TOGGLE_PET_8296_RAMA, n ? BST_CHECKED : BST_UNCHECKED);
}

static INT_PTR CALLBACK mem_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_MEM4K) == BST_CHECKED) {
                    resources_set_int("RamSize", 4);
                } else if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_MEM8K) == BST_CHECKED) {
                    resources_set_int("RamSize", 8);
                } else if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_MEM16K) == BST_CHECKED) {
                    resources_set_int("RamSize", 16);
                } else if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_MEM32K) == BST_CHECKED) {
                    resources_set_int("RamSize", 32);
                } else if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_MEM96K) == BST_CHECKED) {
                    resources_set_int("RamSize", 96);
                } else if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_MEM128K) == BST_CHECKED) {
                    resources_set_int("RamSize", 128);
                }
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_mem_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_SELECT_PET_MEM4K:
                case IDC_SELECT_PET_MEM8K:
                case IDC_SELECT_PET_MEM16K:
                case IDC_SELECT_PET_MEM32K:
                case IDC_SELECT_PET_MEM96K:
                case IDC_SELECT_PET_MEM128K:
                break;
            }
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK io_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_VIDEO_AUTO) == BST_CHECKED) {
                    resources_set_int("VideoSize", 0);
                } else if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_VIDEO_40) == BST_CHECKED) {
                    resources_set_int("VideoSize", 40);
                } else if (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_VIDEO_80) == BST_CHECKED) {
                    resources_set_int("VideoSize", 80);
                }
                resources_set_int("IOSize", (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_IO256) == BST_CHECKED ? 0x100 : 0x800));
                resources_set_int("KeymapIndex", (IsDlgButtonChecked(hwnd, IDC_SELECT_PET_KEYB_GRAPHICS) == BST_CHECKED ? 2 : 0));
                resources_set_int("Crtc", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_PET_CRTC) == BST_CHECKED ? 1 : 0));
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_io_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_SELECT_PET_VIDEO_AUTO:
                case IDC_SELECT_PET_VIDEO_40:
                case IDC_SELECT_PET_VIDEO_80:
                case IDC_SELECT_PET_IO256:
                case IDC_SELECT_PET_IO2K:
                case IDC_SELECT_PET_KEYB_GRAPHICS:
                case IDC_SELECT_PET_KEYB_BUSINESS:
                case IDC_TOGGLE_PET_CRTC:
                    break;
            }
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK superpet_io_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                resources_set_int("SuperPET", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_PET_SUPER_IO_ENABLE) == BST_CHECKED ? 1 : 0));
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_superpet_io_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_TOGGLE_PET_SUPER_IO_ENABLE:
                    break;
            }
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK pet8296_dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int type;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                resources_set_int("Ram9", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_PET_8296_RAM9) == BST_CHECKED ? 1 : 0));
                resources_set_int("RamA", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_PET_8296_RAMA) == BST_CHECKED ? 1 : 0));
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_INITDIALOG:
            init_pet8296_dialog(hwnd);
            return TRUE;
        case WM_COMMAND:
            type = LOWORD(wparam);
            switch (type) {
                case IDC_TOGGLE_PET_8296_RAM9:
                case IDC_TOGGLE_PET_8296_RAMA:
                    break;
            }
            return TRUE;
    }
    return FALSE;
}

void ui_pet_settings_dialog(HWND hwnd)
{
    PROPSHEETPAGE psp[4];
    PROPSHEETHEADER psh;
    int i;

    for (i = 0; i < 4; i++) {
        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
        psp[i].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
        psp[i].pszIcon = NULL;
#else
        psp[i].u2.pszIcon = NULL;
#endif
        psp[i].lParam = 0;
        psp[i].pfnCallback = NULL;
    }

    psp[0].pfnDlgProc = mem_dialog_proc;
    psp[0].pszTitle = intl_translate_tcs(IDS_MEMORY);
    psp[1].pfnDlgProc = io_dialog_proc;
    psp[1].pszTitle = intl_translate_tcs(IDS_INPUT_OUTPUT);
    psp[2].pfnDlgProc = superpet_io_dialog_proc;
    psp[2].pszTitle = TEXT("Super PET");
    psp[3].pfnDlgProc = pet8296_dialog_proc;
    psp[3].pszTitle = TEXT("8296 PET");

#ifdef _ANONYMOUS_UNION
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_MEMORY_DIALOG);
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_IO_DIALOG);
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_SUPER_DIALOG);
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_8296_DIALOG);
#else
    psp[0].u1.pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_MEMORY_DIALOG);
    psp[1].u1.pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_IO_DIALOG);
    psp[2].u1.pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_SUPER_DIALOG);
    psp[3].u1.pszTemplate = MAKEINTRESOURCE(IDD_PET_SETTINGS_8296_DIALOG);
#endif

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = winmain_instance;
    psh.pszCaption = intl_translate_tcs(IDS_PET_SETTINGS);
    psh.nPages = 4;
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

    system_psh_settings(&psh);
    PropertySheet(&psh);
}
