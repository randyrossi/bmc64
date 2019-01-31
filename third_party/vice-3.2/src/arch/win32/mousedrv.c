/*
 * mousedrv.c - Mouse handling for Win32
 *
 * Written by
 *  Tibor Biczo <crown@mail.matav.hu>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "vice.h"

#include <windows.h>

#ifdef HAVE_DINPUT
#include "dinput_handle.h"
#endif

#include <stdio.h>

#include "cmdline.h"
#include "log.h"
#include "mouse.h"
#include "mousedrv.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "types.h"
#include "ui.h"
#include "vsyncapi.h"

int _mouse_x, _mouse_y;
unsigned long _mouse_timestamp = 0;

/* Mouse sensitivity multiplier  */
static int mouse_sensitivity_mult = 40;

#ifdef HAVE_DINPUT
static int mouse_acquired = 0;
static LPDIRECTINPUTDEVICE di_mouse = NULL;
#endif

static mouse_func_t mouse_funcs;

/* ------------------------------------------------------------------------- */

void mousedrv_mouse_changed(void)
{
    mouse_update_mouse_acquire();
}

static int set_mouse_sensitivity_mult(int val, void *param)
{
    if (val < 0) {
        val = 0;
    }

    if (val > 40) {
        val = 40;
    }

    mouse_sensitivity_mult = val;

    return 0;
}

static const resource_int_t resources_int[] = {
    { "MouseSensitivity", 40, RES_EVENT_NO, NULL,
      &mouse_sensitivity_mult, set_mouse_sensitivity_mult, NULL },
    RESOURCE_INT_LIST_END
};

int mousedrv_resources_init(mouse_func_t *funcs)
{
    mouse_funcs.mbl = funcs->mbl;
    mouse_funcs.mbr = funcs->mbr;
    mouse_funcs.mbm = funcs->mbm;
    mouse_funcs.mbu = funcs->mbu;
    mouse_funcs.mbd = funcs->mbd;

    return resources_register_int(resources_int);
}

static const cmdline_option_t cmdline_options[] =
{
    { "-mousesensitivity", SET_RESOURCE, 1,
      NULL, NULL, "MouseSensitivity", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDS_P_SENSITIVITY, IDS_DESC_MOUSE_SENSITIVITY,
      NULL, NULL },
    CMDLINE_LIST_END
};

int mousedrv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

void mouse_update_mouse(void)
{
#ifdef HAVE_DINPUT
    DIMOUSESTATE state;
    HRESULT result;

    if (di_mouse == NULL || !mouse_acquired) {
        return;
    }

    for (result = IDirectInputDevice_GetDeviceState(di_mouse, sizeof(state), &state); result != DI_OK; result = IDirectInputDevice_GetDeviceState(di_mouse, sizeof(state), &state)) {
        if (result != DIERR_INPUTLOST) {
            return;
        }
        result = IDirectInputDevice_Acquire(di_mouse);
        if (result != DI_OK) {
            return;
        }
    }

    if (state.lX || state.lY) {
        _mouse_x += (int)(state.lX * mouse_sensitivity_mult / 10);
        _mouse_y -= (int)(state.lY * mouse_sensitivity_mult / 10);
        _mouse_timestamp = vsyncarch_gettime();
    }

    mouse_funcs.mbl((int)(state.rgbButtons[0] & 0x80));
    mouse_funcs.mbr((int)(state.rgbButtons[1] & 0x80));
    mouse_funcs.mbm((int)(state.rgbButtons[2] & 0x80));
    /* FIXME: back/forward buttons as up/down? Or state.lZ increase/decrease?
    mouse_funcs.mbu((int)(state.rgbButtons[3] & 0x80));
    mouse_funcs.mbd((int)(state.rgbButtons[4] & 0x80));
    */
#endif
}

void mousedrv_init(void)
{
#ifdef HAVE_DINPUT
#ifdef HAVE_DINPUT_LIB
    LPCDIDATAFORMAT mouse_data_format_ptr = &c_dfDIMouse;
#else
    DIOBJECTDATAFORMAT mouse_objects[] = {
        { &GUID_XAxis, 0, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
        { &GUID_YAxis, 4, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
        { &GUID_ZAxis, 8, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
        { &GUID_Button, 12, DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
        { &GUID_Button, 13, DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
        { &GUID_Button, 14, DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
        { &GUID_Button, 15, DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 }
    };

    const DIDATAFORMAT mouse_data_format = {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_RELAXIS,
        sizeof(DIMOUSESTATE),
        sizeof(mouse_objects) / sizeof(*mouse_objects),
        mouse_objects
    };
    LPCDIDATAFORMAT mouse_data_format_ptr = &mouse_data_format;
#endif

    LPDIRECTINPUT di = get_directinput_handle();

    if (di == NULL) {
        return;
    }

    if (IDirectInput_CreateDevice(di, (GUID *)&GUID_SysMouse, &di_mouse, NULL) == S_OK) {
        if (IDirectInputDevice_SetDataFormat(di_mouse, mouse_data_format_ptr) !=S_OK) {
            IDirectInput_Release(di_mouse);
            log_debug("Can't set Mouse DataFormat");
            di_mouse = NULL;
        }
    }
#endif
}

void mouse_update_mouse_acquire(void)
{
#ifdef HAVE_DINPUT
    if (di_mouse == NULL) {
        return;
    }
    if (_mouse_enabled) {
        if (ui_active) {
            IDirectInputDevice_SetCooperativeLevel(di_mouse, ui_active_window, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
            IDirectInputDevice_Acquire(di_mouse);
            mouse_acquired = 1;
        } else {
            IDirectInputDevice_Unacquire(di_mouse);
            mouse_acquired = 0;
        }
    } else {
        if (mouse_acquired) {
            IDirectInputDevice_Unacquire(di_mouse);
            mouse_acquired = 0;
        }
    }
#endif
}

int mousedrv_get_x(void)
{
    return _mouse_x >> 1;
}

int mousedrv_get_y(void)
{
    return _mouse_y >> 1;
}

unsigned long mousedrv_get_timestamp(void)
{
    return _mouse_timestamp;
}

void mousedrv_button_left(int pressed)
{
    mouse_funcs.mbl(pressed);
}

void mousedrv_button_right(int pressed)
{
    mouse_funcs.mbr(pressed);
}

void mousedrv_button_middle(int pressed)
{
    mouse_funcs.mbm(pressed);
}
