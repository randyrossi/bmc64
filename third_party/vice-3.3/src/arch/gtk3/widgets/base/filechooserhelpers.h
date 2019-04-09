/** \file   filechooserhelpers.h
 * \brief   GtkFileChooser helper functions - header
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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
 */

#ifndef VICE_FILECHOOSERHELPERS_H
#define VICE_FILECHOOSERHELPERS_H

#include "vice.h"
#include <gtk/gtk.h>

/** \brief  Object to construct a GtkFileFilter with
 */
typedef struct ui_file_filter_s {
    const gchar *name;          /**< filter name (display in the dialog) */
    const gchar **patterns;     /**< NULL-terminated list of glob patterns */
} ui_file_filter_t;


/* these need extern, so now I'm wondering if this the correct approach
 * perhaps some 'getters' would be better, avoiding problems -- compyx */
extern const char *file_chooser_pattern_all[];
extern const char *file_chooser_pattern_cart[];
extern const char *file_chooser_pattern_disk[];
extern const char *file_chooser_pattern_tape[];
extern const char *file_chooser_pattern_sid[];
extern const char *file_chooser_pattern_fliplist[];
extern const char *file_chooser_pattern_program[];
extern const char *file_chooser_pattern_archive[];
extern const char *file_chooser_pattern_compressed[];
extern const char *file_chooser_pattern_snapshot[];

extern const ui_file_filter_t file_chooser_filter_all;
extern const ui_file_filter_t file_chooser_filter_cart;
extern const ui_file_filter_t file_chooser_filter_disk;
extern const ui_file_filter_t file_chooser_filter_tape;
extern const ui_file_filter_t file_chooser_filter_sid;
extern const ui_file_filter_t file_chooser_filter_fliplist;
extern const ui_file_filter_t file_chooser_filter_program;
extern const ui_file_filter_t file_chooser_filter_archive;
extern const ui_file_filter_t file_chooser_filter_compressed;
extern const ui_file_filter_t file_chooser_filter_snapshot;

GtkFileFilter *create_file_chooser_filter(const ui_file_filter_t filter,
                                          gboolean show_globs);

#endif
