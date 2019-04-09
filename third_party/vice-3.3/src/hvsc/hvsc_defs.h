/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/hvsc_defs.h
 * \brief   Globally used constants and types
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

#ifndef HVSC_HVSC_DEFS_H
#define HVSC_HVSC_DEFS_H

/** \brief  Enable debugging messages via hvsc_dbg()
 */
#if 0
#define HVSC_DEBUG
#endif

/*
 * A bit of preprocessor trickery to properly stringify the version numbers
 */

/** \brief  Helper macro to stringify version numbers
 */
#define _xstr1(s) #s

/** \brief  Helper macro to stringify version numbers
 */
#define _xstr(s) _xstr1(s)

/** \brief  Library version number major
 *
 * This number indicates API version, when this number changes it means there
 * are incompatible changes in the API. Set via the Makefile.
 */
#ifndef HSVC_LIB_VERSION_MAJ
#define HVSC_LIB_VERSION_MAJ    0
#endif

/** \brief  Library version number minor
 *
 * This number indicates 'feature level', when this number changes, a feature
 * was added. Set via the Makefile.
 */
#ifndef HVSC_LIB_VERSION_MIN
#define HVSC_LIB_VERSION_MIN    0
#endif

/** \brief  Library version number revision
 *
 * This is the revision number, this indicates bug fixes/optimizations that do
 * not change the API, nor add functionality. Set via the Makefile
 */
#ifndef HVSC_LIB_VERSION_REV
#define HVSC_LIB_VERSION_REV    0
#endif

/** \brief  Library version string
 */
#define HVSC_LIB_VERSION_STR _xstr(HVSC_LIB_VERSION_MAJ) \
    "." _xstr(HVSC_LIB_VERSION_MIN) \
    "." _xstr(HVSC_LIB_VERSION_REV)


/** \brief  Path to the Songlengths file, relative to the HVSC root
 */
#define HVSC_SLDB_FILE  "DOCUMENTS/Songlengths.md5"


/** \brief  Path to the STIL file, relative to the HVSC root
 */
#define HVSC_STIL_FILE  "DOCUMENTS/STIL.txt"


/** \brief  Path to the BUGlist file, relative to the HVSC root
 */
#define HVSC_BUGS_FILE  "DOCUMENTS/BUGlist.txt"


/** \brief  MD5 digest size in bytes
 */
#define HVSC_DIGEST_SIZE    16





/** \brief  Number of initial entries in the STIL buffer
 */
#define HVSC_STIL_BUFFER_INIT    32



/** \brief  Initial size of the fields array in a hvsc_stil_block_t
 */
#define HVSC_STIL_BLOCK_FIELDS_INIT    32


/** \brief  Initial size of blocks array in a handle
 */
#define HVSC_HANDLE_BLOCKS_INIT    32


#include "hvsc.h"

/** \brief  STIL parser state
 */
typedef struct hvsc_stil_parser_state_s {
    hvsc_stil_t *           handle;     /**< STIL handle */
    hvsc_stil_field_t *     field;      /**< STIL field object */
    int                     tune;       /**< current tune number */
    size_t                  lineno;     /**< line number in STIL text buffer */
    hvsc_stil_block_t *     block;      /**< temporary STIL block */

    hvsc_stil_timestamp_t   ts;         /**< temporary timestamp object */
    size_t                  linelen;    /**< remaining length of the current
                                             line after parsing out the
                                             optional sub fields
                                             (timestamp, album) */

    char *                  album;      /**< album/cover string */
    size_t                  album_len;  /**< length of album string */
} hvsc_stil_parser_state_t;


#endif
