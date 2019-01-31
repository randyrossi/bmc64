/*
 * uivicii.h - Implementation of VIC-II settings dialog box.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Tibor Biczo <crown@axelero.hu>
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

#ifndef VICE_UIVIDEO_H
#define VICE_UIVIDEO_H

#include <windows.h>

#define UI_VIDEO_PAL 0
#define UI_VIDEO_RGB 1

#define UI_VIDEO_CHIP_NONE  -1
#define UI_VIDEO_CHIP_VICII 0
#define UI_VIDEO_CHIP_VIC   1
#define UI_VIDEO_CHIP_CRTC  2
#define UI_VIDEO_CHIP_VDC   3
#define UI_VIDEO_CHIP_TED   4

void ui_video_settings_dialog(HWND hwnd, int chip_type1, int chip_type2);

#endif
