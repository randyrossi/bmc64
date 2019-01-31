/*
 * uisnapshot.c - Implementation of the snapshot load/save dialogs.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commdlg.h>
#include <tchar.h>

#include "drive.h"
#include "fullscrn.h"
#include "interrupt.h"
#include "intl.h"
#include "lib.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uilib.h"
#include "util.h"
#include "vsync.h"
#include "winlong.h"
#include "winmain.h"

#include "uisnapshot.h"


static int save_roms = 0;
static int save_disks = 0;

static uilib_localize_dialog_param snapshot_dialog[] = {
    { IDC_TOGGLE_SNAPSHOT_SAVE_DISKS, IDS_TOGGLE_SNAPSHOT_SAVE_DISKS, 0 },
    { IDC_TOGGLE_SNAPSHOT_SAVE_ROMS, IDS_TOGGLE_SNAPSHOT_SAVE_ROMS, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group snapshot_group[] = {
    { IDC_TOGGLE_SNAPSHOT_SAVE_DISKS, 1 },
    { IDC_TOGGLE_SNAPSHOT_SAVE_ROMS, 1 },
    { 0, 0 }
};

static void init_snapshot_dialog(HWND hwnd)
{
    /* translate all dialog items */
    uilib_localize_dialog(hwnd, snapshot_dialog);
    
    /* adjust the size of the elements in the snapshot group */
    uilib_adjust_group_width(hwnd, snapshot_group);

    CheckDlgButton(hwnd, IDC_TOGGLE_SNAPSHOT_SAVE_DISKS, save_disks
                   ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_TOGGLE_SNAPSHOT_SAVE_ROMS, save_roms
                   ? BST_CHECKED : BST_UNCHECKED);
    SetDlgItemText(hwnd, IDC_SNAPSHOT_SAVE_IMAGE, TEXT(""));
}


static UINT_PTR APIENTRY hook_save_snapshot(HWND hwnd, UINT uimsg, WPARAM wparam, LPARAM lparam)
{
    switch (uimsg) {
        case WM_INITDIALOG:
            init_snapshot_dialog(hwnd);
            break;
        case WM_NOTIFY:
            save_disks = IsDlgButtonChecked(hwnd, IDC_TOGGLE_SNAPSHOT_SAVE_DISKS) == BST_CHECKED ? 1 : 0;
            save_roms = IsDlgButtonChecked(hwnd, IDC_TOGGLE_SNAPSHOT_SAVE_ROMS) == BST_CHECKED ? 1 : 0;
            break;
    }
    return 0;
}


static char *ui_save_snapshot(const TCHAR *title, const char *filter, HWND hwnd, int dialog_template)
{
    TCHAR name[1024] = TEXT("");
    OPENFILENAME ofn;
    char *ret = NULL;
    /* TCHAR *st_filter; */

    /* FIXME: filter is a list of string pairs; system_mbstowcs doesn't work */
    /* st_filter = system_mbstowcs_alloc(filter); */

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = winmain_instance;
    ofn.lpstrFilter = filter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = name;
    ofn.nMaxFile = sizeof(name);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = title;
    ofn.Flags = (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOTESTFILECREATE | OFN_FILEMUSTEXIST | OFN_SHAREAWARE | OFN_ENABLESIZING);
    if (dialog_template) {
        ofn.lpfnHook = hook_save_snapshot;
        ofn.lpTemplateName = MAKEINTRESOURCE(dialog_template);
        ofn.Flags = (ofn.Flags | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE);
    }
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    vsync_suspend_speed_eval();

    if (GetSaveFileName(&ofn)) {
        ret = system_wcstombs_alloc(name);
    }

    return ret;
}

static void ui_snapshot_save_dialog(HWND hwnd)
{
    int filter_len,mask_len;
    char *s;
    char *filter;
    char mask[] = "*.vsf";
    s=translate_text(IDS_SNAPSHOT_FILES_FILTER);
    filter_len = strlen(s);
    mask_len = (int)strlen(mask);
    filter = util_concat(s, "0", mask, "0", NULL);
    filter[filter_len] = '\0';
    filter[filter_len+mask_len + 1] = '\0';

    s = ui_save_snapshot(translate_text(IDS_SAVE_SNAPSHOT_IMAGE), filter, hwnd, translate_res(IDD_SNAPSHOT_SAVE_DIALOG));
    lib_free(filter);
    if (s != NULL) {
        util_add_extension(&s, "vsf");

        if (machine_write_snapshot(s, save_roms, save_disks, 0) < 0) {
            snapshot_display_error();
        }
        lib_free(s);
    }
}

static void ui_snapshot_load_dialog(HWND hwnd)
{
    TCHAR *st_name;

    if ((st_name = uilib_select_file(hwnd, translate_text(IDS_LOAD_SNAPSHOT_IMAGE), UILIB_FILTER_ALL | UILIB_FILTER_SNAPSHOT, UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_SNAPSHOT)) != NULL) {
        char *name;

        name = system_wcstombs_alloc(st_name);
        if (machine_read_snapshot(name, 0) < 0) {
            snapshot_display_error();
        }
        system_wcstombs_free(name);
        lib_free(st_name);
    }
}

static void save_snapshot_trap(uint16_t unused_addr, void *hwnd)
{
    SuspendFullscreenModeKeep(hwnd);
    ui_snapshot_save_dialog(hwnd);
    ResumeFullscreenModeKeep(hwnd);
}

static void load_snapshot_trap(uint16_t unused_addr, void *hwnd)
{
    SuspendFullscreenModeKeep(hwnd);
    ui_snapshot_load_dialog(hwnd);
    ResumeFullscreenModeKeep(hwnd);
}

void ui_snapshot_load(HWND hwnd)
{
    if (!ui_emulation_is_paused()) {
        interrupt_maincpu_trigger_trap(load_snapshot_trap, hwnd);
    } else {
        load_snapshot_trap(0, 0);
    }
}

void ui_snapshot_save(HWND hwnd)
{
    interrupt_maincpu_trigger_trap(save_snapshot_trap, hwnd);
}
