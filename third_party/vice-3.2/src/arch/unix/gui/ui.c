/*
 * ui.c - Common UI routines.
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

/* #define DEBUG_UI */

#include "vice.h"

#include "fullscreenarch.h"
#include "interrupt.h"
#include "joystick.h"
#include "machine.h"
#include "ui.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uicommands.h"
#include "uifliplist.h"
#include "uiromset.h"
#include "uiscreenshot.h"
#include "types.h"
#include "vsync.h"
#include "openGL_sync.h"

#ifdef DEBUG_UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

static int is_paused = 0;

static void pause_trap(uint16_t addr, void *data)
{
    ui_display_paused(1);
    vsync_suspend_speed_eval();
    while (is_paused) {
        ui_dispatch_next_event();
    }
}

void ui_pause_emulation(int flag)
{
    if (flag && !is_paused) {
        is_paused = 1;
        interrupt_maincpu_trigger_trap(pause_trap, 0);
    } else {
        ui_display_paused(0);
        is_paused = 0;
    }
}

int ui_emulation_is_paused(void)
{
    return is_paused;
}

void ui_common_init(void)
{
    uiromset_menu_init();
}

void ui_common_shutdown(void)
{
#ifdef HAVE_FULLSCREEN
    fullscreen_shutdown();
#endif
#if defined HAVE_OPENGL_SYNC && defined HAVE_XRANDR
    openGL_sync_shutdown();
#endif

    uiattach_shutdown();
    uicommands_shutdown();
    uifliplist_shutdown();
    uiscreenshot_shutdown();
}

extern void ui_display_joystick_status_widget(int joystick_number, int status);

void ui_display_joyport(uint8_t *joyport)
{
    if (machine_class != VICE_MACHINE_VSID) {
        int n;
        DBG(("ui_display_joyport %02x %02x %02x %02x %02x\n",
            joyport[0], joyport[1], joyport[2], joyport[3], joyport[4]));
#if 1
        for (n = 0; n < JOYSTICK_NUM; ++n) {
            ui_display_joystick_status_widget(n, joyport[1 + n]);
        }
#else
        ui_display_joystick_status_widget(joyport[0], joyport[1 + n]);
#endif
    }
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
}

void ui_display_volume(int vol)
{
}

char* ui_get_file(const char *format,...)
{
    return NULL;
}
