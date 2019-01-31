/*
 * quicktimedrv.h - Movie driver using Apple QuickTime.
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

#ifndef VICE_QUICKTIMEDRV_H
#define VICE_QUICKTIMEDRV_H

#include "screenshot.h"

#define QUICKTIME_AUDIO_BITRATE_MIN 16000
#define QUICKTIME_AUDIO_BITRATE_MAX 128000
#define QUICKTIME_AUDIO_BITRATE_DEF 64000

#define QUICKTIME_VIDEO_BITRATE_MIN 100000
#define QUICKTIME_VIDEO_BITRATE_MAX 10000000
#define QUICKTIME_VIDEO_BITRATE_DEF 800000

extern void gfxoutput_init_quicktime(int help);

#endif
