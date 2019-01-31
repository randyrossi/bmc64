/*
 * fullscreen.m - MacVICE fullscreen interface
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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
#include "video.h"

void fullscreen_capability(cap_fullscreen_t *cap_fullscreen)
{
    // fullscreen is handled in VICEWindow itself
    cap_fullscreen->device_num = 0;
#if 0
    cap_fullscreen->enable      = fullscreen_enable;
    cap_fullscreen->double_size = fullscreen_double_size;
    cap_fullscreen->double_scan = fullscreen_double_scan;
    cap_fullscreen->device      = fullscreen_device;

    cap_fullscreen->device_name[cap_fullscreen->device_num] = "OpenGL";
    cap_fullscreen->mode[cap_fullscreen->device_num] = fullscreen_mode_opengl;
    cap_fullscreen->device_num++;
#endif
}
