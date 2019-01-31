/*
 * ui_device.h - Device settings
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

#ifndef VICE__UI_DEVICE_H__
#define VICE__UI_DEVICE_H__

const uint32 MESSAGE_DEVICE_P00         = 'MDV1';
const uint32 MESSAGE_DEVICE_BROWSE      = 'MDV2';
const uint32 MESSAGE_DEVICE_BROWSE_END  = 'MDV3';
const uint32 MESSAGE_DEVICE_DIRECTORY   = 'MDV4';

extern void ui_device(void);

#endif
