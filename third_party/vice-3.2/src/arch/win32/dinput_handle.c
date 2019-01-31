/*
* dinput_handle.c - Functionality shared by DirectInput joystick and mouse drivers.
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

#include "vice.h"

#ifndef HAVE_GUIDLIB
/* all GUIDs will be defined in this object file */
#define INITGUID
/* else all GUIDs will be defined in the external library dxguid.lib */
#endif

/* for CoInitializeEx */
#ifndef HAVE_DINPUT_LIB
#define _WIN32_DCOM
#endif

#include "dinput_handle.h"

#ifdef HAVE_DINPUT

#include "winmain.h"

static LPDIRECTINPUT di = NULL;

LPDIRECTINPUT get_directinput_handle(void)
{
#ifndef HAVE_DINPUT_LIB
    HRESULT res;
#endif

    if (di == NULL) {
#ifdef HAVE_DINPUT_LIB
        if (DirectInputCreate(winmain_instance, 0x0500, &di, NULL) != DI_OK) {
            di = NULL;
        }
#else
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

        res = CoCreateInstance(&CLSID_DirectInput, NULL, CLSCTX_INPROC_SERVER, &IID_IDirectInput2A, (PVOID*)&di);
        if (res != S_OK) {
            return NULL;
        }
        if (IDirectInput_Initialize(di, winmain_instance, 0x0500) != S_OK) {
            IDirectInput_Release(di);
            di = NULL;
        }
#endif
    }
    return di;
}
#endif
