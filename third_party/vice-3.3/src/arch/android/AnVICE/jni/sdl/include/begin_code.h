/*
 * begin_code.h
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

/** 
 *  @file begin_code.h
 *  This file sets things up for C dynamic library function definitions,
 *  static inlined functions, and structures aligned at 4-byte alignment.
 *  If you don't like ugly C preprocessor code, don't look at this file. :)
 */

/** 
 *  @file begin_code.h
 *  This shouldn't be nested -- included it around code only.
 */
#ifdef _begin_code_h
#  error Nested inclusion of begin_code.h
#endif

#define _begin_code_h

/** 
 *  @def DECLSPEC
 *  Some compilers use a special export keyword
 */
#ifndef DECLSPEC
#  if defined(__GNUC__) && __GNUC__ >= 4
#    define DECLSPEC	__attribute__ ((visibility("default")))
#  else
#    define DECLSPEC
#  endif
#endif

/** 
 *  @def SDLCALL
 *  By default SDL uses the C calling convention
 */
#ifndef SDLCALL
#  define SDLCALL
#endif /* SDLCALL */

/**
 *  @def SDL_INLINE_OKAY
 *  Set up compiler-specific options for inlining functions
 */
#ifndef SDL_INLINE_OKAY
#  define SDL_INLINE_OKAY
#endif /* SDL_INLINE_OKAY */

/**
 *  @def NULL
 *  Apparently this is needed by several Windows compilers
 */
#ifndef NULL
#  ifdef __cplusplus
#    define NULL 0
#  else
#    define NULL ((void *)0)
#  endif
#endif /* NULL */
