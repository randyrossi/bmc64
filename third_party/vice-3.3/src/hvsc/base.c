/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/base.c
 * \brief   Base library code
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

/** \brief  Size of chunks to read in hvsc_read_file()
 */
#define READFILE_BLOCK_SIZE  65536


/** \brief  Size of chunks to read int hvsc_text_file_read()
 */
#define READFILE_LINE_SIZE  1024


/** \brief  Error messages
 */
static const char *hvsc_err_messages[HVSC_ERR_CODE_COUNT] = {
    "OK",
    "out of memory error",
    "I/O error",
    "file too large error",
    "libgcrypt error",
    "malformed timestamp",
    "object not found",
    "invalid data or operation",
};


/** \brief  List of field indentifiers
 *
 * \see hvsc_stil_field_type_t
 */
static const char *field_identifiers[] = {
    " ARTIST:",
    " AUTHOR:",
    "    BUG:",     /* XXX: only used in BUGlist.txt */
    "COMMENT:",
    "   NAME:",
    "  TITLE:",
    NULL
};


/** \brief  List of field identifier display string for dumping
 *
 * This makes it more clear to distinguish parser errors (ie NAME: showing up
 * in a field text)
 */
static const char *field_displays[] = {
    " {artist}",
    " {author}",
    "    {bug}",     /* XXX: only used in BUGlist.txt */
    "{comment}",
    "   {name}",
    "  {title}",
    NULL
};


/** \brief  Error message to return for invalid error codes
 */
static const char *invalid_err_msg = "<unknown error code>";


/** \brief  Error code for the library
 */
int hvsc_errno;


/** \brief  Absolute path to the HVSC root directory
 */
char *hvsc_root_path;

/** \brief  Absolute path to the SLDB file
 */
char *hvsc_sldb_path;

/** \brief  Absolute path to the STIL file
 */
char *hvsc_stil_path;

/** \brief  Absolute path to the BUGlist file
 */
char *hvsc_bugs_path;


/** \brief  Get error message for errno \a n
 *
 * \param[in]   n   error code
 *
 * \return  error message
 */
const char *hvsc_strerror(int n)
{
    if (n < 0 || n >= HVSC_ERR_CODE_COUNT) {
        return invalid_err_msg;
    }
    return hvsc_err_messages[n];
}


/** \brief  Print error message on `stderr` optionally with a \a prefix
 *
 * Prints error code and message on `stderr`, and when an I/O error was
 * encountered, the C-library's errno and strerror() will also be printed.
 *
 * \param[in]   prefix  optional prefix
 */
void hvsc_perror(const char *prefix)
{
    /* display prefix? */
    if (prefix != NULL && *prefix != '\0') {
        fprintf(stderr, "%s: ", prefix);
    }

    switch (hvsc_errno) {

        case HVSC_ERR_IO:
            /* I/O error */
            fprintf(stderr, "%d: %s (%d: %s)\n",
                    hvsc_errno, hvsc_strerror(hvsc_errno),
                    errno, strerror(errno));
            break;

        default:
            fprintf(stderr, "%d: %s\n", hvsc_errno, hvsc_strerror(hvsc_errno));
            break;
    }
}


/** \brief  Initialize text file handle
 *
 * \param[in,out]   handle  text file handle
 */
void hvsc_text_file_init_handle(hvsc_text_file_t *handle)
{
    handle->fp = NULL;
    handle->path = NULL;
    handle->lineno = 0;
    handle->linelen = 0;
    handle->buffer = NULL;
    handle->buflen = 0;
}


/** \brief  Open text file \a path for reading
 *
 * \param[in]       path    path to file
 * \param[in,out]   handle  file handle, must be allocated by the caller
 *
 * \return  bool
 */
int hvsc_text_file_open(const char *path, hvsc_text_file_t *handle)
{
    printf("%s(): opening '%s'\n", __func__, path);
    hvsc_text_file_init_handle(handle);

    handle->fp = fopen(path, "rb");
    if (handle->fp == NULL) {
        hvsc_errno = HVSC_ERR_IO;
        return 0;
    }
    handle->path = hvsc_strdup(path);
    if (handle->path == NULL) {
        fclose(handle->fp);
        return 0;
    }

    handle->lineno = 0;

    handle->buffer = malloc(READFILE_LINE_SIZE);
    if (handle->buffer == NULL) {
        hvsc_errno = HVSC_ERR_OOM;
        free(handle->path);
        fclose(handle->fp);
        return 0;
    }
    handle->buflen = READFILE_LINE_SIZE;

    return 1;
}


/** \brief  Close text file via \a handle
 *
 * Cleans up memory used by the members of \a handle, but not \a handle itself
 *
 * \param[in,out]   handle  text file handle
 */
void hvsc_text_file_close(hvsc_text_file_t *handle)
{
    if (handle->path != NULL) {
        free(handle->path);
        handle->path = NULL;
    }
    if (handle->buffer != NULL) {
        free(handle->buffer);
        handle->buffer = NULL;
    }
    if (handle->fp != NULL) {
        fclose(handle->fp);
        handle->fp = NULL;
    }
}


/** \brief  Read a line from a text file
 *
 * \param[in,out]   handle  text file handle
 *
 * \return  pointer to current line or `NULL` on failure
 */
const char *hvsc_text_file_read(hvsc_text_file_t *handle)
{
    size_t i = 0;

    while (1) {
        int ch;

        /* resize buffer? */
        if (i == handle->buflen - 1) {
            /* resize buffer */
#ifdef HVSC_BEBUG
            printf("RESIZING BUFFER TO %lu, lineno %ld\n",
                    (unsigned long)(handle->buflen  * 2), handle->lineno);
#endif
            char *tmp = realloc(handle->buffer, handle->buflen * 2);
            if (tmp == NULL) {
                hvsc_errno = HVSC_ERR_OOM;
                return NULL;
            }
            handle->buffer = tmp;
            handle->buflen *= 2;
        }

        ch = fgetc(handle->fp);
        if (ch == EOF) {
            if (feof(handle->fp)) {
                /* OK, proper EOF */
                handle->buffer[i] = '\0';
                if (i == 0) {
                    return NULL;
                } else {
                    return handle->buffer;
                }
            } else {
                hvsc_errno = HVSC_ERR_IO;
                return NULL;
            }
        }

        if (ch == '\n') {
            /* Unix EOL, strip */
            handle->buffer[i] = '\0';
            /* Strip Windows CR */
            if (i > 0 && handle->buffer[i - 1] == '\r') {
                handle->buffer[--i] = '\0';
            }
            handle->lineno++;
            handle->linelen = i;
            return handle->buffer;
        }

        handle->buffer[i++] = (char)ch;
    }
    return handle->buffer;
}


/** @brief  Read data from \a path into \a dest, allocating memory
 *
 * This function reads data from \a path, (re)allocating memory as required.
 * The pointer to the result is stored in \a dest. If this function fails for
 * some reason (file not found, out of memory), -1 is returned and all memory
 * used by this function is freed.
 *
 * READFILE_BLOCK_SIZE bytes are read at a time, and whenever memory runs out,
 * it is doubled in size.
 *
 * @note:   Since this function returns `long`, it can only be used for files
 *          up to 2GB. Should be enough for C64 related files.
 *
 * Example:
 * @code{.c}
 *
 *  unsigned char *data;
 *  int result;
 *
 *  if ((result = hvsc_read_file(&data, "Commando.sid")) < 0) {
 *      fprintf(stderr, "oeps!\n");
 *  } else {
 *      printf("OK, read %ld bytes\n", result);
 *      free(data);
 *  }
 * @endcode
 *
 * @param   dest    destination of data
 * @param   path    path to file
 *
 * @return  number of bytes read, or -1 on failure
 */
long hvsc_read_file(uint8_t **dest, const char *path)
{
    uint8_t *data;
    uint8_t *tmp;
    FILE *fd;
    size_t offset = 0;
    size_t size = READFILE_BLOCK_SIZE;
    size_t result;

    fd = fopen(path, "rb");
    if (fd == NULL) {
        hvsc_errno = HVSC_ERR_IO;
        return -1;
    }

    data = malloc(READFILE_BLOCK_SIZE);
    if (data == NULL) {
        return -1;
    }

    /* keep reading chunks until EOF */
    while (1) {
        /* need to resize? */
        if (offset == size) {
            /* yup */

            /* check limit */
            if (size == (size_t)LONG_MAX + 1) {
                hvsc_errno = HVSC_ERR_FILE_TOO_LARGE;
                free(data);
                fclose(fd);
                return -1;
            }

            tmp = realloc(data, size * 2);
            if (tmp == NULL) {
                fclose(fd);
                free(data);
                return -1;
            }
            data = tmp;
            size *= 2;
        }
        result = fread(data + offset, 1, READFILE_BLOCK_SIZE, fd);
        if (result < READFILE_BLOCK_SIZE) {
            if (feof(fd)) {
                /* OK: EOF */
                /* try to realloc to minimum size required */
                tmp = realloc(data, offset + result);
                if (tmp != NULL) {
                    /* OK, no worries if it fails, the C standard guarantees
                     * the original data is still intact */
                    data = tmp;
                }
                *dest = data;
                fclose(fd);
                return (long)(offset + result);
            } else {
                /* IO error */
                hvsc_errno = HVSC_ERR_IO;
                free(data);
                *dest = NULL;
                fclose(fd);
                return -1;
            }
        }
        offset += READFILE_BLOCK_SIZE;
    }
    /* shouldn't get here */
}


/** \brief  Copy at most \a n chars of \a s
 *
 * This function appends a nul-byte after \a n bytes.
 *
 * \param[in]   s   string to copy
 * \param[in]   n   maximum number of chars to copy
 *
 * \return  heap-allocated, nul-terminated copy of \a n bytes of \a s, or
 *          `NULL` on failure
 */
char *hvsc_strndup(const char *s, size_t n)
{
    char *t = calloc(n + 1, 1);

    if (t == NULL) {
        hvsc_errno = HVSC_ERR_OOM;
        return NULL;
    }

    strncpy(t, s, n);
    return t;
}



/** \brief  Create heap-allocated copy of \a s
 *
 * \param[in]   s   string to copy
 *
 * \return  copy of \a s or `NULL` on error
 */
char *hvsc_strdup(const char *s)
{
    char *t;
    size_t len = strlen(s);

    t = malloc(len + 1);
    if (t == NULL) {
        hvsc_errno = HVSC_ERR_OOM;
        return NULL;
    }
    memcpy(t, s, len + 1);
    return t;
}


/** \brief  Join paths \a p1 and \a p2
 *
 * Concatenates \a p1 and \a p2, putting a path separator between them. \a p1
 * is expected to not contain a trailing separator and \a p2 is expected to
 * not start with a path separator.
 *
 * \param[in]   p1  first path
 * \param[in]   p2  second path
 *
 * \todo    Make more flexible (handle leading/trailing separators
 * \todo    Handle Windows/DOS paths
 *
 * \return  heap-allocated string
 */
char *hvsc_paths_join(const char *p1, const char *p2)
{
    char *result;
    size_t len1;
    size_t len2;

    if (p1 == NULL || p2 == NULL) {
        return NULL;
    }

    len1 = strlen(p1);
    len2 = strlen(p2);

    result = malloc(len1 + len2 + 2);   /* +2 for / and '\0' */
    if (result == NULL) {
        hvsc_errno = HVSC_ERR_OOM;
        return NULL;
    }

    memcpy(result, p1, len1);
#if defined(_WIN32) || defined(_WIN64)
    *(result + len1) = '\\';
#else
    *(result + len1) = '/';
#endif
    memcpy(result + len1 + 1, p2, len2 + 1);    /* add p2 including '\0' */

    return result;
}


/** \brief  Set the path to HVSC root, SLDB and STIL
 *
 * \param[in]   path    path to HVSC root directory
 *
 * \return  bool
 */
int hvsc_set_paths(const char *path)
{
    /* set HVSC root path */
    hvsc_root_path = hvsc_strdup(path);
    if (hvsc_root_path == NULL) {
        return 0;
    }

    /* set SLDB path */
    hvsc_sldb_path = hvsc_paths_join(hvsc_root_path, HVSC_SLDB_FILE);
    if (hvsc_sldb_path == NULL) {
        free(hvsc_root_path);
        hvsc_root_path = NULL;
        return 0;
    }

    /* set STIL path */
    hvsc_stil_path = hvsc_paths_join(hvsc_root_path, HVSC_STIL_FILE);
    if (hvsc_stil_path == NULL) {
        free(hvsc_root_path);
        free(hvsc_sldb_path);
        hvsc_root_path = NULL;
        hvsc_sldb_path = NULL;
        return 0;
    }

    /* set BUGlist path */
    hvsc_bugs_path = hvsc_paths_join(hvsc_root_path, HVSC_BUGS_FILE);
    if (hvsc_bugs_path == NULL) {
        free(hvsc_root_path);
        free(hvsc_sldb_path);
        free(hvsc_stil_path);
        hvsc_root_path = NULL;
        hvsc_sldb_path = NULL;
        hvsc_stil_path = NULL;
        return 0;
    }

    hvsc_dbg("HVSC root = %s\n", hvsc_root_path);
    hvsc_dbg("HVSC sldb = %s\n", hvsc_sldb_path);
    hvsc_dbg("HVSC stil = %s\n", hvsc_stil_path);
    hvsc_dbg("HVSC bugs = %s\n", hvsc_bugs_path);
    return 1;
}


/** \brief  Free memory used by the HSVC paths
 */
void hvsc_free_paths(void)
{
    if (hvsc_root_path != NULL) {
        free(hvsc_root_path);
        hvsc_root_path = NULL;
    }
    if (hvsc_sldb_path != NULL) {
        free(hvsc_sldb_path);
        hvsc_sldb_path = NULL;
    }
    if (hvsc_stil_path != NULL) {
        free(hvsc_stil_path);
        hvsc_stil_path = NULL;
    }
    if (hvsc_bugs_path != NULL) {
        free(hvsc_bugs_path);
        hvsc_bugs_path = NULL;
    }
}


/** \brief  Strip the HSVC root path from \a path
 *
 * \param[in]   path    path to a PSID file inside the HVSC
 *
 * \return  heap-allocated path with the HVSC root stripped, or a heap-allocated
 *          copy of \a path if the HVSC root wasn't present. Returns `NULL` on
 *          memory allocation failure.
 */
char *hvsc_path_strip_root(const char *path)
{
    size_t plen = strlen(path);             /* length of path */
    size_t rlen = strlen(hvsc_root_path);   /* length of HSVC root path */
    char *result;

    if (plen <= rlen) {
        return hvsc_strdup(path);
    }

    if (memcmp(path, hvsc_root_path, rlen) == 0) {
        /* got HSVC root path */
        result = malloc(plen - rlen + 1);
        if (result == NULL) {
            hvsc_errno = HVSC_ERR_OOM;
            return NULL;
        }
        memcpy(result, path + rlen, plen - rlen + 1);
        return result;
    }

    return hvsc_strdup(path);
}


/** \brief  Translate all backslashes into forward slashes
 *
 * Since entries in the SLDB, STIL and BUGlist are listed with forward slashes,
 * on Windows we'll need to fix the directory separators to allow strcmp() to
 * work.
 *
 * \param[in,out]   path    pathname to fix
 */
void hvsc_path_fix_separators(char *path)
{
    while (*path != '\0') {
        if (*path == '\\') {
            *path = '/';
        }
        path++;
    }
}


/** \brief  Check if \a s contains only whitespace
 *
 * \param[in]   s   string to check
 *
 * \return  bool
 */
int hvsc_string_is_empty(const char *s)
{
    while (*s != '\0' && isspace((int)*s)) {
        s++;
    }
    return *s == '\0';
}


/** \brief  Check if \a s is a comment
 *
 * Checks if the first non-whitespace token in \a s is a '#', indicating a
 * comment.
 *
 * \param[in]   s   string to check
 *
 * \return  bool
 */
int hvsc_string_is_comment(const char *s)
{
    /* ignore leading whitespace (not strictly required) */
    while (*s != '\0' && isspace((int)*s)) {
        s++;
    }
    return *s == '#';
}


/** \brief  Parse string \a p for a timestamp and return number of seconds
 *
 * Parse a timestamp in the format HH:MM, return number of seconds.
 *
 * \param[in]   t       timestamp
 * \param[out]  endptr  object to store pointer to first non-timestamp char
 *
 * \return  time in seconds or -1 on error
 */
long hvsc_parse_simple_timestamp(char *t, char **endptr)
{
    long m = 0;
    long s = 0;

    /* minutes */
    while (isdigit((int)*t)) {
        m = m * 10 + *t - '0';
        t++;
    }
    if (*t != ':') {
        /* error */
        *endptr = t;
        hvsc_errno = HVSC_ERR_TIMESTAMP;
        return -1;
    }

    /* seconds */
    t++;
    while (isdigit((int)*t)) {
        s = s * 10 + *t - '0';
        t++;
        if (s > 59) {
            hvsc_errno = HVSC_ERR_TIMESTAMP;
            return -1;
        }
    }

    /* done */
    *endptr = t;
    return m * 60 + s;
}



/** \brief  Determine is \a s hold a field identifier
 *
 * Checks against a list of know field identifiers.
 *
 * \param[in]   s   string to parse
 *
 * \return  field type or -1 (HVSC_FIELD_INVALID) when not found
 *
 * \note    returning -1 does not indicate failure, just that \a s doesn't
 *          contain a field indentifier (ie normal text for a comment or so)
 */
int hvsc_get_field_type(const char *s)
{
    int i = 0;

    while (field_identifiers[i] != NULL) {
        int result = strncmp(s, field_identifiers[i], 8);
        if (result == 0) {
            return i;   /* got it */
        }
        i++;
    }
    return HVSC_FIELD_INVALID;
}


/** \brief  Get display string for field \a type
 *
 * \param[in]   type    field type
 *
 * \return  string
 */
const char *hvsc_get_field_display(int type)
{
    if (type < 0 || type >= HVSC_FIELD_TYPE_COUNT) {
        return "<invalid>";
    }
    return field_displays[type];
}


/** \brief  Get a 16-bit big endian unsigned integer from \a src
 *
 * \param[out]  dest    object to store result
 * \param[in]   src     source data
 */
void hvsc_get_word_be(uint16_t *dest, const uint8_t *src)
{
    *dest = (uint16_t)((src[0] << 8) + src[1]);
}


/** \brief  Get a 16-bit little endian unsigned integer from \a src
 *
 * \param[out]  dest    object to store result
 * \param[in]   src     source data
 */
void hvsc_get_word_le(uint16_t *dest, const uint8_t *src)
{
    *dest = (uint16_t)((src[1] << 8) + src[0]);
}



/** \brief  Get a 32-bit big endian unsigned integer from \a src
 *
 * \param[out]  dest    object to store result
 * \param[in]   src     source data
 */
void hvsc_get_longword_be(uint32_t *dest, const uint8_t *src)
{
    *dest = (uint32_t)((src[0] << 24) + (src[1] << 16) + (src[2] << 8) + src[3]);
}
