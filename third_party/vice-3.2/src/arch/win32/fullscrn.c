/*
 * fullscrn.c - Common fullscreen related support functions for Win32
 *
 * Written by
 *  Tibor Biczo <crown@matavnet.hu>
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

#include <windows.h>
#include <prsht.h>
#include <tchar.h>

#include "fullscrn.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "statusbar.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uilib.h"
#include "util.h"
#include "video.h"
#include "videoarch.h"
#include "viewport.h"
#include "winlong.h"
#include "winmain.h"

static int fullscreen_nesting_level = 0;
static int dx_primary;
static int dx9disable;
static int keep_aspect_ratio, true_aspect_ratio, aspect_ratio;
static int ui_setup_finished = 0;

#ifdef HAVE_D3D9_H
#include <d3d9.h>

static HMENU old_menu;
static RECT old_rect;
static DWORD old_style;
static float old_refreshrate;

typedef struct _DDL {
    struct _DDL *next;
    int isNullGUID;
    GUID guid;
    LPSTR desc;
} DirectDrawDeviceList;

typedef struct _ML {
    struct _ML *next;
    int devicenumber;
    int width;
    int height;
    int bitdepth;
    int refreshrate;
} DirectDrawModeList;

static DirectDrawDeviceList *devices = NULL;
static DirectDrawModeList *modes = NULL;
#endif

void fullscreen_setup_finished(void)
{
    ui_setup_finished = 1;

    if (IsFullscreenEnabled() && number_of_windows > 0) {
        /* Enable fullscreen mode for window of last canvas */
        SwitchToFullscreenMode(window_handles[number_of_windows - 1]);
    }
}

int fullscreen_get_nesting_level(void)
{
    return fullscreen_nesting_level;
}

static void fullscreen_set_res_from_current_display(void)
{
#ifdef HAVE_D3D9_H
    int width, height, refreshrate;

    if (video_dx9_enabled()) {
        D3DDISPLAYMODE mode;

        if (S_OK == IDirect3D9_GetAdapterDisplayMode(d3d, D3DADAPTER_DEFAULT , &mode)) {
            width = mode.Width;
            height = mode.Height;
            refreshrate = mode.RefreshRate;
        } else {
            /* provide defaults if GetDisplayMode fails for some reason */
            log_debug("fullscreen_get_current_display_dx9 failed to get mode!");
            width = 640;
            height = 480;
            refreshrate = 0;
        }
        resources_set_int("FullscreenWidth", width);
        resources_set_int("FullscreenHeight", height);
        resources_set_int("FullscreenRefreshRate", refreshrate);
    }
#endif
}

/* check if the fullscreen resource values are valid */
static int fullscrn_res_valid(void)
{
    int device, width, height, bitdepth, refreshrate;

    GetCurrentModeParameters(&device, &width, &height, &bitdepth, &refreshrate);

    /* FIXME: May use modelist to check if combination is valid */
    if (device < 0 || width <= 0 || height <= 0 || bitdepth <= 0 || refreshrate < 0) {
        return -1;
    }

    return 0;
}

#ifdef HAVE_D3D9_H
static void fullscreen_getmodes_dx9(void)
{
    int adapter, numAdapter, mode, numAdapterModes;
    D3DADAPTER_IDENTIFIER9 d3didentifier;
    D3DDISPLAYMODE displayMode;
    DirectDrawDeviceList *new_device;
    DirectDrawDeviceList *search_device;
    DirectDrawModeList *new_mode;
    DirectDrawModeList *search_mode;

    numAdapter = 0;
    while (D3D_OK == IDirect3D9_GetAdapterIdentifier(d3d, numAdapter, 0, &d3didentifier)) {
        new_device = lib_malloc(sizeof(DirectDrawDeviceList));
        new_device->next = NULL;
        new_device->desc = util_concat(d3didentifier.DeviceName, " - ", d3didentifier.Description, NULL);
        if (devices == NULL) {
            devices = new_device;
        } else {
            search_device = devices;
            while (search_device->next != NULL) {
                search_device = search_device->next;
            }
            search_device->next = new_device;
        }
        numAdapter++;
    }

    for (adapter = 0; adapter < numAdapter; adapter++) {
        numAdapterModes = IDirect3D9_GetAdapterModeCount(d3d, adapter, D3DFMT_X8R8G8B8);

        for (mode = 0; mode < numAdapterModes; mode++) {
            if (S_OK == IDirect3D9_EnumAdapterModes(d3d, adapter, D3DFMT_X8R8G8B8, mode, &displayMode)) {
                new_mode = lib_malloc(sizeof(DirectDrawModeList));
                new_mode->next = NULL;
                new_mode->devicenumber = adapter;
                new_mode->width = displayMode.Width;
                new_mode->height = displayMode.Height;
                new_mode->bitdepth = 32;
                new_mode->refreshrate = displayMode.RefreshRate;

                if (modes == NULL) {
                    modes = new_mode;
                } else {
                    search_mode = modes;
                    while (search_mode->next != NULL) {
                        search_mode = search_mode->next;
                    }
                    search_mode->next = new_mode;
                }
            }
        }
    }
}
#endif

void fullscreen_getmodes(void)
{
#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        if (devices == NULL) {
            fullscreen_getmodes_dx9();
        }
    }
#endif

    /* Use current display parameters if resources are not valid */
    if (fullscrn_res_valid() < 0) {
        fullscreen_set_res_from_current_display();
    }
}

#ifdef HAVE_D3D9_H
int fullscreen_get_devices_amount(void)
{
    DirectDrawDeviceList *dev;
    int amount = 0;

    fullscreen_getmodes();
    dev = devices;
    while (dev != NULL) {
        amount++;
        dev = dev->next;
    }
    return amount;
}

char *fullscreen_get_device(int num)
{
    DirectDrawDeviceList *dev;
    int i = 0;

    dev = devices;
    while (dev != NULL) {
        if (i == num) {
            return dev->desc;
        }
        dev = dev->next;
        i++;
    }
    return NULL;
}
#endif

void ui_fullscreen_init(void)
{
#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        fullscreen_getmodes_dx9();
    }
#endif
}

void ui_fullscreen_shutdown(void)
{
#ifdef HAVE_D3D9_H
    if (video_dx9_available()) {
        DirectDrawModeList *m1, *m2;
        DirectDrawDeviceList *d1, *d2;

        if (devices == NULL) {
            fullscreen_getmodes_dx9();
        }

        m1 = modes;
        while (m1 != NULL) {
            m2 = m1->next;
            lib_free(m1);
            m1 = m2;
        }
        modes = NULL;

        d1 = devices;
        while (d1 != NULL) {
            d2 = d1->next;
            lib_free(d1->desc);
            lib_free(d1);
            d1 = d2;
        }
        devices = NULL;
    }
#endif
}

void GetCurrentModeParameters(int *device, int *width, int *height, int *bitdepth, int *refreshrate)
{
#ifdef HAVE_D3D9_H
    if (machine_class != VICE_MACHINE_VSID) {
        resources_get_int("FullscreenDevice", device);
        *bitdepth = 32;
        resources_get_int("FullscreenWidth", width);
        resources_get_int("FullscreenHeight", height);
        resources_get_int("FullscreenRefreshRate", refreshrate);
    }
#endif
}

int IsFullscreenEnabled(void)
{
#ifdef HAVE_D3D9_H
    int b;

    if (machine_class == VICE_MACHINE_VSID) {
        return 0;
    }
    resources_get_int("FullscreenEnabled", &b);

    return (ui_setup_finished && b);
#else
    return 0;
#endif
}

void SwitchToFullscreenMode(HWND hwnd)
{
    if (!ui_setup_finished) {
        return;
    }

    /* Check for valid fullscreen params */
    if (fullscrn_res_valid() < 0) {
        fullscreen_set_res_from_current_display();
    }

#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        video_canvas_t *c;

        c = video_canvas_for_hwnd(hwnd);


        statusbar_destroy(hwnd);

        /*  Remove Window stuff that prevents fullscreen display */
        old_style = GetWindowLong(hwnd, GWL_STYLE);
        GetWindowRect(hwnd, &old_rect);
        SetWindowLong(hwnd, GWL_STYLE, old_style & ~WS_SYSMENU & ~WS_CAPTION);
        old_menu = GetMenu(hwnd);
        SetMenu(hwnd, NULL);
        ShowCursor(FALSE);

        video_device_release_dx9(c);
        ui_set_render_window(c, 1);
        video_device_create_dx9(c, 1);
        video_canvas_reset_dx9(c);
        video_canvas_refresh_all(c);
    }
#endif
}

static void SwitchToWindowedMode(HWND hwnd)
{
    int alwaysontop;

    if (!ui_setup_finished)
        return;

#ifdef HAVE_D3D9_H
    if (video_dx9_enabled()) {
        video_canvas_t *c;

        c = video_canvas_for_hwnd(hwnd);

        video_device_release_dx9(c);

        /* Create statusbar here to get correct dimensions for client window */
        statusbar_create(hwnd);
        ui_set_render_window(c, 0);

        LockWindowUpdate(hwnd);
        SetWindowLong(hwnd, GWL_STYLE, old_style);
        /* Restore  Menu */
        SetMenu(hwnd,old_menu);
        ui_show_menu();
        SetWindowPos(hwnd, HWND_NOTOPMOST, old_rect.left, old_rect.top, old_rect.right - old_rect.left, old_rect.bottom - old_rect.top, SWP_NOCOPYBITS);
        ShowCursor(TRUE);
        LockWindowUpdate(NULL);

        video_device_create_dx9(c, 0);
        video_canvas_reset_dx9(c);
        video_canvas_refresh_all(c);

        c->refreshrate = old_refreshrate;
    }
#endif
    resources_get_int("AlwaysOnTop", &alwaysontop);
    ui_set_alwaysontop(alwaysontop);
}

static void StartFullscreenMode(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    SwitchToFullscreenMode(hwnd);
    resources_set_int("FullScreenEnabled", 1);
#endif
}

static void EndFullscreenMode(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    SwitchToWindowedMode(hwnd);
    resources_set_int("FullScreenEnabled", 0);
#endif
}

void SwitchFullscreenMode(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    if (IsFullscreenEnabled()) {
        EndFullscreenMode(hwnd);
    } else {
        StartFullscreenMode(hwnd);
    }
#endif
}

void SuspendFullscreenMode(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    if (IsFullscreenEnabled()) {
        if (fullscreen_nesting_level == 0) {
            SwitchToWindowedMode(hwnd);
        }
        fullscreen_nesting_level++;
    }
#endif
}

void ResumeFullscreenMode(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    if (IsFullscreenEnabled()) {
        fullscreen_nesting_level--;
        if (fullscreen_nesting_level == 0) {
            SwitchToFullscreenMode(hwnd);
        }
    }
#endif
}

void SuspendFullscreenModeKeep(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    int device, width, height, bitdepth, rate;

    GetCurrentModeParameters(&device, &width, &height, &bitdepth, &rate);
    if (video_dx9_enabled() || ((width < 640) && (height < 480))) {
        SuspendFullscreenMode(hwnd);
    } else {
        if (IsFullscreenEnabled()) {
            if (fullscreen_nesting_level == 0) {
                ShowCursor(TRUE);
            }
        }
    }
#endif
}

void ResumeFullscreenModeKeep(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    int device, width, height, bitdepth, rate;

    GetCurrentModeParameters(&device, &width, &height, &bitdepth, &rate);
    if (video_dx9_enabled() || ((width < 640) && (height < 480))) {
        ResumeFullscreenMode(hwnd);
    } else {
        if (IsFullscreenEnabled()) {
            if (fullscreen_nesting_level == 0) {
                ShowCursor(FALSE);
            }
        }
    }
#endif
}

/*---------------------------------------------------------------------------*/
/*   Fullscreen settings UI stuff                                            */
/*---------------------------------------------------------------------------*/

typedef struct _VL {
    struct _VL *next;
    struct _VL *prev;
    char *text;
    int value;
} ValueList;

ValueList *bitdepthlist = NULL;
ValueList *resolutionlist = NULL;
ValueList *refresh_rates = NULL;

int fullscreen_device = 0;
int fullscreen_bitdepth = 0;
int fullscreen_width = 0;
int fullscreen_height = 0;
int fullscreen_refreshrate = 0;

#ifdef HAVE_D3D9_H
static void validate_mode(int *device, int *width, int *height, int *bitdepth, int *rate)
{
    DirectDrawModeList  *mode;

    // Validate devicenumber
    mode = modes;
    while (mode != NULL) {
        if (mode->devicenumber == *device) {
            break;
        }
        mode = mode->next;
    }
    if (mode == NULL && modes != NULL) {
        *device = modes->devicenumber;
    }

    // Validate bitdepth
    mode = modes;
    while (mode != NULL) {
        if ((mode->devicenumber == *device) && (mode->bitdepth == *bitdepth)) {
            break;
        }
        mode = mode->next;
    }
    if (mode == NULL) {
        mode = modes;
        while (mode != NULL) {
            if (mode->devicenumber == *device) {
                *bitdepth = mode->bitdepth;
                break;
            }
            mode = mode->next;
        }
    }

    // Validate resolution
    mode = modes;
    while (mode != NULL) {
        if ((mode->devicenumber == *device) && (mode->bitdepth == *bitdepth) && (mode->width == *width) && (mode->height == *height)) {
            break;
        }
        mode = mode->next;
    }
    if (mode == NULL) {
        mode = modes;
        while (mode != NULL) {
            if ((mode->devicenumber == *device) && (mode->bitdepth == *bitdepth)) {
                *width = mode->width;
                *height = mode->height;
                break;
            }
            mode = mode->next;
        }
    }

    // Validate refreshrate
    mode = modes;
    while (mode != NULL) {
        if ((mode->devicenumber == *device) && (mode->bitdepth == *bitdepth) && (mode->width == *width) && (mode->height == *height) && (mode->refreshrate == *rate)) {
            break;
        }
        mode = mode->next;
    }
    if (mode == NULL) {
        *rate = 0;
    }
}

static int GetIndexFromList(ValueList *list, int value)
{
    ValueList *search;
    int pos;

    pos = 0;
    search = list;
    while (search != NULL) {
        if (search->value == value) {
            return pos;
        }
        search = search->next;
        pos++;
    }
    return -1;
}

static int GetValueFromList(ValueList * list, int index)
{
    ValueList *search;
    int pos;

    search = list;
    pos = 0;
    while (search != NULL) {
        if (pos == index) {
            return search->value;
        }
        pos++;
        search = search->next;
    }
    return 0;
}

static void InsertInto(ValueList **list, ValueList *value)
{
    ValueList *after;
    ValueList *before;

    after = *list;
    before = NULL;
    while (after != NULL) {
        if (value->value < after->value) {
            break;
        }
        before = after;
        after = after->next;
    }
    value->prev = before;
    value->next = after;
    if (*list == NULL) {
        *list = value;
    } else if (after == NULL) {
        before->next = value;
    } else if (before == NULL) {
        after->prev = value;
        *list = value;
    } else {
        before->next = value;
        after->prev = value;
    }
}

static void DestroyList(ValueList **list)
{
    ValueList *value;
    ValueList *value2;

    value =* list;
    while (value != NULL) {
        lib_free(value->text);
        value2 = value->next;
        lib_free(value);
        value = value2;
    }
    *list = NULL;
}

static void get_refreshratelist(int device, int bitdepth, int width, int height)
{
    DirectDrawModeList *mode;
    ValueList *value;
    char buff[256];

    DestroyList(&refresh_rates);

    //  We always need 'Default' as when support for different
    //  Refreshrates exists, then it is not reported back
    value = lib_malloc(sizeof(ValueList));
    value->value = 0;
    value->text = lib_stralloc("Default");
    InsertInto(&refresh_rates, value);

    mode = modes;
    while (mode != NULL) {
        if ((mode->devicenumber == device) && (mode->bitdepth == bitdepth) && (mode->width == width) && (mode->height == height)) {
            if (GetIndexFromList(refresh_rates, mode->refreshrate) == -1) {
                value = lib_malloc(sizeof(ValueList));
                value->value = mode->refreshrate;
                itoa(mode->refreshrate, buff, 10);
                value->text = lib_stralloc(buff);
                InsertInto(&refresh_rates, value);
            }
        }
        mode = mode->next;
    }
}

static void get_bitdepthlist(int device)
{
    DirectDrawModeList *mode;
    ValueList *value;
    char buff[256];

    DestroyList(&bitdepthlist);
    mode = modes;
    while (mode != NULL) {
        if ((mode->devicenumber == device)) {
            if (GetIndexFromList(bitdepthlist, mode->bitdepth) == -1) {
                value = lib_malloc(sizeof(ValueList));
                value->value = mode->bitdepth;
                itoa(mode->bitdepth, buff, 10);
                value->text = lib_stralloc(buff);
                InsertInto(&bitdepthlist, value);
            }
        }
        mode = mode->next;
    }
}

static void get_resolutionlist(int device, int bitdepth)
{
    DirectDrawModeList *mode;
    ValueList *value;
    char buff[256];

    DestroyList(&resolutionlist);
    mode = modes;
    while (mode != NULL) {
        if ((mode->devicenumber == device) && (mode->bitdepth == bitdepth)) {
            if (GetIndexFromList(resolutionlist, ((mode->width << 16) + mode->height)) == -1) {
                value = lib_malloc(sizeof(ValueList));
                value->value = (mode->width << 16) + mode->height;
                sprintf(buff, "%dx%d", mode->width, mode->height);
                value->text=lib_stralloc(buff);
                InsertInto(&resolutionlist, value);
            }
        }
        mode = mode->next;
    }
}
#endif

static int vblank_sync;

static uilib_localize_dialog_param fullscreen_dialog_trans[] = {
    {IDC_FULLSCREEN_DRIVER, IDS_FULLSCREEN_DRIVER, 0},
    {IDC_FULLSCREEN_DRIVER_BITDEPTH, IDS_FULLSCREEN_DRVR_BITDEPTH, 0},
    {IDC_FULLSCREEN_DRIVER_RESOLUTION, IDS_FULLSCREEN_DRVR_RESOLUTION, 0},
    {IDC_FULLSCREEN_DRIVER_REFRESHRATE, IDS_FULLSCREEN_DRVR_REFRESHRATE, 0},
    {IDC_TOGGLE_VIDEO_VBLANK_SYNC, IDS_TOGGLE_VIDEO_VBLANK_SYNC, 0},
    {IDC_TOGGLE_VIDEO_DX_PRIMARY, IDS_TOGGLE_VIDEO_DX_PRIMARY, 0},
    {IDC_TOGGLE_DX9DISABLE, IDS_MI_TOGGLE_DX9DISABLE, 0},
    {IDC_TOGGLE_KEEP_ASPECT_RATIO, IDS_TOGGLE_KEEP_ASPECT_RATIO, 0},
    {IDC_TOGGLE_TRUE_ASPECT_RATIO, IDS_TOGGLE_TRUE_ASPECT_RATIO, 0},
    {0, 0, 0}
};

static uilib_dialog_group fullscreen_left_group[] = {
    {IDC_FULLSCREEN_DRIVER_BITDEPTH,  0},
    {IDC_FULLSCREEN_DRIVER_RESOLUTION, 0},
    {IDC_FULLSCREEN_DRIVER_REFRESHRATE, 0},
    {IDC_TOGGLE_KEEP_ASPECT_RATIO, 1},
    {IDC_TOGGLE_TRUE_ASPECT_RATIO, 1},
    {0, 0}
};

static uilib_dialog_group fullscreen_right_group[] = {
    {IDC_FULLSCREEN_BITDEPTH,  0},
    {IDC_FULLSCREEN_RESOLUTION, 0},
    {IDC_FULLSCREEN_REFRESHRATE, 0},
    {IDC_ASPECT_RATIO, 0},
    {IDC_GEOMETRY_ASPECT_RATIO, 0},
    {0, 0}
};

static uilib_dialog_group fullscreen_rest_group[] = {
    {IDC_FULLSCREEN_DRIVER, 0},
    {IDC_TOGGLE_VIDEO_VBLANK_SYNC, 1},
    {IDC_TOGGLE_VIDEO_DX_PRIMARY, 1},
    {IDC_TOGGLE_DX9DISABLE, 1},
    {0, 0}
};

static void enable_aspect_ratio(HWND hwnd)
{
    int enable = keep_aspect_ratio && video_dx9_enabled();

    EnableWindow(GetDlgItem(hwnd, IDC_ASPECT_RATIO), enable);
    EnableWindow(GetDlgItem(hwnd, IDC_TOGGLE_TRUE_ASPECT_RATIO), enable);
}

static void init_fullscreen_dialog(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    HWND setting_hwnd;
    DirectDrawDeviceList *dev;
    ValueList *value;
#endif
    int xpos;
    int xstart;
    int xend;
    int distance;
    int size;
    double fval;
    TCHAR st_text[64];
    video_canvas_t *canvas;

    canvas = video_canvas_for_hwnd(GetParent(GetParent(hwnd)));
    fullscreen_getmodes();

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, fullscreen_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, fullscreen_left_group);

    /* adjust the size of the elements in the rest group */
    uilib_adjust_group_width(hwnd, fullscreen_rest_group);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, fullscreen_right_group, &xend);

    /* get the min x of the right group */
    uilib_get_group_min_x(hwnd, fullscreen_right_group, &xstart);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, fullscreen_left_group, &xpos);

    if (xpos + 10 > xstart) {
        /* set the position of the right group */
        uilib_move_group(hwnd, fullscreen_right_group, xpos + 10);

        /* get the max x of the right group */
        uilib_get_group_max_x(hwnd, fullscreen_right_group, &xpos);

        /* calculate the distance between the old and new max x of the right group */
        distance = xpos - xend;

        /* get the size of the driver element */
        uilib_get_element_size(hwnd, IDC_FULLSCREEN_DEVICE, &size);

        /* set the size of the driver element */
        uilib_set_element_width(hwnd, IDC_FULLSCREEN_DEVICE, size + distance);
    }

#ifdef HAVE_D3D9_H
    validate_mode(&fullscreen_device, &fullscreen_width, &fullscreen_height, &fullscreen_bitdepth, &fullscreen_refreshrate);
    setting_hwnd = GetDlgItem(hwnd, IDC_FULLSCREEN_DEVICE);
    SendMessage(setting_hwnd, CB_RESETCONTENT, 0, 0);
    dev = devices;
    while (dev != NULL) {
        system_mbstowcs(st_text, dev->desc, 64);
        SendMessage(setting_hwnd, CB_ADDSTRING, 0, (LPARAM)st_text);
        dev = dev->next;
    }
    SendMessage(setting_hwnd, CB_SETCURSEL, (WPARAM)fullscreen_device, 0);

    get_bitdepthlist(fullscreen_device);
    setting_hwnd = GetDlgItem(hwnd, IDC_FULLSCREEN_BITDEPTH);
    SendMessage(setting_hwnd, CB_RESETCONTENT, 0, 0);
    value = bitdepthlist;
    while (value != NULL) {
        system_mbstowcs(st_text, value->text, 64);
        SendMessage(setting_hwnd, CB_ADDSTRING, 0, (LPARAM)st_text);
        value = value->next;
    }
    SendMessage(setting_hwnd, CB_SETCURSEL, (WPARAM)GetIndexFromList(bitdepthlist, fullscreen_bitdepth), 0);

    get_resolutionlist(fullscreen_device, fullscreen_bitdepth);
    setting_hwnd = GetDlgItem(hwnd, IDC_FULLSCREEN_RESOLUTION);
    SendMessage(setting_hwnd, CB_RESETCONTENT, 0, 0);
    value = resolutionlist;
    while (value != NULL) {
        system_mbstowcs(st_text, value->text, 64);
        SendMessage(setting_hwnd, CB_ADDSTRING, 0, (LPARAM)st_text);
        value = value->next;
    }
    SendMessage(setting_hwnd, CB_SETCURSEL, (WPARAM)GetIndexFromList(resolutionlist, (fullscreen_width << 16) + fullscreen_height), 0);

    get_refreshratelist(fullscreen_device, fullscreen_bitdepth, fullscreen_width, fullscreen_height);
    setting_hwnd = GetDlgItem(hwnd, IDC_FULLSCREEN_REFRESHRATE);
    SendMessage(setting_hwnd, CB_RESETCONTENT, 0, 0);
    value = refresh_rates;
    while (value != NULL) {
        system_mbstowcs(st_text, value->text, 64);
        SendMessage(setting_hwnd, CB_ADDSTRING, 0, (LPARAM)st_text);
        value = value->next;
    }
    SendMessage(setting_hwnd, CB_SETCURSEL, (WPARAM)GetIndexFromList(refresh_rates, fullscreen_refreshrate), 0);
#endif

    EnableWindow(GetDlgItem(hwnd, IDC_TOGGLE_VIDEO_VBLANK_SYNC), !video_dx9_enabled());
    CheckDlgButton(hwnd, IDC_TOGGLE_VIDEO_VBLANK_SYNC, vblank_sync ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_TOGGLE_VIDEO_DX_PRIMARY, dx_primary ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_TOGGLE_DX9DISABLE, dx9disable ? BST_CHECKED : BST_UNCHECKED);
    EnableWindow(GetDlgItem(hwnd, IDC_TOGGLE_KEEP_ASPECT_RATIO), video_dx9_enabled());
    CheckDlgButton(hwnd, IDC_TOGGLE_KEEP_ASPECT_RATIO, keep_aspect_ratio ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_TOGGLE_TRUE_ASPECT_RATIO, true_aspect_ratio ? BST_CHECKED : BST_UNCHECKED);
    if (video_dx9_enabled()) {
        enable_aspect_ratio(hwnd);

        fval = ((double)aspect_ratio) / 1000.0;
        lib_sntprintf(st_text, 64, TEXT("%.3f"), (float)fval);
        SetDlgItemText(hwnd, IDC_ASPECT_RATIO, st_text);

        fval = canvas->geometry->pixel_aspect_ratio;
        lib_sntprintf(st_text, 64, TEXT("%.3f"), (float)fval);
        SetDlgItemText(hwnd, IDC_GEOMETRY_ASPECT_RATIO, st_text);
    }
}

static void fullscreen_dialog_end(void)
{
#ifdef HAVE_D3D9_H
    if (machine_class != VICE_MACHINE_VSID) {
        resources_set_int("FullScreenDevice", fullscreen_device);
        resources_set_int("FullScreenWidth", fullscreen_width);
        resources_set_int("FullScreenHeight", fullscreen_height);
        resources_set_int("FullScreenRefreshRate", fullscreen_refreshrate);
        resources_set_int("DXPrimarySurfaceRendering", dx_primary);
        resources_set_int("DX9Disable", dx9disable);
    }
#endif
    resources_set_int("VBLANKSync", vblank_sync);
    resources_set_int("TrueAspectRatio", true_aspect_ratio);
    resources_set_int("KeepAspectRatio", keep_aspect_ratio);
    resources_set_int("AspectRatio", aspect_ratio);
}

static void fullscreen_dialog_init(HWND hwnd)
{
#ifdef HAVE_D3D9_H
    if (machine_class != VICE_MACHINE_VSID) {
        resources_get_int("FullscreenDevice", &fullscreen_device);
        fullscreen_bitdepth = 32;
        resources_get_int("FullscreenWidth", &fullscreen_width);
        resources_get_int("FullscreenHeight", &fullscreen_height);
        resources_get_int("FullscreenRefreshRate", &fullscreen_refreshrate);
        resources_get_int("DXPrimarySurfaceRendering", &dx_primary);
        resources_get_int("DX9Disable", &dx9disable);
    }
#endif
    resources_get_int("VBLANKSync", &vblank_sync);
    resources_get_int("KeepAspectRatio", &keep_aspect_ratio);
    resources_get_int("TrueAspectRatio", &true_aspect_ratio);
    resources_get_int("AspectRatio", &aspect_ratio);
    init_fullscreen_dialog(hwnd);
}

INT_PTR CALLBACK dialog_fullscreen_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef HAVE_D3D9_H
    int value;
    int index;
#endif
    int notifycode;
    int item;
    int command;
    TCHAR s[100];
    float tf;

    switch (msg) {
        case WM_NOTIFY:
            if (((NMHDR FAR *)lparam)->code == (UINT)PSN_APPLY) {
                if (video_dx9_enabled()) {
                    GetDlgItemText(hwnd, IDC_ASPECT_RATIO, s, 100);
                    _stscanf(s, TEXT("%f"), &tf);
                    aspect_ratio = (int)(tf * 1000.0 + 0.5);
                    if (aspect_ratio < 500) {
                        ui_error(translate_text(IDS_VAL_F_FOR_S_OUT_RANGE_USE_F), tf, translate_text(IDS_TOGGLE_KEEP_ASPECT_RATIO), 0.5f);
                        aspect_ratio = 500;
                    }
                    if (aspect_ratio > 2000) {
                        ui_error(translate_text(IDS_VAL_F_FOR_S_OUT_RANGE_USE_F), tf, translate_text(IDS_TOGGLE_KEEP_ASPECT_RATIO), 2.0f);
                        aspect_ratio = 2000;
                    }
                }
                fullscreen_dialog_end();
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                return TRUE;
            }
            return FALSE;
        case WM_COMMAND:
            notifycode = HIWORD(wparam);
            item = LOWORD(wparam);
#ifdef HAVE_D3D9_H
            if (notifycode == CBN_SELENDOK) {
                if (item == IDC_FULLSCREEN_DEVICE) { 
                    fullscreen_device = (int)SendMessage(GetDlgItem(hwnd, IDC_FULLSCREEN_DEVICE), CB_GETCURSEL, 0, 0);
                } else if (item == IDC_FULLSCREEN_BITDEPTH) {
                    index = (int)SendMessage(GetDlgItem(hwnd, IDC_FULLSCREEN_BITDEPTH), CB_GETCURSEL, 0, 0);
                    fullscreen_bitdepth = GetValueFromList(bitdepthlist, index);
                } else if (item == IDC_FULLSCREEN_RESOLUTION) {
                    index = (int)SendMessage(GetDlgItem(hwnd, IDC_FULLSCREEN_RESOLUTION), CB_GETCURSEL, 0, 0);
                    value = GetValueFromList(resolutionlist, index);
                    fullscreen_width = value >> 16;
                    fullscreen_height = value & 0xffff;
                } else if (item == IDC_FULLSCREEN_REFRESHRATE) {
                    index = (int)SendMessage(GetDlgItem(hwnd, IDC_FULLSCREEN_REFRESHRATE), CB_GETCURSEL, 0, 0);
                    fullscreen_refreshrate = GetValueFromList(refresh_rates, index);
                }
                init_fullscreen_dialog(hwnd);
            } else
#endif
            {
                command = LOWORD(wparam);
                switch (command) {
                    case IDC_TOGGLE_VIDEO_VBLANK_SYNC:
                        vblank_sync ^= 1;
                        return FALSE;
                    case IDC_TOGGLE_VIDEO_DX_PRIMARY:
                        dx_primary ^= 1;
                        return FALSE;
#ifdef HAVE_D3D9_H
                    case IDC_TOGGLE_DX9DISABLE:
                        dx9disable ^= 1;
                        return FALSE;
#endif
                    case IDC_TOGGLE_KEEP_ASPECT_RATIO:
                        keep_aspect_ratio ^= 1;
                        enable_aspect_ratio(hwnd);
                        return FALSE;
                    case IDC_TOGGLE_TRUE_ASPECT_RATIO:
                        true_aspect_ratio ^= 1;
                        enable_aspect_ratio(hwnd);
                        return FALSE;
                    case IDOK:
                        fullscreen_dialog_end();
                    case IDCANCEL:
                        EndDialog(hwnd, 0);
                        return TRUE;
                }
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            fullscreen_dialog_init(hwnd);
            return TRUE;
    }
    return FALSE;
}
