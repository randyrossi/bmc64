/*
 * uiconsole.c - Implementation of the console "save as" dialog.
 *
 * Written by
 *  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
 *
 * based on uisnapshot.c, written by
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
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <commdlg.h>

#include "drive.h"
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
#include "winlong.h"
#include "winmain.h"
#include "util.h"

static int append_log = 1;
static char name[100];

static uilib_localize_dialog_param save_as_console_dialog_trans[] = {
    { IDC_TOGGLE_CONSOLE_APPEND, IDS_TOGGLE_CONSOLE_APPEND, 0 },
    { 0, 0, 0 }
};

static void init_dialog(HWND hwnd)
{
    /* translate all dialog items */
    uilib_localize_dialog(hwnd, save_as_console_dialog_trans);

    /* adjust the size of the append element */
    uilib_adjust_element_width(hwnd, IDC_TOGGLE_CONSOLE_APPEND);

    CheckDlgButton(hwnd, IDC_TOGGLE_CONSOLE_APPEND, append_log ? BST_CHECKED : BST_UNCHECKED);
    name[0] = '\0';
}

static UINT_PTR APIENTRY hook_save_as_console(HWND hwnd, UINT uimsg, WPARAM wparam, LPARAM lparam)
{
    switch (uimsg) {
        case WM_INITDIALOG:
            init_dialog( hwnd );
            break;
        case WM_NOTIFY:
            append_log = IsDlgButtonChecked(hwnd, IDC_TOGGLE_CONSOLE_APPEND) == BST_CHECKED ? 1 : 0;
            break;
    }
    return 0;
}

static char *ui_save_as_console(const TCHAR *title, const char *filter, HWND hwnd)
{
    TCHAR name[MAX_PATH + 1] = TEXT("");
    OPENFILENAME ofn;
    char *ret = NULL;
    TCHAR *st_filter;

    st_filter = system_mbstowcs_alloc(filter);

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = winmain_instance;
    ofn.lpstrFilter = st_filter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = name;
    ofn.nMaxFile = sizeof(name);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = title;
    ofn.Flags = (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOTESTFILECREATE | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_SHAREAWARE | OFN_ENABLESIZING);
    ofn.lpfnHook = hook_save_as_console;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_CONSOLE_SAVE_DIALOG);
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;

    if (GetSaveFileName(&ofn)) {
        ret = system_wcstombs_alloc(name);
    }

    system_mbstowcs_free(st_filter);

    return ret;
}

FILE *ui_console_save_dialog(HWND hwnd)
{
    FILE *pfile = NULL;
    int filter_len, mask_len;
    char *s;
    char *filter;
    char mask[] = "*.dbg";

    s = translate_text(IDS_LOG_FILES_TYPE);
    filter_len = (int)strlen(s);
    mask_len = (int)strlen(mask);
    filter = util_concat(s, "0", mask, "0", NULL);
    filter[filter_len] = '\0';
    filter[filter_len + mask_len + 1] = '\0';

    s = ui_save_as_console(translate_text(IDS_LOG_CONSOLE_OUTPUT_IMAGE), filter, hwnd);
    lib_free(filter);

    if (s != NULL) {
        util_add_extension(&s, "dbg");

        pfile = fopen(s, append_log ? "at+" : "wt");

        if (!pfile) {
            ui_error(translate_text(IDS_CANNOT_WRITE_LOGFILE_S), s);
        }

        lib_free(s);
    }
    return pfile;
}
