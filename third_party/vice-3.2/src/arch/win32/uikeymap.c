/*
 * uikeymap.c - Implementation of the keymap settings dialog box.
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
#include "keyboard.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uikeymap.h"
#include "winmain.h"

static TCHAR *ui_keymap_index[] = {
    TEXT("Symbolic"),
    TEXT("Positional"),
    TEXT("Symbolic (user)"),
    TEXT("Positional (user)"),
    NULL
};

static int ui_keymap_mapping_id[KBD_MAPPING_NUM + 1];

static uilib_localize_dialog_param keymap_dialog_trans[] = {
    { 0, IDS_KEYMAP_CAPTION, -1 },
    { IDC_KEYMAP_INDEX_LABEL, IDS_KEYMAP_INDEX_LABEL, 0 },
    { IDC_KEYMAP_MAPPING_LABEL, IDS_KEYMAP_MAPPING_LABEL, 0 },
    { IDC_KEYMAP_SYM_USER_FILE_LABEL, IDS_KEYMAP_SYM_USER_FILE_LABEL, 0 },
    { IDC_KEYMAP_POS_USER_FILE_LABEL, IDS_KEYMAP_POS_USER_FILE_LABEL, 0 },
    { IDC_KEYMAP_SYM_USER_BROWSE, IDS_BROWSE, 0 },
    { IDC_KEYMAP_POS_USER_BROWSE, IDS_BROWSE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group keymap_left_group[] = {
    { IDC_KEYMAP_INDEX_LABEL, 0 },
    { IDC_KEYMAP_MAPPING_LABEL, 0 },
    { IDC_KEYMAP_SYM_USER_FILE_LABEL, 0 },
    { IDC_KEYMAP_POS_USER_FILE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group keymap_right_group[] = {
    { IDC_KEYMAP_INDEX, 0 },
    { IDC_KEYMAP_MAPPING, 0 },
    { IDC_KEYMAP_SYM_USER_FILE, 0 },
    { IDC_KEYMAP_POS_USER_FILE, 0 },
    { IDC_KEYMAP_SYM_USER_BROWSE, 0 },
    { IDC_KEYMAP_POS_USER_BROWSE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_keymap_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int res_value;
    const char *filename;
    TCHAR *st_filename;
    int res_value_loop;
    int active_value = 0;
    int xpos;
    RECT rect;
    mapping_info_t *kbdlist = keyboard_get_info_list();
    int num = keyboard_get_num_mappings();
    int i = 0;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, keymap_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, keymap_left_group);

    /* get the max x of the left group elements */
    uilib_get_group_max_x(hwnd, keymap_left_group, &xpos);

    /* move the right group elements to the correct position */
    uilib_move_group(hwnd, keymap_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, keymap_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_KEYMAP_INDEX);
    for (res_value_loop = 0; ui_keymap_index[res_value_loop] != NULL; res_value_loop++) {
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_keymap_index[res_value_loop]);
    }
    resources_get_int("KeymapIndex", &res_value);
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_int("KeyboardMapping", &res_value);
    temp_hwnd = GetDlgItem(hwnd, IDC_KEYMAP_MAPPING);
    while (num) {
        ui_keymap_mapping_id[i] = kbdlist->mapping;
        st_filename = system_mbstowcs_alloc(kbdlist->name);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st_filename);
        system_mbstowcs_free(st_filename);
        if (res_value == ui_keymap_mapping_id[i]) {
            active_value = i;
        }
        i++;
        kbdlist++;
        num--;
    }
    ui_keymap_mapping_id[i] = -1;
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)active_value, 0);

    resources_get_string("KeymapUserSymFile", &filename);
    st_filename = system_mbstowcs_alloc(filename);
    SetDlgItemText(hwnd, IDC_KEYMAP_SYM_USER_FILE, filename != NULL ? st_filename : TEXT(""));
    system_mbstowcs_free(st_filename);

    resources_get_string("KeymapUserPosFile", &filename);
    st_filename = system_mbstowcs_alloc(filename);
    SetDlgItemText(hwnd, IDC_KEYMAP_POS_USER_FILE, filename != NULL ? st_filename : TEXT(""));
    system_mbstowcs_free(st_filename);
}

static void end_keymap_dialog(HWND hwnd)
{
    TCHAR st[MAX_PATH];
    char s[MAX_PATH];

    resources_set_int("KeymapIndex", (int)SendMessage(GetDlgItem(hwnd, IDC_KEYMAP_INDEX), CB_GETCURSEL, 0, 0));
    resources_set_int("KeyboardMapping", ui_keymap_mapping_id[SendMessage(GetDlgItem(hwnd, IDC_KEYMAP_MAPPING), CB_GETCURSEL, 0, 0)]);

    GetDlgItemText(hwnd, IDC_KEYMAP_SYM_USER_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("KeymapUserSymFile", s);

    GetDlgItemText(hwnd, IDC_KEYMAP_POS_USER_FILE, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string("KeymapUserPosFile", s);
}

static void browse_keymap_sym_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_SELECT_FILE_KEYMAP_SYM_USER), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_KEYMAP_SYM_USER_FILE);
}

static void browse_keymap_pos_file(HWND hwnd)
{
    uilib_select_browse(hwnd, intl_translate_tcs(IDS_SELECT_FILE_KEYMAP_POS_USER), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, IDC_KEYMAP_POS_USER_FILE);
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_KEYMAP_SYM_USER_BROWSE:
                    browse_keymap_sym_file(hwnd);
                    break;
                case IDC_KEYMAP_POS_USER_BROWSE:
                    browse_keymap_pos_file(hwnd);
                    break;
                case IDOK:
                    end_keymap_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_keymap_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_keymap_settings_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_KEYMAP_SETTINGS_DIALOG, hwnd, dialog_proc);
}
