/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/psid.h
 * \brief   PSID/RSID file handling - header
 *
 * Non-public psid functions and constants. The descriptions of the various
 * constants can use WORD to indicate a 16-bit unsigned big endian integer and
 * LONGWORD to indicate a 32-bit unsigned big endian integer.
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

#ifndef HVSC_PSID_H
#define HVSC_PSID_H

/** \brief  Minimum number of bytes required for the header parser
 *
 * This is the size of a v2+ header + 2 bytes for an eventual load address
 */
#define HVSC_PSID_HEADER_MIN_SIZE   0x7e


/** \brief  Offset in the header of the magic bytes
 */
#define HVSC_PSID_MAGIC         0x00


/** \brief  Offset in header of the version (WORD)
 */
#define HVSC_PSID_VERSION       0x04

/** \brief  Offset in the header of the data offset (WORD)
 */
#define HVSC_PSID_DATA_OFFSET   0x06

/** \brief  Offset in the header of the load address (WORD)
 */
#define HVSC_PSID_LOAD_ADDRESS  0x08

/** \brief  Offset in the header of the init address (WORD)
 */
#define HVSC_PSID_INIT_ADDRESS  0x0a

/** \brief  Offset in the header of the init address (WORD)
 */
#define HVSC_PSID_PLAY_ADDRESS  0x0c

/** \brief  Offset in the header of the song count (WORD)
 */
#define HVSC_PSID_SONGS         0x0e

/** \brief  Offset in the header of the starting song (WORD)
 */
#define HVSC_PSID_START_SONG    0x10

/** \brief  Offset in the header of the speed bits (LONGWORD)
 */
#define HVSC_PSID_SPEED         0x12

/** \brief  Offset in the header of the name field
 *
 * A 32 character ASCII string, not nul-terminated, in Latin-1 encoding
 */
#define HVSC_PSID_NAME          0x16

/** \brief  Offset in the header of the author field
 *
 * A 32 character ASCII string, not nul-terminated, in Latin-1 encoding
 */
#define HVSC_PSID_AUTHOR        0x36

/** \brief  Offset in the header of the copyright field
 *
 * A 32 character ASCII string, not nul-terminated, in Latin-1 encoding
 */
#define HVSC_PSID_COPYRIGHT     0x56

/*
 * PSID v2NG+ fields
 */

/** \brief  Offset in the header of the PSIDv2NG flags (WORD)
 */
#define HVSC_PSID_FLAGS         0x76

/** \brief  Offset in the header of the PSIDv2NG start page (BYTE)
 */
#define HVSC_PSID_START_PAGE    0x78

/** \brief  Offset in the header of the PSIDv2NG page length (BYTE)
 */
#define HVSC_PSID_PAGE_LENGTH   0x79

/** \brief  Offset in the header of the PSIDv2NG second SID address (BYTE)
 */
#define HVSC_PSID_SECOND_SID    0x7a

/** \brief  Offset in the header of the PSIDv2NG third SID address (BYTE)
 */
#define HVSC_PSID_THIRD_SID     0x7b

#endif
