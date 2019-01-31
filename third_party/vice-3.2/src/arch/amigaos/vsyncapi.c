/*
 * vsyncapi.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#define __USE_INLINE__

#ifdef AMIGA_OS4_ALT
#define __USE_OLD_TIMEVAL__
#include <devices/timer.h>
#endif

#include <stdio.h>
#include "vsyncapi.h"
#include "kbd.h"
#include "kbdbuf.h"
#include "keyboard.h"
#include "private.h"
#include "statusbar.h"

#undef BYTE
#undef WORD

#include "timer.h"
#include "mousedrv.h"
#include "joy.h"
#include "intl.h"
#include "translate.h"

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "private.h"
#include "ui.h"

static int pause_pending = 0;

/* number of timer units per second - used to calc speed and fps */
unsigned long vsyncarch_frequency(void)
{
    return 1000000;
}

/* provide the actual time in timer units */
unsigned long vsyncarch_gettime(void)
{
    struct timeval tv;

    timer_gettime(vice_timer, &tv);
    return (tv.tv_secs * 1000000) + tv.tv_micro;
}

/* call when vsync_init is called */
void vsyncarch_init(void)
{
}

/* display speed(%) and framerate(fps) */
void vsyncarch_display_speed(double speed, double fps, int warp_enabled)
{
    video_canvas_t *canvas;

    for (canvas = canvaslist; canvas; canvas = canvas->next) {
        struct Window *window = canvas->os->window;

        sprintf(canvas->os->window_title, intl_speed_at_text, canvas->os->window_name, (int)(speed + .5), (int)(fps + .5), warp_enabled ? " (warp)" : "");

        SetWindowTitles(window, canvas->os->window_title, (void *)-1);
    }
    statusbar_statustext_update();
}

/* sleep the given amount of timer units */
void vsyncarch_sleep(unsigned long delay)
{
    timer_usleep(vice_timer, delay);
}

/* synchronize with vertical blanks */
void vsyncarch_verticalblank(struct video_canvas_s *c, float rate, int frames)
{
}

/* keep vertical blank sync prepared */
void vsyncarch_prepare_vbl(void)
{
}

/* this is called before vsync_do_vsync does the synchroniation */
void vsyncarch_presync(void)
{
    ui_event_handle();
    mousedrv_sync();
    kbdbuf_flush();
    joystick_update();
}

/* this is called after vsync_do_vsync did the synchroniation */
void vsyncarch_postsync(void)
{
    /* this function is called once a frame, so this
       handles single frame advance */
    if (pause_pending) {
        ui_pause_emulation(1);
        pause_pending = 0;
    }
}

/* set ui dispatcher function */
void_hook_t vsync_set_event_dispatcher(void_hook_t hook)
{
    return hook;
}

int vsyncarch_vbl_sync_enabled(void)
{
    return 0;
}

void vsyncarch_advance_frame(void)
{
    ui_pause_emulation(0);
    pause_pending = 1;
}
