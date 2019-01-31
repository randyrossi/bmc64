/*
 * SDL_stdinc.h
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

/** @file SDL_stdinc.h
 *  This is a general header that includes C language support
 */

#ifndef _SDL_stdinc_h
#define _SDL_stdinc_h

#include "SDL_config.h"


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <ctype.h>

/** The number of elements in an array */
#define SDL_arraysize(array)   (sizeof(array)/sizeof(array[0]))
#define SDL_TABLESIZE(table)   SDL_arraysize(table)

/* Use proper C++ casts when compiled as C++ to be compatible with the option
 -Wold-style-cast of GCC (and -Werror=old-style-cast in GCC 4.2 and above. */
#ifdef __cplusplus
#define SDL_reinterpret_cast(type, expression)   reinterpret_cast<type>(expression)
#define SDL_static_cast(type, expression)        static_cast<type>(expression)
#else
#define SDL_reinterpret_cast(type, expression)   ((type)(expression))
#define SDL_static_cast(type, expression)        ((type)(expression))
#endif

/** @name Basic data types */
/*@{*/
typedef enum {
	SDL_FALSE = 0,
	SDL_TRUE  = 1
} SDL_bool;

typedef int8_t     Sint8;
typedef uint8_t    Uint8;
typedef int16_t    Sint16;
typedef uint16_t   Uint16;
typedef int32_t    Sint32;
typedef uint32_t   Uint32;
typedef int64_t    Sint64;
typedef uint64_t   Uint64;

/*@}*/

/** @name Make sure the types really have the right sizes */
/*@{*/
#define SDL_COMPILE_TIME_ASSERT(name, x)   typedef int SDL_dummy_ ## name[(x) * 2 - 1]

SDL_COMPILE_TIME_ASSERT(uint8, sizeof(Uint8) == 1);
SDL_COMPILE_TIME_ASSERT(sint8, sizeof(Sint8) == 1);
SDL_COMPILE_TIME_ASSERT(uint16, sizeof(Uint16) == 2);
SDL_COMPILE_TIME_ASSERT(sint16, sizeof(Sint16) == 2);
SDL_COMPILE_TIME_ASSERT(uint32, sizeof(Uint32) == 4);
SDL_COMPILE_TIME_ASSERT(sint32, sizeof(Sint32) == 4);
SDL_COMPILE_TIME_ASSERT(uint64, sizeof(Uint64) == 8);
SDL_COMPILE_TIME_ASSERT(sint64, sizeof(Sint64) == 8);
/*@}*/

/** @name Enum Size Check
 *  Check to make sure enums are the size of ints, for structure packing.
 *  For both Watcom C/C++ and Borland C/C++ the compiler option that makes
 *  enums having the size of an int must be enabled.
 *  This is "-b" for Borland C/C++ and "-ei" for Watcom C/C++ (v11).
 */
typedef enum {
	DUMMY_ENUM_VALUE
} SDL_DUMMY_ENUM;

SDL_COMPILE_TIME_ASSERT(enum, sizeof(SDL_DUMMY_ENUM) == sizeof(int));
/*@}*/

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#define SDL_malloc    malloc
#define SDL_calloc    calloc
#define SDL_realloc   realloc
#define SDL_free      free

#ifndef alloca
#include <alloca.h>
#endif

#define SDL_stack_alloc(type, count)   (type*)alloca(sizeof(type) * (count))
#define SDL_stack_free(data)

#define SDL_getenv   getenv
#define SDL_putenv   putenv
#define SDL_qsort    qsort
#define SDL_abs      abs

#define SDL_min(x, y)   (((x) < (y)) ? (x) : (y))
#define SDL_max(x, y)   (((x) > (y)) ? (x) : (y))

#define SDL_isdigit(X)   isdigit(X)
#define SDL_isspace(X)   isspace(X)
#define SDL_toupper(X)   toupper(X)
#define SDL_tolower(X)   tolower(X)

#define SDL_memset   memset

#define SDL_zero(x)    SDL_memset(&(x), 0, sizeof((x)))
#define SDL_zerop(x)   SDL_memset((x), 0, sizeof(*(x)))

#if defined(__GNUC__) && defined(i386)
#define SDL_memset4(dst, val, len)                                 \
do {                                                               \
    int u0, u1, u2;                                                \
    __asm__ __volatile__ (                                         \
        "cld\n\t"                                                  \
        "rep ; stosl\n\t"                                          \
        : "=&D" (u0), "=&a" (u1), "=&c" (u2)                       \
        : "0" (dst), "1" (val), "2" (SDL_static_cast(Uint32, len)) \
        : "memory" );                                              \
} while(0)
#endif

#ifndef SDL_memset4
#define SDL_memset4(dst, val, len)               \
do {                                             \
    unsigned _count = (len);                     \
    unsigned _n = (_count + 3) / 4;              \
    Uint32 *_p = SDL_static_cast(Uint32 *, dst); \
    Uint32 _val = (val);                         \
    if (len == 0) break;                         \
    switch (_count % 4) {                        \
        case 0: do {    *_p++ = _val;            \
        case 3:         *_p++ = _val;            \
        case 2:         *_p++ = _val;            \
        case 1:         *_p++ = _val;            \
        } while ( --_n );                        \
    }                                            \
} while(0)
#endif

#if defined(__GNUC__) && defined(i386)
#define SDL_memcpy(dst, src, len )                                                  \
do {                                                                                \
    int u0, u1, u2;                                                                 \
    __asm__ __volatile__ (                                                          \
        "cld\n\t"                                                                   \
        "rep ; movsl\n\t"                                                           \
        "testb $2,%b4\n\t"                                                          \
        "je 1f\n\t"                                                                 \
        "movsw\n"                                                                   \
        "1:\ttestb $1,%b4\n\t"                                                      \
        "je 2f\n\t"                                                                 \
        "movsb\n"                                                                   \
        "2:"                                                                        \
        : "=&c" (u0), "=&D" (u1), "=&S" (u2)                                        \
        : "0" (SDL_static_cast(unsigned, len) / 4), "q" (len), "1" (dst), "2" (src) \
        : "memory" );                                                               \
} while(0)
#endif

#ifndef SDL_memcpy
#define SDL_memcpy   memcpy
#endif

#if defined(__GNUC__) && defined(i386)
#define SDL_memcpy4(dst, src, len)                                   \
do {                                                                 \
    int ecx, edi, esi;                                               \
    __asm__ __volatile__ (                                           \
        "cld\n\t"                                                    \
        "rep ; movsl"                                                \
        : "=&c" (ecx), "=&D" (edi), "=&S" (esi)                      \
        : "0" (SDL_static_cast(unsigned, len)), "1" (dst), "2" (src) \
        : "memory" );                                                \
} while(0)
#endif

#ifndef SDL_memcpy4
#define SDL_memcpy4(dst, src, len)   SDL_memcpy(dst, src, (len) << 2)
#endif

#if 0
#if defined(__GNUC__) && defined(i386)
#define SDL_revcpy(dst, src, len)                  \
do {                                               \
    int u0, u1, u2;                                \
    char *dstp = SDL_static_cast(char *, dst);     \
    char *srcp = SDL_static_cast(char *, src);     \
    int n = (len);                                 \
    if ( n >= 4 ) {                                \
        __asm__ __volatile__ (                     \
        "std\n\t"                                  \
        "rep ; movsl\n\t"                          \
        "cld\n\t"                                  \
        : "=&c" (u0), "=&D" (u1), "=&S" (u2)       \
        : "0" (n >> 2),                            \
        "1" (dstp + (n - 4)), "2" (srcp + (n - 4)) \
        : "memory" );                              \
    }                                              \
    switch (n & 3) {                               \
        case 3: dstp[2] = srcp[2];                 \
        case 2: dstp[1] = srcp[1];                 \
        case 1: dstp[0] = srcp[0];                 \
            break;                                 \
        default:                                   \
            break;                                 \
    }                                              \
} while(0)
#endif
#endif

extern DECLSPEC void * SDLCALL SDL_revcpy(void *dst, const void *src, size_t len);

#define SDL_memmove   memmove
#define SDL_memcmp    memcmp
#define SDL_strlen    strlen
#define SDL_strlcpy   strlcpy
#define SDL_strlcat   strlcat
#define SDL_strdup    strdup

extern DECLSPEC char * SDLCALL SDL_strrev(char *string);
extern DECLSPEC char * SDLCALL SDL_strupr(char *string);
extern DECLSPEC char * SDLCALL SDL_strlwr(char *string);

#define SDL_strchr    strchr
#define SDL_strrchr   strrchr
#define SDL_strstr    strstr

#define SDL_itoa(value, string, radix)   SDL_ltoa((long)value, string, radix)

extern DECLSPEC char * SDLCALL SDL_ltoa(long value, char *string, int radix);

#define SDL_uitoa(value, string, radix)   SDL_ultoa((long)value, string, radix)

extern DECLSPEC char * SDLCALL SDL_ultoa(unsigned long value, char *string, int radix);

#define SDL_strtol    strtol
#define SDL_strtoul   strtoul

extern DECLSPEC char* SDLCALL SDL_lltoa(Sint64 value, char *string, int radix);
extern DECLSPEC char* SDLCALL SDL_ulltoa(Uint64 value, char *string, int radix);

#define SDL_strtoll       strtoll
#define SDL_strtoull      strtoull
#define SDL_strtod        strtod
#define SDL_atoi          atoi
#define SDL_atof          atof
#define SDL_strcmp        strcmp
#define SDL_strncmp       strncmp
#define SDL_strcasecmp    strcasecmp
#define SDL_strncasecmp   strncasecmp
#define SDL_sscanf        sscanf
#define SDL_snprintf      snprintf
#define SDL_vsnprintf     vsnprintf

/** @name SDL_ICONV Error Codes
 *  The SDL implementation of iconv() returns these error codes 
 */
/*@{*/
#define SDL_ICONV_ERROR    (size_t)-1
#define SDL_ICONV_E2BIG    (size_t)-2
#define SDL_ICONV_EILSEQ   (size_t)-3
#define SDL_ICONV_EINVAL   (size_t)-4
/*@}*/

typedef struct _SDL_iconv_t *SDL_iconv_t;
extern DECLSPEC SDL_iconv_t SDLCALL SDL_iconv_open(const char *tocode, const char *fromcode);
extern DECLSPEC int SDLCALL SDL_iconv_close(SDL_iconv_t cd);

extern DECLSPEC size_t SDLCALL SDL_iconv(SDL_iconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);

/** This function converts a string between encodings in one pass, returning a
 *  string that must be freed with SDL_free() or NULL on error.
 */
extern DECLSPEC char * SDLCALL SDL_iconv_string(const char *tocode, const char *fromcode, const char *inbuf, size_t inbytesleft);
#define SDL_iconv_utf8_locale(S)   SDL_iconv_string("", "UTF-8", S, SDL_strlen(S) + 1)
#define SDL_iconv_utf8_ucs2(S)     (Uint16 *)SDL_iconv_string("UCS-2", "UTF-8", S, SDL_strlen(S) + 1)
#define SDL_iconv_utf8_ucs4(S)     (Uint32 *)SDL_iconv_string("UCS-4", "UTF-8", S, SDL_strlen(S) + 1)

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_stdinc_h */
