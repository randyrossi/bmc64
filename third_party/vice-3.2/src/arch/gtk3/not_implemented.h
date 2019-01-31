/** \file   not_implemented.h
 * \brief   Temporary file to signal implementation state of functions
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

#ifndef VICE_NOT_IMPLEMENTED_H
#define VICE_NOT_IMPLEMENTED_H

#include <stdio.h>
#include <stdlib.h>


/** \brief  Not-implemented message with file, function and lineno, calls exit(1)
 */
#define NOT_IMPLEMENTED() \
    fprintf(stderr, \
            "%s:%d: error: function %s() is not implemented yet, exiting\n", \
            __FILE__, __LINE__, __func__); \
    exit(1)


/** \brief  Not-implemented message with file, function and lineno, only warns
 */
#define NOT_IMPLEMENTED_WARN_ONLY() \
    fprintf(stderr, \
            "%s:%d: warning: function %s() is not implemented yet, continuing\n", \
            __FILE__, __LINE__, __func__)


/** \brief  Not-implemented message, shown at most X times
 *
 * This macro limits the number of 'not implemented' messages appearing on
 * stderr, so the terminal debug output doesn't get flooded.
 *
 * Usage: declare a `static int foo_msgs` somewhere and then in the function you
 *        want to limit the number of messages it spits out, use this macro:
 *        NOT_IMPLEMENTED_WARN_X_TIMES(foo_msgs, 5);    (warn 5 times at most)
 *
 * \param[in,out]   C   counter variable (int)
 * \param[in]       X   maximum number of times to show the warning (int)
 */
#define NOT_IMPLEMENTED_WARN_X_TIMES(C, X) \
    if ((C)++ < (X)) { \
        fprintf(stderr, \
                "%s:%d: warning function %s() is not implemented yet, " \
                "warning %d/%d\n", \
                __FILE__, __LINE__, __func__, (C), (X)); \
    }


/** \brief  Incomplete implementation message, only warns
 */
#define INCOMPLETE_IMPLEMENTATION() \
    fprintf(stderr, \
            "%s:%d: warning: function %s() is not fully implemented yet, continuing\n", \
            __FILE__, __LINE__, __func__)


/** \brief  Temporary implementation message, only warns
 */
#define TEMPORARY_IMPLEMENTATION() \
    fprintf(stderr, \
            "%s:%d: warning: function %s() contains a temporary implementation, continuing\n", \
            __FILE__, __LINE__, __func__)

#endif
