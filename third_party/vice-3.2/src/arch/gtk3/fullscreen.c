/** \file   fullscreen.c
 * \brief   Native GTK3 UI fullscreen stuff
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 */

/*
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

#include "not_implemented.h"

#include "fullscreen.h"
#include "video.h"


static int fullscreen_enable(struct video_canvas_s *canvas, int enable)
{
    NOT_IMPLEMENTED();
    return 0;
}


static int fullscreen_statusbar(struct video_canvas_s *canvas, int enable)
{
    NOT_IMPLEMENTED();
    return 0;
}


static int fullscreen_double_size(struct video_canvas_s *canvas, int enable)
{
    NOT_IMPLEMENTED();
    return 0;
}


static int fullscreen_double_scan(struct video_canvas_s *canvas, int enable)
{
    NOT_IMPLEMENTED();
    return 0;
}


static int fullscreen_device(struct video_canvas_s *canvas, const char *device)
{
    NOT_IMPLEMENTED();
    return 0;
}


static int fullscreen_mode_gtk3(struct video_canvas_s *canvas, int mode)
{
    NOT_IMPLEMENTED();
    return 0;
}




void fullscreen_capability(struct cap_fullscreen_s *cap_fullscreen)
{
    cap_fullscreen->device_num = 0;
    cap_fullscreen->device_name[cap_fullscreen->device_num] = "GTK3";
    cap_fullscreen->enable = fullscreen_enable;
    cap_fullscreen->statusbar = fullscreen_statusbar;
    cap_fullscreen->double_size = fullscreen_double_size;
    cap_fullscreen->double_scan = fullscreen_double_scan;
    cap_fullscreen->device = fullscreen_device;
    cap_fullscreen->mode[cap_fullscreen->device_num] = fullscreen_mode_gtk3;
}

