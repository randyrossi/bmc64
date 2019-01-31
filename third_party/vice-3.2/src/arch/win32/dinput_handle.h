/*
* dinput_handle.h - Functionality shared by DirectInput joystick and mouse drivers.
*
* Written by
*  Fabrizio Gennari <fabrizio.ge@tiscali.it>
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
#ifndef DINPUT_HANDLE_H
#define DINPUT_HANDLE_H

#include "vice.h"

#ifdef HAVE_DINPUT
#define DIRECTINPUT_VERSION 0x0500
#include <dinput.h>

/* not defined in DirectInput headers prior to 8 */
#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL 0x80000000
#endif

extern LPDIRECTINPUT get_directinput_handle(void);
#endif

#endif
