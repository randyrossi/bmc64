/* vim: set et ts=4 sw=4 sts=4 fdm=marker syntax=c.doxygen: */

/** \file   src/lib/base.h
 * \brief   Base library code - header
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

#ifndef HVSC_BASE_H
#define HVSC_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "hvsc_defs.h"

/** \brief  Debug message macro
 *
 * Prints a debug message on stdout if the macro `HVSC_DEBUG` is defined.
 * The function prints file, lineno and function name and then the message.
 * The message can be specified as a printf() call: a format string and a
 * variable list of arguments.
 *
 * An example:
 * \code{.c}
 *  // file foo.c
 *
 *  int bar = 42;
 *
 *  void print_bar(void)
 *  {
 *      hvsc_dbg("Got bar = %d\n", bar);
 *  }
 *
 *  print_bar();
 * \endcode
 *
 * This will ouput "[hvsc-debug] foo.c:7::print_bar(): Got bar = 42".
 */
#ifdef HVSC_DEBUG
# define hvsc_dbg(...) \
    printf("[hvsc-debug] %s:%d::%s(): ", __FILE__, __LINE__, __func__); \
    printf(__VA_ARGS__);
#else
# define hvsc_dbg(...)
#endif


extern char *hvsc_root_path;
extern char *hvsc_sldb_path;
extern char *hvsc_stil_path;
extern char *hvsc_bugs_path;

char *      hvsc_strdup(const char *s);
char *      hvsc_strndup(const char *s, size_t n);
char *      hvsc_paths_join(const char *p1, const char *p2);
long        hvsc_read_file(uint8_t **dest, const char *path);
int         hvsc_set_paths(const char *path);
void        hvsc_free_paths(void);
void        hvsc_text_file_init_handle(hvsc_text_file_t *handle);
int         hvsc_text_file_open(const char *path, hvsc_text_file_t *handle);
const char *hvsc_text_file_read(hvsc_text_file_t *handle);
void        hvsc_text_file_close(hvsc_text_file_t *handle);

char *      hvsc_path_strip_root(const char *path);
void        hvsc_path_fix_separators(char *path);
int         hvsc_string_is_empty(const char *s);
int         hvsc_string_is_comment(const char *s);
long        hvsc_parse_simple_timestamp(char *t, char **endptr);
int         hvsc_get_field_type(const char *s);
const char *hvsc_get_field_display(int type);

void        hvsc_get_word_be(uint16_t *dest, const uint8_t *src);
void        hvsc_get_word_le(uint16_t *dest, const uint8_t *src);
void        hvsc_get_longword_be(uint32_t *dest, const uint8_t *src);

#endif
