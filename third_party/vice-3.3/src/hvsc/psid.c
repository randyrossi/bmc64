/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/psid.c
 * \brief   PSID/RSID file handling
 *
 *
 * Some functions to work with PSID/RSID files.
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
#include <string.h>
#include <inttypes.h>

#include "hvsc.h"
#include "base.h"

#include "psid.h"

/** \brief  Magic bytes to indicate a PSID file
 * \ingroup psid
 */
static const uint8_t psid_magic[HVSC_PSID_MAGIC_LEN] = { 0x50, 0x53, 0x49, 0x44 };

/** \brief  Magic bytes to indicate an RSID file
 * \ingroup psid
 */
static const uint8_t rsid_magic[HVSC_PSID_MAGIC_LEN] = { 0x50, 0x53, 0x49, 0x44 };


/** \brief  SID models
 *
 * Used to display bits 4-5, 6-7 and 8-9 of the 'flags' field
 *
 * \ingroup psid
 */
static const char *sid_models[4] = {
    "unknown",
    "6581",
    "8580",
    "6581 and 8580"
};


/** \brief  Clock speed strings
 *
 * Used to display bits 2-3 of the 'flags' field
 *
 * \ingroup psid
 */
static const char *sid_clocks[4] = {
    "unkown",
    "PAL",
    "NTSC",
    "PAL and NTSC"
};


/** \brief  Initialize \a handle for use
 *
 * \param[in,out]   handle  PSID handle
 *
 * \ingroup psid
 */
static void psid_handle_init(hvsc_psid_t *handle)
{
    handle->path = NULL;
    handle->data = NULL;
    handle->size = 0;
    memset(handle->magic, 0, HVSC_PSID_MAGIC_LEN);
    handle->version = 0;
    handle->data_offset = 0;
    handle->load_address = 0;
    handle->init_address = 0;
    handle->play_address = 0;
    handle->songs = 0;
    handle->start_song = 0;
    handle->speed = 0;
    memset(handle->name, 0, HVSC_PSID_TEXT_LEN);
    memset(handle->author, 0, HVSC_PSID_TEXT_LEN);
    memset(handle->copyright, 0, HVSC_PSID_TEXT_LEN);
    handle->flags = 0;
    handle->start_page = 0;
    handle->second_sid = 0;
    handle->third_sid = 0;
}


/** \brief  Check if \a header contains valid magic
 *
 * \param[in]   header  raw header data
 *
 * \return  bool
 * \ingroup psid
 */
static int psid_header_is_valid(const uint8_t *header)
{
    return (memcmp(header, psid_magic, HVSC_PSID_MAGIC_LEN) == 0)
        || (memcmp(header, rsid_magic, HVSC_PSID_MAGIC_LEN) == 0);
}


/** \brief  Copy a 32-byte non-terminated string for \a src into \a dest
 *
 * \param[out]  dest    destination of terminated C-style string
 * \param[in]   src     location of non-terminated string
 * \ingroup psid
 */
static void psid_copy_string(char *dest, uint8_t *src)
{
    memcpy(dest, src, HVSC_PSID_TEXT_LEN);
    dest[HVSC_PSID_TEXT_LEN] = '\0';
}


/** \brief  Determine if SID \a address byte is valid
 *
 * The \a adress value is the middle two nybbles of a SID I/O address, so bits
 * 4-11. For example $42 translates to $d420.
 *
 * \param[in]   address SID address byte
 *
 * \return  bool
 * \ingroup psid
 */
static int psid_sid_address_is_valid(uint8_t address)
{
    /* only even addresses are valid */
    if (address & 0x01) {
        return 0;
    }
    /* only $d420-$d7e0 or $de00-$dfe0 are valid, in steps of $20 */
    if (address < 0x42 || (address >= 0x80 && address <= 0xdf)) {
        return 0;
    }
    return 1;
}


/** \brief  Parse the PSID header
 *
 * \param[in,out]   handle  PSID handle
 * \ingroup psid
 */
static void psid_parse_header(hvsc_psid_t *handle)
{
    uint8_t sid_addr;

    /* magic */
    memcpy(handle->magic, handle->data + HVSC_PSID_MAGIC, HVSC_PSID_MAGIC_LEN);
    /* version */
    hvsc_get_word_be(&(handle->version),
            handle->data + HVSC_PSID_VERSION);
    /* data offset */
    hvsc_get_word_be(&(handle->data_offset),
            handle->data + HVSC_PSID_DATA_OFFSET);

    /* load address */
    hvsc_get_word_be(&(handle->load_address),
            handle->data + HVSC_PSID_LOAD_ADDRESS);
    /* init address */
    hvsc_get_word_be(&(handle->init_address),
            handle->data + HVSC_PSID_INIT_ADDRESS);
    /* play address */
    hvsc_get_word_be(&(handle->play_address),
            handle->data + HVSC_PSID_PLAY_ADDRESS);

    /* song count */
    hvsc_get_word_be(&(handle->songs), handle->data + HVSC_PSID_SONGS);
    /* starting song */
    hvsc_get_word_be(&(handle->start_song), handle->data + HVSC_PSID_START_SONG);
    /* speed flags */
    hvsc_get_longword_be(&(handle->speed), handle->data + HVSC_PSID_SPEED);

    /* name */
    psid_copy_string(handle->name, handle->data + HVSC_PSID_NAME);
    /* author */
    psid_copy_string(handle->author, handle->data + HVSC_PSID_AUTHOR);
    /* copyright */
    psid_copy_string(handle->copyright, handle->data + HVSC_PSID_COPYRIGHT);

    if (handle->version < 2) {
        return;
    }

    /*
     * PSID v2NG+ fields
     */

    /* flags */
    hvsc_get_word_be(&(handle->flags), handle->data + HVSC_PSID_FLAGS);
    /* start page */
    handle->start_page = handle->data[HVSC_PSID_START_PAGE];
    /* page length */
    handle->page_length = handle->data[HVSC_PSID_PAGE_LENGTH];

    /* second SID */
    if (handle->version >= 3) {
        sid_addr = handle->data[HVSC_PSID_SECOND_SID];
        handle->second_sid = psid_sid_address_is_valid(sid_addr)
            ? (uint16_t)(sid_addr * 16 + 0xd000) : 0;
    }

    /* third SID */
    if (handle->version >= 4) {
        sid_addr = handle->data[HVSC_PSID_THIRD_SID];
        handle->third_sid = psid_sid_address_is_valid(sid_addr)
            ? (uint16_t)(sid_addr * 16 + 0xd000) : 0;
    }
}


/** \brief  Open PSID file and parse its header
 *
 * \param[in]       path    path to PSID file
 * \param[in,out]   handle  PSID handle
 *
 * \return  bool
 * \ingroup psid
 */
int hvsc_psid_open(const char *path, hvsc_psid_t *handle)
{
    long size;
    uint8_t *data;

    psid_handle_init(handle);

    hvsc_dbg("Attempting to read %s .. ", path);

    size = hvsc_read_file(&data, path);
    /* check for errors */
    if (size < 0) {
#ifdef HVSC_DBG
        hvsc_perror("failed");
#endif
        return 0;
    }

    /* check size */
    if (size < HVSC_PSID_HEADER_MIN_SIZE) {
#ifdef HVSC_DBG
        hvsc_perror("failed");
#endif
        hvsc_errno = HVSC_ERR_INVALID;
        free(data);
        return 0;
    }
    hvsc_dbg("OK, got %ld bytes\n", size);

    /* check header */
    if (!psid_header_is_valid(data)) {
        hvsc_dbg("got invalid header magic\n");
        hvsc_errno = HVSC_ERR_INVALID;
        free(data);
        return 0;
    }

    /* set data and size */
    handle->data = data;
    handle->size = (size_t)size;
    /* copy path */
    handle->path = hvsc_strdup(path);
    if (handle->path == NULL) {
        free(handle->data);
        return 0;
    }

    psid_parse_header(handle);
    return 1;
}


/** \brief  Clean up memory used by \a handle, but not the handle itself
 *
 * \param[in,out]   handle
 * \ingroup psid
 */
void hvsc_psid_close(hvsc_psid_t *handle)
{
    if (handle->data != NULL) {
        free(handle->data);
    }
    if (handle->path != NULL) {
        free(handle->path);
    }
    psid_handle_init(handle);
}


/** \brief  Get SID model bits
 *
 * Get the SID model bits for \a sid for \a handle, left-shifted to bit 0-1
 *
 * \param[in]   handle  PSID model
 * \param[in]   sid     SID number (1-3)
 *
 * \return  model (%00-%11)
 * \ingroup psid
 */
unsigned int hvsc_psid_get_model_id(const hvsc_psid_t *handle, int sid)
{
    unsigned int flags = handle->flags;
    unsigned int model;

    switch (sid) {
        case 1:
            /* first/only SID */
            model = (flags & HVSC_PSID_FLAGS_SID_MODEL1) >> 4;
            break;
        case 2:
            /* second SID */
            model = (flags & HVSC_PSID_FLAGS_SID_MODEL2) >> 6;
            break;
        case 3:
            /* third SID */
            model = (flags & HVSC_PSID_FLAGS_SID_MODEL3) >> 8;
            break;
        default:
            model = 0;
    }
    return model;
}


/** \brief  Get SID model description string
 *
 * \param[in]   handle  PSID model
 * \param[in]   sid     SID number (1-3)
 *
 * \return  description
 * \ingroup psid
 */
const char *hvsc_psid_get_model_str(const hvsc_psid_t *handle, int sid)
{
    return sid_models[hvsc_psid_get_model_id(handle, sid)];
}


/** \brief  Get clock bits of the 'flags' field in \a handle
 *
 * \param[in]   handle  PSID handle
 *
 * \return  clocks bit (%00-%11)
 * \ingroup psid
 */
unsigned int hvsc_psid_get_clock_id(const hvsc_psid_t *handle)
{
    return (handle->flags & HVSC_PSID_FLAGS_CLOCK) >> 2;
}


/** \brief  Get clock description string
 *
 * \param[in]   handle  PSID handle
 *
 * \return  description
 * \ingroup psid
 */
const char *hvsc_psid_get_clock_str(const hvsc_psid_t *handle)
{
    return sid_clocks[hvsc_psid_get_clock_id(handle)];
}


/** \brief  Dump information on PSID on stdout
 *
 * Dumps file name, file size and the data in the header on stdout.
 *
 * \param[in]   handle  PSID handle
 * \ingroup psid
 */
void hvsc_psid_dump(const hvsc_psid_t *handle)
{
    char magic[HVSC_PSID_MAGIC_LEN + 1];

    uint16_t load;
    uint16_t end;

    memcpy(magic, handle->magic, HVSC_PSID_MAGIC_LEN);
    magic[HVSC_PSID_MAGIC_LEN] = '\0';

    /* get load and end addresses */
    if (handle->load_address == 0) {
        /* load address inside C64 binary */
        hvsc_get_word_le(&load, handle->data + handle->data_offset);
        end = (uint16_t)(handle->size - handle->data_offset - 2 - 1 + load);
    } else {
        load = handle->load_address;
        end = (uint16_t)(handle->size - handle->data_offset -1 + load);
    }

    /* dump header data on stdout */
    printf("file name       : %s\n", handle->path);
    printf("file size       : %" PRI_SIZE_T "\n", handle->size);
    printf("magic           : %s\n", magic);
    printf("version         : %d\n", (int)handle->version);
    printf("data offset     : $%04x\n", handle->data_offset);
    printf("load            : $%04x-$%04x\n", load, end);
    printf("init            : $%04x\n", handle->init_address);
    printf("play            : $%04x\n", handle->play_address);
    printf("songs           : %d (default %d)\n",
            handle->songs, handle->start_song);
    /* TODO: speed bits */
    printf("name            : %s\n", handle->name);
    printf("author          : %s\n", handle->author);
    printf("copyright       : %s\n", handle->copyright);

    if (handle->version < 2) {
        return;
    }

    /* PSID v2NG+ fields */

    /* flags */
    printf("clock           : %s\n", hvsc_psid_get_clock_str(handle));
    printf("SID model       : %s\n", hvsc_psid_get_model_str(handle, 1));

    /* the following fields are v3+ */
    if (handle->version >= 3) {
        if (handle->second_sid != 0) {
            printf("second SID      : $%04x\n", handle->second_sid);
            printf("second SID model: %s\n", hvsc_psid_get_model_str(handle, 2));
        } else {
            printf("second SID      : none\n");
        }
    }

    if (handle->version >= 4) {
        if (handle->third_sid != 0) {
            printf("third SID       : $%04x\n", handle->third_sid);
            printf("second SID model: %s\n", hvsc_psid_get_model_str(handle, 3));
        } else {
            printf("third SID       : none\n");
        }
    }

    /* page/driver info */
    printf("start page      : $%04x\n", handle->start_page * 256);
    printf("page length     : $%04x\n", handle->page_length * 256);
}


/** \brief  Extract SID binary from \a handle and write to \a path
 *
 * \param[in]   handle  PSID handle
 * \param[in]   path    path/filename to write data to
 *
 * \return  bool
 *
 * \ingroup psid
 */
int hvsc_psid_write_bin(const hvsc_psid_t *handle, const char *path)
{
    FILE *fp;
    size_t size;
    size_t result;

    fp = fopen(path, "wb");
    if (fp == NULL) {
        hvsc_errno = HVSC_ERR_IO;
        return 0;
    }

    /* do we need to write a 2-byte start address? */
    if (handle->load_address != 0) {
        /* write start address in little endian order */
        if (fputc(handle->load_address & 0xff, fp) == EOF) {
            hvsc_errno = HVSC_ERR_IO;
            fclose(fp);
            return 0;
        }
        if (fputc(handle->load_address >> 8, fp) == EOF) {
            hvsc_errno = HVSC_ERR_IO;
            fclose(fp);
            return 0;
        }
    }
    /* write binary data */
    size = handle->size - handle->data_offset;
    hvsc_dbg("writing %" PRI_SIZE_T " bytes\n", size);
    result = fwrite(handle->data + handle->data_offset, 1U, size, fp);
    hvsc_dbg("wrote %" PRI_SIZE_T " bytes\n", result);
    if (result != size) {
        hvsc_errno = HVSC_ERR_IO;
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}
