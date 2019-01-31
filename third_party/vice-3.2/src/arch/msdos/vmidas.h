/*
 * vmidas.h - MIDAS function wrappers.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_VMIDAS_H
#define VICE_VMIDAS_H

/* This is a dirty kludge to avoid that the `midasdll.h' header redefines
   `DWORD', which we already have. */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef int BOOL;

#include "vice.h"
#include "types.h"

#include <midasdll.h>

#define MIDAS_CONFIG_FNAME "midas.cfg"

extern int _midas_available;

BOOL vmidas_startup(void);
BOOL vmidas_init(void);
BOOL vmidas_remove_timer_callbacks(void);
BOOL vmidas_set_timer_callbacks(DWORD rate, BOOL displaySync, void (MIDAS_CALL *preVR)(), void (MIDAS_CALL *immVR)(), void (MIDAS_CALL *inVR)());
BOOL vmidas_config(void);

inline static BOOL vmidas_available(void)
{
    return _midas_available;
}

#endif
