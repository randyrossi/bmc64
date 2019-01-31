/*
 * uirom.c - Implementation of the ROM settings dialog box.
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

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>

#include "intl.h"
#include "lib.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "romset.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uilib.h"
#include "uirom.h"
#include "winmain.h"

static const uirom_settings_t *settings;

static uilib_localize_dialog_param *main_trans;
static uilib_localize_dialog_param *drive_trans;

static generic_trans_table_t *generic_trans;

static uilib_dialog_group *main_left_group;
static uilib_dialog_group *main_middle_group;
static uilib_dialog_group *main_right_group;

static uilib_dialog_group *drive_left_group;
static uilib_dialog_group *drive_middle_group;
static uilib_dialog_group *drive_right_group;

static void init_rom_dialog(HWND hwnd, unsigned int type)
{
    unsigned int n = 0;
    int xpos;
    int i;
    HWND element;

    if (type == UIROM_TYPE_MAIN) {
        /* translate all dialog items */
        uilib_localize_dialog(hwnd, main_trans);

        /* adjust the size of the elements in the main left group */
        uilib_adjust_group_width(hwnd, main_left_group);

        /* get the max x of the main left group */
        uilib_get_group_max_x(hwnd, main_left_group, &xpos);

        /* move the main middle group to the correct position */
        uilib_move_group(hwnd, main_middle_group, xpos + 10);

        /* get the max x of the main middle group */
        uilib_get_group_max_x(hwnd, main_middle_group, &xpos);

        /* move the main right group to the correct position */
        uilib_move_group(hwnd, main_right_group, xpos + 10);

    } else if (type == UIROM_TYPE_DRIVE) {
        /* translate all dialog items */
        uilib_localize_dialog(hwnd, drive_trans);

        /* translate generic items */
        for (i = 0; generic_trans[i].text != NULL; i++) {
            element = GetDlgItem(hwnd, generic_trans[i].idm);
            SetWindowText(element, generic_trans[i].text);
        }

        /* adjust the size of the elements in the drive left group */
        uilib_adjust_group_width(hwnd, drive_left_group);

        /* get the max x of the drive left group */
        uilib_get_group_max_x(hwnd, drive_left_group, &xpos);

        /* move the drive middle group to the correct position */
        uilib_move_group(hwnd, drive_middle_group, xpos + 10);

        /* get the max x of the drive middle group */
        uilib_get_group_max_x(hwnd, drive_middle_group, &xpos);

        /* move the drive right group to the correct position */
        uilib_move_group(hwnd, drive_right_group, xpos + 10);
    }

    while (settings[n].realname != NULL) {
        if (settings[n].type == type) {
            const char *filename;
            TCHAR *st_filename;

            resources_get_string(settings[n].resname, &filename);
            st_filename = system_mbstowcs_alloc(filename);
            SetDlgItemText(hwnd, settings[n].idc_filename, st_filename != NULL ? st_filename : TEXT(""));
            system_mbstowcs_free(st_filename);
        }
        n++;
    }
}

static void set_dialog_proc(HWND hwnd, unsigned int type)
{
    unsigned int n = 0;

    while (settings[n].realname != NULL) {
        if (settings[n].type == type) {
            char filename[MAX_PATH];
            TCHAR st_filename[MAX_PATH];

            GetDlgItemText(hwnd, settings[n].idc_filename, st_filename, MAX_PATH);
            system_wcstombs(filename, st_filename, MAX_PATH);
            resources_set_string(settings[n].resname, filename);
        }
        n++;
    }
}

static BOOL browse_command(HWND hwnd, unsigned int command)
{
    unsigned int n = 0;

    while (settings[n].realname != NULL) {
        if ((unsigned int)command == settings[n].idc_browse) {
            TCHAR st_realname[100];

            _stprintf(st_realname, intl_translate_tcs(IDS_LOAD_S_ROM_IMAGE), settings[n].realname);

            uilib_select_browse(hwnd, st_realname, UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_LOAD, settings[n].idc_filename);
            return TRUE;
        }
        n++;
    }

    return FALSE;
}

static BOOL CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, unsigned int type)
{
    int command;

    switch (msg) {
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_rom_dialog(hwnd, type);
            return TRUE;
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lparam)->code) {
                case PSN_KILLACTIVE:
                    set_dialog_proc(hwnd, type);
                    return TRUE;
            }
            return FALSE;
        case WM_COMMAND:
            command = LOWORD(wparam);
            return browse_command(hwnd, command);
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
    }
    return FALSE;
}

static INT_PTR CALLBACK dialog_proc_main(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return dialog_proc(hwnd, msg, wparam, lparam, UIROM_TYPE_MAIN);
}

static INT_PTR CALLBACK dialog_proc_drive(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return dialog_proc(hwnd, msg, wparam, lparam, UIROM_TYPE_DRIVE);
}

void uirom_settings_dialog(HWND hwnd, unsigned int idd_dialog_main,
                           unsigned int idd_dialog_drive,
                           const uirom_settings_t *uirom_settings,
                           uilib_localize_dialog_param *uirom_main_trans,
                           uilib_localize_dialog_param *uirom_drive_trans,
                           generic_trans_table_t *uirom_generic_trans,
                           uilib_dialog_group *uirom_main_left_group,
                           uilib_dialog_group *uirom_main_middle_group,
                           uilib_dialog_group *uirom_main_right_group,
                           uilib_dialog_group *uirom_drive_left_group,
                           uilib_dialog_group *uirom_drive_middle_group,
                           uilib_dialog_group *uirom_drive_right_group)
{
    PROPSHEETPAGE psp[2];
    PROPSHEETHEADER psh;

    main_trans = uirom_main_trans;
    drive_trans = uirom_drive_trans;
    generic_trans = uirom_generic_trans;
    main_left_group = uirom_main_left_group;
    main_middle_group = uirom_main_middle_group;
    main_right_group = uirom_main_right_group;
    drive_left_group = uirom_drive_left_group;
    drive_middle_group = uirom_drive_middle_group;
    drive_right_group = uirom_drive_right_group;

    settings = uirom_settings;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[0].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[0].pszTemplate = MAKEINTRESOURCE(idd_dialog_main);
    psp[0].pszIcon = NULL;
#else
    psp[0].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(idd_dialog_main);
    psp[0].u2.pszIcon = NULL;
#endif
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
    psp[1].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
    psp[1].pszTemplate = MAKEINTRESOURCE(idd_dialog_drive);
    psp[1].pszIcon = NULL;
#else
    psp[1].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(idd_dialog_drive);
    psp[1].u2.pszIcon = NULL;
#endif
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    psp[0].pfnDlgProc = dialog_proc_main;
    psp[0].pszTitle = intl_translate_tcs(IDS_COMPUTER);
    psp[1].pfnDlgProc = dialog_proc_drive;
    psp[1].pszTitle = intl_translate_tcs(IDS_DRIVE);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = winmain_instance;
    psh.pszCaption = intl_translate_tcs(IDS_ROM_SETTINGS);
    psh.nPages = 2;
#ifdef _ANONYMOUS_UNION
    psh.pszIcon = NULL;
    psh.nStartPage = 0;
    psh.ppsp = psp;
#else
    psh.DUMMYUNIONNAME.pszIcon = NULL;
    psh.u2.nStartPage = 0;
    psh.u3.ppsp = psp;
#endif
    psh.pfnCallback = NULL;

    PropertySheet(&psh);
}
