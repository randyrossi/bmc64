/*
 * statusbar.c - Status bar code.
 *
 * Written by
 *  Tibor Biczo <crown@mtavnet.hu>
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
#include <windowsx.h>
#include <tchar.h>
#ifdef HAVE_COMMCTRL_H
#include <commctrl.h>
#endif

#include "datasette.h"
#include "drive.h"
#include "intl.h"
#include "lib.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "statusbar.h"


#ifndef CDIS_FOCUS
#define CDIS_FOCUS 16
#endif

static HWND status_hwnd[2];
static HWND slider_hwnd[2];
static int number_of_status_windows = 0;
static int status_height;

static unsigned int enabled_drives;
static ui_drive_enable_t status_enabled;
static int status_led[DRIVE_NUM];

/* Translate from window index -> drive index */
static int status_map[DRIVE_NUM];

/* Translate from drive index -> window index */
static int status_partindex[DRIVE_NUM];
static double status_track[DRIVE_NUM];
static int *drive_active_led;

static int tape_motor;
static int tape_counter;
static int tape_control;

static BYTE joyport[3] = { 0, 0, 0 };

static int event_part;
static int event_mode;
static unsigned int event_time_current, event_time_total;

static TCHAR st_emu_status_text[1024];

static HBRUSH b_red;
static HBRUSH b_green;
static HBRUSH b_black;
static HBRUSH b_yellow;
static HBRUSH b_grey;

static void SetStatusWindowParts(HWND hwnd)
{
    int last_part;
    RECT rect;
    int *posx;
    int width;
    int i;
    int disk_update_part;

    /* one for the volume, one part for statusinfo, one for joystick and tape */
    last_part = 3;

    /* the disk parts */
    enabled_drives = 0;
    for (i = 0; i < DRIVE_NUM; i++) {
        int the_drive = 1 << i;

        if (status_enabled & the_drive) {
            status_map[enabled_drives++] = i;
            if (enabled_drives & 1) {
                last_part++;
                status_map[enabled_drives] = -1;
            }
            status_partindex[i] = last_part - 2;
        }
    }
    disk_update_part = last_part - 2;

    /* the event history part */
    if (event_mode != EVENT_OFF) {
        event_part = last_part - 1;
        last_part++;
    }

    posx = lib_malloc(last_part * sizeof(int));
    i = last_part - 1;
    GetWindowRect(hwnd, &rect);
    width = rect.right-rect.left;

    /* Place the volume slider */
    MoveWindow(FindWindowEx(hwnd, NULL, TRACKBAR_CLASS, TEXT("Volume")), width - 20, 3, 20, 36, 0);

    posx[i--] = width;
    width -= 20;

    if (event_mode != EVENT_OFF) {
        posx[i--] = width;
        width -= 80;
    }

    while (i > 0) {
        posx[i--] = width;
        width -= 70;
    }

    posx[0] = width - 20;

    SendMessage(hwnd, SB_SETPARTS, last_part, (LPARAM)posx);
    SendMessage(hwnd, SB_SETTEXT, disk_update_part | SBT_OWNERDRAW, 0);
    SendMessage(hwnd, SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    SendMessage(hwnd, SB_SETTEXT, (last_part - 1) | SBT_OWNERDRAW, 0);

    lib_free(posx);
}


void statusbar_create(HWND hwnd)
{
    RECT rect;
    int res_val;
    int i = 0;

    while (status_hwnd[i] != NULL) {
        i++;
    }
    status_hwnd[i] = CreateStatusWindow(WS_CHILD | WS_VISIBLE, TEXT(""), hwnd, IDM_STATUS_WINDOW);
    SendMessage(status_hwnd[i], SB_SETMINHEIGHT, 40, (LPARAM)0);
    SendMessage(status_hwnd[i], WM_SIZE, 0, (LPARAM)0);
    
    GetClientRect(status_hwnd[i], &rect);
    status_height = rect.bottom;

    /* the volume part */
    slider_hwnd[i] = CreateWindow(TRACKBAR_CLASS, TEXT("Volume"), WS_CHILD | WS_VISIBLE | TBS_VERT | TBS_NOTICKS, 0, 0, 0, 0, status_hwnd[i], (HMENU)IDC_SLIDER, NULL, NULL);

    resources_get_int("SoundVolume", &res_val);
    SendMessage(slider_hwnd[i], TBM_SETPOS, 1, 100 - res_val);

    /* Max Steps */
    SendMessage(slider_hwnd[i], TBM_SETRANGEMAX, 1, 100);

    /* Steps Wide for display the small lines */
    SendMessage(slider_hwnd[i], TBM_SETTICFREQ, 25, 0);

    SetStatusWindowParts(status_hwnd[i]);

    number_of_status_windows++;
}

void statusbar_destroy(HWND hwnd)
{
    int i;

    for (i = 0; i < number_of_status_windows; i++) {
        if (IsChild(hwnd, status_hwnd[i])) {
            DestroyWindow(status_hwnd[i]);
            status_hwnd[i] = NULL;
        }
    }
    status_height = 0;
    number_of_status_windows--;
}


void statusbar_create_brushes(void)
{
    b_green = CreateSolidBrush(0xff00);
    b_red = CreateSolidBrush(0xff);
    b_black = CreateSolidBrush(0x00);
    b_yellow = CreateSolidBrush(0xffff);
    b_grey = CreateSolidBrush(0x808080);
}

void statusbar_delete_brushes(void)
{
    DeleteObject(b_green);
    DeleteObject(b_red);
    DeleteObject(b_black);
    DeleteObject(b_yellow);
    DeleteObject(b_grey);
}

int statusbar_get_status_height(void)
{
    return status_height;
}

void statusbar_setstatustext(const char *text)
{
    int i;

    system_mbstowcs(st_emu_status_text, text, 1024);
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 0 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color)
{
    int i;

    status_enabled = enable;
    drive_active_led = drive_led_color;
    for (i = 0; i < number_of_status_windows; i++) {
        SetStatusWindowParts(status_hwnd[i]);
    }
}

void statusbar_display_drive_track(int drivenum, int drive_base, double track_number)
{
    int i;

    status_track[drivenum] = track_number;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, (status_partindex[drivenum]) | SBT_OWNERDRAW, 0);
    }
}


void statusbar_display_drive_led(int drivenum, int status)
{
    int i;

    status_led[drivenum] = status;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, (status_partindex[drivenum]) | SBT_OWNERDRAW, 0);
    }
}

void statusbar_set_tape_status(int tape_status)
{
    int i;

    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_display_tape_motor_status(int motor)
{   
    int i;

    tape_motor = motor;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_display_tape_control_status(int control)
{
    int i;

    tape_control = control;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_display_tape_counter(int counter)
{
    int i;

    if (counter != tape_counter) {
        tape_counter = counter;
        for (i = 0; i < number_of_status_windows; i++) {
            SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
        }
    }
}

void statusbar_display_joyport(BYTE *joystick_status)
{
    int i;

    joyport[1] = joystick_status[1];
    joyport[2] = joystick_status[2];
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, 1 | SBT_OWNERDRAW, 0);
    }
}

void statusbar_event_status(int mode)
{
    int i;

    event_time_current = 0;
    event_time_total = 0;
    event_mode = mode;
    for (i = 0; i < number_of_status_windows; i++) {
        SetStatusWindowParts(status_hwnd[i]);
    }
}

void statusbar_event_time(unsigned int current, unsigned int total)
{
    int i;

    event_time_current = current;
    event_time_total = total;
    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(status_hwnd[i], SB_SETTEXT, event_part | SBT_OWNERDRAW, 0);
    }
}

void statusbar_handle_WMSIZE(UINT msg, WPARAM wparam, LPARAM lparam, int window_index)
{
    SendMessage(status_hwnd[window_index], msg, wparam, lparam);
    SetStatusWindowParts(status_hwnd[window_index]);
}

void statusbar_handle_WMDRAWITEM(WPARAM wparam, LPARAM lparam)
{
    RECT led;
    TCHAR st[256];

    if (wparam == IDM_STATUS_WINDOW) {
        int part_top = ((DRAWITEMSTRUCT*)lparam)->rcItem.top;
        int part_left = ((DRAWITEMSTRUCT*)lparam)->rcItem.left;
        HDC hDC = ((DRAWITEMSTRUCT*)lparam)->hDC;
        UINT itemID = ((DRAWITEMSTRUCT*)lparam)->itemID;

        SetBkColor(hDC, (COLORREF)GetSysColor(COLOR_3DFACE));
        SetTextColor(hDC, (COLORREF)GetSysColor(COLOR_MENUTEXT));

        if (itemID == 0) {
            /* it's the status info */
            led = ((DRAWITEMSTRUCT*)lparam)->rcItem;
            led.left += 2;
            led.right -= 2;
            led.top += 2;
            led.bottom -= 2;
            DrawText(hDC, st_emu_status_text, -1, &led, DT_WORDBREAK);
            return;
        }
        if (itemID == 1) {
            const int offset_x[] = { 5, 0, -5, 10, -5 };
            const int offset_y[] = { 0, 10, -5, 0, 0 };
            int dir_index, joynum;
            /* tape status */
            POINT tape_control_sign[3];

            /* the leading "Tape:" */
            led.top = part_top + 2;
            led.bottom = part_top + 18;
            led.left = part_left + 2;
            led.right = part_left + 34;

            DrawText(hDC, intl_translate_tcs(IDS_TAPE), -1, &led, 0);

            /* the tape-motor */
            led.top = part_top + 1;
            led.bottom = part_top + 15;
            led.left = part_left + 36;
            led.right = part_left + 50;
            FillRect(hDC, &led, tape_motor ? b_yellow : b_grey);

            /* the tape-control */
            led.top += 3;
            led.bottom -= 3;
            led.left += 3;
            led.right -= 3;
            tape_control_sign[0].x = led.left;
            tape_control_sign[1].x = led.left + 4;
            tape_control_sign[2].x = led.left;
            tape_control_sign[0].y = led.top;
            tape_control_sign[1].y = led.top + 4;
            tape_control_sign[2].y = led.top + 8;
            switch (tape_control) {
            case DATASETTE_CONTROL_STOP:
                FillRect(hDC, &led, b_black);
                break;
            case DATASETTE_CONTROL_START:
            case DATASETTE_CONTROL_RECORD:
                SelectObject(hDC, b_black);
                Polygon(hDC, tape_control_sign, 3);
                if (tape_control == DATASETTE_CONTROL_RECORD) {
                    SelectObject(hDC, b_red);
                    Ellipse(hDC, led.left + 16, led.top + 1, led.left + 23, led.top + 8);
                }
                break;
            case DATASETTE_CONTROL_REWIND:
                tape_control_sign[0].x += 4;
                tape_control_sign[1].x -= 4;
                tape_control_sign[2].x += 4;
            case DATASETTE_CONTROL_FORWARD:
                Polyline(hDC, tape_control_sign, 3);
                tape_control_sign[0].x += 4;
                tape_control_sign[1].x += 4;
                tape_control_sign[2].x += 4;
                Polyline(hDC, tape_control_sign, 3);
            }

            /* the tape-counter */
            led.top = part_top + 2;
            led.bottom = part_top + 18;
            led.left = part_left + 65;
            led.right = part_left + 100;
            lib_sntprintf(st, 256, TEXT("%03i"), tape_counter % 1000);
            DrawText(hDC, st, -1, &led, 0);

            /* the joysticks */
            led.left = part_left + 2;
            led.right = part_left + 48;
            led.top = part_top + 22;
            led.bottom = part_top + 38;

            DrawText(hDC, intl_translate_tcs(IDS_JOYSTICK_C), -1, &led, 0);

            for (joynum = 1; joynum <= 2; joynum ++) {
                led.top = part_top + 22;
                led.left = part_left + (joynum - 1) * 18 + 52;
                led.bottom = led.top + 3;
                led.right = led.left + 3;

                for (dir_index = 0; dir_index < 5; dir_index++) {
                    HBRUSH brush;

                    if (joyport[joynum] & (1 << dir_index)) {
                        brush = (dir_index < 4 ? b_green : b_red);
                    } else {
                        brush = b_grey;
                    }

                    OffsetRect(&led, offset_x[dir_index], offset_y[dir_index]);

                    FillRect(hDC, &led, brush);
                }
            }
            return;
        }
        if (itemID > 1 && itemID <= ((enabled_drives + 3) >> 1)) {
            /* it's a disk part*/
            int y;
            int index = ((itemID - 2) << 1);

            for (y = 0; y < 2 && status_map[index] >= 0; y++, index++) {
                led.top = part_top + 20 * y + 2 ;
                led.bottom = led.top + 16;
                led.left = part_left + 2;
                led.right = part_left + 45;
                lib_sntprintf(st, 256, TEXT("%2d: %.1f"), status_map[index] + 8, status_track[status_map[index]]);
                DrawText(hDC, st, -1, &led, 0);

                led.bottom = led.top + 12;
                led.left = part_left + 47;
                led.right = part_left + 47 + 16;

                switch (status_led[status_map[index]]) {
                    case 0:
                        FillRect(hDC, &led, b_black);
                        break;
                    case MAX_PWM:
                        FillRect(hDC, &led, (drive_active_led[status_map[index]]) ? b_green : b_red);
                        break;
                    default:
                        {
                            HBRUSH brush_led_shade;
                            int col;

                            col = (drive_active_led[status_map[index]] ? COLOR_SHADE_GREEN : COLOR_SHADE_RED) * status_led[status_map[index]] * MAX_COLOR / MAX_PWM;
                            brush_led_shade = CreateSolidBrush(col);
                            FillRect(hDC, &led, brush_led_shade);
                            DeleteBrush(brush_led_shade);
                        }
                        break;
                }
            }
            return;
        }
        if (event_mode != EVENT_OFF && itemID == ((enabled_drives + 5) >> 1)) {
            /* it's the event history part */
            switch (event_mode) {
                case EVENT_RECORDING:
                    lib_sntprintf(st, 256, intl_translate_tcs(IDS_RECORDING), event_time_current / 60, event_time_current % 60);
                    break;
                case EVENT_PLAYBACK:
                    lib_sntprintf(st, 256, intl_translate_tcs(IDS_PLAYBACK), event_time_current / 60, event_time_current % 60, event_time_total / 60, event_time_total % 60);
                    break;
                default:
                    lib_sntprintf(st, 256, intl_translate_tcs(IDS_UNKNOWN));
            }
            led = ((DRAWITEMSTRUCT*)lparam)->rcItem;
            led.left += 2;
            led.right -= 2;
            led.top += 2;
            led.bottom -= 2;
            DrawText(hDC, st, -1, &led, DT_WORDBREAK);
        }
    }
}

void statusbar_notify(HWND window, int window_index, WPARAM wparam, LPARAM lparam)
{
    int slider_pos;
    NMHDR *nmhdr = (NMHDR*)lparam;

    if (wparam == IDC_SLIDER) {

        slider_pos = (int)SendMessage(slider_hwnd[window_index], TBM_GETPOS, 0, 0);
        resources_set_int("SoundVolume", 100 - slider_pos);

        if (nmhdr->code == (UINT)NM_RELEASEDCAPTURE) {
            SetFocus(window);
        }

        if (nmhdr->code == (UINT)NM_CUSTOMDRAW) {
            NMCUSTOMDRAW *lpNMCustomDraw = (NMCUSTOMDRAW*)lparam;
            lpNMCustomDraw->uItemState &= ~CDIS_FOCUS;
        }
    }
}


void statusbar_display_volume(int vol)
{
    int i;

    for (i = 0; i < number_of_status_windows; i++) {
        SendMessage(slider_hwnd[i], TBM_SETPOS, 1, 100 - vol);
    }
}
