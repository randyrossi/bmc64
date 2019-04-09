/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/bugs.c
 * \brief   BUGlist.txt handling
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 *  HVSClib - a library to work with High Voltage SID Collection files
 *  Copyright (C) 2018  Bas Wassink <b.wassink@ziggo.nl>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.*
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include "hvsc.h"

#include "hvsc_defs.h"
#include "base.h"

#include "bugs.h"


/** \brief  Initialize BUGlist \a handle
 *
 * \param[in,out]   handle  BUGlist handle
 */
static void bugs_init_handle(hvsc_bugs_t *handle)
{
    hvsc_text_file_init_handle(&(handle->bugs));
    handle->psid_path = NULL;
    handle->text = NULL;
    handle->user = NULL;

}


/** \brief  Free all members of \a handle but not \a handle itself
 *
 * \param[in,out]   handle  BUGS handle
 */
static void bugs_free_handle(hvsc_bugs_t *handle)
{
    hvsc_text_file_close(&(handle->bugs));
    if (handle->text != NULL) {
        free(handle->text);
    }
    if (handle->user != NULL) {
        free(handle->user);
    }
}


/** \brief  Parse the BUGlist for a BUG: field and the (username) field
 *
 * \param[in,out]   handle  BUGlist handle
 *
 * \return  bool
 */
static int bugs_parse(hvsc_bugs_t *handle)
{
    const char *line;
    char *bug;

    /* grab first line, should contain 'BUG:' */
    line = hvsc_text_file_read(&(handle->bugs));
    if (line == NULL) {
        return 0;
    }

    hvsc_dbg("First line of entry: %s\n", line);
    if (hvsc_get_field_type(line) != HVSC_FIELD_BUG) {
        hvsc_dbg("Fail: not a BUG field\n");
        return 0;
    }

    /* store first line of BUG field */
    bug = hvsc_strdup(line + 9);
    if (bug == NULL) {
        return 0;
    }

    /* add rest of BUG field */
    while (1) {
        line = hvsc_text_file_read(&(handle->bugs));
        if (line == NULL) {
            /* not supposed to happen */
            free(bug);
            return 0;
        }

        if (strncmp("         ", line, 9) == 0) {
            /* new line for the bug field */
            size_t len;
            char *tmp;

            /* strip off 8 spaces, leaving one to add to the result */
            len = strlen(line) - 8;
            tmp = realloc(bug, strlen(bug) + len + 1);
            if (tmp == NULL) {
                hvsc_errno = HVSC_ERR_OOM;
                free(bug);
                return 0;
            }
            bug = tmp;
            strcat(bug, line + 8);
        } else {
            /* store bug in handle */
            handle->text = bug;
            /* assume (user) field */
            handle->user = hvsc_strdup(line);
            if (handle->user == NULL) {
                free(handle->text);
                handle->text = NULL;
                return 0;
            }
            return 1;
        }
    }
    return 1;
}


/** \brief  Open BUGlist and search for file \a psid
 *
 * \param[in]       psid    absolute path to PSID file
 * \param[in,out]   handle  BUGlist handle
 *
 * \return  bool
 */
int hvsc_bugs_open(const char *psid, hvsc_bugs_t *handle)
{
    bugs_init_handle(handle);

    /* open BUGlist.txt */
    if (!hvsc_text_file_open(hvsc_bugs_path, &(handle->bugs))) {
        return 0;
    }

    /* make copy of psid, ripping off the HVSC root directory */
    handle->psid_path = hvsc_path_strip_root(psid);
#if defined(_WIN32) || defined(_WIN64)
    /* fix directory separators */
    hvsc_path_fix_separators(handle->psid_path);
#endif
    hvsc_dbg("stripped path is '%s'\n", handle->psid_path);
    if (handle->psid_path == NULL) {
        hvsc_bugs_close(handle);
        return 0;
    }

    /* find the entry */
    while (1) {
        const char *line;

        line = hvsc_text_file_read(&(handle->bugs));
        if (line == NULL) {
            if (feof(handle->bugs.fp)) {
                /* EOF, so simply not found */
                hvsc_errno = HVSC_ERR_NOT_FOUND;
            }
            hvsc_bugs_close(handle);
            /* I/O error is already set */
            return 0;
        }

        if (strcmp(line, handle->psid_path) == 0) {
            hvsc_dbg("Found '%s' at line %ld\n", line, handle->bugs.lineno);
            return bugs_parse(handle);
        }
    }

    /* not found */
    hvsc_errno = HVSC_ERR_NOT_FOUND;
    hvsc_bugs_close(handle);
    return 1;
}


/** \brief  Clean up memory used by the members of \a handle
 *
 * \param[in,out]   handle  BUGlist handle
 */
void hvsc_bugs_close(hvsc_bugs_t *handle)
{
    bugs_free_handle(handle);
    if (handle->psid_path != NULL) {
        free(handle->psid_path);
    }
}
