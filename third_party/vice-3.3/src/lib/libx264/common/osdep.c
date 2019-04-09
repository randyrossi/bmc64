/*****************************************************************************
 * osdep.c: platform-specific code
 *****************************************************************************
 * Copyright (C) 2003-2014 x264 project
 *
 * Authors: Steven Walters <kemuri9@gmail.com>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Henrik Gramner <henrik@gramner.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#include "common.h"

#ifdef SYS_AROS
#include <math.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

#if SYS_WINDOWS
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <time.h>

#if PTW32_STATIC_LIB
/* this is a global in pthread-win32 to indicate if it has been initialized or not */
extern int ptw32_processInitialized;
#endif

#ifdef SYS_AROS
#if !defined(__amd64__) && !defined(__powerpc__)
float log2f(float p)
{
    return (float)log2((double)p);
}
#endif
#endif

#ifdef __MSDOS__
#define x264_min(x, y)	(((x) < (y)) ? (x) : (y))

static const char ntoa_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z'
};

size_t strlcpy(char *dst, const char *src, size_t maxlen)
{
    size_t srclen = strlen(src);
    if ( maxlen > 0 ) {
        size_t len = x264_min(srclen, maxlen-1);
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return srclen;
}

char *strrev(char *string)
{
    size_t len = strlen(string);
    char *a = &string[0];
    char *b = &string[len-1];
    len /= 2;
    while ( len-- ) {
        char c = *a;
        *a++ = *b;
        *b-- = c;
    }
    return string;
}

char *ltoa(long value, char *string, int radix)
{
    char *bufp = string;

    if ( value < 0 ) {
        *bufp++ = '-';
        value = -value;
    }
    if ( value ) {
        while ( value > 0 ) {
            *bufp++ = ntoa_table[value % radix];
            value /= radix;
        }
    } else {
        *bufp++ = '0';
    }
    *bufp = '\0';

    /* The numbers went into the string backwards. :) */
    if ( *string == '-' ) {
        strrev(string+1);
    } else {
        strrev(string);
    }

    return string;
}

char *ultoa(unsigned long value, char *string, int radix)
{
    char *bufp = string;

    if ( value ) {
        while ( value > 0 ) {
            *bufp++ = ntoa_table[value % radix];
            value /= radix;
        }
    } else {
        *bufp++ = '0';
    }
    *bufp = '\0';

    /* The numbers went into the string backwards. :) */
    strrev(string);

    return string;
}

static size_t x264_PrintLong(char *text, long value, int radix, size_t maxlen)
{
    char num[130];
    size_t size;

    ltoa(value, num, radix);
    size = strlen(num);
    if ( size >= maxlen ) {
        size = maxlen-1;
    }
    strlcpy(text, num, size+1);

    return size;
}

static size_t x264_PrintUnsignedLong(char *text, unsigned long value, int radix, size_t maxlen)
{
    char num[130];
    size_t size;

    ultoa(value, num, radix);
    size = strlen(num);
    if ( size >= maxlen ) {
        size = maxlen-1;
    }
    strlcpy(text, num, size+1);

    return size;
}

static size_t x264_PrintFloat(char *text, double arg, size_t maxlen)
{
    char *textstart = text;
    if ( arg ) {
        /* This isn't especially accurate, but hey, it's easy. :) */
        const double precision = 0.00000001;
        size_t len;
        unsigned long value;

        if ( arg < 0 ) {
            *text++ = '-';
            --maxlen;
            arg = -arg;
        }
        value = (unsigned long)arg;
        len = x264_PrintUnsignedLong(text, value, 10, maxlen);
        text += len;
        maxlen -= len;
        arg -= value;
        if ( arg > precision && maxlen ) {
            int mult = 10;
            *text++ = '.';
            while ( (arg > precision) && maxlen ) {
                value = (unsigned long)(arg * mult);
                len = x264_PrintUnsignedLong(text, value, 10, maxlen);
                text += len;
                maxlen -= len;
                arg -= (double)value / mult;
                mult *= 10;
            }
        }
    } else {
        *text++ = '0';
    }
    return (text - textstart);
}

static size_t x264_PrintString(char *text, const char *string, size_t maxlen)
{
    char *textstart = text;

    while ( *string && maxlen-- ) {
        *text++ = *string++;
    }
    return (text - textstart);
}

int vsnprintf(char *text, size_t maxlen, const char *fmt, va_list ap)
{
    char *textstart = text;
    if ( maxlen <= 0 ) {
        return 0;
    }
    --maxlen; /* For the trailing '\0' */
    while ( *fmt && maxlen ) {
        if ( *fmt == '%' ) {
            int done = 0;
            size_t len = 0;
            int do_lowercase = 0;
            int radix = 10;
            enum {
                DO_INT,
                DO_LONG,
                DO_LONGLONG
            } inttype = DO_INT;

            ++fmt;
            /* FIXME: implement more of the format specifiers */
            while ( *fmt == '.' || (*fmt >= '0' && *fmt <= '9') ) {
                ++fmt;
            }
            while (!done) {
                switch (*fmt) {
                    case '%':
                        *text = '%';
                        len = 1;
                        done = 1;
                        break;
                    case 'c':
                        /* char is promoted to int when passed through (...) */
                        *text = (char)va_arg(ap, int);
                        len = 1;
                        done = 1;
                        break;
                    case 'h':
                        /* short is promoted to int when passed through (...) */
                        break;
                    case 'l':
                        if ( inttype < DO_LONGLONG ) {
                            ++inttype;
                        }
                        break;
                    case 'I':
                        if ( strncmp(fmt, "I64", 3) == 0 ) {
                            fmt += 2;
                            inttype = DO_LONGLONG;
                        }
                        break;
                    case 'i':
                    case 'd':
                        switch (inttype) {
                            case DO_INT:
                                len = x264_PrintLong(text, (long)va_arg(ap, int), radix, maxlen);
                                break;
                            case DO_LONG:
                                len = x264_PrintLong(text, va_arg(ap, long), radix, maxlen);
                                break;
                            case DO_LONGLONG:
                                len = x264_PrintLong(text, va_arg(ap, long), radix, maxlen);
                                break;
                        }
                        done = 1;
                        break;
                    case 'p':
                    case 'x':
                        do_lowercase = 1;
                        /* Fall through to 'X' handling */
                    case 'X':
                        if ( radix == 10 ) {
                            radix = 16;
                        }
                        if ( *fmt == 'p' ) {
                            inttype = DO_LONG;
                        }
                        /* Fall through to unsigned handling */
                    case 'o':
                        if ( radix == 10 ) {
                            radix = 8;
                        }
                        /* Fall through to unsigned handling */
                    case 'u':
                        switch (inttype) {
                            case DO_INT:
                                len = x264_PrintUnsignedLong(text, (unsigned long)va_arg(ap, unsigned int), radix, maxlen);
                                break;
                            case DO_LONG:
                                len = x264_PrintUnsignedLong(text, va_arg(ap, unsigned long), radix, maxlen);
                                break;
                            case DO_LONGLONG:
                                len = x264_PrintUnsignedLong(text, va_arg(ap, unsigned long), radix, maxlen);
                                break;
                        }
                        if ( do_lowercase ) {
                            strlwr(text);
                        }
                        done = 1;
                        break;
                    case 'f':
                        len = x264_PrintFloat(text, va_arg(ap, double), maxlen);
                        done = 1;
                        break;
                    case 's':
                        len = x264_PrintString(text, va_arg(ap, char*), maxlen);
                        done = 1;
                        break;
                    default:
                        done = 1;
                        break;
                }
                ++fmt;
            }
            text += len;
            maxlen -= len;
        } else {
            *text++ = *fmt++;
            --maxlen;
        }
    }
    *text = '\0';

    return (text - textstart);
}

int snprintf(char *text, size_t maxlen, const char *fmt, ...)
{
    va_list ap;
    int retval;

    va_start(ap, fmt);
    retval = vsnprintf(text, maxlen, fmt, ap);
    va_end(ap);

    return retval;
}
#endif

int64_t x264_mdate( void )
{
#if SYS_WINDOWS
    struct timeb tb;
    ftime( &tb );
    return ((int64_t)tb.time * 1000 + (int64_t)tb.millitm) * 1000;
#else
    struct timeval tv_date;
    gettimeofday( &tv_date, NULL );
    return (int64_t)tv_date.tv_sec * 1000000 + (int64_t)tv_date.tv_usec;
#endif
}

#if HAVE_WIN32THREAD || PTW32_STATIC_LIB
/* state of the threading library being initialized */
static volatile LONG x264_threading_is_init = 0;

static void x264_threading_destroy( void )
{
#if PTW32_STATIC_LIB
    pthread_win32_thread_detach_np();
    pthread_win32_process_detach_np();
#else
    x264_win32_threading_destroy();
#endif
}

int x264_threading_init( void )
{
    /* if already init, then do nothing */
    if( InterlockedCompareExchange( &x264_threading_is_init, 1, 0 ) )
        return 0;
#if PTW32_STATIC_LIB
    /* if static pthread-win32 is already initialized, then do nothing */
    if( ptw32_processInitialized )
        return 0;
    if( !pthread_win32_process_attach_np() )
        return -1;
#else
    if( x264_win32_threading_init() )
        return -1;
#endif
    /* register cleanup to run at process termination */
    atexit( x264_threading_destroy );

    return 0;
}
#endif

#if HAVE_MMX
#ifdef __INTEL_COMPILER
/* Agner's patch to Intel's CPU dispatcher from pages 131-132 of
 * http://agner.org/optimize/optimizing_cpp.pdf (2011-01-30)
 * adapted to x264's cpu schema. */

// Global variable indicating cpu
int __intel_cpu_indicator = 0;
// CPU dispatcher function
void x264_intel_cpu_indicator_init( void )
{
    unsigned int cpu = x264_cpu_detect();
    if( cpu&X264_CPU_AVX )
        __intel_cpu_indicator = 0x20000;
    else if( cpu&X264_CPU_SSE42 )
        __intel_cpu_indicator = 0x8000;
    else if( cpu&X264_CPU_SSE4 )
        __intel_cpu_indicator = 0x2000;
    else if( cpu&X264_CPU_SSSE3 )
        __intel_cpu_indicator = 0x1000;
    else if( cpu&X264_CPU_SSE3 )
        __intel_cpu_indicator = 0x800;
    else if( cpu&X264_CPU_SSE2 && !(cpu&X264_CPU_SSE2_IS_SLOW) )
        __intel_cpu_indicator = 0x200;
    else if( cpu&X264_CPU_SSE )
        __intel_cpu_indicator = 0x80;
    else if( cpu&X264_CPU_MMX2 )
        __intel_cpu_indicator = 8;
    else
        __intel_cpu_indicator = 1;
}

/* __intel_cpu_indicator_init appears to have a non-standard calling convention that
 * assumes certain registers aren't preserved, so we'll route it through a function
 * that backs up all the registers. */
void __intel_cpu_indicator_init( void )
{
    x264_safe_intel_cpu_indicator_init();
}
#else
void x264_intel_cpu_indicator_init( void )
{}
#endif
#endif

#ifdef _WIN32
/* Functions for dealing with Unicode on Windows. */
FILE *x264_fopen( const char *filename, const char *mode )
{
    wchar_t filename_utf16[MAX_PATH];
    wchar_t mode_utf16[16];
    if( utf8_to_utf16( filename, filename_utf16 ) && utf8_to_utf16( mode, mode_utf16 ) )
        return _wfopen( filename_utf16, mode_utf16 );
    return NULL;
}

int x264_rename( const char *oldname, const char *newname )
{
    wchar_t oldname_utf16[MAX_PATH];
    wchar_t newname_utf16[MAX_PATH];
    if( utf8_to_utf16( oldname, oldname_utf16 ) && utf8_to_utf16( newname, newname_utf16 ) )
    {
        /* POSIX says that rename() removes the destination, but Win32 doesn't. */
        _wunlink( newname_utf16 );
        return _wrename( oldname_utf16, newname_utf16 );
    }
    return -1;
}

int x264_stat( const char *path, x264_struct_stat *buf )
{
    wchar_t path_utf16[MAX_PATH];
    if( utf8_to_utf16( path, path_utf16 ) )
        return _wstati64( path_utf16, buf );
    return -1;
}

int x264_vfprintf( FILE *stream, const char *format, va_list arg )
{
    HANDLE console = NULL;
    DWORD mode;

    if( stream == stdout )
        console = GetStdHandle( STD_OUTPUT_HANDLE );
    else if( stream == stderr )
        console = GetStdHandle( STD_ERROR_HANDLE );

    /* Only attempt to convert to UTF-16 when writing to a non-redirected console screen buffer. */
    if( GetConsoleMode( console, &mode ) )
    {
        char buf[4096];
        wchar_t buf_utf16[4096];
        int length = vsnprintf( buf, sizeof(buf), format, arg );

        if( length > 0 && length < sizeof(buf) )
        {
            /* WriteConsoleW is the most reliable way to output Unicode to a console. */
            int length_utf16 = MultiByteToWideChar( CP_UTF8, 0, buf, length, buf_utf16, sizeof(buf_utf16)/sizeof(wchar_t) );
            DWORD written;
            WriteConsoleW( console, buf_utf16, length_utf16, &written, NULL );
            return length;
        }
    }
    return vfprintf( stream, format, arg );
}

int x264_is_pipe( const char *path )
{
    wchar_t path_utf16[MAX_PATH];
    if( utf8_to_utf16( path, path_utf16 ) )
        return WaitNamedPipeW( path_utf16, 0 );
    return 0;
}
#endif
