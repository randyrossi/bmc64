/*
 * vsyncarch.c - Mac OS X specific vsync functions
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * Based on Code by
 *  http://www.cocoadev.com/index.pl?DeterminingOSVersion
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

#ifndef DARWIN_COMPILE

#include <CoreVideo/CVHostTime.h>

#include "vsyncapi.h"

/* If you divide the host time with this factor you'll get a microsec value 
   E.g. if the host time is in nanosecs then the factor will be 1000
*/

#ifdef HAVE_NANOSLEEP
#define TICKSPERSECOND 1000000000UL 
unsigned long hostToUsFactor = 1000000UL;
#else
#define TICKSPERSECOND 1000000UL
unsigned long hostToUsFactor = 1000UL;
#endif

/* Number of timer units per second. */
unsigned long vsyncarch_frequency(void)
{
    /* how to convert host time to us */
    hostToUsFactor = (unsigned long)(CVGetHostClockFrequency() / TICKSPERSECOND);
    
    /* Microseconds or nanosecond resolution. */
    return TICKSPERSECOND;
}

/* Get time in timer units. */
unsigned long vsyncarch_gettime(void)
{
    return (unsigned long)(CVGetCurrentHostTime() / hostToUsFactor);
}
#endif
