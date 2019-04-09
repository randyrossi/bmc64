/*
 * Copyright (C) 2002,2009,2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Red Hat Author(s): Behdad Esfahbod
 */

/* The interfaces in this file are subject to change at any time. */

#ifndef novte_ring_h_included
#define novte_ring_h_included

#include <gio/gio.h>
#include "novte.h"

#include "vterowdata.h"
#include "vtestream.h"

G_BEGIN_DECLS


typedef guint32 hyperlink_idx_t;

typedef struct _VteVisualPosition {
    long row, col;
} VteVisualPosition;

typedef struct _VteCellAttrChange {
    gsize text_end_offset;  /* offset of first character no longer using this attr */
        VteStreamCellAttr attr;
} VteCellAttrChange;


/*
 * VteRing: A scrollback buffer ring
 */

typedef struct _VteRing VteRing;
struct _VteRing {
    gulong max;

    gulong start, end;

    /* Writable */
    gulong writable, mask;
    VteRowData *array;

        /* Storage:
         *
         * row_stream contains records of VteRowRecord for each physical row.
         * (This stream is regenerated when the contents rewrap on resize.)
         *
         * text_stream is the text in UTF-8.
         *
         * attr_stream contains entries that consist of:
         *  - a VteCellAttrChange.
         *  - a string of attr.hyperlink_length length containing the (typically empty) hyperlink data.
         *    As far as the ring is concerned, this hyperlink data is opaque. Only the caller cares that
         *    if nonempty, it actually contains the ID and URI separated with a semicolon. Not NUL terminated.
         *  - 2 bytes repeating attr.hyperlink_length so that we can walk backwards.
         */
    VteStream *attr_stream, *text_stream, *row_stream;
    gsize last_attr_text_start_offset;
    VteCellAttr last_attr;
    GString *utf8_buffer;

    VteRowData cached_row;
    gulong cached_row_num;

    gboolean has_streams;
        gulong visible_rows;  /* to keep at least a screenful of lines in memory, bug 646098 comment 12 */

        GPtrArray *hyperlinks;  /* The hyperlink pool. Contains GString* items.
                                   [0] points to an empty GString, [1] to [VTE_HYPERLINK_COUNT_MAX] contain the id;uri pairs. */
        char hyperlink_buf[VTE_HYPERLINK_TOTAL_LENGTH_MAX + 1];  /* One more hyperlink buffer to get the value if it's not placed in the pool. */
        hyperlink_idx_t hyperlink_highest_used_idx;  /* 0 if no hyperlinks at all in the pool. */
        hyperlink_idx_t hyperlink_current_idx;  /* The hyperlink idx used for newly created cells.
                                                   Must not be GC'd even if doesn't occur onscreen. */
        hyperlink_idx_t hyperlink_hover_idx;  /* The hyperlink idx of the hovered cell.
                                                 An idx is allocated on hover even if the cell is scrolled out to the streams. */
        gulong hyperlink_maybe_gc_counter;  /* Do a GC when it reaches 65536. */
};

#define _vte_ring_contains(__ring, __position) \
    (((gulong) (__position) >= (__ring)->start) && \
     ((gulong) (__position) < (__ring)->end))
#define _vte_ring_delta(__ring) ((glong) (__ring)->start)
#define _vte_ring_length(__ring) ((glong) ((__ring)->end - (__ring)->start))
#define _vte_ring_next(__ring) ((glong) (__ring)->end)

const VteRowData *_vte_ring_index (VteRing *ring, gulong position);
VteRowData *_vte_ring_index_writable (VteRing *ring, gulong position);

void _vte_ring_init (VteRing *ring, gulong max_rows, gboolean has_streams);
void _vte_ring_fini (VteRing *ring);
void _vte_ring_hyperlink_maybe_gc (VteRing *ring, gulong increment);
hyperlink_idx_t _vte_ring_get_hyperlink_idx (VteRing *ring, const char *hyperlink);
hyperlink_idx_t _vte_ring_get_hyperlink_at_position (VteRing *ring, gulong position, int col, bool update_hover_idx, const char **hyperlink);
long _vte_ring_reset (VteRing *ring);
void _vte_ring_resize (VteRing *ring, gulong max_rows);
void _vte_ring_shrink (VteRing *ring, gulong max_len);
VteRowData *_vte_ring_insert (VteRing *ring, gulong position);
VteRowData *_vte_ring_append (VteRing *ring);
void _vte_ring_remove (VteRing *ring, gulong position);
void _vte_ring_drop_scrollback (VteRing *ring, gulong position);
void _vte_ring_set_visible_rows (VteRing *ring, gulong rows);
void _vte_ring_rewrap (VteRing *ring, glong columns, VteVisualPosition **markers);
gboolean _vte_ring_write_contents (VteRing *ring,
                   GOutputStream *stream,
                   VteWriteFlags flags,
                   GCancellable *cancellable,
                   GError **error);

G_END_DECLS

#endif
