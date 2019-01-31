/*
 * fullscreen.c
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Andreas Boose <viceteam@t-online.de>
 *  Martin Pottendorfer <pottendo@utanet.at>
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

/* #define SDL_DEBUG */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "fullscreen.h"
#include "fullscreenarch.h"
#include "log.h"
#include "ui.h"
#include "video.h"
#include "videoarch.h"

#ifdef SDL_DEBUG
#define DBG(x)  log_debug x
#else
#define DBG(x)
#endif

void fullscreen_resume(void)
{
    DBG(("%s", __func__));
}

static int fullscreen_enable(struct video_canvas_s *canvas, int enable)
{
    DBG(("%s: %i", __func__, enable));

    if (!canvas->fullscreenconfig->device_set) {
        return 0;
    }

    canvas->fullscreenconfig->enable = enable;

    ui_check_mouse_cursor();

    if (canvas->initialized) {
        /* resize window back to normal when leaving fullscreen */
        video_viewport_resize(canvas, 1);
    }
    return 0;
}

static int fullscreen_statusbar(struct video_canvas_s *canvas, int enable)
{
    DBG(("%s: %i", __func__, enable));
    return 0;
}

static int fullscreen_double_size(struct video_canvas_s *canvas, int double_size)
{
    DBG(("%s: %i", __func__, double_size));
    return 0;
}

static int fullscreen_double_scan(struct video_canvas_s *canvas, int double_scan)
{
    DBG(("%s: %i", __func__, double_scan));
    return 0;
}

static int fullscreen_device(struct video_canvas_s *canvas, const char *device)
{
    DBG(("%s: %s", __func__, device));

    if (strcmp("SDL", device) != 0) {
        canvas->fullscreenconfig->device_set = 0;
        return -1;
    }

    canvas->fullscreenconfig->device_set = 1;
    return 0;
}

static int fullscreen_mode_sdl(struct video_canvas_s *canvas, int mode)
{
    DBG(("%s: %i", __func__, mode));

    canvas->fullscreenconfig->mode = mode;
    return 0;
}

void fullscreen_capability(cap_fullscreen_t *cap_fullscreen)
{
    DBG(("%s", __func__));

    cap_fullscreen->device_num = 0;
    cap_fullscreen->device_name[cap_fullscreen->device_num] = "SDL";
    cap_fullscreen->enable = fullscreen_enable;
    cap_fullscreen->statusbar = fullscreen_statusbar;
    cap_fullscreen->double_size = fullscreen_double_size;
    cap_fullscreen->double_scan = fullscreen_double_scan;
    cap_fullscreen->device = fullscreen_device;
    cap_fullscreen->mode[cap_fullscreen->device_num] = fullscreen_mode_sdl;
    cap_fullscreen->device_num += 1;
}
