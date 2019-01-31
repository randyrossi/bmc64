/*
 * vsyncarch.c - End-of-frame handling for Win32
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

#define WINVER 0x0500
#include <windows.h>

#include "vsync.h"
#include "kbdbuf.h"
#include "lightpendrv.h"
#include "log.h"
#include "machine.h"
#include "ui.h"
#include "uiapi.h"
#include "mousedrv.h"
#include "videoarch.h"

#ifdef HAS_JOYSTICK
#include "joy.h"
#endif

#include "vsyncapi.h"

// -------------------------------------------------------------------------

static int pause_pending = 0;

enum { EXTRA_PRECISION = 10 };

unsigned long vsyncarch_frequency(void)
{
    return 1000 << EXTRA_PRECISION;
}

unsigned long vsyncarch_gettime(void)
{
    return timeGetTime() << EXTRA_PRECISION;
}

void vsyncarch_init(void)
{
    MMRESULT res = timeBeginPeriod(1);
    if (res != TIMERR_NOERROR)
        log_warning(LOG_DEFAULT, "VSYNC: 1 ms scheduling latency not available.");
}

// -------------------------------------------------------------------------

typedef WINUSERAPI UINT (WINAPI *FPTR_SendInput)(UINT, LPINPUT,int);

static FPTR_SendInput pfnSendInput = NULL;
static int SendInput_loaded = 0;

static void load_SendInput(void)
{
    HINSTANCE hDll = LoadLibrary ( "user32.dll");

    pfnSendInput = (FPTR_SendInput)GetProcAddress( hDll, "SendInput");
    SendInput_loaded = 1;
}

static void win32_mouse_jitter(void)
{
    INPUT ip;

    if (!SendInput_loaded) {
        load_SendInput();
    }

    if (pfnSendInput) {
        ip.type = INPUT_MOUSE;
        ip.mi.dx = 0;
        ip.mi.dy = 0;
        ip.mi.mouseData = 0;
        ip.mi.dwFlags = MOUSEEVENTF_MOVE;
        ip.mi.time = 0;
        ip.mi.dwExtraInfo = 0;

        pfnSendInput(1, &ip, sizeof(INPUT));
    }
}

// Display speed (percentage) and frame rate (frames per second).
void vsyncarch_display_speed(double speed, double frame_rate, int warp_enabled)
{
    ui_display_speed((float) speed, (float)frame_rate, warp_enabled);
}

void vsyncarch_sleep(unsigned long delay)
{
    int32_t current_time = (int32_t) timeGetTime();
    int32_t target_time = current_time + (delay >> EXTRA_PRECISION);
    while (current_time < target_time) {
        Sleep(target_time - current_time);
        current_time = timeGetTime();
    }
    //log_debug("Sleep %d ms target reached to %d ms", delay >> EXTRA_PRECISION, current_time - target_time);
}

void vsyncarch_presync(void)
{
    /* Update mouse */
    mouse_update_mouse();

    /* Update lightpen */
    win32_lightpen_update();

    /* Flush keypresses emulated through the keyboard buffer.  */
    kbdbuf_flush();
    joystick_update();
}

void vsyncarch_postsync(void)
{
    /* this function is called once a frame, so this
       handles single frame advance */
    if (pause_pending) {
        ui_pause_emulation(1);
        pause_pending = 0;
    }

    /* Dispatch all the pending UI events.  */
    ui_dispatch_events();

    /* prevent screensaver */
    win32_mouse_jitter();

    ui_frame_update_gui();
}

int vsyncarch_vbl_sync_enabled(void)
{
    return ui_vblank_sync_enabled();
}

void vsyncarch_advance_frame(void)
{
    ui_pause_emulation(0);
    pause_pending = 1;
}
