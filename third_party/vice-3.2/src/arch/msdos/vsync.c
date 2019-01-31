/*
 * vsync.c - End-of-frame handling for MS-DOS.
 *
 * Written by
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

/* This module can either use the MIDAS timer or the Allegro one, depending
   on what sound driver we are using.  In fact, Allegro timers cannot be used
   if MIDAS is used to play sound.  */

#include "vice.h"

#include "types.h"

#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <allegro.h>

#include "clkguard.h"
#include "cmdline.h"
#include "drive.h"
#include "joy.h"
#include "kbd.h"
#include "kbdbuf.h"
#include "lightpendrv.h"
#include "machine.h"
#include "maincpu.h"
#include "resources.h"
#include "sound.h"
#include "ui.h"
#include "translate.h"
#include "vsync.h"

#ifdef USE_MIDAS_SOUND
#include "vmidas.h"
#endif

/* Maximum number of frames we can skip consecutively when adjusting the
   refresh rate dynamically.  */
#define MAX_SKIPPED_FRAMES 10

/* ------------------------------------------------------------------------- */

/* Relative speed of the emulation (%).  0 means "don't limit speed".  */
static int relative_speed;

/* Refresh rate.  0 means "auto".  */
static int refresh_rate;

/* "Warp mode".  If nonzero, attempt to run as fast as possible.  */
static int warp_mode_enabled;

/* FIXME: This should call `set_timers'.  */
static int set_relative_speed(int val, void *param)
{
    relative_speed = val;
    sound_set_relative_speed(relative_speed);
    return 0;
}

static int set_refresh_rate(int val, void *param)
{
    if (val < 0) {
        return -1;
    }

    refresh_rate = val;

    return 0;
}

static int set_warp_mode(int val, void *param)
{
    warp_mode_enabled = val ? 1 : 0;

    sound_set_warp_mode(warp_mode_enabled);

    return 0;
}

/* Vsync-related resources.  */
static const resource_int_t resources_int[] = {
    { "Speed", 100, RES_EVENT_NO, NULL,
      &relative_speed, set_relative_speed, NULL },
    { "RefreshRate", 0, RES_EVENT_STRICT, (resource_value_t)1,
      &refresh_rate, set_refresh_rate, NULL },
    { "WarpMode", 0, RES_EVENT_STRICT, (resource_value_t)0,
      &warp_mode_enabled, set_warp_mode, NULL },
    RESOURCE_INT_LIST_END
};

void vsyncarch_init(void)
{
}

unsigned long vsyncarch_gettime(void)
{
    struct timeval now;

    gettimeofday(&now, NULL);

    return 1000000UL * now.tv_sec + now.tv_usec;
}

int vsync_resources_init(void)
{
    return resources_register_int(resources_int);
}

unsigned long vsyncarch_frequency(void)
{
    return 1000000;
}

/* ------------------------------------------------------------------------- */

/* Vsync-related command-line options.  */
static const cmdline_option_t cmdline_options[] = {
    { "-speed", SET_RESOURCE, 1,
      NULL, NULL, "Speed", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<percent>", "Limit emulation speed to specified value" },
    { "-refresh", SET_RESOURCE, 1,
      NULL, NULL, "RefreshRate", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<value>", "Update every <value> frames (`0' for automatic)" },
    { "-warp", SET_RESOURCE, 0,
      NULL, NULL, "WarpMode", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable warp mode" },
    { "+warp", SET_RESOURCE, 0,
      NULL, NULL, "WarpMode", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable warp mode" },
    CMDLINE_LIST_END
};

int vsync_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* Number of frames per second on the real machine.  */
static double refresh_frequency;

/* Number of clock cycles per seconds on the real machine.  */
static long cycles_per_sec;

/* Function to call at the end of every screen frame.  */
static void (*vsync_hook)(void);

/* ------------------------------------------------------------------------- */

double vsync_get_refresh_frequency(void)
{
    return refresh_frequency;
}

static volatile int timer_patch = 0;
static volatile int elapsed_frames = 0;
static int timer_speed = -1;

static void my_timer_callback(void)
{
    if (timer_patch < 0) {
        timer_patch++;
    } else {
        elapsed_frames++;
    }
}
END_OF_FUNCTION(my_timer_callback)

static void patch_timer(int patch)
{
    timer_patch += patch;
}

#ifdef USE_MIDAS_SOUND

/* Version for MIDAS timer.  */

inline static void register_timer_callback(void)
{
    printf("Installing MIDAS timer...\n");
    if (timer_speed == 0) {
        if (!vmidas_remove_timer_callbacks()) {
            log_error(LOG_DEFAULT, "%s: Warning: Could not remove timer callbacks.\n", __FUNCTION__);
        }
    } else {
        DWORD rate = refresh_frequency * timer_speed * 10;

        if (vmidas_set_timer_callbacks(rate, FALSE, my_timer_callback, NULL, NULL) < 0) {
            log_error(LOG_DEFAULT, "%s: cannot set timer callback at %.2f Hz\n", __FUNCTION__, (double)rate / 1000.0);
            /* FIXME: is this necessary? */
            vmidas_remove_timer_callbacks();
            relative_speed = timer_speed = 0;
        }
    }
}

#else  /* !USE_MIDAS_SOUND */

/* Version for Allegro timer.  */

static void register_timer_callback(void)
{
    if (timer_speed == 0) {
        remove_int(my_timer_callback);
    } else {
        int rate = (int)((double)TIMERS_PER_SECOND / (refresh_frequency * ((double)timer_speed / 100.0)) + .5);

        /* We use `install_int_ex()' instead of `install_int()' for increased
           accuracy.  */
        if (install_int_ex(my_timer_callback, rate) < 0) {
            /* FIXME: Maybe we could handle this better?  Well, it is not
               very likely to happen after all...  */
            relative_speed = timer_speed = 0;
        }
    }
}

#endif /* !USE_MIDAS_SOUND */

static void set_timer_speed(void)
{
    int new_timer_speed;

    /* Force 100% speed if using automatic refresh rate and there is no speed
       limit.  */
    if (warp_mode_enabled) {
        new_timer_speed = 0;
    } else if (relative_speed == 0 && refresh_rate == 0) {
	new_timer_speed = 100;
    } else {
	new_timer_speed = relative_speed;
    }

    if (new_timer_speed == timer_speed) {
        return;
    }

    timer_speed = new_timer_speed;
    register_timer_callback();
}

/* ------------------------------------------------------------------------- */

/* Speed evaluation. */

static CLOCK speed_eval_prev_clk;

static double avg_speed_index;
static double avg_frame_rate;

static int speed_eval_suspended = 1;

void vsync_suspend_speed_eval(void)
{
    speed_eval_suspended = 1;
}

void vsync_sync_reset(void)
{
    speed_eval_suspended = 1;
}

static void calc_avg_performance(int num_frames)
{
    static double prev_time;
    struct timeval tv;
    double curr_time;

    gettimeofday(&tv, NULL);
    curr_time = (double)tv.tv_sec + ((double)tv.tv_usec) / 1000000.0;
    if (!speed_eval_suspended) {
        CLOCK diff_clk;
        double speed_index;
        double frame_rate;

        diff_clk = maincpu_clk - speed_eval_prev_clk;
        frame_rate = (double)num_frames / (curr_time - prev_time);
        speed_index = ((((double)diff_clk / (curr_time - prev_time)) / (double)cycles_per_sec)) * 100.0;
        avg_speed_index = speed_index;
        avg_frame_rate = frame_rate;
    }
    prev_time = curr_time;
    speed_eval_prev_clk = maincpu_clk;
    speed_eval_suspended = 0;
}

double vsync_get_avg_frame_rate(void)
{
    if (speed_eval_suspended) {
        return -1.0;
    } else {
        return avg_frame_rate;
    }
}

double vsync_get_avg_speed_index(void)
{
    if (speed_eval_suspended) {
        return -1.0;
    } else {
        return avg_speed_index;
    }
}

/* ------------------------------------------------------------------------- */

/* This prevents the clock counters from overflowing.  */
static void clk_overflow_callback(CLOCK amount, void *data)
{
    speed_eval_prev_clk -= amount;
}

/* ------------------------------------------------------------------------- */

int vsync_do_vsync(struct video_canvas_s *c, int been_skipped)
{
    static long skip_counter = 0;
    static int num_skipped_frames = 0;
    static int frame_counter = 0;
    int skip_next_frame = 0;

    vsync_hook();

    set_timer_speed();

    if (been_skipped) {
        num_skipped_frames++;
    }

    if (timer_patch > 0) {
        timer_patch--;
        asm volatile ("cli");
        elapsed_frames++;
        asm volatile ("sti");
    }

    if (warp_mode_enabled) { /* Warp mode: run as fast as possible.  */
        if (skip_counter < MAX_SKIPPED_FRAMES) {
            skip_next_frame = 1;
            skip_counter++;
        } else {
            skip_counter = elapsed_frames = 0;
        }
        sound_flush();
    } else if (refresh_rate != 0) { /* Fixed refresh rate.  */
        if (timer_speed != 0) {
            while (skip_counter >= elapsed_frames) {
            }
        }
        if (skip_counter < refresh_rate - 1) {
            skip_next_frame = 1;
            skip_counter++;
        } else {
            skip_counter = elapsed_frames = 0;
        }
        patch_timer(sound_flush());
    } else {                    /* Automatic refresh rate adjustment.  */
        if (timer_speed && skip_counter >= elapsed_frames) {
            while (skip_counter >= elapsed_frames) {
            }
            elapsed_frames = 0;
            skip_counter = 0;
        } else {
            if (skip_counter < MAX_SKIPPED_FRAMES) {
                skip_counter++;
                skip_next_frame = 1;
            } else {
                /* Give up, we are too slow.  */
                skip_next_frame = 0;
                skip_counter = 0;
                elapsed_frames = 0;
            }
        }
        patch_timer(sound_flush());
    }

    if (frame_counter >= refresh_frequency * 2) {
        calc_avg_performance(frame_counter + 1 - num_skipped_frames);
        num_skipped_frames = 0;
        frame_counter = 0;
        ui_display_speed(vsync_get_avg_speed_index(),
        vsync_get_avg_frame_rate(),warp_mode_enabled);
    } else {
	frame_counter++;
    }

    kbd_flush_commands();
    kbdbuf_flush();
    joystick_update();
    msdos_lightpen_update();

    ui_set_warp_status(warp_mode_enabled);
    ui_dispatch_events();

    return skip_next_frame;
}

/* -------------------------------------------------------------------------- */

void vsync_set_machine_parameter(double refresh_rate, long cycles)
{
    refresh_frequency = refresh_rate;
    cycles_per_sec = cycles;
    register_timer_callback();
}

void vsync_init(void (*hook)(void))
{
    LOCK_VARIABLE(elapsed_frames);
    LOCK_FUNCTION(my_timer_callback);

#ifdef USE_MIDAS_SOUND
    vmidas_startup();
    vmidas_init();
#else  /* !USE_MIDAS_SOUND */
    install_timer();
#endif /* !USE_MIDAS_SOUND */

    vsync_hook = hook;
    vsync_suspend_speed_eval();
    vsync_disable_timer();

    clk_guard_add_callback(maincpu_clk_guard, clk_overflow_callback, NULL);
}

int vsync_disable_timer(void)
{
    /* FIXME: Find a more generic solution.  */
    if (machine_class == VICE_MACHINE_CBM5x0 || machine_class == VICE_MACHINE_CBM6x0) {
        timer_speed = 0;
    }
    return 0;
}
