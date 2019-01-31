/*
 * uinetwork.c - Implementation of the network dialog.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
#include <tchar.h>
#include <commdlg.h>

#include "intl.h"
#include "network.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "uiapi.h"
#include "uilib.h"
#include "vsync.h"
#include "winmain.h"

#include "uinetwork.h"


static uilib_localize_dialog_param network_dialog_trans[] = {
    { 0, IDS_NETWORK_CAPTION, -1 },
    { IDC_CURRENT_MODE, IDS_CURRENT_MODE, 0 },
    { IDC_SERVER_BIND, IDS_SERVER_BIND, 0 },
    { IDC_TCP_PORT, IDS_TCP_PORT, 0 },
    { IDC_NETWORK_SERVER, IDS_NETWORK_SERVER, 0 },
    { IDC_NETWORK_CLIENT, IDS_NETWORK_CLIENT, 0 },
    { IDC_NETWORK_DISCONNECT, IDS_NETWORK_DISCONNECT, 0 },
    { IDC_CONTROL, IDS_CONTROL, 0 },
    { IDC_SERVER, IDS_SERVER, 0 },
    { IDC_CLIENT, IDS_CLIENT, 0 },
    { IDC_KEYBOARD, IDS_KEYBOARD, 0 },
    { IDC_JOYSTICK_1, IDS_JOYSTICK_1, 0 },
    { IDC_JOYSTICK_2, IDS_JOYSTICK_2, 0 },
    { IDC_DEVICES, IDS_DEVICES, 0 },
    { IDC_SETTINGS, IDS_SETTINGS, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group network_adjust_group[] = {
    { IDC_CURRENT_MODE, 0 },
    { IDC_SERVER_BIND, 0 },
    { IDC_TCP_PORT, 0 },
    { IDC_NETWORK_SERVER, 1 },
    { IDC_NETWORK_CLIENT, 1 },
    { IDC_NETWORK_DISCONNECT, 1 },
    { 0, 0 }
};

static uilib_dialog_group network_left_group[] = {
    { IDC_CURRENT_MODE, 0 },
    { IDC_SERVER_BIND, 0 },
    { IDC_TCP_PORT, 0 },
    { IDC_NETWORK_CLIENT, 1 },
    { 0, 0 }
};

static uilib_dialog_group network_boxes_group[] = {
    { IDC_NETWORK_MODE, 0 },
    { IDC_NETWORK_SERVER_BIND, 0 },
    { IDC_NETWORK_PORT, 0 },
    { IDC_NETWORK_SERVERNAME, 0 },
    { 0, 0 }
};

static uilib_dialog_group network_middle_group[] = {
    { IDC_KEYBOARD, 0 },
    { IDC_JOYSTICK_1, 0 },
    { IDC_JOYSTICK_2, 0 },
    { IDC_DEVICES, 0 },
    { IDC_SETTINGS, 0 },
    { 0, 0 }
};

static uilib_dialog_group server_check_group[] = {
    { IDC_SERVER, 0 },
    { IDC_NETWORK_KEYB_SERVER, 0 },
    { IDC_NETWORK_JOY1_SERVER, 0 },
    { IDC_NETWORK_JOY2_SERVER, 0 },
    { IDC_NETWORK_DEVC_SERVER, 0 },
    { IDC_NETWORK_RSRC_SERVER, 0 },
    { 0, 0 }
};

static uilib_dialog_group client_check_group[] = {
    { IDC_CLIENT, 0 },
    { IDC_NETWORK_KEYB_CLIENT, 0 },
    { IDC_NETWORK_JOY1_CLIENT, 0 },
    { IDC_NETWORK_JOY2_CLIENT, 0 },
    { IDC_NETWORK_DEVC_CLIENT, 0 },
    { IDC_NETWORK_RSRC_CLIENT, 0 },
    { 0, 0 }
};

static void init_network_dialog(HWND hwnd)
{
    int port;
    const char *server_name;
    const char *server_bind_address;
    int control;
    TCHAR st[256];
    int connected;
    int xpos;
    int xstart;
    int xtemp;
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, network_dialog_trans);

    /* adjust the size of the elements in the adjust group */
    uilib_adjust_group_width(hwnd, network_adjust_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, network_left_group, &xpos);

    /* move the elements of the boxes group */
    uilib_move_group(hwnd, network_boxes_group, xpos + 10);

    /* get the max x of the boxes group */
    uilib_get_group_max_x(hwnd, network_boxes_group, &xpos);

    /* get the size of the start server button */
    uilib_get_element_width(hwnd, IDC_NETWORK_SERVER, &xtemp);

    /* move/center the start server button */
    uilib_move_element(hwnd, IDC_NETWORK_SERVER, (xpos - xtemp) / 2);

    /* get the size of the disconnect button */
    uilib_get_element_width(hwnd, IDC_NETWORK_DISCONNECT, &xtemp);

    /* move/center the disconnect button */
    uilib_move_element(hwnd, IDC_NETWORK_DISCONNECT, (xpos - xtemp) / 2);

    /* move and adjust the middle group */
    uilib_move_and_adjust_group_width(hwnd, network_middle_group, xpos + 20);

    xstart = xpos + 15;

    /* adjust the server element */
    uilib_adjust_element_width(hwnd, IDC_SERVER);

    /* adjust the client element */
    uilib_adjust_element_width(hwnd, IDC_CLIENT);

    /* get the max x of the middle group */
    uilib_get_group_max_x(hwnd, network_middle_group, &xpos);

    /* move the server check group */
    uilib_move_group(hwnd, server_check_group, xpos + 10);

    /* get the size of the server header element */
    uilib_get_element_width(hwnd, IDC_SERVER, &xtemp);

    /* move/center the server header element */
    uilib_move_element(hwnd, IDC_SERVER, xpos + 10 - ((xtemp - 10) / 2));

    /* get the max x of the server check group */
    uilib_get_group_max_x(hwnd, server_check_group, &xpos);

    /* move the client check group */
    uilib_move_group(hwnd, client_check_group, xpos + 10);

    /* get the size of the client header element */
    uilib_get_element_width(hwnd, IDC_CLIENT, &xtemp);

    /* move/center the client header element */
    uilib_move_element(hwnd, IDC_CLIENT, xpos + 10 - ((xtemp - 10) / 2));

    /* get the max x of the client check group */
    uilib_get_group_max_x(hwnd, client_check_group, &xpos);

    /* move and resize the control group element */
    uilib_move_and_set_element_width(hwnd, IDC_CONTROL, xstart, xpos - xstart + 5);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 15, rect.bottom - rect.top, TRUE);

    resources_get_int("NetworkServerPort", &port);
    resources_get_string("NetworkServerName", &server_name);
    resources_get_string("NetworkServerBindAddress", &server_bind_address);
    resources_get_int("NetworkControl", &control);

    SetDlgItemInt(hwnd, IDC_NETWORK_PORT, port, TRUE);
    system_mbstowcs(st, server_name, 256);
    SetDlgItemText(hwnd, IDC_NETWORK_SERVERNAME, st);
    system_mbstowcs(st, server_bind_address, 256);
    SetDlgItemText(hwnd, IDC_NETWORK_SERVER_BIND, st);

    switch(network_get_mode()) {
        case NETWORK_IDLE:
            SetDlgItemText(hwnd, IDC_NETWORK_MODE, intl_translate_tcs(IDS_IDLE));
            break;
        case NETWORK_SERVER:
            SetDlgItemText(hwnd, IDC_NETWORK_MODE, intl_translate_tcs(IDS_SERVER_LISTENING));
            break;
        case NETWORK_SERVER_CONNECTED:
            SetDlgItemText(hwnd, IDC_NETWORK_MODE, intl_translate_tcs(IDS_CONNECTED_SERVER));
            break;
        case NETWORK_CLIENT:
            SetDlgItemText(hwnd, IDC_NETWORK_MODE, intl_translate_tcs(IDS_CONNECTED_CLIENT));
            break;
    }

    CheckDlgButton(hwnd, IDC_NETWORK_KEYB_SERVER, control & NETWORK_CONTROL_KEYB ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_KEYB_CLIENT, control & NETWORK_CONTROL_KEYB << NETWORK_CONTROL_CLIENTOFFSET ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_JOY1_SERVER, control & NETWORK_CONTROL_JOY1 ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_JOY1_CLIENT, control & NETWORK_CONTROL_JOY1 << NETWORK_CONTROL_CLIENTOFFSET ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_JOY2_SERVER, control & NETWORK_CONTROL_JOY2 ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_JOY2_CLIENT, control & NETWORK_CONTROL_JOY2 << NETWORK_CONTROL_CLIENTOFFSET ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_DEVC_SERVER, control & NETWORK_CONTROL_DEVC ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_DEVC_CLIENT, control & NETWORK_CONTROL_DEVC << NETWORK_CONTROL_CLIENTOFFSET ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_RSRC_SERVER, control & NETWORK_CONTROL_RSRC ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_NETWORK_RSRC_CLIENT, control & NETWORK_CONTROL_RSRC << NETWORK_CONTROL_CLIENTOFFSET ? BST_CHECKED : BST_UNCHECKED);

    connected = ((network_get_mode() != NETWORK_IDLE) ? 1 : 0);
    EnableWindow(GetDlgItem(hwnd, IDC_NETWORK_PORT), !connected);
    EnableWindow(GetDlgItem(hwnd, IDC_NETWORK_SERVERNAME), !connected);
    EnableWindow(GetDlgItem(hwnd, IDC_NETWORK_SERVER_BIND), !connected);
    EnableWindow(GetDlgItem(hwnd, IDC_NETWORK_CLIENT), !connected);
    EnableWindow(GetDlgItem(hwnd, IDC_NETWORK_SERVER), !connected);
    EnableWindow(GetDlgItem(hwnd, IDC_NETWORK_DISCONNECT), connected);
    EnableWindow(GetDlgItem(hwnd, IDC_NETWORK_MODE), 0);

    SetFocus(connected ? GetDlgItem(hwnd, IDC_NETWORK_DISCONNECT) : GetDlgItem(hwnd, IDC_NETWORK_SERVER));
}

static int set_resources(HWND hwnd)
{
    TCHAR st_text[MAX_PATH];
    char text[MAX_PATH];
    int port;
    unsigned int control = 0;

    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_KEYB_SERVER) == BST_CHECKED) {
        control |= NETWORK_CONTROL_KEYB;
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_KEYB_CLIENT) == BST_CHECKED) {
        control |= (NETWORK_CONTROL_KEYB << NETWORK_CONTROL_CLIENTOFFSET);
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_JOY1_SERVER) == BST_CHECKED) {
        control |= NETWORK_CONTROL_JOY1;
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_JOY1_CLIENT) == BST_CHECKED) {
        control |= (NETWORK_CONTROL_JOY1 << NETWORK_CONTROL_CLIENTOFFSET);
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_JOY2_SERVER) == BST_CHECKED) {
        control |= NETWORK_CONTROL_JOY2;
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_JOY2_CLIENT) == BST_CHECKED) {
        control |= (NETWORK_CONTROL_JOY2 << NETWORK_CONTROL_CLIENTOFFSET);
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_DEVC_SERVER) == BST_CHECKED) {
        control |= NETWORK_CONTROL_DEVC;
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_DEVC_CLIENT) == BST_CHECKED) {
        control |= (NETWORK_CONTROL_DEVC << NETWORK_CONTROL_CLIENTOFFSET);
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_RSRC_SERVER) == BST_CHECKED) {
        control |= NETWORK_CONTROL_RSRC;
    }
    if (IsDlgButtonChecked(hwnd, IDC_NETWORK_RSRC_CLIENT) == BST_CHECKED) {
        control |= (NETWORK_CONTROL_RSRC << NETWORK_CONTROL_CLIENTOFFSET);
    }

    resources_set_int("NetworkControl", control);

    port = GetDlgItemInt(hwnd, IDC_NETWORK_PORT, NULL, TRUE);
    if (port < 1 || port > 0xFFFF) {
        ui_error(translate_text(IDS_INVALID_PORT_NUMBER));
        return -1;
    }

    resources_set_int("NetworkServerPort", port);

    GetDlgItemText(hwnd, IDC_NETWORK_SERVERNAME, st_text, MAX_PATH);
    system_wcstombs(text, st_text, MAX_PATH);
    resources_set_string("NetworkServerName", text);

    GetDlgItemText(hwnd, IDC_NETWORK_SERVER_BIND, st_text, MAX_PATH);
    system_wcstombs(text, st_text, MAX_PATH);
    resources_set_string("NetworkServerBindAddress", text);

    return 0;
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command=LOWORD(wparam);
            switch (command) {
                case IDCANCEL:
                    EndDialog(hwnd,0);
                    return TRUE;
                case IDC_NETWORK_SERVER:
                    if (set_resources(hwnd) == 0) {
                        if (network_start_server() < 0) {
                            ui_error(translate_text(IDS_ERROR_STARTING_SERVER));
                        }
                    }
                    EndDialog(hwnd,0);
                    return TRUE;
                case IDC_NETWORK_CLIENT:
                    if (set_resources(hwnd) == 0) {
                        if (network_connect_client() < 0) {
                            ui_error(translate_text(IDS_ERROR_CONNECTING_CLIENT));
                        }
                    }
                    EndDialog(hwnd,0);
                    return TRUE;
                case IDC_NETWORK_DISCONNECT:
                    network_disconnect();
                    EndDialog(hwnd,0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            set_resources(hwnd);
            EndDialog(hwnd,0);
            return TRUE;
        case WM_INITDIALOG:
            system_init_dialog(hwnd);
            init_network_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}


void ui_network_dialog(HWND hwnd)
{
    DialogBox(winmain_instance, MAKEINTRESOURCE(IDD_NETWORK_SETTINGS_DIALOG), hwnd, dialog_proc);
}
