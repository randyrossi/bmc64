/** \file   archdep_atexit.c
 * \brief   atexit(3) work arounds
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 * \author  Blacky Stardust
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

#include "vice.h"
#include <stdio.h>
#include <stdlib.h>

#include "archdep_atexit.h"


#if defined(USE_NATIVE_GTK3) && defined(WIN32_COMPILE) && !defined(__cplusplus)
#define ATEXIT_MAX_FUNCS 64

#include "debug_gtk3.h"

/** \brief  List of functions registered with archdep_vice_atexit()
 *
 * The functions get called in the reverse order of which they were registered,
 * so this acts as a stack,
 */
static void (*atexit_functions[ATEXIT_MAX_FUNCS + 1])(void);


/** \brief  Number of register functions in #atexit_functions
 */
static int atexit_counter = 0;


/** \brief  Register \a function to be called on exit() or return from main
 *
 * Wrapper to work around Windows not handling the C library's atexit()
 * mechanism properly
 *
 * \param[in]   function    function to call at exit
 *
 * \return  0 on success, 1 on failure
 */
int archdep_vice_atexit(void (*function)(void))
{
    debug_gtk3("registering function %p.", function);
    if (atexit_counter == ATEXIT_MAX_FUNCS) {
        debug_gtk3("ERROR: max atexit functions reached.");
        return 1;
    }

    atexit_functions[atexit_counter] = function;
    atexit_counter++;

    return 0;
}


/** \brief  Wrapper around exit()
 *
 * \param[in]   excode  exit code
 */
void archdep_vice_exit(int excode)
{
    const void (*f)(void);

    debug_gtk3("unrolling atexit stack:");
    /* don't check for NULL, segfaults allow backtraces in gdb */
    while (atexit_counter > 0) {
        atexit_counter--;
        f = atexit_functions[atexit_counter];
        debug_gtk3("running atexit %d: %p.", atexit_counter, f)
        f();
    }
    exit(excode);
}
#else  /* ifdef WIN32_COMPILE */


/** \brief  Register \a function to be called on exit() or return from main
 *
 * Wrapper to work around Windows not handling the C library's atexit()
 * mechanism properly
 *
 * \param[in]   function    function to call at exit
 *
 * \return  0 on success, 1 on failure
 */
int archdep_vice_atexit(void (*function)(void))
{
    return atexit(function);
}


/** \brief  Wrapper around exit()
 *
 * \param[in]   excode  exit code
 */
void archdep_vice_exit(int excode)
{
    exit(excode);
}

#endif
