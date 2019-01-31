/*
 * SDL_endian.h
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
 *  @file SDL_endian.h
 *  Functions for reading and writing endian-specific values
 */

#ifndef _SDL_endian_h
#define _SDL_endian_h

#include "SDL_stdinc.h"

/** @name SDL_ENDIANs
 *  The two types of endianness 
 */
/*@{*/
#define SDL_LIL_ENDIAN   1234
#define SDL_BIG_ENDIAN   4321
/*@}*/

#ifndef SDL_BYTEORDER	/* Not defined in SDL_config.h? */
#if defined(__MIPS__) && defined(__MISPEB__)
#define SDL_BYTEORDER	SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER	SDL_LIL_ENDIAN
#endif
#endif /* !SDL_BYTEORDER */


#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @name SDL_Swap Functions
 *  Use inline functions for compilers that support them, and static
 *  functions for those that do not.  Because these functions become
 *  static for compilers that do not support inline functions, this
 *  header should only be included in files that actually use them.
 */
/*@{*/
#if defined(__GNUC__) && defined(__i386__) && !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
static __inline__ Uint16 SDL_Swap16(Uint16 x)
{
	__asm__("xchgb %b0,%h0" : "=q" (x) :  "0" (x));
	return x;
}
#else
static __inline__ Uint16 SDL_Swap16(Uint16 x)
{
	return SDL_static_cast(Uint16, ((x << 8) | (x >> 8)));
}
#endif

#if defined(__GNUC__) && defined(__i386__) && !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
static __inline__ Uint32 SDL_Swap32(Uint32 x)
{
	__asm__("bswap %0" : "=r" (x) : "0" (x));
	return x;
}
#else
static __inline__ Uint32 SDL_Swap32(Uint32 x)
{
	return SDL_static_cast(Uint32, ((x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24)));
}
#endif

#if defined(__GNUC__) && defined(__i386__) && !(__GNUC__ == 2 && __GNUC_MINOR__ <= 95 /* broken gcc version */)
static __inline__ Uint64 SDL_Swap64(Uint64 x)
{
	union { 
		struct { Uint32 a,b; } s;
		Uint64 u;
	} v;
	v.u = x;
	__asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1" 
	        : "=r" (v.s.a), "=r" (v.s.b) 
	        : "0" (v.s.a), "1" (v.s.b)); 
	return v.u;
}
#else
static __inline__ Uint64 SDL_Swap64(Uint64 x)
{
	Uint32 hi, lo;

	/* Separate into high and low 32-bit values and swap them */
	lo = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
	x >>= 32;
	hi = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
	x = SDL_Swap32(lo);
	x <<= 32;
	x |= SDL_Swap32(hi);
	return (x);
}
#endif
/*@}*/

/**
 *  @name SDL_SwapLE and SDL_SwapBE Functions
 *  Byteswap item from the specified endianness to the native endianness
 */
/*@{*/
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SDL_SwapLE16(X)   (X)
#define SDL_SwapLE32(X)   (X)
#define SDL_SwapLE64(X)   (X)
#define SDL_SwapBE16(X)   SDL_Swap16(X)
#define SDL_SwapBE32(X)   SDL_Swap32(X)
#define SDL_SwapBE64(X)   SDL_Swap64(X)
#else
#define SDL_SwapLE16(X)   SDL_Swap16(X)
#define SDL_SwapLE32(X)   SDL_Swap32(X)
#define SDL_SwapLE64(X)   SDL_Swap64(X)
#define SDL_SwapBE16(X)   (X)
#define SDL_SwapBE32(X)   (X)
#define SDL_SwapBE64(X)   (X)
#endif
/*@}*/

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_endian_h */
