/*
 * vsyncarch.c - End-of-frame handling for BeOS
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
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

#include "kbdbuf.h"
#include "ui.h"
#include "vsyncapi.h"

#include "joy.h"

#include <OS.h>
#include <sys/time.h>

/* ------------------------------------------------------------------------- */

static int pause_pending = 0;

/* Number of timer units per second. */
unsigned long vsyncarch_frequency(void)
{
    /* Microseconds resolution. */
    return 1000000;
}

/* Get time in timer units. */
unsigned long vsyncarch_gettime(void)
{
    struct timeval now;

    gettimeofday(&now, NULL);

    return 1000000UL * now.tv_sec + now.tv_usec;
}

void vsyncarch_init(void)
{
}

/* Display speed (percentage) and frame rate (frames per second). */
void vsyncarch_display_speed(double speed, double frame_rate, int warp_enabled)
{
    ui_display_speed((float)speed, (float)frame_rate, warp_enabled);
}

/* Sleep a number of timer units. */
void vsyncarch_sleep(unsigned long delay)
{
    snooze(delay);
}

void vsyncarch_presync(void)
{
    kbdbuf_flush();
#ifdef HAS_JOYSTICK
    joystick_update();
#endif
}

void vsyncarch_postsync(void)
{
    /* this function is called once a frame, so this
       handles single frame advance */
    if (pause_pending) {
        ui_pause_emulation(1);
        pause_pending = 0;
    }
    ui_dispatch_events();
}

void vsyncarch_advance_frame(void)
{
    ui_pause_emulation(0);
    pause_pending = 1;
}
