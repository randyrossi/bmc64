/*
 * loader.h
 *
 * Written by
 *  Locnet <android.locnet@gmail.com>
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

#ifndef LOCNET_LOADER
#define LOCNET_LOADER

#include "AndroidOSfunc.h"

#define KEYBOARD_CTRL_FLAG    0x01
#define KEYBOARD_ALT_FLAG     0x02
#define KEYBOARD_SHIFT_FLAG   0x04

#define KEYBOARD_NONE 0

struct loader_config {
    unsigned long rowbytes;
    char *videoBuffer;
    jobject bmph;
    jshortArray audioBuffer;
    unsigned long memsize;
    unsigned long frameskip;
    unsigned long cycles;
    unsigned long soundEnable;
    unsigned long cycleHack;
    unsigned long refreshHack;
    long width;
    long height;
    char abort;
    char pause;
};

extern struct loader_config *loadf;

#endif
