/*
 * uijoyport.c - Implementation of the joyport system settings dialog box.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Marcus Sutton <loggedoubt@gmail.com>
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
#include "joyport.h"
#include "lib.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "types.h"
#include "uilib.h"
#include "uimouse.h"
#include "winmain.h"

#include "uijoyport.h"


static int ports[JOYPORT_MAX_PORTS] = { 0, 0, 0, 0, 0};

typedef struct port_param_s {
    int idc_label;
    int idc_cb;
    const char *resname;
    joyport_desc_t *devices;
} port_param_t;

static port_param_t port_params[JOYPORT_MAX_PORTS] = {
    { IDC_JOYPORT1_LABEL, IDC_JOYPORT1, "JoyPort1Device", NULL },
    { IDC_JOYPORT2_LABEL, IDC_JOYPORT2, "JoyPort2Device", NULL },
    { IDC_JOYPORT3_LABEL, IDC_JOYPORT3, "JoyPort3Device", NULL },
    { IDC_JOYPORT4_LABEL, IDC_JOYPORT4, "JoyPort4Device", NULL },
    { IDC_JOYPORT5_LABEL, IDC_JOYPORT5, "JoyPort5Device", NULL }
};

static void enable_joyport_controls(HWND hwnd)
{
    int port;

    for (port = 0; port < JOYPORT_MAX_PORTS; port++) {
        if (ports[port]) {
            EnableWindow(GetDlgItem(hwnd, port_params[port].idc_cb), 1);
        }
    }
}

static uilib_localize_dialog_param joyport_dialog_trans[] = {
    { 0, IDS_JOYPORT_CAPTION, -1 },
    { IDC_JOYPORT_BBRTC_SAVE, IDS_JOYPORT_BBRTC_SAVE, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group joyport_left_group[JOYPORT_MAX_PORTS + 1];
static uilib_dialog_group joyport_right_group[JOYPORT_MAX_PORTS + 1];

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_joyport_dialog(HWND hwnd)
{
    TCHAR st_name[48];
    TCHAR st_text[64];
    HWND port_hwnd;
    int res_value;
    int xpos;
    RECT rect;
    int port;
    int i = 0;
    int joy = 0;

    /* localize possible port items, and build left and right groups */
    for (port = 0; port < JOYPORT_MAX_PORTS; port++) {
        if (ports[port]) {
            EnableWindow(GetDlgItem(hwnd, port_params[port].idc_cb), 1);
            /* port trans names are in common code, so we must convert them */
            system_mbstowcs(st_name, translate_text(joyport_get_port_trans_name(port)), 48);
            lib_sntprintf(st_text, 64, intl_translate_tcs(IDS_JOYPORT_S_DEVICE), st_name);
            port_hwnd = GetDlgItem(hwnd, port_params[port].idc_label);
            SetWindowText(port_hwnd, st_text);
            joyport_left_group[i].idc = port_params[port].idc_label;
            joyport_left_group[i].element_type = 0;
            joyport_right_group[i].idc = port_params[port].idc_cb;
            joyport_right_group[i].element_type = 0;
            ++i;
        }
    }

    joyport_left_group[i].idc = 0;
    joyport_left_group[i].element_type = 0;
    joyport_right_group[i].idc = 0;
    joyport_right_group[i].element_type = 0;

    /* translate other dialog items */
    uilib_localize_dialog(hwnd, joyport_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, joyport_left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, joyport_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, joyport_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, joyport_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    /* handle items of the ports */
    for (port = 0; port < JOYPORT_MAX_PORTS; port++) {
        if (ports[port]) {
            port_params[port].devices = joyport_get_valid_devices(port);
            port_hwnd = GetDlgItem(hwnd, port_params[port].idc_cb);
            resources_get_int(port_params[port].resname, &joy);
            res_value = 0;
            for (i = 0; port_params[port].devices[i].name; i++) {
                /* device trans names are in common code, so we must convert them */
                system_mbstowcs(st_name, translate_text(port_params[port].devices[i].trans_name), 48);
                SendMessage(port_hwnd, CB_ADDSTRING, 0, (LPARAM)st_name);
                if (port_params[port].devices[i].id == joy) {
                    res_value = i;
                }
            }
            SendMessage(port_hwnd, CB_SETCURSEL, (WPARAM)res_value, 0);
        }
    }

    resources_get_int("BBRTCSave", &res_value);
    CheckDlgButton(hwnd, IDC_JOYPORT_BBRTC_SAVE, res_value ? BST_CHECKED : BST_UNCHECKED);

    enable_joyport_controls(hwnd);
}

static void free_device_ports(void)
{
    int port;

    for (port = 0; port < JOYPORT_MAX_PORTS; port++) {
        if (port_params[port].devices) {
            lib_free(port_params[port].devices);
            port_params[port].devices = NULL;
        }
    }
}

static void end_joyport_dialog(HWND hwnd)
{
    int port;
    int joy = 0;
    int id = 0;

    /* Because all ports need to be set at the same time,
       and actually they are set one at a time,
       set all ports to 'NONE' so there are 'fake' conflicts.
     */
    for (port = 0; port < JOYPORT_MAX_PORTS; port++) {
        if (ports[port]) {
            resources_set_int(port_params[port].resname, JOYPORT_ID_NONE);
        }
    }

    for (port = 0; port < JOYPORT_MAX_PORTS; port++) {
        if (ports[port]) {
            joy = (int)SendMessage(GetDlgItem(hwnd, port_params[port].idc_cb), CB_GETCURSEL, 0, 0);
            id = port_params[port].devices[joy].id;
            resources_set_int(port_params[port].resname, id);
        }
    }

    resources_set_int("BBRTCSave", (IsDlgButtonChecked(hwnd, IDC_JOYPORT_BBRTC_SAVE) == BST_CHECKED ? 1 : 0 ));
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_joyport_dialog(hwnd);
                case IDCANCEL:
                    free_device_ports();
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            free_device_ports();
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_joyport_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

void ui_joyport_settings_dialog(HWND hwnd, int port1, int port2, int port3, int port4, int port5)
{
    int total = (port1 << 4) | (port2 << 3) | (port3 << 2) | (port4 << 1) | port5;
    ports[0] = port1;
    ports[1] = port2;
    ports[2] = port3;
    ports[3] = port4;
    ports[4] = port5;

    switch (total) {
        case 31:
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_JOYPORT12345_SETTINGS_DIALOG, hwnd, dialog_proc);
            break;
        case 30:
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_JOYPORT1234_SETTINGS_DIALOG, hwnd, dialog_proc);
            break;
        case 28:
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_JOYPORT123_SETTINGS_DIALOG, hwnd, dialog_proc);
            break;
        case 24:
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_JOYPORT12_SETTINGS_DIALOG, hwnd, dialog_proc);
            break;
        case 22:
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_JOYPORT134_SETTINGS_DIALOG, hwnd, dialog_proc);
            break;
        case 6:
            DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_JOYPORT34_SETTINGS_DIALOG, hwnd, dialog_proc);
            break;
    }
}
