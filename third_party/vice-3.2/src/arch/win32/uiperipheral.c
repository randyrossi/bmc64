/*
 * uiperipheral.c - Implementation of the device manager dialog box.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@mail.matav.hu>
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
#include <windows.h>
#include <tchar.h>

#ifdef HAVE_SHLOBJ_H
#include <shlobj.h>
#endif

#include "attach.h"
#include "autostart.h"
#include "iecdrive.h"
#include "imagecontents.h"
#include "intl.h"
#include "lib.h"
#include "machine.h"
#include "opencbmlib.h"
#include "printer.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uilib.h"
#include "uiperipheral.h"
#include "winlong.h"
#include "winmain.h"

static int has_userport_printer(void)
{
    switch (machine_class) {
        case VICE_MACHINE_NONE:
        case VICE_MACHINE_PLUS4:
        case VICE_MACHINE_C64DTV:
        case VICE_MACHINE_VSID:
            return 0;
    }
    return 1;
}

/* -------------------------------------------------------------------------- */
/*                             Disk Peripherals (8-11)                        */
/* -------------------------------------------------------------------------- */

static void enable_controls_for_disk_device_type(HWND hwnd, int type)
{
    EnableWindow(GetDlgItem(hwnd, IDC_DISKIMAGE), type == IDC_SELECTDISK);
    EnableWindow(GetDlgItem(hwnd, IDC_BROWSEDISK), type == IDC_SELECTDISK);
    EnableWindow(GetDlgItem(hwnd, IDC_AUTOSTART), type == IDC_SELECTDISK);
    EnableWindow(GetDlgItem(hwnd, IDC_TOGGLE_ATTACH_READONLY), type == IDC_SELECTDISK);
    EnableWindow(GetDlgItem(hwnd, IDC_DIR), type == IDC_SELECTDIR);
    EnableWindow(GetDlgItem(hwnd, IDC_BROWSEDIR), type == IDC_SELECTDIR);
    EnableWindow(GetDlgItem(hwnd, IDC_TOGGLE_READP00), type == IDC_SELECTDIR);
    EnableWindow(GetDlgItem(hwnd, IDC_TOGGLE_WRITEP00), type == IDC_SELECTDIR);
    EnableWindow(GetDlgItem(hwnd, IDC_TOGGLE_HIDENONP00), type == IDC_SELECTDIR);
}

static void enable_controls(HWND hwnd)
{
    int drive_true_emulation, virtual_device_traps;
    BOOL haveIECDevice;

    resources_get_int("DriveTrueEmulation", &drive_true_emulation);
    resources_get_int("VirtualDevices", &virtual_device_traps);
    haveIECDevice = IsDlgButtonChecked(hwnd, IDC_TOGGLE_USEIECDEVICE) == BST_CHECKED;
  
    if ((drive_true_emulation || !virtual_device_traps) && !haveIECDevice) {
        EnableWindow(GetDlgItem(hwnd, IDC_SELECTDISK), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SELECTDIR), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_SELECTNONE), FALSE);
#ifdef HAVE_OPENCBM
        if (opencbmlib_is_available()) {
            EnableWindow(GetDlgItem(hwnd, IDC_SELECTREAL), FALSE);
        }
#endif
    } else {
        EnableWindow(GetDlgItem(hwnd, IDC_SELECTDISK), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_SELECTDIR), TRUE);
        EnableWindow(GetDlgItem(hwnd, IDC_SELECTNONE), TRUE);
#ifdef HAVE_OPENCBM
        if (opencbmlib_is_available()) {
            EnableWindow(GetDlgItem(hwnd, IDC_SELECTREAL), TRUE);
        }
#endif
    }
}

static uilib_localize_dialog_param diskdevice_opencbm_dialog[] = {
    { IDC_TOGGLE_USEIECDEVICE, IDS_TOGGLE_USEIECDEVICE, 0 },
    { IDC_SELECTDISK, IDS_SELECTDISK, 0 },
    { IDC_SELECTDIR, IDS_SELECTDIR, 0 },
#ifdef HAVE_OPENCBM
    { IDC_SELECTREAL, IDS_SELECTREAL, 0 },
#endif
    { IDC_SELECTNONE, IDS_SELECTNONE, 0 },
    { IDC_BROWSEDISK, IDS_BROWSE, 0 },
    { IDC_AUTOSTART, IDS_AUTOSTART, 0 },
    { IDC_TOGGLE_ATTACH_READONLY, IDS_TOGGLE_ATTACH_READONLY, 0 },
    { IDC_BROWSEDIR, IDS_BROWSEDIR, 0 },
    { IDC_DISKDEVICE_OPTIONS, IDS_MP_OPTIONS, 0 },
    { IDC_TOGGLE_READP00, IDS_TOGGLE_READP00, 0 },
    { IDC_TOGGLE_WRITEP00, IDS_TOGGLE_WRITEP00, 0 },
    { IDC_TOGGLE_HIDENONP00, IDS_TOGGLE_HIDENONP00, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param diskdevice_normal_dialog[] = {
    { IDC_TOGGLE_USEIECDEVICE, IDS_TOGGLE_USEIECDEVICE, 0 },
    { IDC_SELECTDISK, IDS_SELECTDISK, 0 },
    { IDC_SELECTDIR, IDS_SELECTDIR, 0 },
    { IDC_SELECTNONE, IDS_SELECTNONE, 0 },
    { IDC_BROWSEDISK, IDS_BROWSE, 0 },
    { IDC_AUTOSTART, IDS_AUTOSTART, 0 },
    { IDC_TOGGLE_ATTACH_READONLY, IDS_TOGGLE_ATTACH_READONLY, 0 },
    { IDC_BROWSEDIR, IDS_BROWSEDIR, 0 },
    { IDC_DISKDEVICE_OPTIONS, IDS_MP_OPTIONS, 0 },
    { IDC_TOGGLE_READP00, IDS_TOGGLE_READP00, 0 },
    { IDC_TOGGLE_WRITEP00, IDS_TOGGLE_WRITEP00, 0 },
    { IDC_TOGGLE_HIDENONP00, IDS_TOGGLE_HIDENONP00, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group diskdevice_read_only[] = {
    { IDC_TOGGLE_ATTACH_READONLY, 1 },
    { 0, 0 }
};

static uilib_dialog_group diskdevice_left_group[] = {
    { IDC_SELECTDISK, 1 },
    { IDC_SELECTDIR, 1 },
    { 0, 0 }
};

static uilib_dialog_group diskdevice_middle_group[] = {
    { IDC_DISKIMAGE, 0 },
    { IDC_BROWSEDISK, 0 },
    { IDC_DIR, 0 },
    { IDC_BROWSEDIR, 0 },
    { 0, 0 }
};

static uilib_dialog_group diskdevice_right_group[] = {
    { IDC_TOGGLE_READP00, 1 },
    { IDC_TOGGLE_WRITEP00, 1 },
    { IDC_TOGGLE_HIDENONP00, 1 },
    { 0, 0 }
};

static uilib_dialog_group diskdevice_filling_group[] = {
    { IDC_TOGGLE_USEIECDEVICE, 0 },
    { IDC_DISKIMAGE, 0 },
    { IDC_DIR, 0 },
    { IDC_DISKDEVICE_OPTIONS, 0 },
    { IDC_SELECTNONE, 0 },
    { 0, 0 }
};

static void init_dialog(HWND hwnd, unsigned int num)
{
    const char *disk_image, *dir;
    TCHAR *st_disk_image, *st_dir;
    int devtype, n;
    int xpos, xpos1, xpos2, xpos3;
    int distance1, distance2;
    RECT rect;
    uilib_localize_dialog_param *diskdevice_dialog =
#ifdef HAVE_OPENCBM
      (opencbmlib_is_available()) ? diskdevice_opencbm_dialog : 
#endif
      diskdevice_normal_dialog;

    if (num >= 8 && num <= 11) {
        /* translate all dialog items */
        uilib_localize_dialog(hwnd, diskdevice_dialog);

        /* adjust the size of the elements in the diskdevice_left_group */
        uilib_adjust_group_width(hwnd, diskdevice_left_group);

        /* adjust the size of the 'read-only' element */
        uilib_adjust_group_width(hwnd, diskdevice_read_only);

        /* get the min x of the read only element */
        uilib_get_element_min_x(hwnd, IDC_TOGGLE_ATTACH_READONLY, &xpos3);

        /* get the min x of the autostart element */
        uilib_get_element_min_x(hwnd, IDC_AUTOSTART, &xpos2);

        /* get the max x of the first browse element */
        uilib_get_element_max_x(hwnd, IDC_BROWSEDISK, &xpos1);

        /* calculate the distance between the browse button and the autostart button */
        distance1 = xpos2 - xpos1;

        /* get the max x of the autostart element */
        uilib_get_element_max_x(hwnd, IDC_AUTOSTART, &xpos2);

        /* calculate the distance between the browse button and the read only element */
        distance2 = xpos3 - xpos2;

        /* get the max x of the disk_device_left_group items */
        uilib_get_group_max_x(hwnd, diskdevice_left_group, &xpos);

        /* move the filename elements and browse buttons to the right position */
        uilib_move_group(hwnd, diskdevice_middle_group, xpos + 10);
        
        /* get the max x of the first browse element */
        uilib_get_element_max_x(hwnd, IDC_BROWSEDISK, &xpos);

        /* move the autostart element to the right position */
        uilib_move_element(hwnd, IDC_AUTOSTART, xpos + distance1);

        /* resize the autostart element */
        uilib_adjust_element_width(hwnd, IDC_AUTOSTART);

        /* get the max x of the autostart element */
        uilib_get_element_max_x(hwnd, IDC_AUTOSTART, &xpos);

        /* move the read only element to the right position */
        uilib_move_element(hwnd, IDC_TOGGLE_ATTACH_READONLY, xpos + distance2);

#ifdef HAVE_OPENCBM
        if (opencbmlib_is_available()) {
            /* adjust the size of the real iec element */
            uilib_adjust_element_width(hwnd, IDC_SELECTREAL);

            /* get the max x of the real iec element */
            uilib_get_element_max_x(hwnd, IDC_SELECTREAL, &xpos1);
        } else {
            xpos1 = 0;
        }
#else
        xpos1 = 0;
#endif

        /* get the max x of the second browse button */
        uilib_get_element_max_x(hwnd, IDC_BROWSEDIR, &xpos2);

        if (xpos2 > xpos1) {
            xpos = xpos2 + 20;
        } else {
            xpos = xpos1 + 20;
        }

        /* move the diskdevice_right_group to the right position */
        uilib_move_group(hwnd, diskdevice_right_group, xpos);
        
        /* move the p00 group element to the right position */
        uilib_move_element(hwnd, IDC_DISKDEVICE_OPTIONS, xpos - 5);

        /* adjust the diskdevice_right_group elements size */
        uilib_adjust_group_width(hwnd, diskdevice_right_group);

        /* adjust the p00 group element size */
        uilib_adjust_element_width(hwnd, IDC_DISKDEVICE_OPTIONS);
        
        /* get the width of the disk_device_right_group items */
        uilib_get_group_width(hwnd, diskdevice_right_group, &xpos1);

        /* get the width of the p00 group element */
        uilib_get_element_width(hwnd, IDC_DISKDEVICE_OPTIONS, &xpos2);

        if (xpos2 + 5 > xpos1) {
            xpos = xpos2 + 15;
        } else {
            xpos = xpos1 + 10;
        }

        /* set the width of the p00 group element */
        uilib_set_element_width(hwnd, IDC_DISKDEVICE_OPTIONS, xpos);

        /* adjust the none element size */
        uilib_adjust_element_width(hwnd, IDC_SELECTNONE);

        /* adjust the size of the use iec device element */
        uilib_adjust_element_width(hwnd, IDC_TOGGLE_USEIECDEVICE);

        /* get the max x of the window filling elements */
        uilib_get_group_max_x(hwnd, diskdevice_filling_group, &xpos);

        /* set the width of the dialog to 'surround' all the elements */
        GetWindowRect(hwnd, &rect);
        MoveWindow(hwnd, rect.left, rect.top, xpos + 10, rect.bottom - rect.top, TRUE);

        disk_image = file_system_get_disk_name(num);
        st_disk_image = system_mbstowcs_alloc(disk_image);
        SetDlgItemText(hwnd, IDC_DISKIMAGE, st_disk_image != NULL ? st_disk_image : TEXT(""));
        system_mbstowcs_free(st_disk_image);

        resources_get_string_sprintf("FSDevice%dDir", &dir, num);
        st_dir = system_mbstowcs_alloc(dir);
        SetDlgItemText(hwnd, IDC_DIR, st_dir != NULL ? st_dir : TEXT(""));
        system_mbstowcs_free(st_dir);

        resources_get_int_sprintf("FSDevice%dConvertP00", &n, num);
        CheckDlgButton(hwnd, IDC_TOGGLE_READP00, n ? BST_CHECKED : BST_UNCHECKED);

        resources_get_int_sprintf("FSDevice%dSaveP00", &n, num);
        CheckDlgButton(hwnd, IDC_TOGGLE_WRITEP00, n ? BST_CHECKED : BST_UNCHECKED);

        resources_get_int_sprintf("FSDevice%dHideCBMFiles", &n, num);
        CheckDlgButton(hwnd, IDC_TOGGLE_HIDENONP00, n ? BST_CHECKED : BST_UNCHECKED);

        resources_get_int_sprintf("AttachDevice%dReadonly", &n, num);
        CheckDlgButton(hwnd, IDC_TOGGLE_ATTACH_READONLY, n ? BST_CHECKED : BST_UNCHECKED);

        resources_get_int_sprintf("FileSystemDevice%d", &devtype, num);
        switch (devtype) {
            case ATTACH_DEVICE_FS:
                if (disk_image != NULL) {
                    n = IDC_SELECTDISK;
                } else {
                    n = IDC_SELECTDIR;
                }
                break;
#ifdef HAVE_OPENCBM
            case ATTACH_DEVICE_REAL:
                n = IDC_SELECTREAL;
                break;
#endif
            default:
                n = IDC_SELECTNONE;
        }

#ifdef HAVE_OPENCBM
        if (opencbmlib_is_available()) {
            CheckRadioButton(hwnd, IDC_SELECTDISK, IDC_SELECTREAL, n);
        } else {
            CheckRadioButton(hwnd, IDC_SELECTDISK, IDC_SELECTDIR, n);
        }
#else
        CheckRadioButton(hwnd, IDC_SELECTDISK, IDC_SELECTDIR, n);
#endif
        enable_controls_for_disk_device_type(hwnd, n);

        if (iec_available_busses() & IEC_BUS_IEC) {
            resources_get_int_sprintf("IECDevice%d", &n, num);
            CheckDlgButton(hwnd, IDC_TOGGLE_USEIECDEVICE, n ? BST_CHECKED : BST_UNCHECKED);
        } else {
            CheckDlgButton(hwnd, IDC_TOGGLE_USEIECDEVICE, BST_UNCHECKED);
            ShowWindow(GetDlgItem(hwnd, IDC_TOGGLE_USEIECDEVICE), FALSE);
        }
        
        enable_controls(hwnd);
    }
}

static BOOL store_dialog_results(HWND hwnd, unsigned int num)
{
    char s[MAX_PATH];
    TCHAR st[MAX_PATH];
    int devtype = ATTACH_DEVICE_NONE;

    if (iec_available_busses() & IEC_BUS_IEC) {
        resources_set_int_sprintf("IECDevice%d", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_USEIECDEVICE) == BST_CHECKED), num);
    }

    if (IsDlgButtonChecked(hwnd, IDC_SELECTDISK) == BST_CHECKED || IsDlgButtonChecked(hwnd, IDC_SELECTDIR) == BST_CHECKED) {
        devtype = ATTACH_DEVICE_FS;
    }
#ifdef HAVE_OPENCBM
    if (opencbmlib_is_available()) {
        if (IsDlgButtonChecked(hwnd, IDC_SELECTREAL) == BST_CHECKED) {
            devtype = ATTACH_DEVICE_REAL;
        }
    }
#endif
    resources_set_int_sprintf("FileSystemDevice%d", devtype, num);

    resources_set_int_sprintf("FSDevice%dConvertP00", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_READP00) == BST_CHECKED), num);
    resources_set_int_sprintf("FSDevice%dSaveP00", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_WRITEP00) == BST_CHECKED), num);
    resources_set_int_sprintf("FSDevice%dHideCBMFiles", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_HIDENONP00) == BST_CHECKED), num);
    resources_set_int_sprintf("AttachDevice%dReadonly", (IsDlgButtonChecked(hwnd, IDC_TOGGLE_ATTACH_READONLY) == BST_CHECKED), num);

    GetDlgItemText(hwnd, IDC_DIR, st, MAX_PATH);
    system_wcstombs(s, st, MAX_PATH);
    resources_set_string_sprintf("FSDevice%dDir", s, num);

    if (IsDlgButtonChecked(hwnd, IDC_SELECTDISK) == BST_CHECKED) {
        GetDlgItemText(hwnd, IDC_DISKIMAGE, st, MAX_PATH);
        system_wcstombs(s, st, MAX_PATH);
        if (file_system_attach_disk(num, s) < 0 ) {
            ui_error(translate_text(IDS_CANNOT_ATTACH_FILE));
            return 0;
        }
    } else {
        if ((IsDlgButtonChecked(hwnd, IDC_SELECTDIR) == BST_CHECKED) && file_system_get_disk_name(num)) {
            file_system_detach_disk(num);
        }
    }

    return 1;
}

static void browse_diskimage(HWND hwnd)
{
    TCHAR *st_name;
    char *name;

    name = uilib_select_file(hwnd, intl_translate_tcs(IDS_ATTACH_DISK_IMAGE),
                    UILIB_FILTER_ALL | UILIB_FILTER_DISK | UILIB_FILTER_ZIP,
                    UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DISK);

    st_name = system_mbstowcs_alloc(name);
    if (st_name != NULL) {
        SetDlgItemText(hwnd, IDC_DISKIMAGE, st_name);
        system_mbstowcs_free(st_name);
    }

    lib_free(name);
}

static void autostart_diskimage(HWND hwnd)
{
    TCHAR *st_name;
    char *name;

    name = uilib_select_file(hwnd, intl_translate_tcs(IDS_AUTOSTART_DISK_IMAGE),
                    UILIB_FILTER_ALL | UILIB_FILTER_DISK | UILIB_FILTER_ZIP,
                    UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DISK);

    st_name = system_mbstowcs_alloc(name);
    if (st_name != NULL) {
        SetDlgItemText(hwnd, IDC_DISKIMAGE, st_name);
        if (autostart_autodetect(name, "*", 0, AUTOSTART_MODE_RUN) < 0) {
            ui_error(translate_text(IDS_CANNOT_AUTOSTART_FILE));
        }
        system_mbstowcs_free(st_name);
    }

    lib_free(name);
}

static void browse_dir(HWND hwnd)
{
    BROWSEINFO bi;
    TCHAR st[MAX_PATH];
    LPITEMIDLIST idlist;

    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = st;
    bi.lpszTitle = intl_translate_tcs(IDS_SELECT_FS_DIRECTORY);
    bi.ulFlags = 0;
    bi.lpfn = NULL;
    bi.lParam = 0;
    bi.iImage = 0;
    if ((idlist = SHBrowseForFolder(&bi)) != NULL) {
        SHGetPathFromIDList(idlist, st);
        LocalFree(idlist);
        /*
        If a root directory is selected, \ is appended
        and has to be deleted.
        */
        if (st[_tcslen(st) - 1] == '\\') {
            st[_tcslen(st) - 1] = '\0';
        }
        SetDlgItemText(hwnd, IDC_DIR, st);
    }
}

static BOOL CALLBACK dialog_proc(unsigned int num, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_INITDIALOG:
            init_dialog(hwnd, num);
            return TRUE;
        case WM_NOTIFY:
            {
                NMHDR *nmhdr = (NMHDR *)(lparam);

                switch (nmhdr->code) {
                    case PSN_APPLY:
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, store_dialog_results(hwnd, num) ? PSNRET_NOERROR : PSNRET_INVALID);
                        return TRUE;
                }
                break;
            }
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDC_SELECTDIR:
                case IDC_SELECTDISK:
                case IDC_SELECTREAL:
                case IDC_SELECTNONE:
                    enable_controls_for_disk_device_type(hwnd, LOWORD(wparam));
                    break;
                case IDC_TOGGLE_USEIECDEVICE:
                    enable_controls(hwnd);
                    break;
                case IDC_BROWSEDISK:
                    browse_diskimage(hwnd);
                    break;
                case IDC_AUTOSTART:
                    autostart_diskimage(hwnd);
                    break;
                case IDC_BROWSEDIR:
                    browse_dir(hwnd);
                    break;
            }
            return TRUE;
    }
    return FALSE;
}

#define _CALLBACK(num)                                                                      \
static INT_PTR CALLBACK callback_##num(HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam) \
{                                                                                           \
    return dialog_proc(num, dialog, msg, wparam, lparam);                                   \
}

_CALLBACK(8)
_CALLBACK(9)
_CALLBACK(10)
_CALLBACK(11)

/* -------------------------------------------------------------------------- */
/*                           Printers (Userport, 4-5)                         */
/* -------------------------------------------------------------------------- */

static int have_printer_userport = -1;

static char *printertextdevice[3] = { NULL, NULL, NULL };

static const int ui_printer_opencbm[] = {
    IDS_NONE,
    IDS_FILE_SYSTEM,
#ifdef HAVE_OPENCBM
    IDS_REAL_IEC_DEVICE,
#endif
    0
};

static const int ui_printer_normal[] = {
    IDS_NONE,
    IDS_FILE_SYSTEM,
    0
};

static const TCHAR *ui_printer_driver[] = {
    TEXT("ASCII"),
    TEXT("MPS803"),
    TEXT("NL10"),
    TEXT("RAW"),
    NULL
};

static const char *ui_printer_driver_ascii[] = {
    "ascii",
    "mps803",
    "nl10",
    "raw",
    NULL
};

static const TCHAR *ui_userprinter_driver[] = {
    TEXT("ASCII"),
    TEXT("NL10"),
    TEXT("RAW"),
    NULL
};

static const char *ui_userprinter_driver_ascii[] = {
    "ascii",
    "nl10",
    "raw",
    NULL
};

static const TCHAR *ui_plotter_driver[] = {
    TEXT("1520"),
    TEXT("RAW"),
    NULL
};

static const char *ui_plotter_driver_1520[] = {
    "1520",
    "raw",
    NULL
};

static const TCHAR *ui_printer_output[] = {
    TEXT("Text"),
    TEXT("Graphics"),
    NULL
};

static const char *ui_printer_output_ascii[] = {
    "text",
    "graphics",
    NULL
};

static const TCHAR *ui_printer_text_device[] = {
    TEXT("1"),
    TEXT("2"),
    TEXT("3"),
    NULL
};

static void enable_printer_controls(unsigned int num, HWND hwnd)
{
    int res_value, is_enabled;
    int drive_true_emulation, virtual_device_traps;
    BOOL haveIECDevice;
  
    resources_get_int("DriveTrueEmulation", &drive_true_emulation);
    resources_get_int("VirtualDevices", &virtual_device_traps);
    haveIECDevice = IsDlgButtonChecked(hwnd, IDC_PRINTER_USEIECDEVICE) == BST_CHECKED;
  
    if (num > 0 && ((drive_true_emulation || !virtual_device_traps) && !haveIECDevice)) {
        EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_TYPE), FALSE);
        SendMessage(GetDlgItem(hwnd, IDC_PRINTER_TYPE), CB_SETCURSEL, 0, 0);
    } else {
        EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_TYPE), TRUE);
    }

    res_value = (int)SendMessage(GetDlgItem(hwnd, IDC_PRINTER_TYPE), CB_GETCURSEL, 0, 0);
  
    is_enabled = res_value == PRINTER_DEVICE_FS;
  
    EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_FORMFEED), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_DRIVER), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_OUTPUT), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_TEXTOUT), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_OUTPUT_FILE1_NAME), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_OUTPUT_FILE2_NAME), is_enabled);
    EnableWindow(GetDlgItem(hwnd, IDC_PRINTER_OUTPUT_FILE3_NAME), is_enabled);
}

static uilib_localize_dialog_param printer_dialog_trans[] = {
    { IDC_PRINTER_EMULATION, IDS_PRINTER_EMULATION, 0 },
    { IDC_PRINTER_FORMFEED, IDS_PRINTER_FORMFEED, 0 },
    { IDC_FILESYSTEM_PRINTER, IDS_FILESYSTEM_PRINTER, 0 },
    { IDC_PRINTER_DRVR, IDS_PRINTER_DRVR, 0 },
    { IDC_OUTPUT, IDS_OUTPUT, 0 },
    { IDC_OUTPUT_TO_FILE, IDS_OUTPUT_TO_FILE, 0 },
    { IDC_OUTPUT_FILE1_NAME, IDS_OUTPUT_FILE1_NAME, 0 },
    { IDC_OUTPUT_FILE2_NAME, IDS_OUTPUT_FILE2_NAME, 0 },
    { IDC_OUTPUT_FILE3_NAME, IDS_OUTPUT_FILE3_NAME, 0 },
    { 0, 0, 0 }
};

static uilib_localize_dialog_param printer_iec_dialog_trans[] = {
    { IDC_PRINTER_USEIECDEVICE, IDS_TOGGLE_USEIECDEVICE, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group printer_left_group[] = {
    { IDC_PRINTER_EMULATION, 0 },
    { IDC_PRINTER_DRVR, 0 },
    { IDC_OUTPUT, 0 },
    { IDC_OUTPUT_TO_FILE, 0 },
    { IDC_OUTPUT_FILE1_NAME, 0 },
    { IDC_OUTPUT_FILE2_NAME, 0 },
    { IDC_OUTPUT_FILE3_NAME, 0 },
    { 0, 0 }
};

static uilib_dialog_group printer_top_right_group[] = {
    { IDC_PRINTER_TYPE, 0 },
    { IDC_PRINTER_DRIVER, 0 },
    { IDC_PRINTER_OUTPUT, 0 },
    { IDC_PRINTER_TEXTOUT, 0 },
    { 0, 0 }
};

static uilib_dialog_group printer_bottom_right_group[] = {
    { IDC_PRINTER_OUTPUT_FILE1_NAME, 0 },
    { IDC_PRINTER_OUTPUT_FILE2_NAME, 0 },
    { IDC_PRINTER_OUTPUT_FILE3_NAME, 0 },
    { 0, 0 }
};

static uilib_dialog_group formfeed_group[] = {
    { IDC_PRINTER_FORMFEED, 1 },
    { 0, 0 }
};

static void init_printer_dialog(unsigned int num, HWND hwnd)
{
    HWND printer_hwnd;
    int i, res_value, res_value_loop;
    char  printer_name[30];
    const char *res_string;
    int current = 0;
    int xmax;
    int xpos;
    int size;
    const int *ui_printer = 
#ifdef HAVE_OPENCBM
      (opencbmlib_is_available()) ? ui_printer_opencbm :
#endif
      ui_printer_normal;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, printer_dialog_trans);

    if (num != 0) {
        /* translate the iec dialog item(s) */
        uilib_localize_dialog(hwnd, printer_iec_dialog_trans);
    }

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, printer_left_group);

    /* get the max x of the bottom right group */
    uilib_get_group_max_x(hwnd, printer_bottom_right_group, &xmax);

    /* get the size of one of the elements of the bottom right group */
    uilib_get_element_size(hwnd, IDC_PRINTER_OUTPUT_FILE3_NAME, &size);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, printer_left_group, &xpos);

    /* move the top right group to the correct position */
    uilib_move_group(hwnd, printer_top_right_group, xpos + 10);

    /* move the bottom right group to the correct position */
    uilib_move_group(hwnd, printer_bottom_right_group, xpos + 10);

    /* get the max x of the bottom right group */
    uilib_get_group_max_x(hwnd, printer_bottom_right_group, &xpos);

    /* set the size of the bottom right group */
    uilib_set_group_width(hwnd, printer_bottom_right_group, size + xpos - xmax);

    /* get the max x of the printer emulation drop down element */
    uilib_get_element_max_x(hwnd, IDC_PRINTER_TYPE, &xpos);

    /* adjust the size of the elements in formfeed group */
    uilib_adjust_group_width(hwnd, formfeed_group);

    /* move the send formfeed button */
    uilib_move_element(hwnd, IDC_PRINTER_FORMFEED, xpos + 10);

    if (num == 0) {
        sprintf(printer_name, "PrinterUserport");
    } else {
        sprintf(printer_name, "Printer%d", num);
    }

    resources_get_int(printer_name, &res_value);
    printer_hwnd = GetDlgItem(hwnd, IDC_PRINTER_TYPE);
    for (res_value_loop = 0; ui_printer[res_value_loop]; res_value_loop++) {
        SendMessage(printer_hwnd, CB_ADDSTRING, 0, (LPARAM)intl_translate_tcs(ui_printer[res_value_loop]));
    }
    SendMessage(printer_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    resources_get_string_sprintf("%sDriver", &res_string, printer_name);
    printer_hwnd = GetDlgItem(hwnd, IDC_PRINTER_DRIVER);
    if (num == 6) {
        for (res_value_loop = 0; ui_plotter_driver[res_value_loop]; res_value_loop++) {
            SendMessage(printer_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_plotter_driver[res_value_loop]);
            if (!strcmp(ui_plotter_driver_1520[res_value_loop], res_string)) {
                current = res_value_loop;
            }
        }
    } else {
        if (num == 0) {
            for (res_value_loop = 0; ui_userprinter_driver[res_value_loop]; res_value_loop++) {
                SendMessage(printer_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_userprinter_driver[res_value_loop]);
                if (!strcmp(ui_userprinter_driver_ascii[res_value_loop], res_string)) {
                    current = res_value_loop;
                }
            }
        } else {
            for (res_value_loop = 0; ui_printer_driver[res_value_loop]; res_value_loop++) {
                SendMessage(printer_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_printer_driver[res_value_loop]);
                if (!strcmp(ui_printer_driver_ascii[res_value_loop], res_string)) {
                    current = res_value_loop;
                }
            }
        }
    }
    SendMessage(printer_hwnd, CB_SETCURSEL, (WPARAM)current, 0);

    resources_get_string_sprintf("%sOutput", &res_string, printer_name);
    printer_hwnd = GetDlgItem(hwnd, IDC_PRINTER_OUTPUT);
    for (res_value_loop = 0; ui_printer_output[res_value_loop]; res_value_loop++) {
        SendMessage(printer_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_printer_output[res_value_loop]);
        if (!strcmp(ui_printer_output_ascii[res_value_loop], res_string)) {
            current = res_value_loop;
        }
    }
    SendMessage(printer_hwnd, CB_SETCURSEL, (WPARAM)current, 0);

    resources_get_int_sprintf("%sTextDevice", &res_value, printer_name);
    printer_hwnd = GetDlgItem(hwnd, IDC_PRINTER_TEXTOUT);
    for (res_value_loop = 0; ui_printer_text_device[res_value_loop]; res_value_loop++) {
        SendMessage(printer_hwnd, CB_ADDSTRING, 0, (LPARAM)ui_printer_text_device[res_value_loop]);
    }
    SendMessage(printer_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);

    if (num > 0 && (iec_available_busses() & IEC_BUS_IEC)) {
        resources_get_int_sprintf("IECDevice%d", &res_value, num);
        CheckDlgButton(hwnd, IDC_PRINTER_USEIECDEVICE, res_value ? BST_CHECKED : BST_UNCHECKED);
    } else {
        ShowWindow(GetDlgItem(hwnd, IDC_PRINTER_USEIECDEVICE), FALSE);
        CheckDlgButton(hwnd, IDC_PRINTER_USEIECDEVICE, BST_UNCHECKED);
    }
    
    for (i = 0; i < 3; i++) {
        resources_get_string_sprintf("PrinterTextDevice%d", &res_string, i + 1);
        if (res_string) {
            strncpy(printertextdevice[i], res_string, MAX_PATH);
        }
    }

    enable_printer_controls(num, hwnd);
}

static BOOL store_printer_dialog_results(HWND hwnd, unsigned int num)
{
    char printer_name[30];
  
    if (num == 0) {
        sprintf(printer_name, "PrinterUserport");
    } else {
        sprintf(printer_name, "Printer%d", num);
    }

    resources_set_int(printer_name, (int)SendMessage(GetDlgItem(hwnd, IDC_PRINTER_TYPE), CB_GETCURSEL, 0, 0));

    if (num == 6) {
        resources_set_string_sprintf("%sDriver", ui_plotter_driver_1520[SendMessage(GetDlgItem(hwnd, IDC_PRINTER_DRIVER), CB_GETCURSEL, 0, 0)], printer_name);
    } else {
        if (num == 0) {
            resources_set_string_sprintf("%sDriver", ui_userprinter_driver_ascii[SendMessage(GetDlgItem(hwnd, IDC_PRINTER_DRIVER), CB_GETCURSEL, 0, 0)], printer_name);
        } else {
            resources_set_string_sprintf("%sDriver", ui_printer_driver_ascii[SendMessage(GetDlgItem(hwnd, IDC_PRINTER_DRIVER), CB_GETCURSEL, 0, 0)], printer_name);
        }
    }

    resources_set_string_sprintf("%sOutput", ui_printer_output_ascii[SendMessage(GetDlgItem(hwnd, IDC_PRINTER_OUTPUT), CB_GETCURSEL, 0, 0)], printer_name);

    resources_set_int_sprintf("%sTextDevice", (int)SendMessage(GetDlgItem(hwnd, IDC_PRINTER_TEXTOUT), CB_GETCURSEL, 0, 0), printer_name);
  
    resources_set_string("PrinterTextDevice1", printertextdevice[0]);
    resources_set_string("PrinterTextDevice2", printertextdevice[1]);
    resources_set_string("PrinterTextDevice3", printertextdevice[2]);

    if (num > 0 && (iec_available_busses() & IEC_BUS_IEC)) {
        resources_set_int_sprintf("IECDevice%d", (IsDlgButtonChecked(hwnd, IDC_PRINTER_USEIECDEVICE) == BST_CHECKED), num);
    }

    return 1;
}

static void printer_set_active(HWND hwnd)
{
    TCHAR *st;

    st = system_mbstowcs_alloc(printertextdevice[0]);
    SetDlgItemText(hwnd, IDC_PRINTER_OUTPUT_FILE1_NAME, st);
    system_mbstowcs_free(st);
    st = system_mbstowcs_alloc(printertextdevice[1]);
    SetDlgItemText(hwnd, IDC_PRINTER_OUTPUT_FILE2_NAME, st);
    system_mbstowcs_free(st);
    st = system_mbstowcs_alloc(printertextdevice[2]);
    SetDlgItemText(hwnd, IDC_PRINTER_OUTPUT_FILE3_NAME, st);
    system_mbstowcs_free(st);
}

static void printer_kill_active(HWND hwnd)
{
    TCHAR st[MAX_PATH];

    GetDlgItemText(hwnd, IDC_PRINTER_OUTPUT_FILE1_NAME, st, MAX_PATH);
    system_wcstombs(printertextdevice[0], st, MAX_PATH);
    GetDlgItemText(hwnd, IDC_PRINTER_OUTPUT_FILE2_NAME, st, MAX_PATH);
    system_wcstombs(printertextdevice[1], st, MAX_PATH);
    GetDlgItemText(hwnd, IDC_PRINTER_OUTPUT_FILE3_NAME, st, MAX_PATH);
    system_wcstombs(printertextdevice[2], st, MAX_PATH);
}

static BOOL CALLBACK printer_dialog_proc(unsigned int num, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDC_PRINTER_TYPE:
                case IDC_PRINTER_USEIECDEVICE:
                    enable_printer_controls(num, hwnd);
                    break;
                case IDC_PRINTER_FORMFEED:
                    switch (num) {
                        case 4:
                            printer_formfeed(0);
                            break;
                        case 5:
                            printer_formfeed(1);
                            break;
                        case 6:
                            printer_formfeed(2);
                            break;
                        case 0:
                            printer_formfeed(3);
                            break;
                    }
                    break;
            }
            return FALSE;
        case WM_NOTIFY:
            {
                NMHDR FAR *nmhdr = (NMHDR FAR *)(lparam);

                switch (nmhdr->code) {
                    case PSN_APPLY:
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, store_printer_dialog_results(hwnd, num) ? PSNRET_NOERROR : PSNRET_INVALID);
                        return TRUE;
                    case PSN_SETACTIVE:
                        printer_set_active(hwnd);
                        return TRUE;
                    case PSN_KILLACTIVE:
                        printer_kill_active(hwnd);
                        return TRUE;
                }
                break;
            }
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_printer_dialog(num, hwnd);
            return TRUE;
    }

    return FALSE;
}


#define _CALLBACK_PRINTER(num)                                                              \
static INT_PTR CALLBACK callback_##num(HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam) \
{                                                                                           \
    return printer_dialog_proc(num, dialog, msg, wparam, lparam);                           \
}

_CALLBACK_PRINTER(0)
_CALLBACK_PRINTER(4)
_CALLBACK_PRINTER(5)
_CALLBACK_PRINTER(6)


#ifdef HAVE_OPENCBM
static void init_device_7_dialog(HWND hwnd)
{
    int res_value;

    /* translate the iec dialog item(s) */
    uilib_localize_dialog(hwnd, printer_iec_dialog_trans);

    if (iec_available_busses() & IEC_BUS_IEC) {
        resources_get_int("IECDevice7", &res_value);
        CheckDlgButton(hwnd, IDC_PRINTER_USEIECDEVICE, res_value ? BST_CHECKED : BST_UNCHECKED);
    } else {
        ShowWindow(GetDlgItem(hwnd, IDC_PRINTER_USEIECDEVICE), FALSE);
        CheckDlgButton(hwnd, IDC_PRINTER_USEIECDEVICE, BST_UNCHECKED);
    }
}

static INT_PTR CALLBACK callback_7(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_COMMAND:
            return FALSE;
        case WM_NOTIFY:
            {
                NMHDR FAR *nmhdr = (NMHDR FAR *)(lparam);

                switch (nmhdr->code) {
                    case PSN_APPLY:
                        resources_set_int("IECDevice7", (IsDlgButtonChecked(hwnd, IDC_PRINTER_USEIECDEVICE) == BST_CHECKED));
                        return TRUE;
                    case PSN_SETACTIVE:
                    case PSN_KILLACTIVE:
                        return TRUE;
                }
                break;
            }
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_device_7_dialog(hwnd);
            return TRUE;
    }

    return FALSE;
}
#endif

/* -------------------------------------------------------------------------- */
/*                               Main Dialog                                  */
/* -------------------------------------------------------------------------- */

static void uiperipheral_dialog(HWND hwnd)
{
    PROPSHEETPAGE psp[9];
    PROPSHEETHEADER psh;
    int i, no_of_drives, no_of_printers;
#ifdef HAVE_OPENCBM
    int EXTRA_DEVICES = opencbmlib_is_available();
#else
/* EXTRA_DEVICES will get optimized away if compiled without opencbm support */
#define EXTRA_DEVICES 0
#endif

    for (i = 0; i < 3; i++ ) {
        printertextdevice[i] = lib_malloc(MAX_PATH);
        strcpy(printertextdevice[i], "");
    }

    no_of_drives = 4;
    no_of_printers = 3;

    if (have_printer_userport < 0) {
        have_printer_userport = has_userport_printer();
    }
    if (have_printer_userport) {
        no_of_printers++;
    }

    for (i = 0; i < no_of_printers; i++) {
        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
        psp[i].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
        psp[i].pszTemplate = MAKEINTRESOURCE(IDD_PRINTER_SETTINGS_DIALOG);
        psp[i].pszIcon = NULL;
#else
        psp[i].DUMMYUNIONNAME.pszTemplate
            = MAKEINTRESOURCE(IDD_PRINTER_SETTINGS_DIALOG);
        psp[i].u2.pszIcon = NULL;
#endif
        psp[i].lParam = 0;
        psp[i].pfnCallback = NULL;
    }

#ifdef HAVE_OPENCBM
    if (EXTRA_DEVICES) {
        psp[i].dwSize = sizeof(PROPSHEETPAGE);
        psp[i].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
        psp[i].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
        psp[i].pszTemplate = MAKEINTRESOURCE(IDD_DEVICE7_SETTINGS_DIALOG);
        psp[i].pszIcon = NULL;
#else
        psp[i].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_DEVICE7_SETTINGS_DIALOG);
        psp[i].u2.pszIcon = NULL;
#endif
        psp[i].lParam = 0;
        psp[i].pfnCallback = NULL;
    }
#endif

    for (i = 0; i < no_of_drives; i++) {
        psp[no_of_printers + EXTRA_DEVICES + i].dwSize = sizeof(PROPSHEETPAGE);
        psp[no_of_printers + EXTRA_DEVICES + i].dwFlags = PSP_USETITLE /*| PSP_HASHELP*/ ;
        psp[no_of_printers + EXTRA_DEVICES + i].hInstance = winmain_instance;
#ifdef _ANONYMOUS_UNION
#ifdef HAVE_OPENCBM
        if (opencbmlib_is_available()) {
            psp[no_of_printers + EXTRA_DEVICES + i].pszTemplate = MAKEINTRESOURCE(IDD_DISKDEVICE_OPENCBM_DIALOG);
        } else
#endif
        {
            psp[no_of_printers + EXTRA_DEVICES + i].pszTemplate = MAKEINTRESOURCE(IDD_DISKDEVICE_DIALOG);
        }
        psp[no_of_printers + EXTRA_DEVICES + i].pszIcon = NULL;
#else
#ifdef HAVE_OPENCBM
        if (opencbmlib_is_available()) {
            psp[no_of_printers + EXTRA_DEVICES + i].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_DISKDEVICE_OPENCBM_DIALOG);
        } else 
#endif
        {
            psp[no_of_printers + EXTRA_DEVICES + i].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(IDD_DISKDEVICE_DIALOG);
        }
        psp[no_of_printers + EXTRA_DEVICES + i].u2.pszIcon = NULL;
#endif
        psp[no_of_printers + EXTRA_DEVICES + i].lParam = 0;
        psp[no_of_printers + EXTRA_DEVICES + i].pfnCallback = NULL;
    }

    if (have_printer_userport) {
        psp[0].pfnDlgProc = callback_0;
        psp[0].pszTitle = intl_translate_tcs(IDS_PRINTER_USERPORT);
        i = 1;
    } else {
        i = 0;
    }

    psp[i + 0].pfnDlgProc = callback_4;
    psp[i + 0].pszTitle = intl_translate_tcs(IDS_PRINTER_4);
    psp[i + 1].pfnDlgProc = callback_5;
    psp[i + 1].pszTitle = intl_translate_tcs(IDS_PRINTER_5);
    psp[i + 2].pfnDlgProc = callback_6;
    psp[i + 2].pszTitle = intl_translate_tcs(IDS_PRINTER_6);
#ifdef HAVE_OPENCBM
    if (EXTRA_DEVICES) {
        psp[i + 3].pfnDlgProc = callback_7;
        psp[i + 3].pszTitle = intl_translate_tcs(IDS_DEVICE_7);
    }
#endif
    psp[i + EXTRA_DEVICES + 3].pfnDlgProc = callback_8;
    psp[i + EXTRA_DEVICES + 3].pszTitle = intl_translate_tcs(IDS_DRIVE_8);
    psp[i + EXTRA_DEVICES + 4].pfnDlgProc = callback_9;
    psp[i + EXTRA_DEVICES + 4].pszTitle = intl_translate_tcs(IDS_DRIVE_9);
    psp[i + EXTRA_DEVICES + 5].pfnDlgProc = callback_10;
    psp[i + EXTRA_DEVICES + 5].pszTitle = intl_translate_tcs(IDS_DRIVE_10);
    psp[i + EXTRA_DEVICES + 6].pfnDlgProc = callback_11;
    psp[i + EXTRA_DEVICES + 6].pszTitle = intl_translate_tcs(IDS_DRIVE_11);

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwnd;
    psh.hInstance = winmain_instance;
    psh.pszCaption = intl_translate_tcs(IDS_PERIPHERAL_SETTINGS);
    psh.nPages = no_of_drives + no_of_printers + EXTRA_DEVICES;
#ifdef _ANONYMOUS_UNION
    psh.pszIcon = NULL;
    psh.nStartPage = i + 3;
    psh.ppsp = psp;
#else
    psh.DUMMYUNIONNAME.pszIcon = NULL;
    psh.u2.nStartPage = i + 3;
    psh.u3.ppsp = psp;
#endif
    psh.pfnCallback = NULL;

    PropertySheet(&psh);

    for (i = 0; i < 3; i++) {
        lib_free(printertextdevice[i]);
    }
}

void uiperipheral_command(HWND hwnd, WPARAM wparam)
{
    switch (wparam & 0xffff) {
        case IDM_DEVICEMANAGER:
            uiperipheral_dialog(hwnd);
            break;
        case IDM_FORMFEED_PRINTERIEC4:
            printer_formfeed(0);
            break;
        case IDM_FORMFEED_PRINTERIEC5:
            printer_formfeed(1);
            break;
        case IDM_FORMFEED_PRINTERIEC6:
            printer_formfeed(2);
            break;
    }
}
