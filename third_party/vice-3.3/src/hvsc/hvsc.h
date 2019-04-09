/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/hvsc.h
 * \brief   Library header
 *
 * This file only contains prototypes, types and defines which are supposed to
 * be public.
 *
 * XXX: currently exposes to much internal stuff, perhaps use some opaque
 *      types (ie void*)?
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

#ifndef HVSC_HVSC_H
#define HVSC_HVSC_H

/* for size_t, fixed width types and bool */
#include <stdlib.h>
#include <stdint.h>

/*
 * Microsoft's msvcrt does support C99's %z printf format specifier, so we
 * need to work around that:
 */
#ifdef _WIN32
# ifdef _WIN64
#  define PRI_SIZE_T    PRIu64
# else
#  define PRI_SIZE_T    PRIu32
# endif
#else
# define PRI_SIZE_T     "zu"
#endif



/** \brief  Error codes
 */
typedef enum hvsc_err_e {
    HVSC_ERR_OK = 0,            /**< no error */
    HVSC_ERR_OOM,               /**< out of memory error */
    HVSC_ERR_IO,                /**< I/O error */
    HVSC_ERR_FILE_TOO_LARGE,    /**< file too large (> 2GB) */
    HVSC_ERR_GCRYPT,            /**< error in gcrypt library */
    HVSC_ERR_TIMESTAMP,         /**< error parsing a timestamp */
    HVSC_ERR_NOT_FOUND,         /**< entry/tune not found */
    HVSC_ERR_INVALID,           /**< invalid data or operation detected */

    HVSC_ERR_CODE_COUNT         /**< number of error messages */

} hvsc_err_t;


/** \brief  STIL entry field type enumeration
 */
typedef enum hvsc_stil_field_type_e {
    HVSC_FIELD_INVALID = -1,    /**< unknow field type */
    HVSC_FIELD_ARTIST,          /**< Artist (in case of cover) */
    HVSC_FIELD_AUTHOR,          /**< Author (of SID/subtune) */
    HVSC_FIELD_BUG,             /**< Bug note (only in BUGlist.txt) */
    HVSC_FIELD_COMMENT,         /**< Comment */
    HVSC_FIELD_NAME,            /**< (sub)tune name */
    HVSC_FIELD_TITLE,           /**< title of the cover */

    HVSC_FIELD_TYPE_COUNT       /**< number of valid field types */
} hvsc_stil_field_type_t;


/** \brief  Handle for the text file reader functions
 */
typedef struct hvsc_text_file_s {
    FILE *  fp;     /**< file pointer */
    char *  path;   /**< copy of the path of the file (for error messages) */
    long    lineno; /**< line number in file */
    size_t  linelen;    /**< line length */
    char *  buffer; /**< buffer for line data, grows when required */
    size_t  buflen; /**< size of buffer, grows when needed */
} hvsc_text_file_t;


/** \brief  STIL timestamp object
 *
 * Set `to` to -1 to signal only `from` should be used. Set `from` to -1 to
 * signal the entire timestamp is unused. Both entries are in seconds.
 *
 * Examples: no timestamp would result in { -1, -1 }
 *           "(0:30)" would result in { 30, -1 }
 *           "(0:30-2:15)" would result in { 30, 135 }
 */
typedef struct hvsc_stil_timestamp_s {
    long from;  /**< 'from' timestamp, or only timestamp */
    long to;    /**< 'to' timestamp, optional */
} hvsc_stil_timestamp_t;


/** \brief  STIL field object
 */
typedef struct hvsc_stil_field_s {
    hvsc_stil_field_type_t      type;       /**< field type */
    char *                      text;       /**< field content */
    hvsc_stil_timestamp_t       timestamp;  /**< timestamp (optional) */
    char *                      album;      /**< cover info (optional) */
} hvsc_stil_field_t;


/** \brief  STIL block object
 */
typedef struct hvsc_stil_block_s {
    int                 tune;           /**< tune number
                                             (0 = global/only tune) */
    hvsc_stil_field_t **fields;         /**< list of STIL fields, in order of
                                             the text in the STIL.txt file */
    size_t              fields_max;     /**< size of the fields array */
    size_t              fields_used;    /**< used entries in the fields array */
} hvsc_stil_block_t;


/** \brief  Handle for the STIL functions
 */
typedef struct hvsc_stil_s {
    hvsc_text_file_t    stil;           /**< handle for the STIL.txt file */
    char *              psid_path;      /**< path to PSID file */
    char **             entry_buffer;   /**< content of the STIL entry */
    size_t              entry_bufmax;   /**< number of available entries in
                                             the entry_buffer */
    size_t              entry_bufused;  /**< number of used entries in the
                                             entry_buffer */
    char *              sid_comment;    /**< global comment (optional) */
    hvsc_stil_block_t **blocks;         /**< STIL blocks */
    size_t              blocks_max;     /**< number of available blocks */
    size_t              blocks_used;    /**< number of used blocks */
} hvsc_stil_t;


/** \brief  Handle for the BUGlist functions
 */
typedef struct hvsc_bugs_s {
    hvsc_text_file_t        bugs;       /**< handle for the BUGlist.txt file */
    char *                  psid_path;  /**< path to PSID file */
    char *                  text;       /**< text about the bug */
    char *                  user;       /**< person reporting the bug */
} hvsc_bugs_t;


/** \brief  STIL tune entry object
 */
typedef struct hvsc_stil_tune_entry_s {
    int                 tune;           /**< tune number in the SID (1-256) */
    hvsc_stil_field_t **fields;         /**< STIL fields array */
    size_t              field_count;    /**< number of fields in \a fields */
} hvsc_stil_tune_entry_t;



/*
 * psid.c public defines and types
 */

/** \brief  Length of the magic bytes in the header
 * \ingroup psid
 */
#define HVSC_PSID_MAGIC_LEN         0x04

/** \brief  Length of a text field in the header
 *
 * This is the size of the name, author, and copyright fields. These fields are
 * in ASCII (Latin-1) and are NOT nul-terminated.
 *
 * \ingroup psid
 */
#define HVSC_PSID_TEXT_LEN          0x20


/*
 * Bitmasks for the 'flags' word in a PSID header
 */

/** \brief  Bit 0: Compute!'s Sidplayer
 * \ingroup psid
 */
#define HVSC_PSID_FLAGS_MUS_PLAYER      0x0001

/** \brief  Bit 1: tune is PlaySID specific
 * \ingroup psid
 */
#define HVSC_PSID_FLAGS_PSID_SPECIFIC   0x0002

/** \brief  Bits 2-3: video standard (clock)
 *
 * 00 = unknown, 01 = PAL, 10 = NTSC, 11 = PAL and NTSC
 * \ingroup psid
 */
#define HVSC_PSID_FLAGS_CLOCK           0x000c

/** \brief  Bits 4-5: first SID model
 *
 * 00 = unkown, 01 = MOS6581, 10 = MOS8580, 11 = MOS6581 and MOS8580
 * \ingroup psid
 */
#define HVSC_PSID_FLAGS_SID_MODEL1      0x0030

/** \brief  Bits 6-7: second SID model
 *
 * 00 = unkown, 01 = MOS6581, 10 = MOS8580, 11 = MOS6581 and MOS8580
 * \ingroup psid
 */
#define HVSC_PSID_FLAGS_SID_MODEL2      0x00c0

/** \brief  Bits 8-9: third SID model
 *
 * 00 = unkown, 01 = MOS6581, 10 = MOS8580, 11 = MOS6581 and MOS8580
 * \ingroup psid
 */
#define HVSC_PSID_FLAGS_SID_MODEL3      0x0300

/*
 * bits 10-15 of the 'flags' word are reserved for future use
 */

/** \brief  PSID handle
 *
 * \see https://www.hvsc.c64.org/download/C64Music/DOCUMENTS/SID_file_format.txt
 * \ingroup psid
 */
typedef struct hvsc_psid_s {
    /*
     * information on the entire file
     */
    char *      path;   /**< path to psid file */
    uint8_t *   data;   /**< data of psid file */
    size_t      size;   /**< size of psid file */

    /*
     * header data
     */

    uint8_t     magic[HVSC_PSID_MAGIC_LEN]; /**< magic bytes */
    uint16_t    version;                    /**< version number */

    uint16_t    data_offset;                /**< offset to SID data */
    uint16_t    load_address;               /**< load address on C64 */
    uint16_t    init_address;               /**< init address on C64 */
    uint16_t    play_address;               /**< play address on C64 */

    uint16_t    songs;                      /**< number of songs */
    uint16_t    start_song;                 /**< starting song */
    uint32_t    speed;                      /**< song speed flags */

    char        name[HVSC_PSID_TEXT_LEN + 1];   /**< SID name */
    char        author[HVSC_PSID_TEXT_LEN + 1]; /**< SID author */
    char        copyright[HVSC_PSID_TEXT_LEN + 1]; /**< SID copyright */

    /*
     * PSIDv2NG+ only fields
     */

    uint16_t    flags;                      /**< PSID flags */
    uint8_t     start_page;                 /**< starting page of free memory                                                         not touched by the SID */
    uint8_t     page_length;                /**< number of free pages after
                                                 start page */
    uint16_t    second_sid;                 /**< second SID I/O address */
    uint16_t    third_sid;                  /**< third SID I/O adress */

} hvsc_psid_t;


/*
 * main.c stuff
 */

int         hvsc_init(const char *path);
void        hvsc_exit(void);
const char *hvsc_lib_version_str(void);
void        hvsc_lib_version_num(int *major, int *minor, int *revision);

/*
 * base.c stuff
 */


extern int hvsc_errno;

const char *hvsc_strerror(int n);
void        hvsc_perror(const char *prefix);


/*
 * sldb.c stuff
 */

#ifdef HVSC_USE_MD5
char *      hvsc_sldb_get_entry_md5(const char *psid);
#endif
char *      hvsc_sldb_get_entry_txt(const char *psid);
int         hvsc_sldb_get_lengths(const char *psid, long **lengths);


/*
 * stil.c stuff
 */

int         hvsc_stil_open(const char *psid, hvsc_stil_t *handle);
void        hvsc_stil_close(hvsc_stil_t *handle);
int         hvsc_stil_read_entry(hvsc_stil_t *handle);
void        hvsc_stil_dump_entry(hvsc_stil_t *handle);
int         hvsc_stil_parse_entry(hvsc_stil_t *handle);
void        hvsc_stil_dump(hvsc_stil_t *handle);

/* XXX: needs much better name
 *
 * This combines calls of stil_open(), stil_read_entry() and stil_parse_entry()
 * It's probably best to make those functions static and leave this one.
 * */
int         hvsc_stil_get(hvsc_stil_t *stil, const char *path);

int         hvsc_stil_get_tune_entry(const hvsc_stil_t *handle,
                                     hvsc_stil_tune_entry_t *entry,
                                     int tune);
void        hvsc_stil_dump_tune_entry(const hvsc_stil_tune_entry_t *entry);

/*
 * bugs.c stuff
 */

int         hvsc_bugs_open(const char *psid, hvsc_bugs_t *handle);
void        hvsc_bugs_close(hvsc_bugs_t *handle);


/*
 * psid.c stuff
 */

int             hvsc_psid_open(const char *path, hvsc_psid_t *handle);
void            hvsc_psid_close(hvsc_psid_t *handle);
void            hvsc_psid_dump(const hvsc_psid_t *handle);
int             hvsc_psid_write_bin(const hvsc_psid_t *handle, const char *path);
unsigned int    hvsc_psid_get_model_id(const hvsc_psid_t *handle, int sid);
const char *    hvsc_psid_get_model_str(const hvsc_psid_t *handle, int sid);
unsigned int    hvsc_psid_get_clock_id(const hvsc_psid_t *handle);
const char *    hvsc_psid_get_clock_str(const hvsc_psid_t *handle);

/*
 * Misc
 */

const char *    hvsc_get_field_display(int type);


#endif
