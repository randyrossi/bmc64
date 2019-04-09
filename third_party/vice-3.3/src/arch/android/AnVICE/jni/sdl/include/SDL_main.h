/*
 * SDL_main.h
 *
 * Written by
 *  Sam Lantinga <slouken@libsdl.org>
 *
 * This file is a modified SDL header.
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

#ifndef _SDL_main_h
#define _SDL_main_h

#include "SDL_stdinc.h"

/** @file SDL_main.h
 *  Redefine main() on Win32 and MacOS so that it is called by winmain.c
 */

#ifdef __cplusplus
#  define C_LINKAGE	"C"
#else
#  define C_LINKAGE
#endif /* __cplusplus */

/** The application's main() function must be called with C linkage,
 *  and should be declared like this:
 *      @code
 *      #ifdef __cplusplus
 *      extern "C"
 *      #endif
 *	int main(int argc, char *argv[])
 *	{
 *	}
 *      @endcode
 */
#define main	SDL_main

/** The prototype for the application's main() function */
extern C_LINKAGE int SDL_main(int argc, char *argv[]);

#endif /* _SDL_main_h */
