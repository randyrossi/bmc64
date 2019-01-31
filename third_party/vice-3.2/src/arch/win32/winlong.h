/*
 * winlong.h - GetWindowLongPtr/SetWindowLongPtr and other definitions which are not available on all windows.h headers
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Spiro Trikaliotis
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

#ifndef VICE_WINLONG_H
#define VICE_WINLONG_H

#include <winuser.h>

#if !defined(_WIN64) && !defined(GetWindowLongPtr)
# define GetWindowLongPtr GetWindowLong
#endif

#if !defined(_WIN64) && !defined(SetWindowLongPtr)
# define SetWindowLongPtr SetWindowLong
#endif

#ifndef GWLP_HWNDPARENT
# define GWLP_HWNDPARENT GWL_HWNDPARENT
#endif

#ifndef GWLP_USERDATA
# define GWLP_USERDATA GWL_USERDATA
#endif

#ifndef DWLP_MSGRESULT
# define DWLP_MSGRESULT DWL_MSGRESULT
#endif

#ifndef GWLP_WNDPROC
# define GWLP_WNDPROC GWL_WNDPROC
#endif

#ifndef SIF_TRACKPOS
# define SIF_TRACKPOS 0x0010
#endif

#if defined _MSC_VER && _MSC_VER < 1300 
# ifndef InterlockedExchangePointer
#  define InterlockedExchangePointer(_address, _what) ((void*)InterlockedExchange((LPLONG)_address, (LONG)_what))
# endif
#endif

#if defined _MSC_VER && _MSC_VER < 1300 
# ifndef InterlockedCompareExchangePointer
#  define InterlockedCompareExchangePointer(_address, _exchange, _comp) (InterlockedCompareExchange(_address, _exchange, _comp))
# endif
#endif


#ifndef PBT_APMSUSPEND
# define PBT_APMSUSPEND 4
#endif

#ifndef PBT_APMRESUMECRITICAL
# define PBT_APMRESUMECRITICAL 6
#endif

#ifndef PBT_APMRESUMESUSPEND
# define PBT_APMRESUMESUSPEND  7
#endif

#ifndef VK_OEM_PLUS
# define VK_OEM_PLUS 0xbb
#endif

/* Mingw & pre VC 6 headers doesn't have this definition */
#ifndef OFN_ENABLESIZING
# define OFN_ENABLESIZING 0x00800000
#endif

#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4142 )
#endif


#if !defined(_WIN64) && defined _MSC_VER && _MSC_VER < 1500 && defined WINVER && WINVER < 0x0500 && !defined(WATCOM_COMPILE)
typedef __int32 LONG_PTR;
#endif


#endif
