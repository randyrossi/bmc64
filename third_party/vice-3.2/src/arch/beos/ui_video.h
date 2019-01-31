/*
 * ui_video.h - Video settings
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

#ifndef VICE__UI_VIDEO_H__
#define VICE__UI_VIDEO_H__

const uint32 MESSAGE_VIDEO_COLOR           = 'MVD1';
const uint32 MESSAGE_VIDEO_PALETTEFILE     = 'MVD2';
const uint32 MESSAGE_VIDEO_EXTERNALPALETTE = 'MVD3';
const uint32 MESSAGE_VIDEO_PALMODE         = 'MVD4';
const uint32 MESSAGE_VIDEO_PALSHADE        = 'MVD5';
const uint32 MESSAGE_VIDEO_AUDIO_LEAK      = 'MVD6';

#define UI_VIDEO_CHIP_NONE  -1
#define UI_VIDEO_CHIP_VICII  0
#define UI_VIDEO_CHIP_VIC    1
#define UI_VIDEO_CHIP_CRTC   2
#define UI_VIDEO_CHIP_VDC    3
#define UI_VIDEO_CHIP_TED    4

extern void ui_video(int chip_type);
extern void ui_video_two_chip(int chip1_type, int chip2_type);

#endif
