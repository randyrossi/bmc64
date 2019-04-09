/** \file   archdep_defs.h
 * \brief   Defines, enums and types used by the archdep functions
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
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

#ifndef VICE_ARCHDEP_DEFS_H
#define VICE_ARCHDEP_DEFS_H

#include "vice.h"


/** \brief  Various OS-identification macros
 *
 * The question marks indicate ports I have my doubts about they'll even run
 * VICE at all.
 *
 * <pre>
 *  ARCHDEP_OS_UNIX
 *    ARCHDEP_OS_OSX
 *    ARCHDEP_OS_LINUX
 *    ARCHDEP_OS_BSD
 *      ARCHDEP_OS_BSD_FREE
 *      ARCHDEP_OS_BSD_NET
 *      ARCHDEP_OS_BSD_OPEN
 *      ARCHDEP_OS_BSD_DRAGON
 *    ARCHDEP_OS_QNX (?)
 *    ARCHDEP_OS_SOLARIS (?)
 *  ARCHDEP_OS_WINDOWS
 *  ARCHDEP_OS_OS2 (?)
 *  ARCHDEP_OS_BEOS
 *  ARCHDEP_OS_MSDOS (?)
 *  ARCHDEP_OS_AMIGA
 * </pre>
 */
#ifdef UNIX_COMPILE

/** \brief  OS is UNIX */
# define ARCHDEP_OS_UNIX

# if defined(MACOSX_SUPPORT)

/** \brief  OS is Unix and OSX */
#  define ARCHDEP_OS_OSX

# elif defined(__linux__)

/** \brief  OS is Linux (yay!) */
#  define ARCHDEP_OS_LINUX

# elif defined(__FreeBSD__)

/** \brief  OS is BSD */
#  define ARCHDEP_OS_BSD

/** \brief  OS is FreeBSD */
#  define ARCHDEP_OS_BSD_FREE

# elif defined(__NetBSD__)

#  define ARCHDEP_OS_BSD
/** \brief  OS is NetBSD */
#  define ARCHDEP_OS_BSD_NET

# elif defined(__OpenBSD__)
#  define ARCHDEP_OS_BSD

/** \brief  OS is OpenBSD */
#  define ARCHDEP_OS_BSD_OPEN

# elif defined(__DragonFly__)
#  define ARCHDEP_OS_BSD


/** \brief  OS is DragonFly BSD */
#  define ARCHDEP_OS_BSD_DRAGON

# elif defined(__QNX__)

/**\brief   OS is QNX (do we even support this anymore?) */
#  define ARCHDEP_OS_QNX

# elif defined(sun) || defined(__sun)

/** \brief  OS is Solaris (same question) */
#  define ARCHDEP_OS_SOLARIS

# endif /* ifdef UNIX_COMPILE */

#elif defined(WIN32_COMPILE)

/** \brief  OS is Windows */
# define ARCHDEP_OS_WINDOWS

#elif defined(OS2_COMPILE)

/** \brief  OS is OS/2 (again: has anyone even tested this?) */
# define ARCHDEP_OS_OS2

#elif defined(BEOS_COMPILE)

/** \brief  OS is in the BeOS family */
# define ARCHDEP_OS_BEOS

# if defined(__BEOS__)

/** \brief  Assume OS is BeOS R5 compatible (if we still support that) */
#  define ARCHDEP_OS_BEOS_R5

# elif defined(__HAIKU__)

/** \brief  OS is Haiku */
#  define ARCHDEP_OS_HAIKU

# endif /* ifdef BEOS_COMPILE */

#elif defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__) || defined(__DOS__)

/** \brief  OS is MS-DOS (really?) */
# define ARCHDEP_OS_DOS

#elif defined(AMIGA_SUPPORT)

/** \brief  OS is AmigaOS
 *
 * May have to split/refine this for AROS etc
 */
# define ARCHDEP_OS_AMIGA
#endif


/** \brief  Arch-dependent directory separator used in paths
 */
#if defined(ARCHDEP_OS_WINDOWS) || defined(ARCHDEP_OS_OS2) \
    || defined(ARCHDEP_OS_DOS)

/** \brief  OS-dependent directory separator
 */
# define ARCHDEP_DIR_SEPARATOR  '\\'
#else
# define ARCHDEP_DIR_SEPARATOR  '/'
#endif

/** \brief  Extension used for autostart disks
 */
#define ARCHDEP_AUTOSTART_DICK_EXTENSION    "d64"


#if defined(ARCHEP_OS_AMIGA) || defined(ARCHDEP_OS_MSDOS) \
    || defined(ARCHDEP_OS_OS2) || defined(ARCHDEP_OS_WINDOWS)
/** \brief  Separator used for a pathlist
 */
# define ARCHDEP_FINDPATH_SEPARATOR_STRING  ";"

#else

/** \brief  Separator used for a pathlist
 */
# define ARCHDEP_FINDPATH_SEPARATOR_STRING  ":"
#endif


/* set LIBDIR and DOCDIR */
#ifdef ARCHDEP_OS_UNIX
/** \brief  Set VICE library dir
 *
 * This is completely wrong
 */
# define LIBDIR VICEDIR

/** 'brief  Set documentation dir
 *
 * Equally wrong, we should use XDG
 */
# define DOCDIR LIBDIR "/doc"
#endif


/*
 * Determine if we compile against SDL
 */
#if defined(USE_SDLUI) || defined(USE_SDLUI2)
# define ARCHDEP_USE_SDL
#endif

#if defined(ARCHDEP_OS_WINDOWS) || defined(ARCHDEP_OS_OS2) \
    || defined(ARCHDEP_OS_MSDOS) || defined(ARCHDEP_OS_BEOS)
# ifdef ARCHDEP_USE_SDL
#  define ARCHDEP_VICERC_NAME   "sdl-vice.ini"
/* Just copying stuff, I'm backwards */
#  define ARCHDEP_VICE_RTC_NAME "sdl-vice.rtc"
# else
#  define ARCHDEP_VICERC_NAME   "vice.ini"
#  define ARCHDEP_VICE_RTC_NAME "vice.rtc"
# endif
#else
# ifdef ARCHDEP_USE_SDL
#  define ARCHDEP_VICERC_NAME   "sdl-vicerc"
#  define ARCHDEP_VICE_RTC_NAME "sdl-vice.rtc"
# else
#  define ARCHDEP_VICERC_NAME   "vicerc"
#  define ARCHDEP_VICE_RTC_NAME "vice.rtc"
# endif
#endif

/** \brief  Autostart diskimage prefix
 */
#define ARCHDEP_AUTOSTART_DISKIMAGE_PREFIX  "autostart-"

/** \brief  Autostart diskimage suffix
 */
#define ARCHDEP_AUTOSTART_DISKIMAGE_SUFFIX  ".d64"


#endif
