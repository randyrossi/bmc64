/*
 * Copyright (C) 2001-2004,2009,2010 Red Hat, Inc.
 * Copyright © 2008, 2009, 2010 Christian Persch
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
 */

#include <vice.h>

#define GLIB_DISABLE_DEPRECATION_WARNINGS /* FIXME */

#include <math.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>
#define howmany(x,y) (((x)+((y)-1))/(y))
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#include <glib.h>

#include "novte.h"
#include "vteinternal.hh"
#include "buffer.h"
#include "debug.h"
#include "vteconv.h"
#include "vtedraw.hh"
#include "ring.h"
#include "caps.hh"

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_SYS_SYSLIMITS_H
#include <sys/syslimits.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <pango/pango.h>
#include "iso2022.h"
#include "keymap.h"
#include "marshal.h"
#include "matcher.hh"
#include "vteaccess.h"
#include "vtegtk.hh"

#include <new> /* placement new */

#ifndef LINE_MAX    /* FIXME */
#define LINE_MAX 256
#endif

/* Some sanity checks */
/* FIXMEchpe: move this to there when splitting _vte_incoming_chunk into its own file */
static_assert(sizeof(struct _vte_incoming_chunk) <= VTE_INPUT_CHUNK_SIZE, "_vte_incoming_chunk too large");
static_assert(offsetof(struct _vte_incoming_chunk, data) == offsetof(struct _vte_incoming_chunk, dataminusone) + 1, "_vte_incoming_chunk layout wrong");


#ifndef HAVE_ROUND
static inline double round(double x) {
    if(x - floor(x) < 0.5) {
        return floor(x);
    } else {
        return ceil(x);
    }
}
#endif

#define WORD_CHAR_EXCEPTIONS_DEFAULT "-#%&+,./=?@\\_~\302\267"

#define I_(string) (g_intern_static_string(string))

static int _vte_unichar_width(gunichar c, int utf8_ambiguous_width);
static void stop_processing(VteTerminalPrivate *that);
static void add_process_timeout(VteTerminalPrivate *that);
static void add_update_timeout(VteTerminalPrivate *that);
static void remove_update_timeout(VteTerminalPrivate *that);

static gboolean process_timeout (gpointer data);
static gboolean update_timeout (gpointer data);
static cairo_region_t *vte_cairo_get_clip_region (cairo_t *cr);

/* these static variables are guarded by the GDK mutex */
static guint process_timeout_tag = 0;
static gboolean in_process_timeout;
static guint update_timeout_tag = 0;
static gboolean in_update_timeout;
static GList *g_active_terminals;

static int _vte_unichar_width(gunichar c, int utf8_ambiguous_width)
{
        if (G_LIKELY (c < 0x80)) {
                return 1;
        }
        if (G_UNLIKELY (g_unichar_iszerowidth (c))) {
                return 0;
        }
        if (G_UNLIKELY (g_unichar_iswide (c))) {
                return 2;
        }
        if (G_LIKELY (utf8_ambiguous_width == 1)) {
                return 1;
        }
        if (G_UNLIKELY (g_unichar_iswide_cjk (c))) {
                return 2;
        }
        return 1;
}

/* process incoming data without copying */
static struct _vte_incoming_chunk *free_chunks;
static struct _vte_incoming_chunk *get_chunk (void)
{
    struct _vte_incoming_chunk *chunk = NULL;
    if (free_chunks) {
        chunk = free_chunks;
        free_chunks = free_chunks->next;
    }
    if (chunk == NULL) {
        chunk = g_new (struct _vte_incoming_chunk, 1);
    }
    chunk->next = NULL;
    chunk->len = 0;
    return chunk;
}
static void release_chunk (struct _vte_incoming_chunk *chunk)
{
    chunk->next = free_chunks;
    chunk->len = free_chunks ? free_chunks->len + 1 : 0;
    free_chunks = chunk;
}
static void prune_chunks (guint len)
{
    struct _vte_incoming_chunk *chunk = NULL;
    if (len && free_chunks != NULL) {
        if (free_chunks->len > len) {
            struct _vte_incoming_chunk *last;
            chunk = free_chunks;
            while (free_chunks->len > len) {
                last = free_chunks;
                free_chunks = free_chunks->next;
            }
            last->next = NULL;
        }
    } else {
        chunk = free_chunks;
        free_chunks = NULL;
    }
    while (chunk != NULL) {
        struct _vte_incoming_chunk *next = chunk->next;
        g_free (chunk);
        chunk = next;
    }
}
static void _vte_incoming_chunks_release (struct _vte_incoming_chunk *chunk)
{
    while (chunk) {
        struct _vte_incoming_chunk *next = chunk->next;
        release_chunk (chunk);
        chunk = next;
    }
}
static gsize _vte_incoming_chunks_length (struct _vte_incoming_chunk *chunk)
{
    gsize len = 0;
    while (chunk) {
        len += chunk->len;
        chunk = chunk->next;
    }
    return len;
}
#if 0
static gsize _vte_incoming_chunks_count (struct _vte_incoming_chunk *chunk)
{
    gsize cnt = 0;
    while (chunk) {
        cnt ++;
        chunk = chunk->next;
    }
    return cnt;
}
#endif
static struct _vte_incoming_chunk *_vte_incoming_chunks_reverse(struct _vte_incoming_chunk *chunk)
{
    struct _vte_incoming_chunk *prev = NULL;
    while (chunk) {
        struct _vte_incoming_chunk *next = chunk->next;
        chunk->next = prev;
        prev = chunk;
        chunk = next;
    }
    return prev;
}

static void vte_g_array_fill(GArray *array, gconstpointer item, guint final_size)
{
    if (array->len >= final_size) {
        return;
    }

    final_size -= array->len;
    do {
        g_array_append_vals(array, item, 1);
    } while (--final_size);
}

/* FIXMEchpe replace this with a method on VteRing */
VteRowData* VteTerminalPrivate::ring_insert(vte::grid::row_t position, bool fill)
{
    VteRowData *row;
    VteRing *ring = m_screen->row_data;
    bool const not_default_bg = (m_fill_defaults.attr.back() != VTE_DEFAULT_BG);

    while (G_UNLIKELY (_vte_ring_next (ring) < position)) {
        row = _vte_ring_append (ring);
        if (not_default_bg) {
            _vte_row_data_fill (row, &m_fill_defaults, m_column_count);
        }
    }
    row = _vte_ring_insert (ring, position);
    if (fill && not_default_bg) {
        _vte_row_data_fill (row, &m_fill_defaults, m_column_count);
    }
    return row;
}

// FIXMEchpe replace this with a method on VteRing
VteRowData* VteTerminalPrivate::ring_append(bool fill)
{
    return ring_insert(_vte_ring_next(m_screen->row_data), fill);
}

// FIXMEchpe replace this with a method on VteRing
void VteTerminalPrivate::ring_remove(vte::grid::row_t position)
{
    _vte_ring_remove(m_screen->row_data, position);
}

/* Reset defaults for character insertion. */
void
VteTerminalPrivate::reset_default_attributes(bool reset_hyperlink)
{
    hyperlink_idx_t hyperlink_idx_save = m_defaults.attr.hyperlink_idx;
    m_defaults = m_color_defaults = m_fill_defaults = basic_cell;
    if (!reset_hyperlink) {
            m_defaults.attr.hyperlink_idx = hyperlink_idx_save;
    }
}

/* FIXMEchpe this function is bad */
inline vte::view::coord_t VteTerminalPrivate::scroll_delta_pixel() const
{
        return round(m_screen->scroll_delta * m_cell_height);
}

/*
 * VteTerminalPrivate::pixel_to_row:
 * @y: Y coordinate is relative to viewport, top padding excluded
 *
 * Returns: absolute row
 */
inline vte::grid::row_t VteTerminalPrivate::pixel_to_row(vte::view::coord_t y) const
{
        return (scroll_delta_pixel() + y) / m_cell_height;
}

/*
 * VteTerminalPrivate::pixel_to_row:
 * @row: absolute row
 *
 * Returns: Y coordinate relative to viewport with top padding excluded. If the row is
 *   outside the viewport, may return any value < 0 or >= height
 */
inline vte::view::coord_t VteTerminalPrivate::row_to_pixel(vte::grid::row_t row) const
{
        /* FIXMEchpe this is bad! */
        return row * m_cell_height - (glong)round(m_screen->scroll_delta * m_cell_height);
}

inline vte::grid::row_t VteTerminalPrivate::first_displayed_row() const
{
        return pixel_to_row(0);
}

inline vte::grid::row_t VteTerminalPrivate::last_displayed_row() const
{
        /* Get the logical row number displayed at the bottom pixel position */
        auto r = pixel_to_row(m_view_usable_extents.height() - 1);

        /* If we have an extra padding at the bottom which is currently unused,
         * this number is one too big. Adjust here.
         * E.g. have a terminal of size 80 x 24.5.
         * Initially the bottom displayed row is (0-based) 23, but r is now 24.
         * After producing more than a screenful of content and scrolling back
         * all the way to the top, the bottom displayed row is (0-based) 24. */
        r = MIN (r, m_screen->insert_delta + m_row_count - 1);
        return r;
}

void VteTerminalPrivate::invalidate_cells(vte::grid::column_t column_start,
                                            int n_columns,
                                            vte::grid::row_t row_start,
                                            int n_rows)
{
    if (G_UNLIKELY (!widget_realized())) {
                return;
    }

    /* FIXMEchpe: == 0 is fine, but somehow sometimes we
    * get an actual negative n_columns value passed!?
    */
    if (n_columns <= 0 || n_rows <= 0) {
        return;
    }

    if (m_invalidated_all) {
        return;
    }

    _vte_debug_print (VTE_DEBUG_UPDATES, "Invalidating cells at (%ld,%ld)x(%d,%d).\n",
                        column_start, row_start, n_columns, n_rows);
    _vte_debug_print (VTE_DEBUG_WORK, "?");

    if (n_columns == m_column_count && n_rows == m_row_count) {
        invalidate_all();
        return;
    }

    cairo_rectangle_int_t rect;
    /* Convert the column and row start and end to pixel values
     * by multiplying by the size of a character cell.
     * Always include the extra pixel border and overlap pixel.
     */
    rect.x = column_start * m_cell_width - 1;
    /* The extra + 1 is for the faux-bold overdraw */
    int xend = (column_start + n_columns) * m_cell_width + 1 + 1;
    rect.width = xend - rect.x;

    rect.y = row_to_pixel(row_start) - 1;
    int yend = row_to_pixel(row_start + n_rows) + 1;
    rect.height = yend - rect.y;

    _vte_debug_print (VTE_DEBUG_UPDATES, "Invalidating pixels at (%d,%d)x(%d,%d).\n",
                        rect.x, rect.y, rect.width, rect.height);

    if (m_active_terminals_link != nullptr) {
        g_array_append_val(m_update_rects, rect);
        /* Wait a bit before doing any invalidation, just in
         * case updates are coming in really soon. */
        add_update_timeout(this);
    } else {
        auto allocation = get_allocated_rect();
        rect.x += allocation.x + m_padding.left;
        rect.y += allocation.y + m_padding.top;
        cairo_region_t *region = cairo_region_create_rectangle(&rect);
        gtk_widget_queue_draw_region(m_widget, region);
        cairo_region_destroy(region);
    }

    _vte_debug_print (VTE_DEBUG_WORK, "!");
}

void VteTerminalPrivate::invalidate_region(vte::grid::column_t scolumn,
                                            vte::grid::column_t ecolumn,
                                            vte::grid::row_t srow,
                                            vte::grid::row_t erow,
                                            bool block)
{
    if (block || srow == erow) {
        invalidate_cells(scolumn, ecolumn - scolumn + 1, srow, erow - srow + 1);
    } else {
        invalidate_cells(scolumn, m_column_count - scolumn, srow, 1);
        invalidate_cells(0, m_column_count, srow + 1, erow - srow - 1);
        invalidate_cells(0, ecolumn + 1, erow, 1);
    }
}

void VteTerminalPrivate::invalidate(vte::grid::span const& s, bool block)
{
    invalidate_region(s.start_column(), s.end_column(), s.start_row(), s.end_row(), block);
}

void VteTerminalPrivate::invalidate_all()
{
    if (G_UNLIKELY (!widget_realized())) {
                return;
    }

    if (m_invalidated_all) {
        return;
    }

    _vte_debug_print (VTE_DEBUG_WORK, "*");
    _vte_debug_print (VTE_DEBUG_UPDATES, "Invalidating all.\n");

    /* replace invalid regions with one covering the whole terminal */
    reset_update_rects();
    m_invalidated_all = TRUE;

    if (m_active_terminals_link != nullptr) {
        auto allocation = get_allocated_rect();
        cairo_rectangle_int_t rect;
        rect.x = -m_padding.left;
        rect.y = -m_padding.top;
        rect.width = allocation.width;
        rect.height = allocation.height;

        g_array_append_val(m_update_rects, rect);
        /* Wait a bit before doing any invalidation, just in
         * case updates are coming in really soon. */
        add_update_timeout(this);
    } else {
        gtk_widget_queue_draw(m_widget);
    }
}

/* FIXMEchpe: remove this obsolete function. It became useless long ago
 * when we stopped moving window contents around on scrolling. */
/* Scroll a rectangular region up or down by a fixed number of lines,
 * negative = up, positive = down. */
void VteTerminalPrivate::scroll_region (long row, long count, long delta)
{
    if ((delta == 0) || (count == 0)) {
        /* Shenanigans! */
        return;
    }

    if (count >= m_row_count) {
        /* We have to repaint the entire window. */
        invalidate_all();
    } else {
        /* We have to repaint the area which is to be
         * scrolled. */
        invalidate_cells(
                     0, m_column_count,
                     row, count);
    }
}

/* Find the row in the given position in the backscroll buffer. */
/* FIXMEchpe replace this with a method on VteRing */
VteRowData const* VteTerminalPrivate::find_row_data(vte::grid::row_t row) const
{
    VteRowData const* rowdata = nullptr;

    if (G_LIKELY(_vte_ring_contains(m_screen->row_data, row))) {
        rowdata = _vte_ring_index(m_screen->row_data, row);
    }
    return rowdata;
}

/* Find the row in the given position in the backscroll buffer. */
/* FIXMEchpe replace this with a method on VteRing */
VteRowData* VteTerminalPrivate::find_row_data_writable(vte::grid::row_t row) const
{
    VteRowData *rowdata = nullptr;

    if (G_LIKELY (_vte_ring_contains(m_screen->row_data, row))) {
        rowdata = _vte_ring_index_writable(m_screen->row_data, row);
    }
    return rowdata;
}

/* Find the character an the given position in the backscroll buffer. */
/* FIXMEchpe replace this with a method on VteRing */
VteCell const* VteTerminalPrivate::find_charcell(vte::grid::column_t col,
                                  vte::grid::row_t row) const
{
    VteRowData const* rowdata;
    VteCell const* ret = nullptr;

    if (_vte_ring_contains(m_screen->row_data, row)) {
        rowdata = _vte_ring_index(m_screen->row_data, row);
        ret = _vte_row_data_get (rowdata, col);
    }
    return ret;
}

/* FIXMEchpe replace this with a method on VteRing */
vte::grid::column_t VteTerminalPrivate::find_start_column(vte::grid::column_t col,
                                                            vte::grid::row_t row) const
{
    VteRowData const* row_data = find_row_data(row);
    if (G_UNLIKELY (col < 0)) {
        return col;
    }
    if (row_data != nullptr) {
        const VteCell *cell = _vte_row_data_get (row_data, col);
        while (col > 0 && cell != NULL && cell->attr.fragment()) {
            cell = _vte_row_data_get (row_data, --col);
        }
    }
    return MAX(col, 0);
}

/* FIXMEchpe replace this with a method on VteRing */
vte::grid::column_t VteTerminalPrivate::find_end_column(vte::grid::column_t col,
                                                        vte::grid::row_t row) const
{
    VteRowData const* row_data = find_row_data(row);
    gint columns = 0;
    if (G_UNLIKELY (col < 0)) {
        return col;
    }
    if (row_data != NULL) {
        const VteCell *cell = _vte_row_data_get (row_data, col);
        while (col > 0 && cell != NULL && cell->attr.fragment()) {
            cell = _vte_row_data_get (row_data, --col);
        }
        if (cell) {
            columns = cell->attr.columns() - 1;
        }
    }
    // FIXMEchp m__column_count - 1 ?
    return MIN(col + columns, m_column_count);
}

/* Determine the width of the portion of the preedit string which lies
 * to the left of the cursor, or the entire string, in columns. */
/* FIXMEchpe this is for the view, so use int not gssize */
/* FIXMEchpe this is only ever called with left_only=false, so remove the param */
gssize VteTerminalPrivate::get_preedit_width(bool left_only)
{
    gssize ret = 0;

    if (m_im_preedit != NULL) {
        char const *preedit = m_im_preedit;
        for (int i = 0;
             /* FIXMEchpe preddit is != NULL at the start, and next_char never returns NULL either */
             (preedit != NULL) &&
             (preedit[0] != '\0') &&
             (!left_only || (i < m_im_preedit_cursor));
             i++) {
            gunichar c = g_utf8_get_char(preedit);
            ret += _vte_unichar_width(c, m_utf8_ambiguous_width);
            preedit = g_utf8_next_char(preedit);
        }
    }

    return ret;
}

/* Determine the length of the portion of the preedit string which lies
 * to the left of the cursor, or the entire string, in gunichars. */
/* FIXMEchpe this returns gssize but inside it uses int... */
gssize VteTerminalPrivate::get_preedit_length(bool left_only)
{
    int i = 0;

    if (m_im_preedit != NULL) {
        char const *preedit = m_im_preedit;
        for (i = 0;
             /* FIXMEchpe useless check, see above */
             (preedit != NULL) &&
             (preedit[0] != '\0') &&
             (!left_only || (i < m_im_preedit_cursor));
             i++) {
            preedit = g_utf8_next_char(preedit);
        }
    }

    return i;
}

void VteTerminalPrivate::invalidate_cell(vte::grid::column_t col, vte::grid::row_t row)
{
    int columns;
    guint style;

    if (G_UNLIKELY (!widget_realized())) {
        return;
    }

    if (m_invalidated_all) {
        return;
    }

    columns = 1;
    auto row_data = find_row_data(row);
    if (row_data != NULL) {
        const VteCell *cell;
        cell = _vte_row_data_get (row_data, col);
        if (cell != NULL) {
            while (cell->attr.fragment() && col> 0) {
                cell = _vte_row_data_get (row_data, --col);
            }
            columns = cell->attr.columns();
            style = _vte_draw_get_style(cell->attr.bold(), cell->attr.italic());
            if (cell->c != 0) {
                int right;
                _vte_draw_get_char_edges(m_draw, cell->c, columns, style, NULL, &right);
                columns = MAX(columns, howmany(right, m_cell_width));
            }
        }
    }

    _vte_debug_print(VTE_DEBUG_UPDATES, "Invalidating cell at (%ld,%ld-%ld).\n",
                        row, col, col + columns);

    invalidate_cells(col, columns, row, 1);
}

void VteTerminalPrivate::invalidate_cursor_once(bool periodic)
{
    if (G_UNLIKELY(!widget_realized())) {
        return;
    }

    if (m_invalidated_all) {
        return;
    }

    if (periodic) {
        if (!m_cursor_blinks) {
            return;
        }
    }

    if (m_cursor_visible) {
        auto preedit_width = get_preedit_width(false);
        auto row = m_screen->cursor.row;
        auto column = m_screen->cursor.col;
        long columns = 1;
        column = find_start_column(column, row);

        auto cell = find_charcell(column, row);
        if (cell != NULL) {
            columns = cell->attr.columns();
            auto style = _vte_draw_get_style(cell->attr.bold(), cell->attr.italic());
            if (cell->c != 0) {
                int right;
                _vte_draw_get_char_edges(m_draw, cell->c, columns, style, NULL, &right);
                columns = MAX(columns, howmany(right, m_cell_width));
            }
        }
        columns = MAX(columns, preedit_width);
        if (column + columns > m_column_count) {
            column = MAX(0, m_column_count - columns);
        }

        _vte_debug_print(VTE_DEBUG_UPDATES, "Invalidating cursor at (%ld,%ld-%ld).\n",
                                                row, column, column + columns);
        invalidate_cells(column, columns, row, 1);
    }
}

/* Invalidate the cursor repeatedly. */
/* FIXMEchpe this continually adds and removes the blink timeout. Find a better solution */
static gboolean invalidate_cursor_periodic_cb(VteTerminalPrivate *that)
{
    that->invalidate_cursor_periodic();
    return G_SOURCE_REMOVE;
}

void VteTerminalPrivate::invalidate_cursor_periodic()
{
    m_cursor_blink_state = !m_cursor_blink_state;
    m_cursor_blink_time += m_cursor_blink_cycle;

    m_cursor_blink_tag = 0;
    invalidate_cursor_once(true);

    /* only disable the blink if the cursor is currently shown.
     * else, wait until next time.
     */
    if (m_cursor_blink_time / 1000 >= m_cursor_blink_timeout && m_cursor_blink_state) {
        return;
    }

    m_cursor_blink_tag = g_timeout_add_full(G_PRIORITY_LOW,
                                                m_cursor_blink_cycle,
                                                (GSourceFunc)invalidate_cursor_periodic_cb,
                                                this,
                                                NULL);
}

/* Emit a "selection_changed" signal. */
void VteTerminalPrivate::emit_selection_changed()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `selection-changed'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_SELECTION_CHANGED], 0);
}

/* Emit a "commit" signal. */
void VteTerminalPrivate::emit_commit(char const* text, gssize length)
{
    char const* result = NULL;
    char *wrapped = NULL;

    _vte_debug_print(VTE_DEBUG_SIGNALS,
            "Emitting `commit' of %" G_GSSIZE_FORMAT" bytes.\n", length);

    if (length == -1) {
        length = strlen(text);
        result = text;
    } else {
        /* FIXMEchpe why use the slice allocator here? */
        result = wrapped = (char *) g_slice_alloc(length + 1);
        memcpy(wrapped, text, length);
        wrapped[length] = '\0';
    }

    g_signal_emit(m_terminal, signals[SIGNAL_COMMIT], 0, result, (guint)length);

    if(wrapped) {
        g_slice_free1(length+1, wrapped);
    }
}

void VteTerminalPrivate::queue_contents_changed()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Queueing `contents-changed'.\n");
    m_contents_changed_pending = true;
}

/* FIXMEchpe this has only one caller */
void VteTerminalPrivate::queue_cursor_moved()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Queueing `cursor-moved'.\n");
    m_cursor_moved_pending = true;
}

static gboolean emit_eof_idle_cb(VteTerminal *terminal)
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_enter ();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    _vte_terminal_get_impl(terminal)->emit_eof();

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_leave ();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    return G_SOURCE_REMOVE;
}

void VteTerminalPrivate::emit_eof()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `eof'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_EOF], 0);
}

/* Emit a "eof" signal. */
/* FIXMEchpe any particular reason not to handle this immediately? */
void VteTerminalPrivate::queue_eof()
{
        _vte_debug_print(VTE_DEBUG_SIGNALS, "Queueing `eof'.\n");
        g_idle_add_full(G_PRIORITY_HIGH,
                        (GSourceFunc)emit_eof_idle_cb,
                        g_object_ref(m_terminal),
                        g_object_unref);
}

/* Emit a "char-size-changed" signal. */
void VteTerminalPrivate::emit_char_size_changed(int width, int height)
{
    _vte_debug_print(VTE_DEBUG_SIGNALS,
            "Emitting `char-size-changed'.\n");
    /* FIXME on next API break, change the signature */
    g_signal_emit(m_terminal, signals[SIGNAL_CHAR_SIZE_CHANGED], 0,
                    (guint)width, (guint)height);
}

/* Emit an "increase-font-size" signal. */
void VteTerminalPrivate::emit_increase_font_size()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `increase-font-size'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_INCREASE_FONT_SIZE], 0);
}

/* Emit a "decrease-font-size" signal. */
void VteTerminalPrivate::emit_decrease_font_size()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `decrease-font-size'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_DECREASE_FONT_SIZE], 0);
}

/* Emit a "text-inserted" signal. */
void VteTerminalPrivate::emit_text_inserted()
{
    if (!m_accessible_emit) {
        return;
    }
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `text-inserted'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_TEXT_INSERTED], 0);
}

/* Emit a "text-deleted" signal. */
void VteTerminalPrivate::emit_text_deleted()
{
    if (!m_accessible_emit) {
        return;
    }
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `text-deleted'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_TEXT_DELETED], 0);
}

/* Emit a "text-modified" signal. */
void VteTerminalPrivate::emit_text_modified()
{
    if (!m_accessible_emit) {
        return;
    }
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `text-modified'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_TEXT_MODIFIED], 0);
}

/* Emit a "text-scrolled" signal. */
void VteTerminalPrivate::emit_text_scrolled(long delta)
{
    if (!m_accessible_emit) {
        return;
    }
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `text-scrolled'(%ld).\n", delta);
    /* FIXMEchpe fix signal signature? */
    g_signal_emit(m_terminal, signals[SIGNAL_TEXT_SCROLLED], 0, (int)delta);
}

void VteTerminalPrivate::emit_copy_clipboard()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting 'copy-clipboard'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_COPY_CLIPBOARD], 0);
}

void VteTerminalPrivate::emit_paste_clipboard()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting 'paste-clipboard'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_PASTE_CLIPBOARD], 0);
}

/* Emit a "hyperlink_hover_uri_changed" signal. */
void VteTerminalPrivate::emit_hyperlink_hover_uri_changed(const GdkRectangle *bbox)
{
        GObject *object = G_OBJECT(m_terminal);

        _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `hyperlink-hover-uri-changed'.\n");
        g_signal_emit(m_terminal, signals[SIGNAL_HYPERLINK_HOVER_URI_CHANGED], 0, m_hyperlink_hover_uri, bbox);
        g_object_notify_by_pspec(object, pspecs[PROP_HYPERLINK_HOVER_URI]);
}

void VteTerminalPrivate::deselect_all()
{
    if (m_has_selection) {
        gint sx, sy, ex, ey, extra;

        _vte_debug_print(VTE_DEBUG_SELECTION, "Deselecting all text.\n");

        m_has_selection = FALSE;
        /* Don't free the current selection, as we need to keep
         * hold of it for async copying from the clipboard. */

        emit_selection_changed();

        sx = m_selection_start.col;
        sy = m_selection_start.row;
        ex = m_selection_end.col;
        ey = m_selection_end.row;
        extra = m_selection_block_mode ? (VTE_TAB_WIDTH_MAX - 1) : 0;
        invalidate_region(MIN (sx, ex), MAX (sx, ex) + extra,
                            MIN (sy, ey),   MAX (sy, ey),
                            false);
    }
}

/* FIXMEchpe make m_tabstops a hashset */

/* Remove a tabstop. */
void VteTerminalPrivate::clear_tabstop(int column)
{
    if (m_tabstops) {
        /* Remove a tab stop from the hash table. */
        g_hash_table_remove(m_tabstops, GINT_TO_POINTER(2 * column + 1));
    }
}

/* Check if we have a tabstop at a given position. */
bool VteTerminalPrivate::get_tabstop(int column)
{
    if (m_tabstops != NULL) {
        auto hash = g_hash_table_lookup(m_tabstops, GINT_TO_POINTER(2 * column + 1));
        return hash != nullptr;
    }

    return false;
}

/* Reset the set of tab stops to the default. */
void VteTerminalPrivate::set_tabstop(int column)
{
    if (m_tabstops != NULL) {
        /* Just set a non-NULL pointer for this column number. */
        g_hash_table_insert(m_tabstops, GINT_TO_POINTER(2 * column + 1), m_terminal);
    }
}

/* Reset the set of tab stops to the default. */
void VteTerminalPrivate::set_default_tabstops()
{
    if (m_tabstops) {
        g_hash_table_destroy(m_tabstops);
    }
    m_tabstops = g_hash_table_new(nullptr, nullptr);
    for (int i = 0; i <= VTE_TAB_MAX; i += VTE_TAB_WIDTH) {
        set_tabstop(i);
    }
}

/* Clear the cache of the screen contents we keep. */
void VteTerminalPrivate::match_contents_clear()
{
    match_hilite_clear();
    if (m_match_contents != nullptr) {
        g_free(m_match_contents);
        m_match_contents = nullptr;
    }
    if (m_match_attributes != nullptr) {
        g_array_free(m_match_attributes, TRUE);
        m_match_attributes = nullptr;
    }
}

void VteTerminalPrivate::match_contents_refresh()
{
    match_contents_clear();
    GArray *array = g_array_new(FALSE, TRUE, sizeof(struct _VteCharAttributes));
    auto match_contents = get_text_displayed(true /* wrap */,
                                                false /* include trailing whitespace */,
                                                array);
    m_match_contents = g_string_free(match_contents, FALSE);
    m_match_attributes = array;
}

GdkCursor *VteTerminalPrivate::widget_cursor_new(GdkCursorType cursor_type) const
{
    return gdk_cursor_new_for_display(gtk_widget_get_display(m_widget), cursor_type);
}

/*
 * VteTerminalPrivate::view_coords_from_event:
 * @event: a #GdkEvent
 *
 * Translates the event coordinates to view coordinates, by
 * subtracting the padding and window offset.
 * Coordinates < 0 or >= m_usable_view_extents.width() or .height()
 * mean that the event coordinates are outside the usable area
 * at that side; use view_coords_visible() to check for that.
 */
vte::view::coords VteTerminalPrivate::view_coords_from_event(GdkEvent const* event) const
{
    double x, y;
    if (event == nullptr ||
        ((reinterpret_cast<GdkEventAny const*>(event))->window != m_event_window) ||
        !gdk_event_get_coords(event, &x, &y)) {
        return vte::view::coords(-1, -1);
    }
    return vte::view::coords(x - m_padding.left, y - m_padding.top);
}

/*
 * VteTerminalPrivate::grid_coords_from_event:
 * @event: a #GdkEvent
 *
 * Translates the event coordinates to view coordinates, by
 * subtracting the padding and window offset.
 * Coordinates < 0 or >= m_usable_view_extents.width() or .height()
 * mean that the event coordinates are outside the usable area
 * at that side; use grid_coords_visible() to check for that.
 */
vte::grid::coords VteTerminalPrivate::grid_coords_from_event(GdkEvent const* event) const
{
    return grid_coords_from_view_coords(view_coords_from_event(event));
}

/*
 * VteTerminalPrivate::confined_grid_coords_from_event:
 * @event: a #GdkEvent
 *
 * Like grid_coords_from_event(), but also confines the coordinates
 * to an actual cell in the visible area.
 */
vte::grid::coords VteTerminalPrivate::confined_grid_coords_from_event(GdkEvent const* event) const
{
    auto pos = view_coords_from_event(event);
    return confined_grid_coords_from_view_coords(pos);
}

/*
 * VteTerminalPrivate::grid_coords_from_view_coords:
 * @pos: the view coordinates
 *
 * Translates view coordinates to grid coordinates. If the view coordinates point to
 * cells that are not visible, may return any value < 0 or >= m_column_count, and
 * < first_displayed_row() or > last_displayed_row(), resp.
 */
vte::grid::coords VteTerminalPrivate::grid_coords_from_view_coords(vte::view::coords const& pos) const
{
    vte::grid::column_t col;
    if (pos.x >= 0 && pos.x < m_view_usable_extents.width()) {
        col = pos.x / m_cell_width;
    } else if (pos.x < 0) {
        col = -1;
    } else {
        col = m_column_count;
    }

    vte::grid::row_t row = pixel_to_row(pos.y);

    return vte::grid::coords(row, col);
}

/*
 * VteTerminalPrivate::confined_grid_coords_from_view_coords:
 * @pos: the view coordinates
 *
 * Like grid_coords_from_view_coords(), but also confines the coordinates
 * to an actual cell in the visible area.
 */
vte::grid::coords VteTerminalPrivate::confined_grid_coords_from_view_coords(vte::view::coords const& pos) const
{
    auto rowcol = grid_coords_from_view_coords(pos);
    return confine_grid_coords(rowcol);
}

/*
 * VteTerminalPrivate::view_coords_from_grid_coords:
 * @rowcol: the grid coordinates
 *
 * Translates grid coordinates to view coordinates. If the view coordinates are
 * outside the usable area, may return any value < 0 or >= m_usable_view_extents.
 *
 * Returns: %true if the coordinates are inside the usable area
 */
vte::view::coords VteTerminalPrivate::view_coords_from_grid_coords(vte::grid::coords const& rowcol) const
{
    return vte::view::coords(rowcol.column() * m_cell_width, row_to_pixel(rowcol.row()));
}

bool VteTerminalPrivate::view_coords_visible(vte::view::coords const& pos) const
{
    return pos.x >= 0 && pos.x < m_view_usable_extents.width() &&
            pos.y >= 0 && pos.y < m_view_usable_extents.height();
}

bool VteTerminalPrivate::grid_coords_visible(vte::grid::coords const& rowcol) const
{
    return rowcol.column() >= 0 &&
            rowcol.column() < m_column_count &&
            rowcol.row() >= first_displayed_row() &&
            rowcol.row() <= last_displayed_row();
}

vte::grid::coords VteTerminalPrivate::confine_grid_coords(vte::grid::coords const& rowcol) const
{
    /* Confine clicks to the nearest actual cell. This is especially useful for
        * fullscreen vte so that you can click on the very edge of the screen.
        */
    auto first_row = first_displayed_row();
    auto last_row = last_displayed_row();

    return vte::grid::coords(CLAMP(rowcol.row(), first_row, last_row),
                                CLAMP(rowcol.column(), 0, m_column_count - 1));
}

bool VteTerminalPrivate::rowcol_from_event(GdkEvent *event, long *column, long *row)
{
    auto rowcol = grid_coords_from_event(event);
    if (!grid_coords_visible(rowcol)) {
        return false;
    }

    *column = rowcol.column();
    *row = rowcol.row();
    return true;
}

char *VteTerminalPrivate::hyperlink_check(GdkEvent *event)
{
    long col, row;
    const char *hyperlink;
    const char *separator;

    if (!m_allow_hyperlink || !rowcol_from_event(event, &col, &row)) {
        return NULL;
    }

    _vte_ring_get_hyperlink_at_position(m_screen->row_data, row, col, false, &hyperlink);

    if (hyperlink != NULL) {
        /* URI is after the first semicolon */
        separator = strchr(hyperlink, ';');
        g_assert(separator != NULL);
        hyperlink = separator + 1;
    }

    _vte_debug_print (VTE_DEBUG_HYPERLINK,
                        "hyperlink_check: \"%s\"\n",
                        hyperlink);

    return g_strdup(hyperlink);
}


/* Emit an adjustment changed signal on our adjustment object. */
void VteTerminalPrivate::emit_adjustment_changed()
{
    if (m_adjustment_changed_pending) {
        bool changed = false;
        gdouble current, v;

        g_object_freeze_notify (G_OBJECT(m_vadjustment));

        v = _vte_ring_delta (m_screen->row_data);
        current = gtk_adjustment_get_lower(m_vadjustment);
        if (!_vte_double_equal(current, v)) {
            _vte_debug_print(VTE_DEBUG_ADJ,
                    "Changing lower bound from %.0f to %f\n",
                     current, v);
            gtk_adjustment_set_lower(m_vadjustment, v);
            changed = true;
        }

        v = m_screen->insert_delta + m_row_count;
        current = gtk_adjustment_get_upper(m_vadjustment);
        if (!_vte_double_equal(current, v)) {
            _vte_debug_print(VTE_DEBUG_ADJ,
                    "Changing upper bound from %.0f to %f\n",
                     current, v);
            gtk_adjustment_set_upper(m_vadjustment, v);
            changed = true;
        }

        /* The step increment should always be one. */
        v = gtk_adjustment_get_step_increment(m_vadjustment);
        if (!_vte_double_equal(v, 1)) {
            _vte_debug_print(VTE_DEBUG_ADJ,
                    "Changing step increment from %.0lf to 1\n", v);
            gtk_adjustment_set_step_increment(m_vadjustment, 1);
            changed = true;
        }

        /* Set the number of rows the user sees to the number of rows the
         * user sees. */
        v = gtk_adjustment_get_page_size(m_vadjustment);
        if (!_vte_double_equal(v, m_row_count)) {
            _vte_debug_print(VTE_DEBUG_ADJ,
                    "Changing page size from %.0f to %ld\n",
                     v, m_row_count);
            gtk_adjustment_set_page_size(m_vadjustment,
                             m_row_count);
            changed = true;
        }

        /* Clicking in the empty area should scroll one screen, so set the
         * page size to the number of visible rows. */
        v = gtk_adjustment_get_page_increment(m_vadjustment);
        if (!_vte_double_equal(v, m_row_count)) {
            _vte_debug_print(VTE_DEBUG_ADJ,
                    "Changing page increment from "
                    "%.0f to %ld\n",
                    v, m_row_count);
            gtk_adjustment_set_page_increment(m_vadjustment,
                              m_row_count);
            changed = true;
        }

        g_object_thaw_notify (G_OBJECT (m_vadjustment));

        if (changed) {
            _vte_debug_print(VTE_DEBUG_SIGNALS,
                    "Emitting adjustment_changed.\n");
        }
        m_adjustment_changed_pending = FALSE;
    }
    if (m_adjustment_value_changed_pending) {
        double v, delta;
        _vte_debug_print(VTE_DEBUG_SIGNALS,
                "Emitting adjustment_value_changed.\n");
        m_adjustment_value_changed_pending = FALSE;
        v = gtk_adjustment_get_value(m_vadjustment);
        if (!_vte_double_equal(v, m_screen->scroll_delta)) {
            /* this little dance is so that the scroll_delta is
             * updated immediately, but we still handled scrolling
             * via the adjustment - e.g. user interaction with the
             * scrollbar
             */
            delta = m_screen->scroll_delta;
            m_screen->scroll_delta = v;
            gtk_adjustment_set_value(m_vadjustment, delta);
        }
    }
}

/* Queue an adjustment-changed signal to be delivered when convenient. */
/* FIXMEchpe this has just one caller, fold it into the call site */
void VteTerminalPrivate::queue_adjustment_changed()
{
    m_adjustment_changed_pending = true;
    add_update_timeout(this);
}

void VteTerminalPrivate::queue_adjustment_value_changed(double v)
{
    if (!_vte_double_equal(v, m_screen->scroll_delta)) {
        _vte_debug_print(VTE_DEBUG_ADJ,
                            "Adjustment value changed to %f\n",
                            v);
        m_screen->scroll_delta = v;
        m_adjustment_value_changed_pending = true;
        add_update_timeout(this);
    }
}

void VteTerminalPrivate::queue_adjustment_value_changed_clamped(double v)
{
    double lower = gtk_adjustment_get_lower(m_vadjustment);
    double upper = gtk_adjustment_get_upper(m_vadjustment);

    v = CLAMP(v, lower, MAX (lower, upper - m_row_count));

    queue_adjustment_value_changed(v);
}

void VteTerminalPrivate::adjust_adjustments()
{
    g_assert(m_screen != nullptr);
    g_assert(m_screen->row_data != nullptr);

    queue_adjustment_changed();

    /* The lower value should be the first row in the buffer. */
    long delta = _vte_ring_delta(m_screen->row_data);
    /* Snap the insert delta and the cursor position to be in the visible
     * area.  Leave the scrolling delta alone because it will be updated
     * when the adjustment changes. */
    m_screen->insert_delta = MAX(m_screen->insert_delta, delta);
    m_screen->cursor.row = MAX(m_screen->cursor.row,
                                m_screen->insert_delta);

    if (m_screen->scroll_delta > m_screen->insert_delta) {
        queue_adjustment_value_changed(m_screen->insert_delta);
    }
}

/* Update the adjustment field of the widget.  This function should be called
 * whenever we add rows to or remove rows from the history or switch screens. */
void VteTerminalPrivate::adjust_adjustments_full()
{
    g_assert(m_screen != NULL);
    g_assert(m_screen->row_data != NULL);

    adjust_adjustments();
    queue_adjustment_changed();
}

/* Scroll a fixed number of lines up or down in the current screen. */
void VteTerminalPrivate::scroll_lines(long lines)
{
    double destination;
    _vte_debug_print(VTE_DEBUG_ADJ, "Scrolling %ld lines.\n", lines);
    /* Calculate the ideal position where we want to be before clamping. */
    destination = m_screen->scroll_delta;
    /* Snap to whole cell offset. */
    if (lines > 0) {
        destination = floor(destination);
    } else if (lines < 0) {
        destination = ceil(destination);
    }
    destination += lines;
    /* Tell the scrollbar to adjust itself. */
    queue_adjustment_value_changed_clamped(destination);
}

/* Scroll so that the scroll delta is the minimum value. */
void VteTerminalPrivate::maybe_scroll_to_top()
{
    queue_adjustment_value_changed(_vte_ring_delta(m_screen->row_data));
}

void VteTerminalPrivate::maybe_scroll_to_bottom()
{
    queue_adjustment_value_changed(m_screen->insert_delta);
    _vte_debug_print(VTE_DEBUG_ADJ,
                        "Snapping to bottom of screen\n");
}

/*
 * VteTerminalPrivate::set_encoding:
 * @codeset: (allow-none): a valid #GIConv target, or %NULL to use UTF-8
 *
 * Changes the encoding the terminal will expect data from the child to
 * be encoded with.  For certain terminal types, applications executing in the
 * terminal can change the encoding. If @codeset is %NULL, it uses "UTF-8".
 *
 * Returns: %true if the encoding could be changed to the specified one
 */
bool VteTerminalPrivate::set_encoding(char const* codeset)
{
    VteConv conv;

    GObject *object = G_OBJECT(m_terminal);

    if (codeset == NULL) {
        codeset = "UTF-8";
    }
    if ((m_encoding != nullptr) && g_str_equal(codeset, m_encoding)) {
        /* Nothing to do! */
        return true;
    }

    /* Open new conversions. */
    conv = _vte_conv_open(codeset, "UTF-8");
    if (conv == VTE_INVALID_CONV) {
        return false;
    }

    auto old_codeset = m_encoding;

    g_object_freeze_notify(object);

    if (m_outgoing_conv != VTE_INVALID_CONV) {
        _vte_conv_close(m_outgoing_conv);
    }
    m_outgoing_conv = conv;

    /* Set the terminal's encoding to the new value. */
    m_encoding = g_intern_string(codeset);

    /* Convert any buffered output bytes. */
    if ((_vte_byte_array_length(m_outgoing) > 0) && (old_codeset != nullptr)) {
        char *obuf1, *obuf2;
        gsize bytes_written;

        /* Convert back to UTF-8. */
        obuf1 = g_convert((char *)m_outgoing->data,
                            _vte_byte_array_length(m_outgoing),
                            "UTF-8",
                            old_codeset,
                            NULL,
                            &bytes_written,
                            NULL);
        if (obuf1 != NULL) {
            /* Convert to the new encoding. */
            obuf2 = g_convert(obuf1,
                                bytes_written,
                                codeset,
                                "UTF-8",
                                NULL,
                                &bytes_written,
                                NULL);
            if (obuf2 != NULL) {
                _vte_byte_array_clear(m_outgoing);
                _vte_byte_array_append(m_outgoing, obuf2, bytes_written);
                g_free(obuf2);
            }
            g_free(obuf1);
        }
    }

    /* Set the encoding for incoming text. */
    _vte_iso2022_state_set_codeset(m_iso2022, m_encoding);

    _vte_debug_print(VTE_DEBUG_IO,
                        "Set terminal encoding to `%s'.\n",
                        m_encoding);
    _vte_debug_print(VTE_DEBUG_SIGNALS,
                        "Emitting `encoding-changed'.\n");
    g_signal_emit(object, signals[SIGNAL_ENCODING_CHANGED], 0);
    g_object_notify_by_pspec(object, pspecs[PROP_ENCODING]);

    g_object_thaw_notify(object);

    return true;
}

bool VteTerminalPrivate::set_cjk_ambiguous_width(int width)
{
    g_assert(width == 1 || width == 2);

    if (m_utf8_ambiguous_width == width) {
        return false;
    }

    m_utf8_ambiguous_width = width;
    return true;
}

/* FIXMEchpe replace this with a method on VteRing */
VteRowData *VteTerminalPrivate::insert_rows (guint cnt)
{
    VteRowData *row;
    do {
        row = ring_append(false);
    } while(--cnt);
    return row;
}

/* Make sure we have enough rows and columns to hold data at the current
 * cursor position. */
VteRowData *VteTerminalPrivate::ensure_row()
{
    VteRowData *row;

    /* Figure out how many rows we need to add. */
        //FIXMEchpe use long, not int
    int delta = m_screen->cursor.row - _vte_ring_next(m_screen->row_data) + 1;
    if (delta > 0) {
        row = insert_rows(delta);
        adjust_adjustments();
    } else {
        /* Find the row the cursor is in. */
        row = _vte_ring_index_writable(m_screen->row_data, m_screen->cursor.row);
    }
    g_assert(row != NULL);

    return row;
}

VteRowData *VteTerminalPrivate::ensure_cursor()
{
    VteRowData *row = ensure_row();
    _vte_row_data_fill(row, &basic_cell, m_screen->cursor.col);

    return row;
}

/* Update the insert delta so that the screen which includes it also
 * includes the end of the buffer. */
void VteTerminalPrivate::update_insert_delta()
{
    /* The total number of lines.  Add one to the cursor offset
     * because it's zero-based. */
    auto rows = _vte_ring_next(m_screen->row_data);
    auto delta = m_screen->cursor.row - rows + 1;
    if (G_UNLIKELY (delta > 0)) {
        insert_rows(delta);
        rows = _vte_ring_next(m_screen->row_data);
    }

    /* Make sure that the bottom row is visible, and that it's in
     * the buffer (even if it's empty).  This usually causes the
     * top row to become a history-only row. */
    delta = m_screen->insert_delta;
    delta = MIN(delta, rows - m_row_count);
    delta = MAX(delta, m_screen->cursor.row - (m_row_count - 1));
    delta = MAX(delta, _vte_ring_delta(m_screen->row_data));

    /* Adjust the insert delta and scroll if needed. */
    if (delta != m_screen->insert_delta) {
        m_screen->insert_delta = delta;
        adjust_adjustments();
    }
}

/* Apply the desired mouse pointer, based on certain member variables. */
void VteTerminalPrivate::apply_mouse_cursor()
{
    if (!widget_realized()) {
        return;
    }

   /* Show the cursor if over the widget and not autohidden, this is obvious.
    * Also show the cursor if outside the widget regardless of the autohidden state, so that if a popover is opened
    * and then the cursor returns (which doesn't trigger enter/motion events), it is visible.
    * That is, only hide the cursor if it's over the widget and is autohidden.
    * See bug 789390 and bug 789536 comment 6 for details. */
    if (!(m_mouse_autohide && m_mouse_cursor_autohidden && m_mouse_cursor_over_widget)) {
        if (m_hyperlink_hover_idx != 0) {
            _vte_debug_print(VTE_DEBUG_CURSOR, "Setting hyperlink mouse cursor.\n");
            gdk_window_set_cursor(m_event_window, m_mouse_hyperlink_cursor);
        } else if (m_mouse_tracking_mode) {
            _vte_debug_print(VTE_DEBUG_CURSOR, "Setting mousing cursor.\n");
            gdk_window_set_cursor(m_event_window, m_mouse_mousing_cursor);
        } else {
            _vte_debug_print(VTE_DEBUG_CURSOR, "Setting default mouse cursor.\n");
            gdk_window_set_cursor(m_event_window, m_mouse_default_cursor);
        }
    } else {
        _vte_debug_print(VTE_DEBUG_CURSOR, "Setting to invisible cursor.\n");
        gdk_window_set_cursor(m_event_window, m_mouse_inviso_cursor);
    }
}

/* Show or hide the pointer if autohiding is enabled. */
void VteTerminalPrivate::set_pointer_autohidden(bool autohidden)
{
    if (autohidden == m_mouse_cursor_autohidden) {
        return;
    }

    m_mouse_cursor_autohidden = autohidden;

    if (m_mouse_autohide) {
        hyperlink_hilite_update();
        apply_mouse_cursor();
    }
}

/*
 * Get the actually used color from the palette.
 * The return value can be NULL only if entry is one of VTE_CURSOR_BG,
 * VTE_CURSOR_FG, VTE_HIGHLIGHT_BG or VTE_HIGHLIGHT_FG.
 */
vte::color::rgb const* VteTerminalPrivate::get_color(int entry) const
{
    VtePaletteColor const* palette_color = &m_palette[entry];
    guint source;
    for (source = 0; source < G_N_ELEMENTS(palette_color->sources); source++) {
        if (palette_color->sources[source].is_set) {
            return &palette_color->sources[source].color;
        }
    }
    return nullptr;
}

/* Set up a palette entry with a more-or-less match for the requested color. */
void VteTerminalPrivate::set_color(int entry, int source, vte::color::rgb const& proposed)
{
    g_assert(entry >= 0 && entry < VTE_PALETTE_SIZE);

    VtePaletteColor *palette_color = &m_palette[entry];

    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color[%d] to (%04x,%04x,%04x).\n",
                        source == VTE_COLOR_SOURCE_ESCAPE ? "escape" : "API",
                        entry, proposed.red, proposed.green, proposed.blue);

    if (palette_color->sources[source].is_set &&
        palette_color->sources[source].color == proposed) {
        return;
    }
    palette_color->sources[source].is_set = TRUE;
    palette_color->sources[source].color = proposed;

    /* If we're not realized yet, there's nothing else to do. */
    if (!widget_realized()) {
        return;
    }

    /* and redraw */
    if (entry == VTE_CURSOR_BG || entry == VTE_CURSOR_FG) {
        invalidate_cursor_once();
    } else {
        invalidate_all();
    }
}

void VteTerminalPrivate::reset_color(int entry, int source)
{
    g_assert(entry >= 0 && entry < VTE_PALETTE_SIZE);

    VtePaletteColor *palette_color = &m_palette[entry];

    _vte_debug_print(VTE_DEBUG_MISC,
                        "Reset %s color[%d].\n",
                        source == VTE_COLOR_SOURCE_ESCAPE ? "escape" : "API",
                        entry);

    if (!palette_color->sources[source].is_set) {
        return;
    }
    palette_color->sources[source].is_set = FALSE;

    /* If we're not realized yet, there's nothing else to do. */
    if (!widget_realized()) {
        return;
    }

    /* and redraw */
    if (entry == VTE_CURSOR_BG || entry == VTE_CURSOR_FG) {
        invalidate_cursor_once();
    } else {
        invalidate_all();
    }
}

bool VteTerminalPrivate::set_background_alpha(double alpha)
{
    g_assert(alpha >= 0. && alpha <= 1.);

    if (_vte_double_equal(alpha, m_background_alpha)) {
        return false;
    }

    _vte_debug_print(VTE_DEBUG_MISC, "Setting background alpha to %.3f\n", alpha);
    m_background_alpha = alpha;

    invalidate_all();

    return true;
}

void VteTerminalPrivate::set_colors_default()
{
    set_colors(nullptr, nullptr, nullptr, 0);
}

/*
 * VteTerminalPrivate::set_colors:
 * @terminal: a #VteTerminal
 * @foreground: (allow-none): the new foreground color, or %NULL
 * @background: (allow-none): the new background color, or %NULL
 * @palette: (array length=palette_size zero-terminated=0): the color palette
 * @palette_size: the number of entries in @palette
 *
 * @palette specifies the new values for the 256 palette colors: 8 standard colors,
 * their 8 bright counterparts, 6x6x6 color cube, and 24 grayscale colors.
 * Omitted entries will default to a hardcoded value.
 *
 * @palette_size must be 0, 8, 16, 232 or 256.
 *
 * If @foreground is %NULL and @palette_size is greater than 0, the new foreground
 * color is taken from @palette[7].  If @background is %NULL and @palette_size is
 * greater than 0, the new background color is taken from @palette[0].
 */
void VteTerminalPrivate::set_colors(vte::color::rgb const* foreground,
                                    vte::color::rgb const* background,
                                    vte::color::rgb const* new_palette,
                                    gsize palette_size)
{
    _vte_debug_print(VTE_DEBUG_MISC,
            "Set color palette [%" G_GSIZE_FORMAT " elements].\n",
            palette_size);

    /* Accept NULL as the default foreground and background colors if we
     * got a palette. */
    if ((foreground == NULL) && (palette_size >= 8)) {
        foreground = &new_palette[7];
    }
    if ((background == NULL) && (palette_size >= 8)) {
        background = &new_palette[0];
    }

    /* Initialize each item in the palette if we got any entries to work
     * with. */
    for (gsize i = 0; i < G_N_ELEMENTS(m_palette); i++) {
        vte::color::rgb color;
        bool unset = false;

        if (i < 16) {
            color.blue = (i & 4) ? 0xc000 : 0;
            color.green = (i & 2) ? 0xc000 : 0;
            color.red = (i & 1) ? 0xc000 : 0;
            if (i > 7) {
                color.blue += 0x3fff;
                color.green += 0x3fff;
                color.red += 0x3fff;
            }
        } else if (i < 232) {
            int j = i - 16;
            int r = j / 36, g = (j / 6) % 6, b = j % 6;
            int red =   (r == 0) ? 0 : r * 40 + 55;
            int green = (g == 0) ? 0 : g * 40 + 55;
            int blue =  (b == 0) ? 0 : b * 40 + 55;
            color.red   = red | red << 8  ;
            color.green = green | green << 8;
            color.blue  = blue | blue << 8;
        } else if (i < 256) {
            int shade = 8 + (i - 232) * 10;
            color.red = color.green = color.blue = shade | shade << 8;
        } else switch (i) {
            case VTE_DEFAULT_BG:
                if (background) {
                    color = *background;
                } else {
                    color.red = 0;
                    color.blue = 0;
                    color.green = 0;
                }
                break;
            case VTE_DEFAULT_FG:
                if (foreground) {
                    color = *foreground;
                } else {
                    color.red = 0xc000;
                    color.blue = 0xc000;
                    color.green = 0xc000;
                }
                break;
            case VTE_BOLD_FG:
                unset = true;
                break;
            case VTE_HIGHLIGHT_BG:
                unset = true;
                break;
            case VTE_HIGHLIGHT_FG:
                unset = true;
                break;
            case VTE_CURSOR_BG:
                unset = true;
                break;
            case VTE_CURSOR_FG:
                unset = true;
                break;
            }

        /* Override from the supplied palette if there is one. */
        if (i < palette_size) {
            color = new_palette[i];
        }

        /* Set up the color entry. */
        if (unset) {
            reset_color(i, VTE_COLOR_SOURCE_API);
        } else {
            set_color(i, VTE_COLOR_SOURCE_API, color);
        }
    }
}

/*
 * VteTerminalPrivate::set_color_bold:
 * @bold: (allow-none): the new bold color or %NULL
 *
 * Sets the color used to draw bold text in the default foreground color.
 * If @bold is %NULL then the default color is used.
 */
void VteTerminalPrivate::set_color_bold(vte::color::rgb const& color)
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color to (%04x,%04x,%04x).\n", "bold",
                        color.red, color.green, color.blue);
    set_color(VTE_BOLD_FG, VTE_COLOR_SOURCE_API, color);
}

void VteTerminalPrivate::reset_color_bold()
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Reset %s color.\n", "bold");
    reset_color(VTE_BOLD_FG, VTE_COLOR_SOURCE_API);
}

/*
 * VteTerminalPrivate::set_color_foreground:
 * @foreground: the new foreground color
 *
 * Sets the foreground color used to draw normal text.
 */
void VteTerminalPrivate::set_color_foreground(vte::color::rgb const& color)
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color to (%04x,%04x,%04x).\n", "foreground",
                        color.red, color.green, color.blue);
    set_color(VTE_DEFAULT_FG, VTE_COLOR_SOURCE_API, color);
}

/*
 * VteTerminalPrivate::set_color_background:
 * @background: the new background color
 *
 * Sets the background color for text which does not have a specific background
 * color assigned.  Only has effect when no background image is set and when
 * the terminal is not transparent.
 */
void VteTerminalPrivate::set_color_background(vte::color::rgb const& color)
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color to (%04x,%04x,%04x).\n", "background",
                        color.red, color.green, color.blue);
    set_color(VTE_DEFAULT_BG, VTE_COLOR_SOURCE_API, color);
}

/*
 * VteTerminalPrivate::set_color_cursor_background:
 * @cursor_background: (allow-none): the new color to use for the text cursor, or %NULL
 *
 * Sets the background color for text which is under the cursor.  If %NULL, text
 * under the cursor will be drawn with foreground and background colors
 * reversed.
 */
void VteTerminalPrivate::set_color_cursor_background(vte::color::rgb const& color)
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color to (%04x,%04x,%04x).\n", "cursor background",
                        color.red, color.green, color.blue);
    set_color(VTE_CURSOR_BG, VTE_COLOR_SOURCE_API, color);
}

void VteTerminalPrivate::reset_color_cursor_background()
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Reset %s color.\n", "cursor background");
    reset_color(VTE_CURSOR_BG, VTE_COLOR_SOURCE_API);
}

/*
 * VteTerminalPrivate::set_color_cursor_foreground:
 * @cursor_foreground: (allow-none): the new color to use for the text cursor, or %NULL
 *
 * Sets the foreground color for text which is under the cursor.  If %NULL, text
 * under the cursor will be drawn with foreground and background colors
 * reversed.
 */
void VteTerminalPrivate::set_color_cursor_foreground(vte::color::rgb const& color)
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color to (%04x,%04x,%04x).\n", "cursor foreground",
                        color.red, color.green, color.blue);
    set_color(VTE_CURSOR_FG, VTE_COLOR_SOURCE_API, color);
}

void VteTerminalPrivate::reset_color_cursor_foreground()
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Reset %s color.\n", "cursor foreground");
    reset_color(VTE_CURSOR_FG, VTE_COLOR_SOURCE_API);
}

/*
 * VteTerminalPrivate::set_color_highlight_background:
 * @highlight_background: (allow-none): the new color to use for highlighted text, or %NULL
 *
 * Sets the background color for text which is highlighted.  If %NULL,
 * it is unset.  If neither highlight background nor highlight foreground are set,
 * highlighted text (which is usually highlighted because it is selected) will
 * be drawn with foreground and background colors reversed.
 */
void VteTerminalPrivate::set_color_highlight_background(vte::color::rgb const& color)
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color to (%04x,%04x,%04x).\n", "highlight background",
                        color.red, color.green, color.blue);
    set_color(VTE_HIGHLIGHT_BG, VTE_COLOR_SOURCE_API, color);
}

void VteTerminalPrivate::reset_color_highlight_background()
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Reset %s color.\n", "highlight background");
    reset_color(VTE_HIGHLIGHT_BG, VTE_COLOR_SOURCE_API);
}

/*
 * VteTerminalPrivate::set_color_highlight_foreground:
 * @highlight_foreground: (allow-none): the new color to use for highlighted text, or %NULL
 *
 * Sets the foreground color for text which is highlighted.  If %NULL,
 * it is unset.  If neither highlight background nor highlight foreground are set,
 * highlighted text (which is usually highlighted because it is selected) will
 * be drawn with foreground and background colors reversed.
 */
void VteTerminalPrivate::set_color_highlight_foreground(vte::color::rgb const& color)
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Set %s color to (%04x,%04x,%04x).\n", "highlight foreground",
                        color.red, color.green, color.blue);
    set_color(VTE_HIGHLIGHT_FG, VTE_COLOR_SOURCE_API, color);
}

void VteTerminalPrivate::reset_color_highlight_foreground()
{
    _vte_debug_print(VTE_DEBUG_MISC,
                        "Reset %s color.\n", "highlight foreground");
    reset_color(VTE_HIGHLIGHT_FG, VTE_COLOR_SOURCE_API);
}

/*
 * VteTerminalPrivate::cleanup_fragments:
 * @start: the starting column, inclusive
 * @end: the end column, exclusive
 *
 * Needs to be called before modifying the contents in the cursor's row,
 * between the two given columns.  Cleans up TAB and CJK fragments to the
 * left of @start and to the right of @end.  If a CJK is split in half,
 * the remaining half is replaced by a space.  If a TAB at @start is split,
 * it is replaced by spaces.  If a TAB at @end is split, it is replaced by
 * a shorter TAB.  @start and @end can be equal if characters will be
 * inserted at the location rather than overwritten.
 *
 * The area between @start and @end is not cleaned up, hence the whole row
 * can be left in an inconsistent state.  It is expected that the caller
 * will fill up that range afterwards, resulting in a consistent row again.
 *
 * Invalidates the cells that visually change outside of the range,
 * because the caller can't reasonably be expected to take care of this.
 */
void VteTerminalPrivate::cleanup_fragments(long start, long end)
{
    VteRowData *row = ensure_row();
    const VteCell *cell_start;
    VteCell *cell_end, *cell_col;
    gboolean cell_start_is_fragment;
    long col;

    g_assert(end >= start);

    /* Remember whether the cell at start is a fragment.  We'll need to know it when
        * handling the left hand side, but handling the right hand side first might
        * overwrite it if start == end (inserting to the middle of a character). */
    cell_start = _vte_row_data_get (row, start);
    cell_start_is_fragment = cell_start != NULL && cell_start->attr.fragment();

    /* On the right hand side, try to replace a TAB by a shorter TAB if we can.
        * This requires that the TAB on the left (which might be the same TAB) is
        * not yet converted to spaces, so start on the right hand side. */
    cell_end = _vte_row_data_get_writable (row, end);
    if (G_UNLIKELY (cell_end != NULL && cell_end->attr.fragment())) {
        col = end;
        do {
            col--;
            g_assert(col >= 0);  /* The first cell can't be a fragment. */
            cell_col = _vte_row_data_get_writable (row, col);
        } while (cell_col->attr.fragment());
        if (cell_col->c == '\t') {
            _vte_debug_print(VTE_DEBUG_MISC,
                                "Replacing right part of TAB with a shorter one at %ld (%ld cells) => %ld (%ld cells)\n",
                                col, (long) cell_col->attr.columns(), end, (long) cell_col->attr.columns() - (end - col));
            cell_end->c = '\t';
            cell_end->attr.set_fragment(false);
            g_assert(cell_col->attr.columns() > end - col);
            cell_end->attr.set_columns(cell_col->attr.columns() - (end - col));
        } else {
            _vte_debug_print(VTE_DEBUG_MISC,
                                "Cleaning CJK right half at %ld\n",
                                end);
            g_assert(end - col == 1 && cell_col->attr.columns() == 2);
            cell_end->c = ' ';
            cell_end->attr.set_fragment(false);
            cell_end->attr.set_columns(1);
            invalidate_cells(end, 1, m_screen->cursor.row, 1);
        }
    }

    /* Handle the left hand side.  Converting longer TABs to shorter ones probably
        * wouldn't make that much sense here, so instead convert to spaces. */
    if (G_UNLIKELY (cell_start_is_fragment)) {
        gboolean keep_going = TRUE;
        col = start;
        do {
            col--;
            g_assert(col >= 0);  /* The first cell can't be a fragment. */
            cell_col = _vte_row_data_get_writable (row, col);
            if (!cell_col->attr.fragment()) {
                if (cell_col->c == '\t') {
                    _vte_debug_print(VTE_DEBUG_MISC,
                                        "Replacing left part of TAB with spaces at %ld (%ld => %ld cells)\n",
                                        col, (long)cell_col->attr.columns(), start - col);
                    /* nothing to do here */
                } else {
                    _vte_debug_print(VTE_DEBUG_MISC,
                                        "Cleaning CJK left half at %ld\n",
                                        col);
                    g_assert(start - col == 1);
                    invalidate_cells(col, 1, m_screen->cursor.row, 1);
                }
                keep_going = FALSE;
            }
            cell_col->c = ' ';
            cell_col->attr.set_fragment(false);
            cell_col->attr.set_columns(1);
        } while (keep_going);
    }
}

/* Cursor down, with scrolling. */
void VteTerminalPrivate::cursor_down(bool explicit_sequence)
{
    long start, end;

    if (m_scrolling_restricted) {
        start = m_screen->insert_delta + m_scrolling_region.start;
        end = m_screen->insert_delta + m_scrolling_region.end;
    } else {
        start = m_screen->insert_delta;
        end = start + m_row_count - 1;
    }
    if (m_screen->cursor.row == end) {
        if (m_scrolling_restricted) {
            if (start == m_screen->insert_delta) {
                /* Scroll this line into the scrollback
                 * buffer by inserting a line at the next
                 * line and scrolling the area up. */
                m_screen->insert_delta++;
                m_screen->cursor.row++;
                /* update start and end, as they are relative
                 * to insert_delta. */
                start++;
                end++;
                ring_insert(m_screen->cursor.row, false);
                /* Force the areas below the region to be
                 * redrawn -- they've moved. */
                scroll_region(start, end - start + 1, 1);
                /* Force scroll. */
                adjust_adjustments();
            } else {
                /* If we're at the bottom of the scrolling
                 * region, add a line at the top to scroll the
                 * bottom off. */
                ring_remove(start);
                ring_insert(end, true);
                /* Update the display. */
                scroll_region(start, end - start + 1, -1);
                invalidate_cells(0, m_column_count, end - 2, 2);
            }
        } else {
            /* Scroll up with history. */
            m_screen->cursor.row++;
            update_insert_delta();
        }

       /* Handle bce (background color erase), however, diverge from xterm:
        * only fill the new row with the background color if scrolling
        * happens due to an explicit escape sequence, not due to autowrapping.
        * See bug 754596 for details. */
        bool const not_default_bg = (m_fill_defaults.attr.back() != VTE_DEFAULT_BG);

        if (explicit_sequence && not_default_bg) {
            VteRowData *rowdata = ensure_row();
            _vte_row_data_fill (rowdata, &m_fill_defaults, m_column_count);
        }
    } else {
        /* Otherwise, just move the cursor down. */
        m_screen->cursor.row++;
    }
}

/* Drop the scrollback. */
void VteTerminalPrivate::drop_scrollback()
{
    /* Only for normal screen; alternate screen doesn't have a scrollback. */
    _vte_ring_drop_scrollback (m_normal_screen.row_data, m_normal_screen.insert_delta);

    if (m_screen == &m_normal_screen) {
        queue_adjustment_value_changed(m_normal_screen.insert_delta);
        adjust_adjustments_full();
    }
}

/* Restore cursor on a screen. */
void VteTerminalPrivate::restore_cursor(VteScreen *screen__)
{
    screen__->cursor.col = screen__->saved.cursor.col;
    screen__->cursor.row = screen__->insert_delta + CLAMP(screen__->saved.cursor.row,
                                                            0, m_row_count - 1);

    m_reverse_mode = screen__->saved.reverse_mode;
    m_origin_mode = screen__->saved.origin_mode;
    m_sendrecv_mode = screen__->saved.sendrecv_mode;
    m_insert_mode = screen__->saved.insert_mode;
    m_linefeed_mode = screen__->saved.linefeed_mode;
    m_defaults = screen__->saved.defaults;
    m_color_defaults = screen__->saved.color_defaults;
    m_fill_defaults = screen__->saved.fill_defaults;
    m_character_replacements[0] = screen__->saved.character_replacements[0];
    m_character_replacements[1] = screen__->saved.character_replacements[1];
    m_character_replacement = screen__->saved.character_replacement;
}

/* Save cursor on a screen__. */
void VteTerminalPrivate::save_cursor(VteScreen *screen__)
{
    screen__->saved.cursor.col = screen__->cursor.col;
    screen__->saved.cursor.row = screen__->cursor.row - screen__->insert_delta;

    screen__->saved.reverse_mode = m_reverse_mode;
    screen__->saved.origin_mode = m_origin_mode;
    screen__->saved.sendrecv_mode = m_sendrecv_mode;
    screen__->saved.insert_mode = m_insert_mode;
    screen__->saved.linefeed_mode = m_linefeed_mode;
    screen__->saved.defaults = m_defaults;
    screen__->saved.color_defaults = m_color_defaults;
    screen__->saved.fill_defaults = m_fill_defaults;
    screen__->saved.character_replacements[0] = m_character_replacements[0];
    screen__->saved.character_replacements[1] = m_character_replacements[1];
    screen__->saved.character_replacement = m_character_replacement;
}

/* Insert a single character into the stored data array. */
bool VteTerminalPrivate::insert_char(gunichar c,
                                        bool insert,
                                        bool invalidate_now)
{
    VteCellAttr attr;
    VteRowData *row;
    long col;
    int columns, i;
    bool line_wrapped = false; /* cursor moved before char inserted */
    gunichar c_unmapped = c;

    /* DEC Special Character and Line Drawing Set.  VT100 and higher (per XTerm docs). */
    static gunichar line_drawing_map[31] = {
        0x25c6,  /* ` => diamond */
        0x2592,  /* a => checkerboard */
        0x2409,  /* b => HT symbol */
        0x240c,  /* c => FF symbol */
        0x240d,  /* d => CR symbol */
        0x240a,  /* e => LF symbol */
        0x00b0,  /* f => degree */
        0x00b1,  /* g => plus/minus */
        0x2424,  /* h => NL symbol */
        0x240b,  /* i => VT symbol */
        0x2518,  /* j => downright corner */
        0x2510,  /* k => upright corner */
        0x250c,  /* l => upleft corner */
        0x2514,  /* m => downleft corner */
        0x253c,  /* n => cross */
        0x23ba,  /* o => scan line 1/9 */
        0x23bb,  /* p => scan line 3/9 */
        0x2500,  /* q => horizontal line (also scan line 5/9) */
        0x23bc,  /* r => scan line 7/9 */
        0x23bd,  /* s => scan line 9/9 */
        0x251c,  /* t => left t */
        0x2524,  /* u => right t */
        0x2534,  /* v => bottom t */
        0x252c,  /* w => top t */
        0x2502,  /* x => vertical line */
        0x2264,  /* y => <= */
        0x2265,  /* z => >= */
        0x03c0,  /* { => pi */
        0x2260,  /* | => not equal */
        0x00a3,  /* } => pound currency sign */
        0x00b7,  /* ~ => bullet */
    };

    insert |= m_insert_mode;
    invalidate_now |= insert;

    /* If we've enabled the special drawing set, map the characters to
     * Unicode. */
    if (G_UNLIKELY (*m_character_replacement == VTE_CHARACTER_REPLACEMENT_LINE_DRAWING)) {
        if (c >= 96 && c <= 126) {
                c = line_drawing_map[c - 96];
        }
    } else if (G_UNLIKELY (*m_character_replacement == VTE_CHARACTER_REPLACEMENT_BRITISH)) {
        if (G_UNLIKELY (c == '#')) {
                c = 0x00a3;  /* pound sign */
        }
    }

    /* Figure out how many columns this character should occupy. */
    columns = _vte_unichar_width(c, m_utf8_ambiguous_width);

    /* If we're autowrapping here, do it. */
        col = m_screen->cursor.col;
    if (G_UNLIKELY (columns && col + columns > m_column_count)) {
        if (m_autowrap) {
            _vte_debug_print(VTE_DEBUG_ADJ, "Autowrapping before character\n");
            /* Wrap. */
            /* XXX clear to the end of line */
            col = m_screen->cursor.col = 0;
            /* Mark this line as soft-wrapped. */
            row = ensure_row();
            row->attr.soft_wrapped = 1;
            cursor_down(false);
        } else {
            /* Don't wrap, stay at the rightmost column. */
            col = m_screen->cursor.col =
            m_column_count - columns;
        }
        line_wrapped = true;
    }

    _vte_debug_print(VTE_DEBUG_PARSE,
            "Inserting %ld '%c' (colors %" G_GUINT64_FORMAT ") (%ld+%d, %ld), delta = %ld; ",
            (long)c, c < 256 ? c : ' ',
            m_color_defaults.attr.colors(),
            col, columns, (long)m_screen->cursor.row,
            (long)m_screen->insert_delta);

    if (G_UNLIKELY (columns == 0)) {

        /* It's a combining mark */

        long row_num;
        VteCell *cell;

        _vte_debug_print(VTE_DEBUG_PARSE, "combining U+%04X", c);

        row_num = m_screen->cursor.row;
        row = NULL;
        if (G_UNLIKELY (col == 0)) {
            /* We are at first column.  See if the previous line softwrapped.
             * If it did, move there.  Otherwise skip inserting. */

            if (G_LIKELY (row_num > 0)) {
                row_num--;
                row = find_row_data_writable(row_num);

                if (row) {
                    if (!row->attr.soft_wrapped) {
                        row = NULL;
                    } else {
                        col = _vte_row_data_length (row);
                    }
                }
            }
        } else {
            row = find_row_data_writable(row_num);
        }

        if (G_UNLIKELY (!row || !col)) {
            goto not_inserted;
        }

        /* Combine it on the previous cell */

        col--;
        cell = _vte_row_data_get_writable (row, col);

        if (G_UNLIKELY (!cell)) {
            goto not_inserted;
        }

        /* Find the previous cell */
        while (cell && cell->attr.fragment() && col > 0) {
            cell = _vte_row_data_get_writable (row, --col);
        }
        if (G_UNLIKELY (!cell || cell->c == '\t')) {
            goto not_inserted;
        }

        /* Combine the new character on top of the cell string */
        c = _vte_unistr_append_unichar (cell->c, c);

        /* And set it */
        columns = cell->attr.columns();
        for (i = 0; i < columns; i++) {
            cell = _vte_row_data_get_writable (row, col++);
            cell->c = c;
        }

        /* Always invalidate since we put the mark on the *previous* cell
         * and the higher level code doesn't know this. */
        invalidate_cells(col - columns, columns, row_num, 1);

        goto done;
    } else {
        m_last_graphic_character = c_unmapped;
    }

    /* Make sure we have enough rows to hold this data. */
    row = ensure_cursor();
    g_assert(row != NULL);

    if (insert) {
        cleanup_fragments(col, col);
        for (i = 0; i < columns; i++) {
            _vte_row_data_insert (row, col + i, &m_color_defaults);
        }
    } else {
        cleanup_fragments(col, col + columns);
        _vte_row_data_fill (row, &basic_cell, col + columns);
    }

    attr = m_defaults.attr;
    attr.copy_colors(m_color_defaults.attr);
    attr.set_columns(columns);

    {
        VteCell *pcell = _vte_row_data_get_writable (row, col);
        pcell->c = c;
        pcell->attr = attr;
        col++;
    }

    /* insert wide-char fragments */
    attr.set_fragment(true);
    for (i = 1; i < columns; i++) {
        VteCell *pcell = _vte_row_data_get_writable (row, col);
        pcell->c = c;
        pcell->attr = attr;
        col++;
    }
    if (_vte_row_data_length (row) > m_column_count) {
        cleanup_fragments(m_column_count, _vte_row_data_length (row));
    }
    _vte_row_data_shrink (row, m_column_count);

    /* Signal that this part of the window needs drawing. */
    if (G_UNLIKELY (invalidate_now)) {
        invalidate_cells(col - columns,
                            insert ? m_column_count : columns,
                            m_screen->cursor.row, 1);
    }

    m_screen->cursor.col = col;

done:
    /* We added text, so make a note of it. */
    m_text_inserted_flag = TRUE;

not_inserted:
    _vte_debug_print(VTE_DEBUG_ADJ|VTE_DEBUG_PARSE,
                        "insertion delta => %ld.\n",
                        (long)m_screen->insert_delta);
    return line_wrapped;
}

/* Reset the input method context. */
void VteTerminalPrivate::im_reset()
{
    if (widget_realized() && m_im_context) {
        gtk_im_context_reset(m_im_context);
    }

    if (m_im_preedit) {
        g_free(m_im_preedit);
        m_im_preedit = nullptr;
    }
    if (m_im_preedit_attrs) {
        pango_attr_list_unref(m_im_preedit_attrs);
        m_im_preedit_attrs = nullptr;
    }
}

/* Process incoming data, first converting it to unicode characters, and then
 * processing control sequences. */
void VteTerminalPrivate::process_incoming()
{
    VteVisualPosition saved_cursor;
    gboolean saved_cursor_visible;
    VteCursorStyle saved_cursor_style;
    GdkPoint bbox_topleft, bbox_bottomright;
    gunichar *wbuf, c;
    long wcount, start;
    gboolean leftovers, modified, bottom, again;
    gboolean invalidated_text;
    gboolean in_scroll_region;
    GArray *unichars;
    struct _vte_incoming_chunk *chunk, *next_chunk, *achunk = NULL;

    _vte_debug_print(VTE_DEBUG_IO,
            "Handler processing %" G_GSIZE_FORMAT " bytes over %" G_GSIZE_FORMAT " chunks + %d bytes pending.\n",
            _vte_incoming_chunks_length(m_incoming),
            _vte_incoming_chunks_count(m_incoming),
            m_pending->len);
    _vte_debug_print (VTE_DEBUG_WORK, "(");

    auto previous_screen = m_screen;

    bottom = m_screen->insert_delta == (long)m_screen->scroll_delta;

    auto top_row = first_displayed_row();
    auto bottom_row = last_displayed_row();

    /* Save the current cursor position. */
    saved_cursor = m_screen->cursor;
    saved_cursor_visible = m_cursor_visible;
    saved_cursor_style = m_cursor_style;

    in_scroll_region = m_scrolling_restricted
        && (m_screen->cursor.row >= (m_screen->insert_delta + m_scrolling_region.start))
        && (m_screen->cursor.row <= (m_screen->insert_delta + m_scrolling_region.end));

    /* We should only be called when there's data to process. */
    g_assert(m_incoming || (m_pending->len > 0));

    /* Convert the data into unicode characters. */
    unichars = m_pending;
    for (chunk = _vte_incoming_chunks_reverse (m_incoming);
            chunk != NULL;
            chunk = next_chunk) {
        gsize processed;
        next_chunk = chunk->next;
        if (chunk->len == 0) {
            goto skip_chunk;
        }
        processed = _vte_iso2022_process(m_iso2022, chunk->data, chunk->len, unichars);
        if (G_UNLIKELY (processed != chunk->len)) {
            /* shuffle the data about */
            g_memmove (chunk->data, chunk->data + processed, chunk->len - processed);
            chunk->len = chunk->len - processed;
            processed = sizeof (chunk->data) - chunk->len;
            if (processed != 0 && next_chunk !=  NULL) {
                if (next_chunk->len <= processed) {
                    /* consume it entirely */
                    memcpy (chunk->data + chunk->len,
                            next_chunk->data,
                            next_chunk->len);
                    chunk->len += next_chunk->len;
                    chunk->next = next_chunk->next;
                    release_chunk (next_chunk);
                } else {
                    /* next few bytes */
                    memcpy (chunk->data + chunk->len,
                            next_chunk->data,
                            processed);
                    chunk->len += processed;
                    g_memmove (next_chunk->data,
                                next_chunk->data + processed,
                                next_chunk->len - processed);
                    next_chunk->len -= processed;
                }
                next_chunk = chunk; /* repeat */
            } else {
                break;
            }
        } else {
skip_chunk:
            /* cache the last chunk */
            if (achunk) {
                release_chunk (achunk);
            }
            achunk = chunk;
        }
    }
    if (achunk) {
        if (chunk != NULL) {
            release_chunk (achunk);
        } else {
            chunk = achunk;
            chunk->next = NULL;
            chunk->len = 0;
        }
    }
    m_incoming = chunk;

    /* Compute the number of unicode characters we got. */
    wbuf = &g_array_index(unichars, gunichar, 0);
    wcount = unichars->len;

    /* Try initial substrings. */
    start = 0;
    modified = leftovers = again = FALSE;
    invalidated_text = FALSE;

    bbox_bottomright.x = bbox_bottomright.y = -G_MAXINT;
    bbox_topleft.x = bbox_topleft.y = G_MAXINT;

    while (start < wcount && !leftovers) {
        const gunichar *next;
        vte::parser::Params params{nullptr};

        /* Try to match any control sequences. */
        sequence_handler_t handler = nullptr;
        auto match_result = _vte_matcher_match(m_matcher,
                                                       &wbuf[start],
                                                       wcount - start,
                                                       &handler,
                                                       &next,
                                                       &params.m_values);
        switch (match_result) {
        /* We're in one of three possible situations now.
         * First, the match returned a handler, and next
         * points to the first character which isn't part of this
         * sequence. */
            case VTE_MATCHER_RESULT_MATCH: {
                _VTE_DEBUG_IF(VTE_DEBUG_PARSE)
                params.print();

                /* Call the sequence handler */
                (this->*handler)(params);

                m_last_graphic_character = 0;

                /* Skip over the proper number of unicode chars. */
                start = (next - wbuf);
                modified = TRUE;

                // FIXME m_screen may be != previous_screen, check for that!

                gboolean new_in_scroll_region = m_scrolling_restricted
                    && (m_screen->cursor.row >= (m_screen->insert_delta + m_scrolling_region.start))
                    && (m_screen->cursor.row <= (m_screen->insert_delta + m_scrolling_region.end));

                /* delta may have changed from sequence. */
                top_row = first_displayed_row();
                bottom_row = last_displayed_row();

               /* if we have moved greatly during the sequence handler, or moved
                * into a scroll_region from outside it, restart the bbox.
                */
                if (invalidated_text &&
                    ((new_in_scroll_region && !in_scroll_region) ||
                                         (m_screen->cursor.col > bbox_bottomright.x + VTE_CELL_BBOX_SLACK ||
                                          m_screen->cursor.col < bbox_topleft.x - VTE_CELL_BBOX_SLACK     ||
                                          m_screen->cursor.row > bbox_bottomright.y + VTE_CELL_BBOX_SLACK ||
                                          m_screen->cursor.row < bbox_topleft.y - VTE_CELL_BBOX_SLACK))) {
                    /* Clip off any part of the box which isn't already on-screen. */
                    bbox_topleft.x = MAX(bbox_topleft.x, 0);
                    bbox_topleft.y = MAX(bbox_topleft.y, top_row);
                    bbox_bottomright.x = MIN(bbox_bottomright.x, m_column_count);
                    /* lazily apply the +1 to the cursor_row */
                    bbox_bottomright.y = MIN(bbox_bottomright.y + 1, bottom_row + 1);

                    invalidate_cells(bbox_topleft.x,
                                        bbox_bottomright.x - bbox_topleft.x,
                                        bbox_topleft.y,
                                        bbox_bottomright.y - bbox_topleft.y);

                    invalidated_text = FALSE;
                    bbox_bottomright.x = bbox_bottomright.y = -G_MAXINT;
                    bbox_topleft.x = bbox_topleft.y = G_MAXINT;
                }

                in_scroll_region = new_in_scroll_region;

                break;
            }
           /* Second, we have no match, and next points to the very
            * next character in the buffer.  Insert the character which
            * we're currently examining into the screen. */
            case VTE_MATCHER_RESULT_NO_MATCH: {
                c = wbuf[start];
                /* If it's a control character, permute the order, per
                * vttest. */
                if ((c != *next) &&
                    ((*next & 0x1f) == *next) &&
                                //FIXMEchpe what about C1 controls
                    (start + 1 < next - wbuf)) {
                    const gunichar *tnext = NULL;
                    gunichar ctrl;
                    int i;
                   /* We don't want to permute it if it's another
                    * control sequence, so check if it is. */
                    sequence_handler_t thandler;
                    _vte_matcher_match(m_matcher,
                                        next,
                                        wcount - (next - wbuf),
                                        &thandler,
                                        &tnext,
                                        NULL);
                   /* We only do this for non-control-sequence
                    * characters and random garbage. */
                    if (tnext == next + 1) {
                        /* Save the control character. */
                        ctrl = *next;
                       /* Move everything before it up a
                        * slot.  */
                        /* FIXMEchpe memmove! */
                        for (i = next - wbuf; i > start; i--) {
                            wbuf[i] = wbuf[i - 1];
                        }
                        /* Move the control character to the
                        * front. */
                        wbuf[i] = ctrl;
                        goto next_match;
                    }
                }
                _VTE_DEBUG_IF(VTE_DEBUG_PARSE) {
                    if (c > 255) {
                        g_printerr("U+%04lx\n", (long) c);
                    } else {
                        if (c > 127) {
                            g_printerr("%ld = ", (long) c);
                        }
                        if (c < 32) {
                            g_printerr("^%c\n", c + 64);
                        } else {
                            g_printerr("`%c'\n", c);
                        }
                    }
                }

                bbox_topleft.x = MIN(bbox_topleft.x, m_screen->cursor.col);
                bbox_topleft.y = MIN(bbox_topleft.y, m_screen->cursor.row);

                /* Insert the character. */
                /* FIXMEchpe should not use UNLIKELY here */
                if (G_UNLIKELY(insert_char(c, false, false))) {
                    /* line wrapped, correct bbox */
                    if (invalidated_text &&
                        (m_screen->cursor.col > bbox_bottomright.x + VTE_CELL_BBOX_SLACK    ||
                         m_screen->cursor.col < bbox_topleft.x - VTE_CELL_BBOX_SLACK    ||
                         m_screen->cursor.row > bbox_bottomright.y + VTE_CELL_BBOX_SLACK    ||
                         m_screen->cursor.row < bbox_topleft.y - VTE_CELL_BBOX_SLACK)) {
                        /* Clip off any part of the box which isn't already on-screen. */
                        bbox_topleft.x = MAX(bbox_topleft.x, 0);
                        bbox_topleft.y = MAX(bbox_topleft.y, top_row);
                        bbox_bottomright.x = MIN(bbox_bottomright.x, m_column_count);
                        /* lazily apply the +1 to the cursor_row */
                        bbox_bottomright.y = MIN(bbox_bottomright.y + 1, bottom_row + 1);

                        invalidate_cells(bbox_topleft.x,
                                            bbox_bottomright.x - bbox_topleft.x,
                                            bbox_topleft.y,
                                            bbox_bottomright.y - bbox_topleft.y);
                        bbox_bottomright.x = bbox_bottomright.y = -G_MAXINT;
                        bbox_topleft.x = bbox_topleft.y = G_MAXINT;
                    }
                    bbox_topleft.x = MIN(bbox_topleft.x, 0);
                    bbox_topleft.y = MIN(bbox_topleft.y, m_screen->cursor.row);
                }
               /* Add the cells over which we have moved to the region
                * which we need to refresh for the user. */
                bbox_bottomright.x = MAX(bbox_bottomright.x, m_screen->cursor.col);
               /* cursor.row + 1 (defer until inv.) */
                bbox_bottomright.y = MAX(bbox_bottomright.y, m_screen->cursor.row);
                invalidated_text = TRUE;

                /* We *don't* emit flush pending signals here. */
                modified = TRUE;
                start++;

                break;
            }
            case VTE_MATCHER_RESULT_PARTIAL: {
               /* Case three: the read broke in the middle of a
                * control sequence, so we're undecided with no more
                * data to consult. If we have data following the
                * middle of the sequence, then it's just garbage data,
                * and for compatibility, we should discard it. */
                if (wbuf + wcount > next) {
                    _vte_debug_print(VTE_DEBUG_PARSE,
                                        "Invalid control "
                                        "sequence, discarding %ld "
                                        "characters.\n",
                                        (long)(next - (wbuf + start)));
                    /* Discard. */
                    start = next - wbuf + 1;
                } else {
                    /* Pause processing here and wait for more
                    * data before continuing. */
                    leftovers = TRUE;
                }

                break;
            }
        }

#ifdef VTE_DEBUG
        /* Some safety checks: ensure the visible parts of the buffer
         * are all in the buffer. */
        g_assert_cmpint(m_screen->insert_delta, >=, _vte_ring_delta(m_screen->row_data));

        /* The cursor shouldn't be above or below the addressable
         * part of the display buffer. */
        g_assert_cmpint(m_screen->cursor.row, >=, m_screen->insert_delta);
#endif

next_match:
        /* Free any parameters we don't care about any more. */
        params.recycle(m_matcher);
    }

    /* Remove most of the processed characters. */
    if (start < wcount) {
        g_array_remove_range(m_pending, 0, start);
    } else {
        g_array_set_size(m_pending, 0);
        /* If we're out of data, we needn't pause to let the
         * controlling application respond to incoming data, because
         * the main loop is already going to do that. */
    }

    if (modified) {
        /* Keep the cursor on-screen if we scroll on output, or if
         * we're currently at the bottom of the buffer. */
        update_insert_delta();
        if (m_scroll_on_output || bottom) {
            maybe_scroll_to_bottom();
        }
        /* Deselect the current selection if its contents are changed
         * by this insertion. */
        if (m_has_selection) {
            /* FIXMEchpe: this is atrocious */
            auto selection = get_selected_text();
            if ((selection == nullptr) ||
                (m_selection[VTE_SELECTION_PRIMARY] == nullptr) ||
                (strcmp(selection->str, m_selection[VTE_SELECTION_PRIMARY]->str) != 0)) {
                deselect_all();
            }
            if (selection) {
                g_string_free(selection, TRUE);
            }
        }
    }

    if (modified || (m_screen != previous_screen)) {
        /* Signal that the visible contents changed. */
        queue_contents_changed();
    }

    emit_pending_signals();

    if (invalidated_text) {
        /* Clip off any part of the box which isn't already on-screen. */
        bbox_topleft.x = MAX(bbox_topleft.x, 0);
        bbox_topleft.y = MAX(bbox_topleft.y, top_row);
        bbox_bottomright.x = MIN(bbox_bottomright.x, m_column_count);
        /* lazily apply the +1 to the cursor_row */
        bbox_bottomright.y = MIN(bbox_bottomright.y + 1, bottom_row + 1);

        invalidate_cells(bbox_topleft.x,
                            bbox_bottomright.x - bbox_topleft.x,
                            bbox_topleft.y,
                            bbox_bottomright.y - bbox_topleft.y);
    }

    /* FIXMEchpe: also need to take into account if the number of columns the cursor
     * occupies has changed due to the cell it's on being changed... */
    if ((saved_cursor.col != m_screen->cursor.col) ||
        (saved_cursor.row != m_screen->cursor.row)) {
        /* invalidate the old and new cursor positions */
        if (saved_cursor_visible) {
            invalidate_cell(saved_cursor.col, saved_cursor.row);
        }
        invalidate_cursor_once();
        check_cursor_blink();
        /* Signal that the cursor moved. */
        queue_cursor_moved();
    } else if ((saved_cursor_visible != m_cursor_visible) ||
                (saved_cursor_style != m_cursor_style)) {
        invalidate_cell(saved_cursor.col, saved_cursor.row);
        check_cursor_blink();
    }

    /* Tell the input method where the cursor is. */
    im_update_cursor();

    /* After processing some data, do a hyperlink GC. The multiplier is totally arbitrary, feel free to fine tune. */
    _vte_ring_hyperlink_maybe_gc(m_screen->row_data, wcount * 4);

    _vte_debug_print (VTE_DEBUG_WORK, ")");
    _vte_debug_print (VTE_DEBUG_IO,
            "%ld chars and %ld bytes in %" G_GSIZE_FORMAT " chunks left to process.\n",
            (long) unichars->len,
            (long) _vte_incoming_chunks_length(m_incoming),
            _vte_incoming_chunks_count(m_incoming));
}

void VteTerminalPrivate::feed_chunks(struct _vte_incoming_chunk *chunks)
{
    struct _vte_incoming_chunk *last;

    _vte_debug_print(VTE_DEBUG_IO, "Feed %" G_GSIZE_FORMAT " bytes, in %" G_GSIZE_FORMAT " chunks.\n",
            _vte_incoming_chunks_length(chunks),
            _vte_incoming_chunks_count(chunks));

    for (last = chunks; last->next != NULL; last = last->next) ;
    last->next = m_incoming;
    m_incoming = chunks;
}

/*
 * VteTerminalPrivate::feed:
 * @data: (array length=length) (element-type guint8): a string in the terminal's current encoding
 * @length: the length of the string, or -1 to use the full length or a nul-terminated string
 *
 * Interprets @data as if it were data received from a child process.  This
 * can either be used to drive the terminal without a child process, or just
 * to mess with your users.
 */
void VteTerminalPrivate::feed(char const* data,
                                gssize length,
                                bool start_processing_)
{
    g_assert(length == 0 || data != nullptr);

    if (length == -1) {
        length = strlen(data);
    }

    /* If we have data, modify the incoming buffer. */
    if (length > 0) {
        struct _vte_incoming_chunk *chunk;
        if (m_incoming &&
                (gsize)length < sizeof (m_incoming->data) - m_incoming->len) {
            chunk = m_incoming;
        } else {
            chunk = get_chunk ();
            feed_chunks(chunk);
        }
        do { /* break the incoming data into chunks */
            gsize rem = sizeof (chunk->data) - chunk->len;
            gsize len = (gsize) length < rem ? (gsize) length : rem;
            memcpy (chunk->data + chunk->len, data, len);
            chunk->len += len;
            length -= len;
            if (length == 0) {
                break;
            }
            data += len;

            chunk = get_chunk ();
            feed_chunks(chunk);
        } while (1);

        if (start_processing_) {
            start_processing();
        }
    }
}

#if 0 /* FIXME: remove */
bool
VteTerminalPrivate::pty_io_write(GIOChannel *channel,
                                 GIOCondition condition)
{
    gssize count;
    int fd;
    gboolean leave_open;

    fd = g_io_channel_unix_get_fd(channel);

    count = write(fd, m_outgoing->data,
              _vte_byte_array_length(m_outgoing));
    if (count != -1) {
        _VTE_DEBUG_IF (VTE_DEBUG_IO) {
            gssize i;
            for (i = 0; i < count; i++) {
                g_printerr("Wrote %c%c\n",
                    ((guint8)m_outgoing->data[i]) >= 32 ?
                    ' ' : '^',
                    ((guint8)m_outgoing->data[i]) >= 32 ?
                    m_outgoing->data[i] :
                    ((guint8)m_outgoing->data[i])  + 64);
            }
        }
        _vte_byte_array_consume(m_outgoing, count);
    }

    if (_vte_byte_array_length(m_outgoing) == 0) {
        leave_open = FALSE;
    } else {
        leave_open = TRUE;
    }

    return leave_open;
}
#endif

void VteTerminalPrivate::im_commit(char const* text)
{
    _vte_debug_print(VTE_DEBUG_EVENTS,
            "Input method committed `%s'.\n", text);

    /* feed_child_using_modes(text, -1); */ /* FIXME: removed */

    /* Committed text was committed because the user pressed a key, so
     * we need to obey the scroll-on-keystroke setting. */
    if (m_scroll_on_keystroke) {
        maybe_scroll_to_bottom();
    }
}

/* We've started pre-editing. */
static void vte_terminal_im_preedit_start_cb(GtkIMContext *im_context,
                                                VteTerminalPrivate *that)
{
    that->im_preedit_start();
}

void VteTerminalPrivate::im_preedit_start()
{
    _vte_debug_print(VTE_DEBUG_EVENTS, "Input method pre-edit started.\n");
    m_im_preedit_active = true;
}

/* We've stopped pre-editing. */
static void vte_terminal_im_preedit_end_cb(GtkIMContext *im_context,
                                            VteTerminalPrivate *that)
{
    that->im_preedit_end();
}

void VteTerminalPrivate::im_preedit_end()
{
    _vte_debug_print(VTE_DEBUG_EVENTS, "Input method pre-edit ended.\n");
    m_im_preedit_active = false;
}

/* The pre-edit string changed. */
static void vte_terminal_im_preedit_changed_cb(GtkIMContext *im_context,
                                                VteTerminalPrivate *that)
{
    that->im_preedit_changed();
}

void VteTerminalPrivate::im_preedit_changed()
{
    gchar *str;
    PangoAttrList *attrs;
    gint cursorpos;

    gtk_im_context_get_preedit_string(m_im_context, &str, &attrs, &cursorpos);
    _vte_debug_print(VTE_DEBUG_EVENTS, "Input method pre-edit changed (%s,%d).\n", str, cursorpos);

    /* Queue the area where the current preedit string is being displayed
     * for repainting. */
    invalidate_cursor_once();

    g_free(m_im_preedit);
    m_im_preedit = str;

    if (m_im_preedit_attrs != NULL) {
        pango_attr_list_unref(m_im_preedit_attrs);
    }
    m_im_preedit_attrs = attrs;

    m_im_preedit_cursor = cursorpos;

    /* Invalidate again with the new cursor position */
    invalidate_cursor_once();

    /* And tell the input method where the cursor is on the screen */
    im_update_cursor();
}

static gboolean vte_terminal_im_retrieve_surrounding_cb(GtkIMContext *im_context,
                                                        VteTerminalPrivate *that)
{
    return that->im_retrieve_surrounding();
}

bool VteTerminalPrivate::im_retrieve_surrounding()
{
    /* FIXME: implement this! Bug #726191 */
    _vte_debug_print(VTE_DEBUG_EVENTS, "Input method retrieve-surrounding.\n");
    return false;
}

static gboolean vte_terminal_im_delete_surrounding_cb(GtkIMContext *im_context,
                                                        int offset,
                                                        int n_chars,
                                                        VteTerminalPrivate *that)
{
    return that->im_delete_surrounding(offset, n_chars);
}

bool VteTerminalPrivate::im_delete_surrounding(int offset, int n_chars)
{
    /* FIXME: implement this! Bug #726191 */
    _vte_debug_print(VTE_DEBUG_EVENTS,
                        "Input method delete-surrounding offset %d n-chars %d.\n",
                        offset, n_chars);
    return false;
}

void VteTerminalPrivate::im_update_cursor()
{
    if (!widget_realized()) {
        return;
    }

    cairo_rectangle_int_t rect;
    rect.x = m_screen->cursor.col * m_cell_width + m_padding.left +
                get_preedit_width(false) * m_cell_width;
    rect.width = m_cell_width; /* FIXMEchpe: if columns > 1 ? */
    rect.y = row_to_pixel(m_screen->cursor.row) + m_padding.top;
    rect.height = m_cell_height;
    gtk_im_context_set_cursor_location(m_im_context, &rect);
}

void VteTerminalPrivate::widget_style_updated()
{
    set_font_desc(m_unscaled_font_desc);

    auto context = gtk_widget_get_style_context(m_widget);
    GtkBorder new_padding;
    gtk_style_context_get_padding(context, gtk_style_context_get_state(context),
                                    &new_padding);
    if (memcmp(&new_padding, &m_padding, sizeof(GtkBorder)) != 0) {
        _vte_debug_print(VTE_DEBUG_MISC | VTE_DEBUG_WIDGET_SIZE,
                            "Setting padding to (%d,%d,%d,%d)\n",
                            new_padding.left, new_padding.right,
                            new_padding.top, new_padding.bottom);

        m_padding = new_padding;
        update_view_extents();
        gtk_widget_queue_resize(m_widget);
    } else {
        _vte_debug_print(VTE_DEBUG_MISC | VTE_DEBUG_WIDGET_SIZE,
                            "Keeping padding the same at (%d,%d,%d,%d)\n",
                            new_padding.left, new_padding.right,
                            new_padding.top, new_padding.bottom);

    }

    float aspect;
    gtk_widget_style_get(m_widget, "cursor-aspect-ratio", &aspect, nullptr);
    if (!_vte_double_equal(aspect, m_cursor_aspect_ratio)) {
        m_cursor_aspect_ratio = aspect;
        invalidate_cursor_once();
    }

}

void VteTerminalPrivate::add_cursor_timeout()
{
    if (m_cursor_blink_tag) {
        return; /* already added */
    }

    m_cursor_blink_time = 0;
    m_cursor_blink_tag = g_timeout_add_full(G_PRIORITY_LOW,
                                                m_cursor_blink_cycle,
                                                (GSourceFunc)invalidate_cursor_periodic_cb,
                                                this,
                                                NULL);
}

void VteTerminalPrivate::remove_cursor_timeout()
{
    if (m_cursor_blink_tag == 0) {
        return; /* already removed */
    }

    g_source_remove(m_cursor_blink_tag);
    m_cursor_blink_tag = 0;
    if (!m_cursor_blink_state) {
        invalidate_cursor_once();
        m_cursor_blink_state = true;
    }
}

/* Activates / disactivates the cursor blink timer to reduce wakeups */
void VteTerminalPrivate::check_cursor_blink()
{
    if (m_has_focus &&
        m_cursor_blinks &&
        m_cursor_visible) {
        add_cursor_timeout();
    } else {
        remove_cursor_timeout();
    }
}

void VteTerminalPrivate::remove_text_blink_timeout()
{
    if (m_text_blink_tag == 0) {
        return;
    }

    g_source_remove (m_text_blink_tag);
    m_text_blink_tag = 0;
}

void VteTerminalPrivate::beep()
{
    if (m_audible_bell) {
        GdkWindow *window = gtk_widget_get_window(m_widget);
        gdk_window_beep(window);
    }
}

guint VteTerminalPrivate::translate_ctrlkey(GdkEventKey *event)
{
    guint keyval;
    GdkKeymap *keymap;
    unsigned int i;

    if (event->keyval < 128) {
        return event->keyval;
    }
    keymap = gdk_keymap_get_for_display(gdk_window_get_display (event->window));

    /* Try groups in order to find one mapping the key to ASCII */
    for (i = 0; i < 4; i++) {
        GdkModifierType consumed_modifiers;

        gdk_keymap_translate_keyboard_state (keymap,
                                                event->hardware_keycode,
                                                (GdkModifierType)event->state,
                                                i,
                                                &keyval, NULL, NULL, &consumed_modifiers);
        if (keyval < 128) {
            _vte_debug_print (VTE_DEBUG_EVENTS,
                    "ctrl+Key, group=%d de-grouped into keyval=0x%x\n",
                    event->group, keyval);
            return keyval;
        }
    }

    return event->keyval;
}

void VteTerminalPrivate::read_modifiers(GdkEvent *event)
{
    GdkKeymap *keymap;
    GdkModifierType mods;
    guint mask;

    /* Read the modifiers. */
    if (!gdk_event_get_state((GdkEvent*)event, &mods)) {
        return;
    }

    keymap = gdk_keymap_get_for_display(gdk_window_get_display(((GdkEventAny*)event)->window));

    gdk_keymap_add_virtual_modifiers (keymap, &mods);

    mask = (guint)mods;
#if 1
    /* HACK! Treat ALT as META; see bug #663779. */
    if (mask & GDK_MOD1_MASK) {
            mask |= VTE_META_MASK;
    }
#endif

    m_modifiers = mask;
}

bool VteTerminalPrivate::widget_key_press(GdkEventKey *event)
{
    char *normal = NULL;
    gssize normal_length = 0;
    int i;
    gboolean scrolled = FALSE, steal = FALSE, modifier = FALSE, handled,
         /*suppress_meta_esc = FALSE,*/ add_modifiers = FALSE;
    guint keyval = 0;
    gunichar keychar = 0;
    char keybuf[VTE_UTF8_BPC];

    /* If it's a keypress, record that we got the event, in case the
     * input method takes the event from us. */
    if (event->type == GDK_KEY_PRESS) {
        /* Store a copy of the key. */
        keyval = event->keyval;
        read_modifiers((GdkEvent*)event);

        /* FIXMEchpe? */
        if (m_cursor_blink_tag != 0) {
            remove_cursor_timeout();
            add_cursor_timeout();
        }

        /* Determine if this is just a modifier key. */
        modifier = _vte_keymap_key_is_modifier(keyval);

        /* Unless it's a modifier key, hide the pointer. */
        if (!modifier) {
            set_pointer_autohidden(true);
        }

        _vte_debug_print(VTE_DEBUG_EVENTS,
                "Keypress, modifiers=0x%x, "
                "keyval=0x%x, raw string=`%s'.\n",
                m_modifiers,
                keyval, event->string);

        /* We steal many keypad keys here. */
        if (!m_im_preedit_active) {
            switch (keyval) {
                case GDK_KEY_KP_Add:
                case GDK_KEY_KP_Subtract:
                case GDK_KEY_KP_Multiply:
                case GDK_KEY_KP_Divide:
                case GDK_KEY_KP_Enter:
                    steal = TRUE;
                    break;
                default:
                    break;
            }
            if (m_modifiers & VTE_META_MASK) {
                steal = TRUE;
            }
            switch (keyval) {
                case GDK_KEY_ISO_Lock:
                case GDK_KEY_ISO_Level2_Latch:
                case GDK_KEY_ISO_Level3_Shift:
                case GDK_KEY_ISO_Level3_Latch:
                case GDK_KEY_ISO_Level3_Lock:
                case GDK_KEY_ISO_Level5_Shift:
                case GDK_KEY_ISO_Level5_Latch:
                case GDK_KEY_ISO_Level5_Lock:
                case GDK_KEY_ISO_Group_Shift:
                case GDK_KEY_ISO_Group_Latch:
                case GDK_KEY_ISO_Group_Lock:
                case GDK_KEY_ISO_Next_Group:
                case GDK_KEY_ISO_Next_Group_Lock:
                case GDK_KEY_ISO_Prev_Group:
                case GDK_KEY_ISO_Prev_Group_Lock:
                case GDK_KEY_ISO_First_Group:
                case GDK_KEY_ISO_First_Group_Lock:
                case GDK_KEY_ISO_Last_Group:
                case GDK_KEY_ISO_Last_Group_Lock:
                case GDK_KEY_Multi_key:
                case GDK_KEY_Codeinput:
                case GDK_KEY_SingleCandidate:
                case GDK_KEY_MultipleCandidate:
                case GDK_KEY_PreviousCandidate:
                case GDK_KEY_Kanji:
                case GDK_KEY_Muhenkan:
                case GDK_KEY_Henkan_Mode:
                /* case GDK_KEY_Henkan: is GDK_KEY_Henkan_Mode */
                case GDK_KEY_Romaji:
                case GDK_KEY_Hiragana:
                case GDK_KEY_Katakana:
                case GDK_KEY_Hiragana_Katakana:
                case GDK_KEY_Zenkaku:
                case GDK_KEY_Hankaku:
                case GDK_KEY_Zenkaku_Hankaku:
                case GDK_KEY_Touroku:
                case GDK_KEY_Massyo:
                case GDK_KEY_Kana_Lock:
                case GDK_KEY_Kana_Shift:
                case GDK_KEY_Eisu_Shift:
                case GDK_KEY_Eisu_toggle:
                /* case GDK_KEY_Kanji_Bangou: is GDK_KEY_Codeinput */
                /* case GDK_KEY_Zen_Koho: is GDK_KEY_MultipleCandidate */
                /* case GDK_KEY_Mae_Koho: is GDK_KEY_PreviousCandidate */
                /* case GDK_KEY_kana_switch: is GDK_KEY_ISO_Group_Shift */
                case GDK_KEY_Hangul:
                case GDK_KEY_Hangul_Start:
                case GDK_KEY_Hangul_End:
                case GDK_KEY_Hangul_Hanja:
                case GDK_KEY_Hangul_Jamo:
                case GDK_KEY_Hangul_Romaja:
                /* case GDK_KEY_Hangul_Codeinput: is GDK_KEY_Codeinput */
                case GDK_KEY_Hangul_Jeonja:
                case GDK_KEY_Hangul_Banja:
                case GDK_KEY_Hangul_PreHanja:
                case GDK_KEY_Hangul_PostHanja:
                /* case GDK_KEY_Hangul_SingleCandidate: is GDK_KEY_SingleCandidate */
                /* case GDK_KEY_Hangul_MultipleCandidate: is GDK_KEY_MultipleCandidate */
                /* case GDK_KEY_Hangul_PreviousCandidate: is GDK_KEY_PreviousCandidate */
                case GDK_KEY_Hangul_Special:
                /* case GDK_KEY_Hangul_switch: is GDK_KEY_ISO_Group_Shift */

                    steal = FALSE;
                    break;
                default:
                    break;
            }
        }
    }

    /* Let the input method at this one first. */
    if (!steal && m_input_enabled) {
        if (m_im_context && gtk_im_context_filter_keypress(m_im_context, event)) {
            _vte_debug_print(VTE_DEBUG_EVENTS, "Keypress taken by IM.\n");
            return true;
        }
    }

    /* Now figure out what to send to the child. */
    if ((event->type == GDK_KEY_PRESS) && !modifier) {
        handled = FALSE;
        /* Map the key to a sequence name if we can. */
        switch (keyval) {
            case GDK_KEY_BackSpace:
                switch (m_backspace_binding) {
                    case VTE_ERASE_ASCII_BACKSPACE:
                        normal = g_strdup("");
                        normal_length = 1;
                        /*suppress_meta_esc = FALSE;*/
                        break;
                    case VTE_ERASE_ASCII_DELETE:
                        normal = g_strdup("");
                        normal_length = 1;
                        /*suppress_meta_esc = FALSE;*/
                        break;
                    case VTE_ERASE_DELETE_SEQUENCE:
                        normal = g_strdup("\e[3~");
                        normal_length = 4;
                        add_modifiers = TRUE;
                        /*suppress_meta_esc = TRUE;*/
                        break;
                    case VTE_ERASE_TTY:
#if 0
                        if (m_pty != nullptr &&
                            tcgetattr(vte_pty_get_fd(m_pty), &tio) != -1)
                        {
                            normal = g_strdup_printf("%c", tio.c_cc[VERASE]);
                            normal_length = 1;
                        }
                        suppress_meta_esc = FALSE;
#endif
                        break;
                    case VTE_ERASE_AUTO:
                    default:
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE '\0'
#endif

#if 0
                        if (m_pty != nullptr &&
                            tcgetattr(vte_pty_get_fd(m_pty), &tio) != -1 &&
                            tio.c_cc[VERASE] != _POSIX_VDISABLE)
                        {
                            normal = g_strdup_printf("%c", tio.c_cc[VERASE]);
                            normal_length = 1;
                        }
                        else
#endif
                        {
                            normal = g_strdup("");
                            normal_length = 1;
                            /*suppress_meta_esc = FALSE;*/
                        }
                        /*suppress_meta_esc = FALSE;*/
                        break;
                }
                /* Toggle ^H vs ^? if Ctrl is pressed */
                if (normal_length == 1 && m_modifiers & GDK_CONTROL_MASK) {
                    if (normal[0] == '\010') {
                        normal[0] = '\177';
                    } else if (normal[0] == '\177') {
                        normal[0] = '\010';
                    }
                }
                handled = TRUE;
                break;
            case GDK_KEY_KP_Delete:
            case GDK_KEY_Delete:
                switch (m_delete_binding) {
                    case VTE_ERASE_ASCII_BACKSPACE:
                        normal = g_strdup("\010");
                        normal_length = 1;
                        break;
                    case VTE_ERASE_ASCII_DELETE:
                        normal = g_strdup("\177");
                        normal_length = 1;
                        break;
                    case VTE_ERASE_TTY:
#if 0
                        if (m_pty != nullptr &&
                            tcgetattr(vte_pty_get_fd(m_pty), &tio) != -1)
                        {
                            normal = g_strdup_printf("%c", tio.c_cc[VERASE]);
                            normal_length = 1;
                        }
#endif
                        /*suppress_meta_esc = FALSE;*/
                        break;
                    case VTE_ERASE_DELETE_SEQUENCE:
                    case VTE_ERASE_AUTO:
                        default:
                            normal = g_strdup("\e[3~");
                            normal_length = 4;
                            add_modifiers = TRUE;
                            break;
                }
                handled = TRUE;
                /* FIXMEchpe: why? this overrides the FALSE set above? */
                /*suppress_meta_esc = TRUE;*/
                break;
            case GDK_KEY_KP_Insert:
            case GDK_KEY_Insert:
                if (m_modifiers & GDK_SHIFT_MASK) {
                    if (m_modifiers & GDK_CONTROL_MASK) {
                        emit_paste_clipboard();
                        handled = TRUE;
                        /*suppress_meta_esc = TRUE;*/
                    } else {
                        widget_paste(GDK_SELECTION_PRIMARY);
                        handled = TRUE;
                        /*suppress_meta_esc = TRUE;*/
                    }
                } else if (m_modifiers & GDK_CONTROL_MASK) {
                    emit_copy_clipboard();
                    handled = TRUE;
                    /*suppress_meta_esc = TRUE;*/
                }
                break;
            /* Keypad/motion keys. */
            case GDK_KEY_KP_Up:
            case GDK_KEY_Up:
                if (m_screen == &m_normal_screen &&
                    m_modifiers & GDK_CONTROL_MASK &&
                    m_modifiers & GDK_SHIFT_MASK) {
                    scroll_lines(-1);
                    scrolled = TRUE;
                    handled = TRUE;
                    /*suppress_meta_esc = TRUE;*/
                }
                break;
            case GDK_KEY_KP_Down:
            case GDK_KEY_Down:
                if (m_screen == &m_normal_screen &&
                    m_modifiers & GDK_CONTROL_MASK &&
                    m_modifiers & GDK_SHIFT_MASK) {
                    scroll_lines(1);
                    scrolled = TRUE;
                    handled = TRUE;
                    /*suppress_meta_esc = TRUE;*/
                }
                break;
            case GDK_KEY_KP_Page_Up:
            case GDK_KEY_Page_Up:
                if (m_screen == &m_normal_screen &&
                    m_modifiers & GDK_SHIFT_MASK) {
                    scroll_pages(-1);
                    scrolled = TRUE;
                    handled = TRUE;
                    /*suppress_meta_esc = TRUE;*/
                }
                break;
            case GDK_KEY_KP_Page_Down:
            case GDK_KEY_Page_Down:
                if (m_screen == &m_normal_screen &&
                    m_modifiers & GDK_SHIFT_MASK) {
                    scroll_pages(1);
                    scrolled = TRUE;
                    handled = TRUE;
                    /*suppress_meta_esc = TRUE;*/
                }
                break;
            case GDK_KEY_KP_Home:
            case GDK_KEY_Home:
                if (m_screen == &m_normal_screen &&
                    m_modifiers & GDK_SHIFT_MASK) {
                    maybe_scroll_to_top();
                    scrolled = TRUE;
                    handled = TRUE;
                }
                break;
            case GDK_KEY_KP_End:
            case GDK_KEY_End:
                if (m_screen == &m_normal_screen &&
                    m_modifiers & GDK_SHIFT_MASK) {
                    maybe_scroll_to_bottom();
                    scrolled = TRUE;
                    handled = TRUE;
                }
                break;
            /* Let Shift +/- tweak the font, like XTerm does. */
            case GDK_KEY_KP_Add:
            case GDK_KEY_KP_Subtract:
                if (m_modifiers & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) {
                    switch (keyval) {
                        case GDK_KEY_KP_Add:
                            emit_increase_font_size();
                            handled = TRUE;
                            /*suppress_meta_esc = TRUE;*/
                            break;
                        case GDK_KEY_KP_Subtract:
                            emit_decrease_font_size();
                            handled = TRUE;
                            /*suppress_meta_esc = TRUE;*/
                            break;
                    }
                }
                break;
            default:
                break;
        }
        /* If the above switch statement didn't do the job, try mapping
         * it to a literal or capability name. */
        if (handled == FALSE) {
            _vte_keymap_map(keyval, m_modifiers,
                    m_cursor_mode == VTE_KEYMODE_APPLICATION,
                    m_keypad_mode == VTE_KEYMODE_APPLICATION,
                    &normal,
                    &normal_length);
            /* If we found something this way, suppress
             * escape-on-meta. */
            if (normal != NULL && normal_length > 0) {
                /*suppress_meta_esc = TRUE;*/
            }
        }

        /* Shall we do this here or earlier?  See bug 375112 and bug 589557 */
        if (m_modifiers & GDK_CONTROL_MASK) {
            keyval = translate_ctrlkey(event);
        }

        /* If we didn't manage to do anything, try to salvage a
         * printable string. */
        if (handled == FALSE && normal == NULL) {

            /* Convert the keyval to a gunichar. */
            keychar = gdk_keyval_to_unicode(keyval);
            normal_length = 0;
            if (keychar != 0) {
                /* Convert the gunichar to a string. */
                normal_length = g_unichar_to_utf8(keychar, keybuf);
                if (normal_length != 0) {
                    normal = (char *)g_malloc(normal_length + 1);
                    memcpy(normal, keybuf, normal_length);
                    normal[normal_length] = '\0';
                } else {
                    normal = NULL;
                }
            }
            if ((normal != NULL) && (m_modifiers & GDK_CONTROL_MASK)) {
                /* Replace characters which have "control"
                 * counterparts with those counterparts. */
                for (i = 0; i < normal_length; i++) {
                    if ((((guint8)normal[i]) >= 0x40) &&
                        (((guint8)normal[i]) <  0x80)) {
                        normal[i] &= (~(0x60));
                    }
                }
            }
            _VTE_DEBUG_IF (VTE_DEBUG_EVENTS) {
                if (normal) g_printerr(
                        "Keypress, modifiers=0x%x, "
                        "keyval=0x%x, cooked string=`%s'.\n",
                        m_modifiers,
                        keyval, normal);
            }
        }
        /* If we got normal characters, send them to the child. */
        if (normal != NULL) {
            if (add_modifiers) {
                _vte_keymap_key_add_key_modifiers(keyval,
                                                    m_modifiers,
                                                    m_cursor_mode == VTE_KEYMODE_APPLICATION,
                                                    &normal,
                                                    &normal_length);
            }
#if 0
            if (m_meta_sends_escape &&
                !suppress_meta_esc &&
                (normal_length > 0) &&
                (m_modifiers & VTE_META_MASK)) {
                feed_child(_VTE_CAP_ESC, 1);
            }
#endif
#if 0
            if (normal_length > 0) {
                feed_child_using_modes(normal, normal_length);
            }
#endif
            g_free(normal);
        }
        /* Keep the cursor on-screen. */
        if (!scrolled && !modifier &&
            m_scroll_on_keystroke) {
            maybe_scroll_to_bottom();
        }
        return true;
    }
    return false;
}

bool VteTerminalPrivate::widget_key_release(GdkEventKey *event)
{
    read_modifiers((GdkEvent*)event);

    if (m_input_enabled &&
            m_im_context &&
            gtk_im_context_filter_keypress(m_im_context, event)) {
        return true;
    }

    return false;
}

static int compare_unichar_p(const void *u1p, const void *u2p)
{
    const gunichar u1 = *(gunichar*)u1p;
    const gunichar u2 = *(gunichar*)u2p;
    return u1 < u2 ? -1 : u1 > u2 ? 1 : 0;
}

static const guint8 word_char_by_category[] = {
    [G_UNICODE_CONTROL]             = 2,
    [G_UNICODE_FORMAT]              = 2,
    [G_UNICODE_UNASSIGNED]          = 2,
    [G_UNICODE_PRIVATE_USE]         = 0,
    [G_UNICODE_SURROGATE]           = 2,
    [G_UNICODE_LOWERCASE_LETTER]    = 1,
    [G_UNICODE_MODIFIER_LETTER]     = 1,
    [G_UNICODE_OTHER_LETTER]        = 1,
    [G_UNICODE_TITLECASE_LETTER]    = 1,
    [G_UNICODE_UPPERCASE_LETTER]    = 1,
    [G_UNICODE_SPACING_MARK]        = 0,
    [G_UNICODE_ENCLOSING_MARK]      = 0,
    [G_UNICODE_NON_SPACING_MARK]    = 0,
    [G_UNICODE_DECIMAL_NUMBER]      = 1,
    [G_UNICODE_LETTER_NUMBER]       = 1,
    [G_UNICODE_OTHER_NUMBER]        = 1,
    [G_UNICODE_CONNECT_PUNCTUATION] = 0,
    [G_UNICODE_DASH_PUNCTUATION]    = 0,
    [G_UNICODE_CLOSE_PUNCTUATION]   = 0,
    [G_UNICODE_FINAL_PUNCTUATION]   = 0,
    [G_UNICODE_INITIAL_PUNCTUATION] = 0,
    [G_UNICODE_OTHER_PUNCTUATION]   = 0,
    [G_UNICODE_OPEN_PUNCTUATION]    = 0,
    [G_UNICODE_CURRENCY_SYMBOL]     = 0,
    [G_UNICODE_MODIFIER_SYMBOL]     = 0,
    [G_UNICODE_MATH_SYMBOL]         = 0,
    [G_UNICODE_OTHER_SYMBOL]        = 0,
    [G_UNICODE_LINE_SEPARATOR]      = 2,
    [G_UNICODE_PARAGRAPH_SEPARATOR] = 2,
    [G_UNICODE_SPACE_SEPARATOR]     = 2,
};

/*
 * VteTerminalPrivate::is_word_char:
 * @c: a candidate Unicode code point
 *
 * Checks if a particular character is considered to be part of a word or not.
 *
 * Returns: %TRUE if the character is considered to be part of a word
 */
bool VteTerminalPrivate::is_word_char(gunichar c) const
{
    const guint8 v = word_char_by_category[g_unichar_type(c)];

    if (v) {
        return v == 1;
    }

    /* Do we have an exception? */
    return bsearch(&c,
                    m_word_char_exceptions,
                    m_word_char_exceptions_len,
                    sizeof(gunichar),
                    compare_unichar_p) != NULL;
}

/* Check if the characters in the two given locations are in the same class
 * (word vs. non-word characters). */
bool VteTerminalPrivate::is_same_class(vte::grid::column_t acol,
                                        vte::grid::row_t arow,
                                        vte::grid::column_t bcol,
                                        vte::grid::row_t brow) const
{
    VteCell const* pcell = nullptr;
    bool word_char;
    if ((pcell = find_charcell(acol, arow)) != nullptr && pcell->c != 0) {
        word_char = is_word_char(_vte_unistr_get_base(pcell->c));

        /* Lets not group non-wordchars together (bug #25290) */
        if (!word_char) {
            return false;
        }

        pcell = find_charcell(bcol, brow);
        if (pcell == NULL || pcell->c == 0) {
            return false;
        }
        if (word_char != is_word_char(_vte_unistr_get_base(pcell->c))) {
            return false;
        }
        return true;
    }
    return false;
}

/* Check if we soft-wrapped on the given line. */
/* FIXMEchpe replace this with a method on VteRing */
bool VteTerminalPrivate::line_is_wrappable(vte::grid::row_t row) const
{
    VteRowData const* rowdata = find_row_data(row);
    return rowdata && rowdata->attr.soft_wrapped;
}

/* Check if the given point is in the region between the two points */
static gboolean vte_cell_is_between(glong col, glong row,
                                glong acol, glong arow, glong bcol, glong brow)
{
    /* Negative between never allowed. */
    if ((arow > brow) || ((arow == brow) && (acol > bcol))) {
        return FALSE;
    }
    /* Degenerate span? */
    if ((row == arow) && (row == brow) && (col == acol) && (col == bcol)) {
        return TRUE;
    }
    /* A cell is between two points if it's on a line after the
     * specified area starts, or before the line where it ends,
     * or any of the lines in between. */
    if ((row > arow) && (row < brow)) {
        return TRUE;
    }
    /* It's also between the two points if they're on the same row
     * the cell lies between the start and end columns. */
    if ((row == arow) && (row == brow)) {
        if (col >= acol) {
            if (col < bcol) {
                return TRUE;
            } else {
                if (col == bcol) {
                    return TRUE;
                } else {
                    return FALSE;
                }
            }
        } else {
            return FALSE;
        }
    }
    /* It's also "between" if it's on the line where the area starts and
     * at or after the start column, or on the line where the area ends and
     * before the end column. */
    if ((row == arow) && (col >= acol)) {
        return TRUE;
    } else {
        if (row == brow) {
            if (col < bcol) {
                return TRUE;
            } else {
                if (col == bcol) {
                    return TRUE;
                } else {
                    return FALSE;
                }
            }
        } else {
            return FALSE;
        }
    }
    return FALSE;
}

/* Check if a cell is selected or not. */
/* FIXMEchpe: replace this by just using vte::grid::span for selection and then this simply becomes .contains() */
bool VteTerminalPrivate::cell_is_selected(vte::grid::column_t col,
                                            vte::grid::row_t row) const
{
    /* If there's nothing selected, it's an easy question to answer. */
    if (!m_has_selection) {
        return false;
    }

    /* If the selection is obviously bogus, then it's also very easy. */
    auto const& ss = m_selection_start;
    auto const& se = m_selection_end;
    if ((ss.row < 0) || (se.row < 0)) {
        return false;
    }

    /* Limit selection in block mode. */
    if (m_selection_block_mode) {
        if (col < ss.col || col > se.col) {
            return false;
        }
    }

    /* Now it boils down to whether or not the point is between the
     * begin and endpoint of the selection. */
    return vte_cell_is_between(col, row, ss.col, ss.row, se.col, se.row);
}

void VteTerminalPrivate::widget_paste_received(char const* text)
{
    gchar *paste, *p;
    gsize run;
    unsigned char c;

    if (text == nullptr) {
        return;
    }

    gsize len = strlen(text);
    _vte_debug_print(VTE_DEBUG_SELECTION,
                        "Pasting %" G_GSIZE_FORMAT " UTF-8 bytes.\n", len);
    /* FIXMEchpe this cannot happen ever */
    if (!g_utf8_validate(text, len, NULL)) {
        g_warning("Paste not valid UTF-8, dropping.");
        return;
    }

   /* Convert newlines to carriage returns, which more software
    * is able to cope with (cough, pico, cough).
    * Filter out control chars except HT, CR (even stricter than xterm).
    * Also filter out C1 controls: U+0080 (0xC2 0x80) - U+009F (0xC2 0x9F). */
    p = paste = (gchar *) g_malloc(len + 1);
    while (p != nullptr && text[0] != '\0') {
        run = strcspn(text, "\x01\x02\x03\x04\x05\x06\x07"
                        "\x08\x0A\x0B\x0C\x0E\x0F"
                        "\x10\x11\x12\x13\x14\x15\x16\x17"
                        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
                        "\x7F\xC2");
        memcpy(p, text, run);
        p += run;
        text += run;
        switch (text[0]) {
            case '\x00':
                break;
            case '\x0A':
                *p = '\x0D';
                p++;
                text++;
                break;
            case '\xC2':
                c = text[1];
                if (c >= 0x80 && c <= 0x9F) {
                    /* Skip both bytes of a C1 */
                    text += 2;
                } else {
                    /* Move along, nothing to see here */
                    *p = '\xC2';
                    p++;
                    text++;
                }
                break;
            default:
                /* Swallow this byte */
                text++;
                break;
        }
    }
#if 0
    if (m_bracketed_paste_mode) {
        feed_child("\e[200~", -1);
    }
    /* FIXMEchpe add a way to avoid the extra string copy done here */
    feed_child(paste, p - paste);
    if (m_bracketed_paste_mode) {
            feed_child("\e[201~", -1);
    }
#endif
    g_free(paste);
}

bool VteTerminalPrivate::feed_mouse_event(vte::grid::coords const& rowcol /* confined */,
                                            int button,
                                            bool is_drag,
                                            bool is_release)
{
    unsigned char cb = 0;

    char buf[LINE_MAX];
    /*gint len = 0;*/

    /* Don't send events on scrollback contents: bug 755187. */
    if (grid_coords_in_scrollback(rowcol)) {
        return false;
    }

    /* Make coordinates 1-based. */
    auto cx = rowcol.column() + 1;
    auto cy = rowcol.row() - m_screen->insert_delta + 1;

    /* Encode the button information in cb. */
    switch (button) {
        case 0:                 /* No button, just dragging. */
            cb = 3;
            break;
        case 1:         /* Left. */
            cb = 0;
            break;
        case 2:         /* Middle. */
            cb = 1;
            break;
        case 3:         /* Right. */
            cb = 2;
            break;
        case 4:
            cb = 64;    /* Scroll up. */
            break;
        case 5:
            cb = 65;    /* Scroll down. */
            break;
    }

    /* With the exception of the 1006 mode, button release is also encoded here. */
    /* Note that if multiple extensions are enabled, the 1006 is used, so it's okay to check for only that. */
    if (is_release && !m_mouse_xterm_extension) {
        cb = 3;
    }

    /* Encode the modifiers. */
    if (m_modifiers & GDK_SHIFT_MASK) {
        cb |= 4;
    }
    if (m_modifiers & VTE_META_MASK) {
        cb |= 8;
    }
    if (m_modifiers & GDK_CONTROL_MASK) {
        cb |= 16;
    }

    /* Encode a drag event. */
    if (is_drag) {
        cb |= 32;
    }

    /* Check the extensions in decreasing order of preference. Encoding the release event above assumes that 1006 comes first. */
    if (m_mouse_xterm_extension) {
        /* xterm's extended mode (1006) */
        /*len =*/ g_snprintf(buf, sizeof(buf), _VTE_CAP_CSI "<%d;%ld;%ld%c", cb, cx, cy, is_release ? 'm' : 'M');
    } else if (m_mouse_urxvt_extension) {
        /* urxvt's extended mode (1015) */
        /*len =*/ g_snprintf(buf, sizeof(buf), _VTE_CAP_CSI "%d;%ld;%ldM", 32 + cb, cx, cy);
    } else if (cx <= 231 && cy <= 231) {
        /* legacy mode */
        /*len =*/ g_snprintf(buf, sizeof(buf), _VTE_CAP_CSI "M%c%c%c", 32 + cb, 32 + (guchar)cx, 32 + (guchar)cy);
    }
#if 0
    /* Send event direct to the child, this is binary not text data */
    feed_child_binary((guint8*) buf, len);
#endif
        return true;
}

void VteTerminalPrivate::feed_focus_event(bool in)
{
#if 0
    char buf[8];
    gsize len;

    len = g_snprintf(buf, sizeof(buf), _VTE_CAP_CSI "%c", in ? 'I' : 'O');
    feed_child_binary((guint8 *)buf, len);
#endif
}

void VteTerminalPrivate::feed_focus_event_initial()
{
    /* We immediately send the terminal a focus event, since otherwise
        * it has no way to know the current status.
        */
    feed_focus_event(gtk_widget_has_focus(m_widget));
}

void VteTerminalPrivate::maybe_feed_focus_event(bool in)
{
    if (m_focus_tracking_mode) {
            feed_focus_event(in);
    }
}

/*
 * VteTerminalPrivate::maybe_send_mouse_button:
 * @terminal:
 * @event:
 *
 * Sends a mouse button click or release notification to the application,
 * if the terminal is in mouse tracking mode.
 *
 * Returns: %TRUE iff the event was consumed
 */
bool VteTerminalPrivate::maybe_send_mouse_button(vte::grid::coords const& unconfined_rowcol,
                                                    GdkEventType event_type,
                                                    int event_button)
{
    switch (event_type) {
        case GDK_BUTTON_PRESS:
            if (m_mouse_tracking_mode < MOUSE_TRACKING_SEND_XY_ON_CLICK) {
                return false;
            }
            break;
        case GDK_BUTTON_RELEASE: 
        {
            if (m_mouse_tracking_mode < MOUSE_TRACKING_SEND_XY_ON_BUTTON) {
                return false;
            }
            break;
        }
        default:
            return false;
            break;
    }

    auto rowcol = confine_grid_coords(unconfined_rowcol);
    return feed_mouse_event(rowcol, event_button, false /* not drag */, event_type == GDK_BUTTON_RELEASE);
}

/*
 * VteTerminalPrivate::maybe_send_mouse_drag:
 * @terminal:
 * @event:
 *
 * Sends a mouse motion notification to the application,
 * if the terminal is in mouse tracking mode.
 *
 * Returns: %TRUE iff the event was consumed
 */
bool VteTerminalPrivate::maybe_send_mouse_drag(vte::grid::coords const& unconfined_rowcol,
                                                GdkEventType event_type)
{
    auto rowcol = confine_grid_coords(unconfined_rowcol);

    /* First determine if we even want to send notification. */
    switch (event_type) {
        case GDK_MOTION_NOTIFY:
            if (m_mouse_tracking_mode < MOUSE_TRACKING_CELL_MOTION_TRACKING) {
                return false;
            }

            if (m_mouse_tracking_mode < MOUSE_TRACKING_ALL_MOTION_TRACKING) {

                if (m_mouse_pressed_buttons == 0) {
                    return false;
                }
               /* The xterm doc is not clear as to whether
                * all-tracking also sends degenerate same-cell events;
                * we don't.
                */
                if (rowcol == confined_grid_coords_from_view_coords(m_mouse_last_position)) {
                    return false;
                }
            }
            break;
        default:
            return false;
            break;
    }

    /* As per xterm, report the leftmost pressed button - if any. */
    int button;
    if (m_mouse_pressed_buttons & 1) {
        button = 1;
    } else if (m_mouse_pressed_buttons & 2) {
        button = 2;
    } else if (m_mouse_pressed_buttons & 4) {
        button = 3;
    } else {
        button = 0;
    }

    return feed_mouse_event(rowcol, button, true /* drag */, false /* not release */);
}

/*
 * VteTerminalPrivate::hyperlink_invalidate_and_get_bbox
 *
 * Invalidates cells belonging to the non-zero hyperlink idx, in order to
 * stop highlighting the previously hovered hyperlink or start highlighting
 * the new one. Optionally stores the coordinates of the bounding box.
 */
void VteTerminalPrivate::hyperlink_invalidate_and_get_bbox(hyperlink_idx_t idx, GdkRectangle *bbox)
{
    auto first_row = first_displayed_row();
    auto end_row = last_displayed_row() + 1;
    vte::grid::row_t row, top = LONG_MAX, bottom = -1;
    vte::grid::column_t col, left = LONG_MAX, right = -1;
    const VteRowData *rowdata;

    g_assert (idx != 0);

    for (row = first_row; row < end_row; row++) {
        rowdata = _vte_ring_index(m_screen->row_data, row);
        if (rowdata != NULL) {
            for (col = 0; col < rowdata->len; col++) {
                if (G_UNLIKELY (rowdata->cells[col].attr.hyperlink_idx == idx)) {
                    invalidate_cells(col, 1, row, 1);
                    top = MIN(top, row);
                    bottom = MAX(bottom, row);
                    left = MIN(left, col);
                    right = MAX(right, col);
                }
            }
        }
    }

    if (bbox == NULL) {
        return;
    }

    /* If bbox != NULL, we're looking for the new hovered hyperlink which always has onscreen bits. */
    g_assert (top != LONG_MAX && bottom != -1 && left != LONG_MAX && right != -1);

    auto allocation = get_allocated_rect();
    bbox->x = allocation.x + m_padding.left + left * m_cell_width;
    bbox->y = allocation.y + m_padding.top + row_to_pixel(top);
    bbox->width = (right - left + 1) * m_cell_width;
    bbox->height = (bottom - top + 1) * m_cell_height;
    _vte_debug_print (VTE_DEBUG_HYPERLINK,
                        "Hyperlink bounding box: x=%d y=%d w=%d h=%d\n",
                        bbox->x, bbox->y, bbox->width, bbox->height);
}

/*
 * VteTerminalPrivate::hyperlink_hilite_update:
 *
 * Checks the coordinates for hyperlink. Updates m_hyperlink_hover_idx
 * and m_hyperlink_hover_uri, and schedules to update the highlighting.
 */
void VteTerminalPrivate::hyperlink_hilite_update()
{
    const VteRowData *rowdata;
    bool do_check_hilite;
    vte::grid::coords rowcol;
    hyperlink_idx_t new_hyperlink_hover_idx = 0;
    GdkRectangle bbox;
    const char *separator;

    if (!m_allow_hyperlink) {
        return;
    }

    _vte_debug_print (VTE_DEBUG_HYPERLINK,
                        "hyperlink_hilite_update\n");

    /* m_mouse_last_position contains the current position, see bug 789536 comment 24. */
    auto pos = m_mouse_last_position;

    /* Whether there's any chance we'd highlight something */
    do_check_hilite = view_coords_visible(pos) &&
                        m_mouse_cursor_over_widget &&
                        !(m_mouse_autohide && m_mouse_cursor_autohidden) &&
                        !m_selecting;
    if (do_check_hilite) {
        rowcol = grid_coords_from_view_coords(pos);
        rowdata = find_row_data(rowcol.row());
        if (rowdata && rowcol.column() < rowdata->len) {
            new_hyperlink_hover_idx = rowdata->cells[rowcol.column()].attr.hyperlink_idx;
        }
    }

    if (new_hyperlink_hover_idx == m_hyperlink_hover_idx) {
        _vte_debug_print (VTE_DEBUG_HYPERLINK,
                            "hyperlink did not change\n");
        return;
    }

    /* Invalidate cells of the old hyperlink. */
    if (m_hyperlink_hover_idx != 0) {
        hyperlink_invalidate_and_get_bbox(m_hyperlink_hover_idx, NULL);
    }

    /* This might be different from new_hyperlink_hover_idx. If in the stream, that one contains
        * the pseudo idx VTE_HYPERLINK_IDX_TARGET_IN_STREAM and now a real idx is allocated.
        * Plus, the ring's internal belief of the hovered hyperlink is also updated. */
    if (do_check_hilite) {
        m_hyperlink_hover_idx = _vte_ring_get_hyperlink_at_position(m_screen->row_data, rowcol.row(), rowcol.column(), true, &m_hyperlink_hover_uri);
    } else {
        m_hyperlink_hover_idx = 0;
        m_hyperlink_hover_uri = nullptr;
    }

    /* Invalidate cells of the new hyperlink. Get the bounding box. */
    if (m_hyperlink_hover_idx != 0) {
        /* URI is after the first semicolon */
        separator = strchr(m_hyperlink_hover_uri, ';');
        g_assert(separator != NULL);
        m_hyperlink_hover_uri = separator + 1;

        hyperlink_invalidate_and_get_bbox(m_hyperlink_hover_idx, &bbox);
        g_assert(bbox.width > 0 && bbox.height > 0);
    }
    _vte_debug_print(VTE_DEBUG_HYPERLINK,
                        "Hover idx: %d \"%s\"\n",
                        m_hyperlink_hover_idx,
                        m_hyperlink_hover_uri);

    /* Underlining hyperlinks has precedence over regex matches. So when the hovered hyperlink changes,
        * the regex match might need to become or stop being underlined. */
    invalidate_match_span();

    apply_mouse_cursor();

    emit_hyperlink_hover_uri_changed(m_hyperlink_hover_idx != 0 ? &bbox : NULL);
}

/*
 * VteTerminalPrivate::match_hilite_clear:
 *
 * Reset match variables and invalidate the old match region if highlighted.
 */
void VteTerminalPrivate::match_hilite_clear()
{
    invalidate_match_span();

    m_match_span.clear();
    m_match_tag = -1;

    if (m_match != nullptr) {
        g_free (m_match);
        m_match = nullptr;
    }
}

void VteTerminalPrivate::invalidate_match_span()
{
    _vte_debug_print(VTE_DEBUG_EVENTS,
                        "Invalidating match span %s\n", m_match_span.to_string());
    invalidate(m_match_span);
}

/* Note that the clipboard has cleared. */
static void clipboard_clear_cb(GtkClipboard *clipboard, gpointer user_data)
{
    VteTerminalPrivate *that = reinterpret_cast<VteTerminalPrivate*>(user_data);
    that->widget_clipboard_cleared(clipboard);
}

void VteTerminalPrivate::widget_clipboard_cleared(GtkClipboard *clipboard_)
{
    if (m_changing_selection) {
        return;
    }

    if (clipboard_ == m_clipboard[VTE_SELECTION_PRIMARY]) {
        if (m_selection_owned[VTE_SELECTION_PRIMARY] && m_has_selection) {
            _vte_debug_print(VTE_DEBUG_SELECTION, "Lost selection.\n");
            deselect_all();
        }
        m_selection_owned[VTE_SELECTION_PRIMARY] = false;
    } else if (clipboard_ == m_clipboard[VTE_SELECTION_CLIPBOARD]) {
        m_selection_owned[VTE_SELECTION_CLIPBOARD] = false;
    }
}

/* Supply the selected text to the clipboard. */
static void clipboard_copy_cb(GtkClipboard *clipboard,
                                GtkSelectionData *data,
                                guint info,
                                gpointer user_data)
{
    VteTerminalPrivate *that = reinterpret_cast<VteTerminalPrivate*>(user_data);
    that->widget_clipboard_requested(clipboard, data, info);
}

static char* text_to_utf16_mozilla(GString* text,
                      gsize* len_ptr)
{
   /* Use g_convert() instead of g_utf8_to_utf16() since the former
    * adds a BOM which Mozilla requires for text/html format.
    */
    return g_convert(text->str, text->len,
                        "UTF-16", /* conver to UTF-16 */
                        "UTF-8", /* convert from UTF-8 */
                        nullptr /* out bytes_read */,
                        len_ptr,
                        nullptr);
}

void VteTerminalPrivate::widget_clipboard_requested(GtkClipboard *target_clipboard,
                                                    GtkSelectionData *data,
                                                    guint info)
{
    for (auto sel = 0; sel < LAST_VTE_SELECTION; sel++) {
        if (target_clipboard == m_clipboard[sel] && m_selection[sel] != nullptr) {
            _VTE_DEBUG_IF(VTE_DEBUG_SELECTION) {
                int i;
                g_printerr("Setting selection %d (%" G_GSIZE_FORMAT " UTF-8 bytes.) for target %s\n",
                            sel, m_selection[sel]->len, gdk_atom_name(gtk_selection_data_get_target(data)));
                char const* selection_text = m_selection[sel]->str;
                for (i = 0; selection_text[i] != '\0'; i++) {
                    g_printerr("0x%04x ", selection_text[i]);
                    if ((i & 0x7) == 0x7) {
                        g_printerr("\n");
                    }
                }
                g_printerr("\n");
            }
            if (info == VTE_TARGET_TEXT) {
                gtk_selection_data_set_text(data,
                                            m_selection[sel]->str,
                                            m_selection[sel]->len);
            } else if (info == VTE_TARGET_HTML) {
                gsize len;
                auto selection = text_to_utf16_mozilla(m_selection[sel], &len);
                /* FIXMEchpe this makes yet another copy of the data... :( */
                if (selection) {
                    gtk_selection_data_set(data,
                                            gdk_atom_intern_static_string("text/html"),
                                            16,
                                            (const guchar *)selection,
                                            len);
                }
                g_free(selection);
            } else {
                /* Not reached */
            }
        }
    }
}

/* Convert the internal color code (either index or RGB) into RGB. */
template <unsigned int redbits,
          unsigned int greenbits,
          unsigned int bluebits>
void VteTerminalPrivate::rgb_from_index(guint index,
                                        vte::color::rgb& color) const
{
    bool dim = false;
    if (!(index & VTE_RGB_COLOR_MASK(redbits, greenbits, bluebits)) && (index & VTE_DIM_COLOR)) {
        index &= ~VTE_DIM_COLOR;
        dim = true;
    }

    if (index >= VTE_LEGACY_COLORS_OFFSET && index < VTE_LEGACY_COLORS_OFFSET + VTE_LEGACY_FULL_COLOR_SET_SIZE) {
        index -= VTE_LEGACY_COLORS_OFFSET;
    }
    if (index < VTE_PALETTE_SIZE) {
        color = *get_color(index);
        if (dim) {
            /* magic formula taken from xterm */
            color.red = color.red * 2 / 3;
            color.green = color.green * 2 / 3;
            color.blue = color.blue * 2 / 3;
        }
    } else if (index & VTE_RGB_COLOR_MASK(redbits, greenbits, bluebits)) {
        color.red   = VTE_RGB_COLOR_GET_COMPONENT(index, greenbits + bluebits, redbits) * 0x101U;
        color.green = VTE_RGB_COLOR_GET_COMPONENT(index, bluebits, greenbits) * 0x101U;
        color.blue  = VTE_RGB_COLOR_GET_COMPONENT(index, 0, bluebits) * 0x101U;
    } else {
        g_assert_not_reached();
    }
}

GString* VteTerminalPrivate::get_text(vte::grid::row_t start_row,
                                        vte::grid::column_t start_col,
                                        vte::grid::row_t end_row,
                                        vte::grid::column_t end_col,
                                        bool block,
                                        bool wrap,
                                        bool include_trailing_spaces,
                                        GArray *attributes)
{
    const VteCell *pcell = NULL;
    GString *string;
    struct _VteCharAttributes attr;
    vte::color::rgb fore, back;

    if (attributes) {
        g_array_set_size (attributes, 0);
    }

    string = g_string_new(NULL);
    memset(&attr, 0, sizeof(attr));

    if (start_col < 0) {
            start_col = 0;
    }

    vte::grid::column_t next_first_column = block ? start_col : 0;
    vte::grid::column_t col = start_col;
    vte::grid::row_t row;
    for (row = start_row; row < end_row + 1; row++, col = next_first_column) {
        VteRowData const* row_data = find_row_data(row);
        gsize last_empty, last_nonempty;
        vte::grid::column_t last_emptycol, last_nonemptycol;
        vte::grid::column_t line_last_column = (block || row == end_row) ? end_col : G_MAXLONG;

        last_empty = last_nonempty = string->len;
        last_emptycol = last_nonemptycol = -1;

        attr.row = row;
        attr.column = col;
        pcell = NULL;
        if (row_data != NULL) {
            while (col <= line_last_column && (pcell = _vte_row_data_get (row_data, col))) {

                attr.column = col;

                /* If it's not part of a multi-column character,
                 * and passes the selection criterion, add it to
                 * the selection. */
                if (!pcell->attr.fragment()) {
                    /* Store the attributes of this character. */
                    /* FIXMEchpe shouldn't this use determine_colors? */
                    uint32_t fg, bg, dc;
                    vte_color_triple_get(pcell->attr.colors(), &fg, &bg, &dc);
                    rgb_from_index<8, 8, 8>(fg, fore);
                    rgb_from_index<8, 8, 8>(bg, back);
                    attr.fore.red = fore.red;
                    attr.fore.green = fore.green;
                    attr.fore.blue = fore.blue;
                    attr.back.red = back.red;
                    attr.back.green = back.green;
                    attr.back.blue = back.blue;
                    attr.underline = (pcell->attr.underline() == 1);
                    attr.strikethrough = pcell->attr.strikethrough();

                    /* Store the cell string */
                    if (pcell->c == 0) {
                        g_string_append_c (string, ' ');
                        last_empty = string->len;
                        last_emptycol = col;
                    } else {
                        _vte_unistr_append_to_string (pcell->c, string);
                        last_nonempty = string->len;
                        last_nonemptycol = col;
                    }

                    /* If we added text to the string, record its
                     * attributes, one per byte. */
                    if (attributes) {
                        vte_g_array_fill(attributes,
                                &attr, string->len);
                    }
                }

                col++;
            }
        }

           /* If the last thing we saw was a empty, and we stopped at the
        * right edge of the range, trim the trailing spaces
        * off of the line. */
        if (!include_trailing_spaces && last_empty > last_nonempty) {

            col = last_emptycol + 1;

            if (row_data != NULL) {
                while ((pcell = _vte_row_data_get (row_data, col))) {
                    col++;

                    if (pcell->attr.fragment()) {
                        continue;
                    }

                    if (pcell->c != 0) {
                        break;
                    }
                }
            }
            if (pcell == NULL) {
                g_string_truncate(string, last_nonempty);
                if (attributes) {
                    g_array_set_size(attributes, string->len);
                }
                attr.column = last_nonemptycol;
            }
        }

        /* Adjust column, in case we want to append a newline */
        /* FIXMEchpe MIN ? */
        attr.column = MAX(m_column_count, attr.column + 1);

        /* Add a newline in block mode. */
        if (block) {
            string = g_string_append_c(string, '\n');
        }
        /* Else, if the last visible column on this line was in range and
         * not soft-wrapped, append a newline. */
        else if (row < end_row) {
            /* If we didn't softwrap, add a newline. */
            /* XXX need to clear row->soft_wrap on deletion! */
            if (!line_is_wrappable(row)) {
                string = g_string_append_c(string, '\n');
            }
        }

        /* Make sure that the attributes array is as long as the string. */
        if (attributes) {
            vte_g_array_fill (attributes, &attr, string->len);
        }
    }

    /* Sanity check. */
    if (attributes != nullptr) {
        g_assert_cmpuint(string->len, ==, attributes->len);
    }

    return string;
}

GString* VteTerminalPrivate::get_text_displayed(bool wrap,
                                                bool include_trailing_spaces,
                                                GArray *attributes)
{
    return get_text(first_displayed_row(), 0,
                    last_displayed_row() + 1, -1,
                    false /* block */, wrap, include_trailing_spaces,
                    attributes);
}

/* This is distinct from just using first/last_displayed_row since a11y
 * doesn't know about sub-row displays.
 */
GString* VteTerminalPrivate::get_text_displayed_a11y(bool wrap,
                                            bool include_trailing_spaces,
                                            GArray *attributes)
{
    return get_text(m_screen->scroll_delta, 0,
                    m_screen->scroll_delta + m_row_count - 1 + 1, -1,
                    false /* block */, wrap, include_trailing_spaces,
                    attributes);
}

GString* VteTerminalPrivate::get_selected_text(GArray *attributes)
{
    return get_text(m_selection_start.row,
                        m_selection_start.col,
                        m_selection_end.row,
                        m_selection_end.col,
                        m_selection_block_mode,
                        true /* wrap */,
                        false /* include trailing whitespace */,
                        attributes);
}

#ifdef VTE_DEBUG
unsigned int VteTerminalPrivate::checksum_area(vte::grid::row_t start_row,
                                                vte::grid::column_t start_col,
                                                vte::grid::row_t end_row,
                                                vte::grid::column_t end_col)
{
    unsigned int checksum = 0;

    auto text = get_text(start_row, start_col, end_row, end_col,
                            true /* block */, false /* wrap */,
                            true /* trailing whitespace */,
                            nullptr /* not interested in attributes */);
    if (text == nullptr) {
        return checksum;
    }

    char const* end = (char const*)text->str + text->len;
    for (char const *p = text->str; p < end; p = g_utf8_next_char(p)) {
        auto const c = g_utf8_get_char(p);
        if (c == '\n') {
            continue;
        }
        checksum += c;
    }
    g_string_free(text, true);

    return checksum & 0xffff;
}
#endif /* VTE_DEBUG */

/*
 * Compares the visual attributes of a VteCellAttr for equality, but ignores
 * attributes that tend to change from character to character or are otherwise
 * strange (in particular: fragment, columns).
 */
/* FIXMEchpe: make VteCellAttr a class with operator== */
static bool vte_terminal_cellattr_equal(VteCellAttr const* attr1,
                            VteCellAttr const* attr2)
{
    /* FIXMEchpe why exclude DIM here? */
    return (((attr1->attr ^ attr2->attr) & VTE_ATTR_ALL_MASK) == 0 &&
                attr1->colors()       == attr2->colors()   &&
                attr1->hyperlink_idx  == attr2->hyperlink_idx);
}

/*
 * Wraps a given string according to the VteCellAttr in HTML tags. Used
 * old-style HTML (and not CSS) for better compatibility with, for example,
 * evolution's mail editor component.
 */
char *VteTerminalPrivate::cellattr_to_html(VteCellAttr const* attr,
                                            char const* text) const
{
    GString *string;
    guint fore, back, deco;

    string = g_string_new(text);

    determine_colors(attr, false, false, &fore, &back, &deco);

    if (attr->bold()) {
        g_string_prepend(string, "<b>");
        g_string_append(string, "</b>");
    }
    if (attr->italic()) {
        g_string_prepend(string, "<i>");
        g_string_append(string, "</i>");
    }
    /* <u> should be inside <font> so that it inherits its color by default */
    if (attr->underline() != 0) {
        static const char styles[][7] = {"", "single", "double", "wavy"};
        char *tag, *colorattr;

        if (deco != VTE_DEFAULT_FG) {
            vte::color::rgb color;

            rgb_from_index<4, 5, 4>(deco, color);
            colorattr = g_strdup_printf(";text-decoration-color:#%02X%02X%02X",
                                        color.red >> 8,
                                        color.green >> 8,
                                        color.blue >> 8);
        } else {
            colorattr = g_strdup("");
        }

        tag = g_strdup_printf("<u style=\"text-decoration-style:%s%s\">",
                                styles[attr->underline()],
                                colorattr);
        g_string_prepend(string, tag);
        g_free(tag);
        g_free(colorattr);
        g_string_append(string, "</u>");
    }
    if (fore != VTE_DEFAULT_FG || attr->reverse()) {
        vte::color::rgb color;
        char *tag;

        rgb_from_index<8, 8, 8>(fore, color);
        tag = g_strdup_printf("<font color=\"#%02X%02X%02X\">",
                                color.red >> 8,
                                color.green >> 8,
                                color.blue >> 8);
        g_string_prepend(string, tag);
        g_free(tag);
        g_string_append(string, "</font>");
    }
    if (back != VTE_DEFAULT_BG || attr->reverse()) {
        vte::color::rgb color;
        char *tag;

        rgb_from_index<8, 8, 8>(back, color);
        tag = g_strdup_printf("<span style=\"background-color:#%02X%02X%02X\">",
                                color.red >> 8,
                                color.green >> 8,
                                color.blue >> 8);
        g_string_prepend(string, tag);
        g_free(tag);
        g_string_append(string, "</span>");
    }
    if (attr->strikethrough()) {
        g_string_prepend(string, "<strike>");
        g_string_append(string, "</strike>");
    }
    if (attr->overline()) {
        g_string_prepend(string, "<span style=\"text-decoration-line:overline\">");
        g_string_append(string, "</span>");
    }
    if (attr->blink()) {
        g_string_prepend(string, "<blink>");
        g_string_append(string, "</blink>");
    }
    /* reverse and invisible are not supported */

    return g_string_free(string, FALSE);
}

/*
 * Similar to find_charcell(), but takes a VteCharAttribute for
 * indexing and returns the VteCellAttr.
 */
VteCellAttr const* VteTerminalPrivate::char_to_cell_attr(VteCharAttributes const* attr) const
{
    VteCell const* cell = find_charcell(attr->column, attr->row);
    if (cell) {
        return &cell->attr;
    }
    return nullptr;
}

/*
 * VteTerminalPrivate::attributes_to_html:
 * @text: A string as returned by the vte_terminal_get_* family of functions.
 * @attrs: (array) (element-type Vte.CharAttributes): text attributes, as created by vte_terminal_get_*
 *
 * Marks the given text up according to the given attributes, using HTML <span>
 * commands, and wraps the string in a <pre> element. The attributes have to be
 * "fresh" in the sense that the terminal must not have changed since they were
 * obtained using the vte_terminal_get* function.
 *
 * Returns: (transfer full): a newly allocated text string, or %NULL.
 */
GString* VteTerminalPrivate::attributes_to_html(GString* text_string, GArray* attrs)
{
    GString *string;
    guint from,to;
    const VteCellAttr *attr;
    char *escaped, *marked;

    char const* text = text_string->str;
    auto len = text_string->len;
    g_assert_cmpuint(len, ==, attrs->len);

    /* Initial size fits perfectly if the text has no attributes and no
     * characters that need to be escaped
     */
    string = g_string_sized_new (len + 11);

    g_string_append(string, "<pre>");
    /* Find streches with equal attributes. Newlines are treated specially,
     * so that the <span> do not cover multiple lines.
     */
    from = to = 0;
    while (text[from] != '\0') {
        g_assert(from == to);
        if (text[from] == '\n') {
            g_string_append_c(string, '\n');
            from = ++to;
        } else {
            attr = char_to_cell_attr(
                &g_array_index(attrs, VteCharAttributes, from));
            while (text[to] != '\0' && text[to] != '\n' &&
                    vte_terminal_cellattr_equal(attr, char_to_cell_attr(
                        &g_array_index(attrs, VteCharAttributes, to)))) {
                to++;
            }
            escaped = g_markup_escape_text(text + from, to - from);
            marked = cellattr_to_html(attr, escaped);
            g_string_append(string, marked);
            g_free(escaped);
            g_free(marked);
            from = to;
        }
    }
    g_string_append(string, "</pre>");

    return string;
}

static GtkTargetEntry* targets_for_format(VteFormat format, int *n_targets)
{
    switch (format) {
        case VTE_FORMAT_TEXT: 
        {
            static GtkTargetEntry *text_targets = nullptr;
            static int n_text_targets;

            if (text_targets == nullptr) {
                auto list = gtk_target_list_new (nullptr, 0);
                gtk_target_list_add_text_targets (list, VTE_TARGET_TEXT);

                text_targets = gtk_target_table_new_from_list (list, &n_text_targets);
                gtk_target_list_unref (list);
            }

            *n_targets = n_text_targets;
            return text_targets;
        }

        case VTE_FORMAT_HTML: 
        {
            static GtkTargetEntry *html_targets = nullptr;
            static int n_html_targets;

            if (html_targets == nullptr) {
                auto list = gtk_target_list_new (nullptr, 0);
                gtk_target_list_add_text_targets (list, VTE_TARGET_TEXT);
                gtk_target_list_add (list,
                                        gdk_atom_intern_static_string("text/html"),
                                        0,
                                        VTE_TARGET_HTML);

                html_targets = gtk_target_table_new_from_list (list, &n_html_targets);
                gtk_target_list_unref (list);
            }

            *n_targets = n_html_targets;
            return html_targets;
        }
        default:
            g_assert_not_reached();
    }
}

/* Place the selected text onto the clipboard.  Do this asynchronously so that
 * we get notified when the selection we placed on the clipboard is replaced. */
void VteTerminalPrivate::widget_copy(VteSelection sel, VteFormat format)
{
    /* Only put HTML on the CLIPBOARD, not PRIMARY */
    g_assert(sel == VTE_SELECTION_CLIPBOARD || format == VTE_FORMAT_TEXT);

    /* Chuck old selected text and retrieve the newly-selected text. */
    GArray *attributes = g_array_new(FALSE, TRUE, sizeof(struct _VteCharAttributes));
    auto selection = get_selected_text(attributes);

    if (m_selection[sel]) {
        g_string_free(m_selection[sel], TRUE);
        m_selection[sel] = nullptr;
    }

    if (selection == nullptr) {
        g_array_free(attributes, TRUE);
        m_has_selection = FALSE;
        m_selection_owned[sel] = false;
        return;
    }

    if (format == VTE_FORMAT_HTML) {
        m_selection[sel] = attributes_to_html(selection, attributes);
        g_string_free(selection, TRUE);
    } else {
        m_selection[sel] = selection;
    }

    g_array_free (attributes, TRUE);

    if (sel == VTE_SELECTION_PRIMARY) {
        m_has_selection = TRUE;
    }

    /* Place the text on the clipboard. */
    _vte_debug_print(VTE_DEBUG_SELECTION, "Assuming ownership of selection.\n");

    int n_targets;
    auto targets = targets_for_format(format, &n_targets);

    m_changing_selection = true;
    gtk_clipboard_set_with_data(m_clipboard[sel],
                                targets,
                                n_targets,
                                clipboard_copy_cb,
                                clipboard_clear_cb,
                                this);
    m_changing_selection = false;

    gtk_clipboard_set_can_store(m_clipboard[sel], nullptr, 0);
    m_selection_owned[sel] = true;
    m_selection_format[sel] = format;
}

/* Paste from the given clipboard. */
void VteTerminalPrivate::widget_paste(GdkAtom board)
{
    if (!m_input_enabled) {
        return;
    }

    auto clip = gtk_clipboard_get_for_display(gtk_widget_get_display(m_widget), board);
    if (!clip) {
        return;
    }

    _vte_debug_print(VTE_DEBUG_SELECTION, "Requesting clipboard contents.\n");

    m_paste_request.request_text(clip, &VteTerminalPrivate::widget_paste_received, this);
}

void VteTerminalPrivate::invalidate_selection()
{
    invalidate_region(m_selection_start.col,
                        m_selection_end.col,
                        m_selection_start.row,
                        m_selection_end.row,
                        m_selection_block_mode);
}

/* Confine coordinates into the visible area. Padding is already subtracted. */
void VteTerminalPrivate::confine_coordinates(long *xp, long *yp)
{
    long x = *xp;
    long y = *yp;
    long y_stop;

    /* Allow to use the bottom extra padding only if there's content there. */
    y_stop = MIN(m_view_usable_extents.height(),
                    row_to_pixel(m_screen->insert_delta + m_row_count));

    if (y < 0) {
        y = 0;
        if (!m_selection_block_mode) {
            x = 0;
        }
    } else if (y >= y_stop) {
        y = y_stop - 1;
        if (!m_selection_block_mode) {
            x = m_column_count * m_cell_width - 1;
        }
    }
    if (x < 0) {
        x = 0;
    } else if (x >= m_column_count * m_cell_width) {
        x = m_column_count * m_cell_width - 1;
    }

    *xp = x;
    *yp = y;
}

/* Start selection at the location of the event. */
void VteTerminalPrivate::start_selection(long x, long y, enum vte_selection_type type)
{
    if (m_selection_block_mode) {
        type = selection_type_char;
    }

    /* Confine coordinates into the visible area. (#563024, #722635c7) */
    confine_coordinates(&x, &y);

    /* Record that we have the selection, and where it started. */
    m_has_selection = TRUE;
    m_selection_last.x = x;
    m_selection_last.y = scroll_delta_pixel() + y;

    /* Decide whether or not to restart on the next drag. */
    switch (type) {
        case selection_type_char:
            /* Restart selection once we register a drag. */
            m_selecting_restart = TRUE;
            m_has_selection = FALSE;
            m_selecting_had_delta = FALSE;

            m_selection_origin = m_selection_last;
            break;
        case selection_type_word:
        case selection_type_line:
            /* Mark the newly-selected areas now. */
            m_selecting_restart = FALSE;
            m_has_selection = FALSE;
            m_selecting_had_delta = FALSE;
            break;
    }

    /* Record the selection type. */
    m_selection_type = type;
    m_selecting = TRUE;
    m_selecting_after_threshold = FALSE;

    _vte_debug_print(VTE_DEBUG_SELECTION,
                         "Selection started at (%ld,%ld).\n",
                         m_selection_start.col,
                         m_selection_start.row);

        /* Take care of updating the display. */
        extend_selection(x, y, false, true);
#if 0 /* FIXME: removed */
    /* Temporarily stop caring about input from the child. */
    disconnect_pty_read();
#endif
}

bool VteTerminalPrivate::maybe_end_selection()
{
    if (m_selecting) {
        /* Copy only if something was selected. */
        if (m_has_selection &&
            !m_selecting_restart &&
            m_selecting_had_delta) {
            widget_copy(VTE_SELECTION_PRIMARY, VTE_FORMAT_TEXT);
            emit_selection_changed();
        }
        m_selecting = false;
#if 0 /* FIXME: removed */
        /* Reconnect to input from the child if we paused it. */
        connect_pty_read();
#endif
        return true;
    }

    if (m_selecting_after_threshold) {
        return true;
    }

    return false;
}

static long math_div (long a, long b)
{
    if (G_LIKELY (a >= 0)) {
        return a / b;
    } else {
        return (a / b) - 1;
    }
}

/* Helper */
void VteTerminalPrivate::extend_selection_expand()
{
    long i, j;
    const VteCell *cell;
    VteVisualPosition *sc, *ec;

    if (m_selection_block_mode) {
        return;
    }

    sc = &m_selection_start;
    ec = &m_selection_end;

    /* Extend the selection to handle end-of-line cases, word, and line
     * selection.  We do this here because calculating it once is cheaper
     * than recalculating for each cell as we render it. */

    /* Handle end-of-line at the start-cell. */
    VteRowData const* rowdata = find_row_data(sc->row);
    if (rowdata != NULL) {
        /* Find the last non-empty character on the first line. */
        for (i = _vte_row_data_length (rowdata); i > 0; i--) {
            cell = _vte_row_data_get (rowdata, i - 1);
            if (cell->attr.fragment() || cell->c != 0) {
                break;
            }
        }
    } else {
        i = 0;
    }
    if (sc->col > i) {
        if (m_selection_type == selection_type_char) {
           /* If the start point is neither over the used cells, nor over the first
            * unused one, then move it to the next line. This way you can still start
            * selecting at the newline character by clicking over the first unused cell.
            * See bug 725909. */
            sc->col = -1;
            sc->row++;
        } else if (m_selection_type == selection_type_word) {
            sc->col = i;
        }
    }
    sc->col = find_start_column(sc->col, sc->row);

    /* Handle end-of-line at the end-cell. */
    rowdata = find_row_data(ec->row);
    if (rowdata != NULL) {
        /* Find the last non-empty character on the last line. */
        for (i = _vte_row_data_length (rowdata); i > 0; i--) {
            cell = _vte_row_data_get (rowdata, i - 1);
            if (cell->attr.fragment() || cell->c != 0) {
                break;
            }
        }
        /* If the end point is to its right, then extend the
         * endpoint to the beginning of the next row. */
        if (ec->col >= i) {
            ec->col = -1;
            ec->row++;
        }
    } else {
        /* Snap to the beginning of the next line, only if
         * selecting anything of this row. */
        if (ec->col >= 0) {
            ec->col = -1;
            ec->row++;
        }
    }
    ec->col = find_end_column(ec->col, ec->row);

    /* Now extend again based on selection type. */
    switch (m_selection_type) {
        case selection_type_char:
            /* Nothing more to do. */
            break;
        case selection_type_word:
            /* Keep selecting to the left as long as the next character we
            * look at is of the same class as the current start point. */
            i = sc->col;
            j = sc->row;
            while (_vte_ring_contains (m_screen->row_data, j)) {
                /* Get the data for the row we're looking at. */
                rowdata = _vte_ring_index(m_screen->row_data, j);
                if (rowdata == NULL) {
                    break;
                }
                /* Back up. */
                for (i = (j == sc->row) ? sc->col : m_column_count; i > 0; i--) {
                    if (is_same_class(i - 1, j, i, j)) {
                        sc->col = i - 1;
                        sc->row = j;
                    } else {
                        break;
                    }
                }
                if (i > 0) {
                    /* We hit a stopping point, so stop. */
                    break;
                } else {
                    if (line_is_wrappable(j - 1) && is_same_class(m_column_count - 1,
                                                                    j - 1,
                                                                    0,
                                                                    j)) {
                        /* Move on to the previous line. */
                        j--;
                        sc->col = m_column_count - 1;
                        sc->row = j;
                    } else {
                        break;
                    }
                }
            }
            /* Keep selecting to the right as long as the next character we
            * look at is of the same class as the current end point. */
            i = ec->col;
            j = ec->row;
            while (_vte_ring_contains (m_screen->row_data, j)) {
                /* Get the data for the row we're looking at. */
                rowdata = _vte_ring_index(m_screen->row_data, j);
                if (rowdata == NULL) {
                    break;
                }
                /* Move forward. */
                for (i = (j == ec->row) ? ec->col : 0; i < m_column_count - 1; i++) {
                    if (is_same_class(i, j, i + 1, j)) {
                        ec->col = i + 1;
                        ec->row = j;
                    } else {
                        break;
                    }
                }
                if (i < m_column_count - 1) {
                    /* We hit a stopping point, so stop. */
                    break;
                } else {
                    if (line_is_wrappable(j) && is_same_class(
                                                                m_column_count - 1,
                                                                j,
                                                                0,
                                                                j + 1)) {
                        /* Move on to the next line. */
                        j++;
                        ec->col = 0;
                        ec->row = j;
                    } else {
                        break;
                    }
                }
            }
            break;
        case selection_type_line:
            /* Extend the selection to the beginning of the start line. */
            sc->col = 0;
            /* Now back up as far as we can go. */
            j = sc->row;
            while (_vte_ring_contains (m_screen->row_data, j - 1) && line_is_wrappable(j - 1)) {
                j--;
                sc->row = j;
            }
            /* And move forward as far as we can go. */
            if (ec->col < 0) {
               /* If triple clicking on an unused area, ec already points
                * to the beginning of the next line after the second click.
                * Go back to the actual row we're at. See bug 725909. */
                ec->row--;
            }
            j = ec->row;
            while (_vte_ring_contains (m_screen->row_data, j) &&
                line_is_wrappable(j)) {
                j++;
                ec->row = j;
            }
            /* Make sure we include all of the last line by extending
            * to the beginning of the next line. */
            ec->row++;
            ec->col = -1;
            break;
    }
}

/* Extend selection to include the given event coordinates. */
void VteTerminalPrivate::extend_selection(long x, long y, bool always_grow, bool force)
{
    long residual;
    long row;
    vte::view::coords *origin, *last, *start, *end;
    VteVisualPosition old_start, old_end, *sc, *ec, *so, *eo;
    gboolean invalidate_selected = FALSE;
    gboolean had_selection;

    /* Confine coordinates into the visible area. (#563024, #722635c7) */
    confine_coordinates(&x, &y);

    old_start = m_selection_start;
    old_end = m_selection_end;
    so = &old_start;
    eo = &old_end;

    /* If we're restarting on a drag, then mark this as the start of
     * the selected block. */
    if (m_selecting_restart) {
        deselect_all();
        invalidate_selected = TRUE;
        _vte_debug_print(VTE_DEBUG_SELECTION,
                "Selection delayed start at (%ld,%ld).\n",
                m_selection_origin.x / m_cell_width,
                m_selection_origin.y / m_cell_height);
    }

    /* Recognize that we've got a selected block. */
    had_selection = m_has_selection;
    m_has_selection = TRUE;
    m_selecting_had_delta = TRUE;
    m_selecting_restart = FALSE;

    /* If we're not in always-grow mode, update the last location of
     * the selection. */
    last = &m_selection_last;

    /* Map the origin and last selected points to a start and end. */
    origin = &m_selection_origin;
    if (m_selection_block_mode) {
        last->x = x;
        last->y = scroll_delta_pixel() + y;

        /* We don't support always_grow in block mode */
        if (always_grow) {
            invalidate_selection();
        }

        if (origin->y <= last->y) {
            /* The origin point is "before" the last point. */
            start = origin;
            end = last;
        } else {
            /* The last point is "before" the origin point. */
            start = last;
            end = origin;
        }
    } else {
        if (!always_grow) {
            last->x = x;
            last->y = scroll_delta_pixel() + y;
        }

        if ((origin->y / m_cell_height < last->y / m_cell_height) ||
            ((origin->y / m_cell_height == last->y / m_cell_height) &&
             (origin->x / m_cell_width < last->x / m_cell_width ))) {
            /* The origin point is "before" the last point. */
            start = origin;
            end = last;
        } else {
            /* The last point is "before" the origin point. */
            start = last;
            end = origin;
        }

        /* Extend the selection by moving whichever end of the selection is
         * closer to the new point. */
        if (always_grow) {
            /* New endpoint is before existing selection. */
                        row = pixel_to_row(y);
            if ((row < start->y / m_cell_height) ||
                ((row == start->y / m_cell_height) &&
                 (x / m_cell_width < start->x / m_cell_width))) {
                start->x = x;
                start->y = scroll_delta_pixel() + y;
            } else {
                /* New endpoint is after existing selection. */
                end->x = x;
                end->y = scroll_delta_pixel() + y;
            }
        }
    }

#if 0
    _vte_debug_print(VTE_DEBUG_SELECTION,
            "Selection is (%ld,%ld) to (%ld,%ld).\n",
            start->x, start->y, end->x, end->y);
#endif

    /* Recalculate the selection area in terms of cell positions. */

    sc = &m_selection_start;
    ec = &m_selection_end;

    sc->row = MAX (0, start->y / m_cell_height);
    ec->row = MAX (0, end->y   / m_cell_height);

    /* Sort x using row cell coordinates */
    if ((m_selection_block_mode || sc->row == ec->row) && (start->x > end->x)) {
                vte::view::coords *tmp;
        tmp = start;
        start = end;
        end = tmp;
    }

    /* We want to be more lenient on the user with their column selection.
     * We round to the closest logical position (positions are located between
     * cells).  But we don't want to fully round.  So we divide the cell
     * width into three parts.  The side parts round to their nearest
     * position.  The middle part is always inclusive in the selection.
     *
     * math_div and no MAX, to allow selecting no cells in the line,
     * ie. ec->col = -1, which is essentially equal to copying the
     * newline from previous line but no chars from current line. */
    residual = (m_cell_width + 1) / 3;
    sc->col = math_div (start->x + residual, m_cell_width);
    ec->col = math_div (end->x - residual, m_cell_width);

    extend_selection_expand();

    if (!invalidate_selected && !force &&
        0 == memcmp (sc, so, sizeof (*sc)) &&
        0 == memcmp (ec, eo, sizeof (*ec))) {
        /* No change */
        return;
    }

    /* Invalidate */

    if (had_selection) {

        if (m_selection_block_mode) {
            /* Update the selection area diff in block mode. */

            /* The top band */
            invalidate_region(
                        MIN(sc->col, so->col),
                        MAX(ec->col, eo->col),
                        MIN(sc->row, so->row),
                        MAX(sc->row, so->row) - 1,
                        true);
            /* The bottom band */
            invalidate_region(
                        MIN(sc->col, so->col),
                        MAX(ec->col, eo->col),
                        MIN(ec->row, eo->row) + 1,
                        MAX(ec->row, eo->row),
                        true);
            /* The left band */
            invalidate_region(
                        MIN(sc->col, so->col),
                        MAX(sc->col, so->col) - 1 + (VTE_TAB_WIDTH_MAX - 1),
                        MIN(sc->row, so->row),
                        MAX(ec->row, eo->row),
                        true);
            /* The right band */
            invalidate_region(
                        MIN(ec->col, eo->col) + 1,
                        MAX(ec->col, eo->col) + (VTE_TAB_WIDTH_MAX - 1),
                        MIN(sc->row, so->row),
                        MAX(ec->row, eo->row),
                        true);
        } else {
            /* Update the selection area diff in non-block mode. */

            /* The before band */
            if (sc->row < so->row) {
                invalidate_region(
                            sc->col, so->col - 1,
                            sc->row, so->row,
                            false);
            } else if (sc->row > so->row) {
                invalidate_region(
                            so->col, sc->col - 1,
                            so->row, sc->row,
                            false);
            } else {
                invalidate_region(
                            MIN(sc->col, so->col), MAX(sc->col, so->col) - 1,
                            sc->row, sc->row,
                            true);
            }

            /* The after band */
            if (ec->row < eo->row) {
                invalidate_region(
                            ec->col + 1, eo->col,
                            ec->row, eo->row,
                            false);
            } else if (ec->row > eo->row) {
                invalidate_region(
                            eo->col + 1, ec->col,
                            eo->row, ec->row,
                            false);
            } else {
                invalidate_region(
                            MIN(ec->col, eo->col) + 1, MAX(ec->col, eo->col),
                            ec->row, ec->row,
                            true);
            }
        }
    }

    if (invalidate_selected || !had_selection) {
        _vte_debug_print(VTE_DEBUG_SELECTION, "Invalidating selection.");
        invalidate_selection();
    }

    _vte_debug_print(VTE_DEBUG_SELECTION,
            "Selection changed to "
            "(%ld,%ld) to (%ld,%ld).\n",
            sc->col, sc->row, ec->col, ec->row);
}

/*
 * VteTerminalPrivate::select_all:
 *
 * Selects all text within the terminal (including the scrollback buffer).
 */
void VteTerminalPrivate::select_all()
{
    deselect_all();

    m_has_selection = TRUE;
    m_selecting_had_delta = TRUE;
    m_selecting_restart = FALSE;

    m_selection_start.row = _vte_ring_delta (m_screen->row_data);
    m_selection_start.col = 0;
    m_selection_end.row = _vte_ring_next (m_screen->row_data);
    m_selection_end.col = -1;

    _vte_debug_print(VTE_DEBUG_SELECTION, "Selecting *all* text.\n");

    widget_copy(VTE_SELECTION_PRIMARY, VTE_FORMAT_TEXT);
    emit_selection_changed();

    invalidate_all();
}

/* Autoscroll a bit. */
static gboolean vte_terminal_autoscroll_cb(VteTerminalPrivate *that)
{
    return that->autoscroll() ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

/*
 * VteTerminalPrivate::autoscroll():
 *
 * Returns: %true to continue autoscrolling, %false to stop
 */
bool VteTerminalPrivate::autoscroll()
{
    bool extend = false;
    long x, y, xmax, ymax;
    glong adj;

    /* Provide an immediate effect for mouse wigglers. */
    if (m_mouse_last_position.y < 0) {
        if (m_vadjustment) {
            /* Try to scroll up by one line. */
            adj = m_screen->scroll_delta - 1;
            queue_adjustment_value_changed_clamped(adj);
            extend = true;
        }
        _vte_debug_print(VTE_DEBUG_EVENTS, "Autoscrolling down.\n");
    }
    if (m_mouse_last_position.y >= m_view_usable_extents.height()) {
        if (m_vadjustment) {
            /* Try to scroll up by one line. */
            adj = m_screen->scroll_delta + 1;
            queue_adjustment_value_changed_clamped(adj);
            extend = true;
        }
        _vte_debug_print(VTE_DEBUG_EVENTS, "Autoscrolling up.\n");
    }
    if (extend) {
        /* FIXMEchpe use confine_view_coords here */
        /* Don't select off-screen areas.  That just confuses people. */
        xmax = m_column_count * m_cell_width;
        ymax = m_row_count * m_cell_height;

        x = CLAMP(m_mouse_last_position.x, 0, xmax);
        y = CLAMP(m_mouse_last_position.y, 0, ymax);
        /* If we clamped the Y, mess with the X to get the entire
         * lines. */
        if (m_mouse_last_position.y < 0 && !m_selection_block_mode) {
            x = 0;
        }
        if (m_mouse_last_position.y >= ymax && !m_selection_block_mode) {
            x = m_column_count * m_cell_width;
        }
        /* Extend selection to cover the newly-scrolled area. */
        extend_selection(x, y, false, true);
    } else {
        /* Stop autoscrolling. */
        m_mouse_autoscroll_tag = 0;
    }
    return (m_mouse_autoscroll_tag != 0);
}

/* Start autoscroll. */
void VteTerminalPrivate::start_autoscroll()
{
    if (m_mouse_autoscroll_tag != 0) {
        return;
    }

    m_mouse_autoscroll_tag =
            g_timeout_add_full(G_PRIORITY_LOW,
                                666 / m_row_count, /* FIXME WTF? */
                                (GSourceFunc)vte_terminal_autoscroll_cb,
                                this,
                                NULL);/* FIXME make sure m_mouse_autoscroll_tag is nulled! */
}

/* Stop autoscroll. */
void VteTerminalPrivate::stop_autoscroll()
{
    if (m_mouse_autoscroll_tag == 0) {
        return;
    }

    g_source_remove(m_mouse_autoscroll_tag);
    m_mouse_autoscroll_tag = 0;
}

bool VteTerminalPrivate::widget_motion_notify(GdkEventMotion *event)
{
    bool handled = false;

    GdkEvent* base_event = reinterpret_cast<GdkEvent*>(event);
    auto pos = view_coords_from_event(base_event);
    auto rowcol = grid_coords_from_view_coords(pos);

    _vte_debug_print(VTE_DEBUG_EVENTS,
                         "Motion notify %s %s\n",
                         pos.to_string(), rowcol.to_string());

    read_modifiers(base_event);

    switch (event->type) {
        case GDK_MOTION_NOTIFY:
            if (m_selecting_after_threshold) {
                if (!gtk_drag_check_threshold (m_widget,
                                m_mouse_last_position.x,
                                m_mouse_last_position.y,
                                pos.x, pos.y)) {
                    return true;
                }

                start_selection(m_mouse_last_position.x, m_mouse_last_position.y,
                                            selection_type_char);
            }

            if (m_selecting && (m_mouse_handled_buttons & 1) != 0) {
                _vte_debug_print(VTE_DEBUG_EVENTS, "Mousing drag 1.\n");
                extend_selection(pos.x, pos.y, false, false);

                /* Start scrolling if we need to. */
                if (pos.y < 0 || pos.y >= m_view_usable_extents.height()) {
                    /* Give mouse wigglers something. */
                    stop_autoscroll();
                    autoscroll();
                    /* Start a timed autoscroll if we're not doing it
                    * already. */
                    start_autoscroll();
                }

                handled = true;
            }

            if (!handled && m_input_enabled) {
                maybe_send_mouse_drag(rowcol, event->type);
            }
            break;
        default:
            break;
    }

    if (pos != m_mouse_last_position) {
        m_mouse_last_position = pos;

        set_pointer_autohidden(false);
        hyperlink_hilite_update();
    }

    return handled;
}

bool VteTerminalPrivate::widget_button_press(GdkEventButton *event)
{
    bool handled = false;
    gboolean start_selecting = FALSE, extend_selecting = FALSE;

    GdkEvent* base_event = reinterpret_cast<GdkEvent*>(event);
    auto pos = view_coords_from_event(base_event);
    auto rowcol = grid_coords_from_view_coords(pos);

    read_modifiers(base_event);

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            _vte_debug_print(VTE_DEBUG_EVENTS,
                                    "Button %d single-click at %s\n",
                                    event->button,
                                    rowcol.to_string());
            /* Handle this event ourselves. */
            switch (event->button) {
                case 1:
                    _vte_debug_print(VTE_DEBUG_EVENTS, "Handling click ourselves.\n");
                    /* Grab focus. */
                    if (!gtk_widget_has_focus(m_widget)) {
                        gtk_widget_grab_focus(m_widget);
                    }

                    /* If we're in event mode, and the user held down the
                    * shift key, we start selecting. */
                    if (m_mouse_tracking_mode) {
                        if (m_modifiers & GDK_SHIFT_MASK) {
                            start_selecting = TRUE;
                        }
                    } else {
                        /* If the user hit shift, then extend the
                        * selection instead. */
                        if ((m_modifiers & GDK_SHIFT_MASK) &&
                            (m_has_selection ||
                            m_selecting_restart) &&
                            !cell_is_selected(rowcol.column(), rowcol.row())) {
                            extend_selecting = TRUE;
                        } else {
                            start_selecting = TRUE;
                        }
                    }
                    if (start_selecting) {
                        deselect_all();
                        m_selecting_after_threshold = TRUE;
                        m_selection_block_mode = !!(m_modifiers & GDK_CONTROL_MASK);
                        handled = true;
                    }
                    if (extend_selecting) {
                        extend_selection(pos.x, pos.y, !m_selecting_restart, true);
                        /* The whole selection code needs to be
                        * rewritten.  For now, put this here to
                        * fix bug 614658 */
                        m_selecting = TRUE;
                        handled = true;
                    }
                    break;
                /* Paste if the user pressed shift or we're not sending events
                * to the app. */
                case 2:
                    if ((m_modifiers & GDK_SHIFT_MASK) ||
                        !m_mouse_tracking_mode) {
                        gboolean do_paste;

                        g_object_get (gtk_widget_get_settings(m_widget),
                                    "gtk-enable-primary-paste",
                                    &do_paste, nullptr);
                        if (do_paste) {
                            widget_paste(GDK_SELECTION_PRIMARY);
                        }
                        handled = do_paste;
                    }
                    break;
                case 3:
                default:
                    break;
            }
            if (event->button >= 1 && event->button <= 3) {
                if (handled) {
                    m_mouse_handled_buttons |= (1 << (event->button - 1));
                } else {
                    m_mouse_handled_buttons &= ~(1 << (event->button - 1));
                }
            }
            /* If we haven't done anything yet, try sending the mouse
            * event to the app. */
            if (handled == FALSE) {
                handled = maybe_send_mouse_button(rowcol, event->type, event->button);
            }
            break;
        case GDK_2BUTTON_PRESS:
            _vte_debug_print(VTE_DEBUG_EVENTS,
                                    "Button %d double-click at %s\n",
                                    event->button,
                                    rowcol.to_string());
            switch (event->button) {
                case 1:
                    if (m_selecting_after_threshold) {
                        start_selection(pos.x, pos.y, selection_type_char);
                        handled = true;
                    }
                    if ((m_mouse_handled_buttons & 1) != 0) {
                        start_selection(pos.x, pos.y, selection_type_word);
                        handled = true;
                    }
                    break;
                case 2:
                case 3:
                default:
                    break;
            }
            break;
        case GDK_3BUTTON_PRESS:
            _vte_debug_print(VTE_DEBUG_EVENTS,
                                    "Button %d triple-click at %s\n",
                                    event->button,
                                    rowcol.to_string());
            switch (event->button) {
                case 1:
                    if ((m_mouse_handled_buttons & 1) != 0) {
                        start_selection(pos.x, pos.y, selection_type_line);
                        handled = true;
                    }
                    break;
                case 2:
                case 3:
                default:
                    break;
            }
        default:
            break;
    }

    /* Save the pointer state for later use. */
    if (event->button >= 1 && event->button <= 3) {
        m_mouse_pressed_buttons |= (1 << (event->button - 1));
    }

    m_mouse_last_position = pos;

    set_pointer_autohidden(false);
    hyperlink_hilite_update();

    return handled;
}

bool VteTerminalPrivate::widget_button_release(GdkEventButton *event)
{
    bool handled = false;

    GdkEvent* base_event = reinterpret_cast<GdkEvent*>(event);
    auto pos = view_coords_from_event(base_event);
    auto rowcol = grid_coords_from_view_coords(pos);

    stop_autoscroll();

    read_modifiers(base_event);

    switch (event->type) {
        case GDK_BUTTON_RELEASE:
            _vte_debug_print(VTE_DEBUG_EVENTS,
                                    "Button %d released at %s\n",
                                    event->button, rowcol.to_string());
            switch (event->button) {
                case 1:
                    if ((m_mouse_handled_buttons & 1) != 0) {
                            handled = maybe_end_selection();
                    }
                    break;
                case 2:
                    handled = (m_mouse_handled_buttons & 2) != 0;
                    m_mouse_handled_buttons &= ~2;
                    break;
                case 3:
                default:
                    break;
            }
            if (!handled && m_input_enabled) {
                handled = maybe_send_mouse_button(rowcol, event->type, event->button);
            }
            break;
        default:
            break;
    }

    /* Save the pointer state for later use. */
    if (event->button >= 1 && event->button <= 3) {
            m_mouse_pressed_buttons &= ~(1 << (event->button - 1));
    }

    m_mouse_last_position = pos;
    m_selecting_after_threshold = false;

    set_pointer_autohidden(false);
    hyperlink_hilite_update();

    return handled;
}

void VteTerminalPrivate::widget_focus_in(GdkEventFocus *event)
{
    _vte_debug_print(VTE_DEBUG_EVENTS, "Focus in.\n");

    gtk_widget_grab_focus(m_widget);

    /* Read the keyboard modifiers, though they're probably garbage. */
    read_modifiers((GdkEvent*)event);

    /* We only have an IM context when we're realized, and there's not much
     * point to painting the cursor if we don't have a window. */
    if (widget_realized()) {
        m_cursor_blink_state = TRUE;
        m_has_focus = TRUE;

       /* If blinking gets enabled now, do a full repaint.
        * If blinking gets disabled, only repaint if there's blinking stuff present
        * (we could further optimize by checking its current phase). */
        if (m_text_blink_mode == VTE_TEXT_BLINK_FOCUSED ||
            (m_text_blink_mode == VTE_TEXT_BLINK_UNFOCUSED && m_text_blink_tag != 0)) {
            invalidate_all();
        }

        check_cursor_blink();

        gtk_im_context_focus_in(m_im_context);
        invalidate_cursor_once();
        maybe_feed_focus_event(true);
    }
}

void VteTerminalPrivate::widget_focus_out(GdkEventFocus *event)
{
    _vte_debug_print(VTE_DEBUG_EVENTS, "Focus out.\n");

    /* Read the keyboard modifiers, though they're probably garbage. */
    read_modifiers((GdkEvent*)event);

    /* We only have an IM context when we're realized, and there's not much
     * point to painting ourselves if we don't have a window. */
    if (widget_realized()) {
        maybe_feed_focus_event(false);

        maybe_end_selection();

        /* If blinking gets enabled now, do a full repaint.
            * If blinking gets disabled, only repaint if there's blinking stuff present
            * (we could further optimize by checking its current phase). */
        if (m_text_blink_mode == VTE_TEXT_BLINK_UNFOCUSED ||
            (m_text_blink_mode == VTE_TEXT_BLINK_FOCUSED && m_text_blink_tag != 0)) {
            invalidate_all();
        }

        gtk_im_context_focus_out(m_im_context);
        invalidate_cursor_once();

        m_mouse_pressed_buttons = 0;
        m_mouse_handled_buttons = 0;
    }

    m_has_focus = false;
    check_cursor_blink();
}

void VteTerminalPrivate::widget_enter(GdkEventCrossing *event)
{
    GdkEvent* base_event = reinterpret_cast<GdkEvent*>(event);
    auto pos = view_coords_from_event(base_event);

    _vte_debug_print(VTE_DEBUG_EVENTS, "Enter at %s\n", pos.to_string());

    m_mouse_cursor_over_widget = TRUE;
    m_mouse_last_position = pos;

    set_pointer_autohidden(false);
    hyperlink_hilite_update();
    apply_mouse_cursor();
}

void VteTerminalPrivate::widget_leave(GdkEventCrossing *event)
{
    GdkEvent* base_event = reinterpret_cast<GdkEvent*>(event);
    auto pos = view_coords_from_event(base_event);

    _vte_debug_print(VTE_DEBUG_EVENTS, "Leave at %s\n", pos.to_string());

    m_mouse_cursor_over_widget = FALSE;
    m_mouse_last_position = pos;

    hyperlink_hilite_update();
    apply_mouse_cursor();
}

static G_GNUC_UNUSED inline const char *visibility_state_str(GdkVisibilityState state)
{
    switch(state) {
        case GDK_VISIBILITY_FULLY_OBSCURED:
            return "fully-obscured";
        case GDK_VISIBILITY_UNOBSCURED:
            return "unobscured";
        default:
            return "partial";
    }
}

/* Apply the changed metrics, and queue a resize if need be.
 *
 * The cell's height consists of 4 parts, from top to bottom:
 * - char_spacing.top: half of the extra line spacing,
 * - char_ascent: the font's ascent,
 * - char_descent: the font's descent,
 * - char_spacing.bottom: the other half of the extra line spacing.
 * Extra line spacing is typically 0, beef up cell_height_scale to get actual pixels
 * here. Similarly, increase cell_width_scale to get nonzero char_spacing.{left,right}.
 */
void VteTerminalPrivate::apply_font_metrics(int cell_width,
                                            int cell_height,
                                            int char_ascent,
                                            int char_descent,
                                            GtkBorder char_spacing)
{
    int char_height;
    bool resize = false, cresize = false;

    /* Sanity check for broken font changes. */
    cell_width = MAX(cell_width, 1);
    cell_height = MAX(cell_height, 2);
    char_ascent = MAX(char_ascent, 1);
    char_descent = MAX(char_descent, 1);

    /* For convenience only. */
    char_height = char_ascent + char_descent;

    /* Change settings, and keep track of when we've changed anything. */
    if (cell_width != m_cell_width) {
        resize = cresize = true;
        m_cell_width = cell_width;
    }
    if (cell_height != m_cell_height) {
        resize = cresize = true;
        m_cell_height = cell_height;
    }
    if (char_ascent != m_char_ascent) {
        resize = true;
        m_char_ascent = char_ascent;
    }
    if (char_descent != m_char_descent) {
        resize = true;
        m_char_descent = char_descent;
    }
    if (memcmp(&char_spacing, &m_char_padding, sizeof(GtkBorder)) != 0) {
        resize = true;
        m_char_padding = char_spacing;
    }
    m_line_thickness = MAX (MIN (char_descent / 2, char_height / 14), 1);
    /* FIXME take these from pango_font_metrics_get_{underline,strikethrough}_{position,thickness} */
    m_underline_thickness = m_line_thickness;
    m_underline_position = MIN (char_spacing.top + char_ascent + m_line_thickness, cell_height - m_underline_thickness);
    m_double_underline_thickness = m_line_thickness;
    /* FIXME make sure this doesn't reach the baseline (switch to thinner lines, or one thicker line in that case) */
    m_double_underline_position = MIN (char_spacing.top + char_ascent + m_line_thickness, cell_height - 3 * m_double_underline_thickness);
    m_undercurl_thickness = m_line_thickness;
    m_undercurl_position = MIN (char_spacing.top + char_ascent + m_line_thickness, cell_height - _vte_draw_get_undercurl_height(cell_width, m_undercurl_thickness));
    m_strikethrough_thickness = m_line_thickness;
    m_strikethrough_position = char_spacing.top + char_ascent - char_height / 4;
    m_overline_thickness = m_line_thickness;
    m_overline_position = char_spacing.top;  /* FIXME */
    m_regex_underline_thickness = 1;  /* FIXME */
    m_regex_underline_position = char_spacing.top + char_height - m_regex_underline_thickness;  /* FIXME */

    /* Queue a resize if anything's changed. */
    if (resize) {
        if (widget_realized()) {
            gtk_widget_queue_resize_no_redraw(m_widget);
        }
    }
    /* Emit a signal that the font changed. */
    if (cresize) {
        emit_char_size_changed(m_cell_width, m_cell_height);
    }
    /* Repaint. */
    invalidate_all();
}

void VteTerminalPrivate::ensure_font()
{
    if (m_draw != NULL) {
        /* Load default fonts, if no fonts have been loaded. */
        if (!m_has_fonts) {
            set_font_desc(m_unscaled_font_desc);
        }
        if (m_fontdirty) {
            int cell_width, cell_height;
            int char_ascent, char_descent;
            GtkBorder char_spacing;
            m_fontdirty = FALSE;
            _vte_draw_set_text_font (m_draw, m_widget, m_fontdesc, 
                                     m_cell_width_scale, m_cell_height_scale);
            _vte_draw_get_text_metrics (m_draw,
                                        &cell_width, &cell_height,
                                        &char_ascent, &char_descent,
                                        &char_spacing);
            apply_font_metrics(cell_width, cell_height,
                                char_ascent, char_descent,
                                char_spacing);
        }
    }
}

void VteTerminalPrivate::update_font()
{
    /* We'll get called again later */
    if (m_unscaled_font_desc == nullptr) {
        return;
    }

    auto desc = pango_font_description_copy(m_unscaled_font_desc);

    double size = pango_font_description_get_size(desc);
    if (pango_font_description_get_size_is_absolute(desc)) {
        pango_font_description_set_absolute_size(desc, m_font_scale * size);
    } else {
        pango_font_description_set_size(desc, m_font_scale * size);
    }

    if (m_fontdesc) {
        pango_font_description_free(m_fontdesc);
    }
    m_fontdesc = desc;

    m_fontdirty = TRUE;
    m_has_fonts = TRUE;

    /* Set the drawing font. */
    if (widget_realized()) {
        ensure_font();
    }
}

/*
 * VteTerminalPrivate::set_font_desc:
 * @font_desc: (allow-none): a #PangoFontDescription for the desired font, or %nullptr
 *
 * Sets the font used for rendering all text displayed by the terminal,
 * overriding any fonts set using gtk_widget_modify_font().  The terminal
 * will immediately attempt to load the desired font, retrieve its
 * metrics, and attempt to resize itself to keep the same number of rows
 * and columns.  The font scale is applied to the specified font.
 */
bool VteTerminalPrivate::set_font_desc(PangoFontDescription const* font_desc)
{
    /* Create an owned font description. */
    PangoFontDescription *desc;

    auto context = gtk_widget_get_style_context(m_widget);
    gtk_style_context_save(context);
    gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);
    gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL, "font", &desc, nullptr);
    gtk_style_context_restore(context);

    pango_font_description_set_family_static (desc, "monospace");
    if (font_desc != nullptr) {
        pango_font_description_merge (desc, font_desc, TRUE);
        _VTE_DEBUG_IF(VTE_DEBUG_MISC) {
            if (desc) {
                char *tmp;
                tmp = pango_font_description_to_string(desc);
                g_printerr("Using pango font \"%s\".\n", tmp);
                g_free (tmp);
            }
        }
    } else {
        _vte_debug_print(VTE_DEBUG_MISC,
                "Using default monospace font.\n");
    }

    bool same_desc = m_unscaled_font_desc &&
            pango_font_description_equal(m_unscaled_font_desc, desc);

    /* Note that we proceed to recreating the font even if the description
     * are the same.  This is because maybe screen
     * font options were changed, or new fonts installed.  Those will be
     * detected at font creation time and respected.
     */

    /* Free the old font description and save the new one. */
    if (m_unscaled_font_desc != nullptr) {
        pango_font_description_free(m_unscaled_font_desc);
    }

    m_unscaled_font_desc = desc /* adopted */;

    update_font();

    return !same_desc;
}

bool VteTerminalPrivate::set_font_scale(gdouble scale)
{
    /* FIXME: compare old and new scale in pixel space */
    if (_vte_double_equal(scale, m_font_scale)) {
        return false;
    }

    m_font_scale = scale;
    update_font();

    return true;
}

bool VteTerminalPrivate::set_cell_width_scale(double scale)
{
    /* FIXME: compare old and new scale in pixel space */
    if (_vte_double_equal(scale, m_cell_width_scale)) {
        return false;
    }

    m_cell_width_scale = scale;
    /* Set the drawing font. */
    m_fontdirty = TRUE;
    if (widget_realized()) {
        ensure_font();
    }

    return true;
}

bool VteTerminalPrivate::set_cell_height_scale(double scale)
{
    /* FIXME: compare old and new scale in pixel space */
    if (_vte_double_equal(scale, m_cell_height_scale)) {
        return false;
    }

    m_cell_height_scale = scale;
    /* Set the drawing font. */
    m_fontdirty = TRUE;
    if (widget_realized()) {
        ensure_font();
    }

    return true;
}

/* Resize the given screen (normal or alternate) of the terminal. */
void VteTerminalPrivate::screen_set_size(VteScreen *screen_,
                                            long old_columns,
                                            long old_rows,
                                            bool do_rewrap)
{
    VteRing *ring = screen_->row_data;
    VteVisualPosition cursor_saved_absolute;
    VteVisualPosition below_viewport;
    VteVisualPosition below_current_paragraph;
    VteVisualPosition *markers[7];
    gboolean was_scrolled_to_top = ((long) ceil(screen_->scroll_delta) == _vte_ring_delta(ring));
    gboolean was_scrolled_to_bottom = ((long) screen_->scroll_delta == screen_->insert_delta);
    glong old_top_lines;
    double new_scroll_delta;

    if (m_selection_block_mode && do_rewrap && old_columns != m_column_count) {
        deselect_all();
    }

    _vte_debug_print(VTE_DEBUG_RESIZE,
            "Resizing %s screen_\n"
            "Old  insert_delta=%ld  scroll_delta=%f\n"
                        "     cursor (absolute)  row=%ld  col=%ld\n"
            "     cursor_saved (relative to insert_delta)  row=%ld  col=%ld\n",
            screen_ == &m_normal_screen ? "normal" : "alternate",
            screen_->insert_delta, screen_->scroll_delta,
            screen_->cursor.row, screen_->cursor.col,
            screen_->saved.cursor.row, screen_->saved.cursor.col);

    cursor_saved_absolute.row = screen_->saved.cursor.row + screen_->insert_delta;
    cursor_saved_absolute.col = screen_->saved.cursor.col;
    below_viewport.row = screen_->scroll_delta + old_rows;
    below_viewport.col = 0;
    below_current_paragraph.row = screen_->cursor.row + 1;
    while (below_current_paragraph.row < _vte_ring_next(ring)
        && _vte_ring_index(ring, below_current_paragraph.row - 1)->attr.soft_wrapped) {
        below_current_paragraph.row++;
    }
    below_current_paragraph.col = 0;
    memset(&markers, 0, sizeof(markers));
    markers[0] = &cursor_saved_absolute;
    markers[1] = &below_viewport;
    markers[2] = &below_current_paragraph;
    markers[3] = &screen_->cursor;
    if (m_has_selection) {
        /* selection_end is inclusive, make it non-inclusive, see bug 722635. */
        m_selection_end.col++;
        markers[4] = &m_selection_start;
        markers[5] = &m_selection_end;
    }

    old_top_lines = below_current_paragraph.row - screen_->insert_delta;

    if (do_rewrap && old_columns != m_column_count) {
        _vte_ring_rewrap(ring, m_column_count, markers);
    }

    if (_vte_ring_length(ring) > m_row_count) {
        /* The content won't fit without scrollbars. Before figuring out the position, we might need to
           drop some lines from the ring if the cursor is not at the bottom, as XTerm does. See bug 708213.
           This code is really tricky, see ../doc/rewrap.txt for details! */
        glong new_top_lines, drop1, drop2, drop3, drop;
        screen_->insert_delta = _vte_ring_next(ring) - m_row_count;
        new_top_lines = below_current_paragraph.row - screen_->insert_delta;
        drop1 = _vte_ring_length(ring) - m_row_count;
        drop2 = _vte_ring_next(ring) - below_current_paragraph.row;
        drop3 = old_top_lines - new_top_lines;
        drop = MIN(MIN(drop1, drop2), drop3);
        if (drop > 0) {
            int new_ring_next = screen_->insert_delta + m_row_count - drop;
            _vte_debug_print(VTE_DEBUG_RESIZE,
                    "Dropping %ld [== MIN(%ld, %ld, %ld)] rows at the bottom\n",
                    drop, drop1, drop2, drop3);
            _vte_ring_shrink(ring, new_ring_next - _vte_ring_delta(ring));
        }
    }

    if (m_has_selection) {
        /* Make selection_end inclusive again, see above. */
        m_selection_end.col--;
    }

    /* Figure out new insert and scroll deltas */
    if (_vte_ring_length(ring) <= m_row_count) {
        /* Everything fits without scrollbars. Align at top. */
        screen_->insert_delta = _vte_ring_delta(ring);
        new_scroll_delta = screen_->insert_delta;
        _vte_debug_print(VTE_DEBUG_RESIZE, "Everything fits without scrollbars\n");
    } else {
        /* Scrollbar required. Can't afford unused lines at bottom. */
        screen_->insert_delta = _vte_ring_next(ring) - m_row_count;
        if (was_scrolled_to_bottom) {
            /* Was scrolled to bottom, keep this way. */
            new_scroll_delta = screen_->insert_delta;
            _vte_debug_print(VTE_DEBUG_RESIZE, "Scroll to bottom\n");
        } else if (was_scrolled_to_top) {
            /* Was scrolled to top, keep this way. Not sure if this special case is worth it. */
            new_scroll_delta = _vte_ring_delta(ring);
            _vte_debug_print(VTE_DEBUG_RESIZE, "Scroll to top\n");
        } else {
            /* Try to scroll so that the bottom visible row stays.
               More precisely, the character below the bottom left corner stays in that
               (invisible) row.
               So if the bottom of the screen_ was at a hard line break then that hard
               line break will stay there.
               TODO: What would be the best behavior if the bottom of the screen_ is a
               soft line break, i.e. only a partial line is visible at the bottom? */
            new_scroll_delta = below_viewport.row - m_row_count;
            /* Keep the old fractional part. */
            new_scroll_delta += screen_->scroll_delta - floor(screen_->scroll_delta);
            _vte_debug_print(VTE_DEBUG_RESIZE, "Scroll so bottom row stays\n");
        }
    }

    /* Don't clamp, they'll be clamped when restored. Until then remember off-screen_ values
       since they might become on-screen_ again on subsequent resizes. */
        screen_->saved.cursor.row = cursor_saved_absolute.row - screen_->insert_delta;
        screen_->saved.cursor.col = cursor_saved_absolute.col;

    _vte_debug_print(VTE_DEBUG_RESIZE,
            "New  insert_delta=%ld  scroll_delta=%f\n"
                        "     cursor (absolute)  row=%ld  col=%ld\n"
            "     cursor_saved (relative to insert_delta)  row=%ld  col=%ld\n\n",
            screen_->insert_delta, new_scroll_delta,
                        screen_->cursor.row, screen_->cursor.col,
                        screen_->saved.cursor.row, screen_->saved.cursor.col);

    if (screen_ == m_screen) {
        queue_adjustment_value_changed(new_scroll_delta);
    } else {
        screen_->scroll_delta = new_scroll_delta;
    }
}

void VteTerminalPrivate::set_size(long columns, long rows)
{
    glong old_columns, old_rows;

    _vte_debug_print(VTE_DEBUG_RESIZE,
            "Setting PTY size to %ldx%ld.\n",
            columns, rows);

    old_rows = m_row_count;
    old_columns = m_column_count;
    {
        m_row_count = rows;
        m_column_count = columns;
    }
    if (old_rows != m_row_count || old_columns != m_column_count) {
        m_scrolling_restricted = FALSE;

        _vte_ring_set_visible_rows(m_normal_screen.row_data, m_row_count);
        _vte_ring_set_visible_rows(m_alternate_screen.row_data, m_row_count);

        /* Resize the normal screen and (if rewrapping is enabled) rewrap it even if the alternate screen is visible: bug 415277 */
        screen_set_size(&m_normal_screen, old_columns, old_rows, m_rewrap_on_resize);
        /* Resize the alternate screen if it's the current one, but never rewrap it: bug 336238 comment 60 */
        if (m_screen == &m_alternate_screen) {
            screen_set_size(&m_alternate_screen, old_columns, old_rows, false);
        }

        /* Ensure scrollback buffers cover the screen. */
        set_scrollback_lines(m_scrollback_lines);

        /* Ensure the cursor is valid */
        m_screen->cursor.row = CLAMP (m_screen->cursor.row,
                                        _vte_ring_delta (m_screen->row_data),
                                        MAX (_vte_ring_delta (m_screen->row_data),
                                            _vte_ring_next (m_screen->row_data) - 1));

        adjust_adjustments_full();
        gtk_widget_queue_resize_no_redraw(m_widget);
        /* Our visible text changed. */
        emit_text_modified();
    }
}

/* Redraw the widget. */
static void vte_terminal_vadjustment_value_changed_cb(VteTerminalPrivate *that)
{
    that->vadjustment_value_changed();
}

void VteTerminalPrivate::vadjustment_value_changed()
{
    /* Read the new adjustment value and save the difference. */
    double adj = gtk_adjustment_get_value(m_vadjustment);
    double dy = adj - m_screen->scroll_delta;
    m_screen->scroll_delta = adj;

    /* Sanity checks. */
    if (G_UNLIKELY(!widget_realized())) {
            return;
    }

        /* FIXME: do this check in pixel space */
    if (!_vte_double_equal(dy, 0)) {
        _vte_debug_print(VTE_DEBUG_ADJ,
                "Scrolling by %f\n", dy);
                invalidate_all();
        emit_text_scrolled(dy);
        queue_contents_changed();
    } else {
        _vte_debug_print(VTE_DEBUG_ADJ, "Not scrolling\n");
    }
}

void VteTerminalPrivate::widget_set_hadjustment(GtkAdjustment *adjustment)
{
    if (adjustment == m_hadjustment) {
        return;
    }

    if (m_hadjustment) {
        g_object_unref (m_hadjustment);
    }

    m_hadjustment = adjustment ? (GtkAdjustment *)g_object_ref_sink(adjustment) : nullptr;
}

void VteTerminalPrivate::widget_set_vadjustment(GtkAdjustment *adjustment)
{
    if (adjustment != nullptr && adjustment == m_vadjustment) {
        return;
    }
    if (adjustment == nullptr && m_vadjustment != nullptr) {
        return;
    }

    if (adjustment == nullptr) {
        adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 0, 0, 0, 0));
    }

    /* Add a reference to the new adjustment object. */
    g_object_ref_sink(adjustment);
    /* Get rid of the old adjustment object. */
    if (m_vadjustment != nullptr) {
        /* Disconnect our signal handlers from this object. */
        g_signal_handlers_disconnect_by_func(m_vadjustment,
                             (void*)vte_terminal_vadjustment_value_changed_cb,
                             this);
        g_object_unref(m_vadjustment);
    }

    /* Set the new adjustment object. */
    m_vadjustment = adjustment;

    /* We care about the offset, not the top or bottom. */
    g_signal_connect_swapped(m_vadjustment,
                 "value-changed",
                 G_CALLBACK(vte_terminal_vadjustment_value_changed_cb),
                 this);
}

VteTerminalPrivate::VteTerminalPrivate(VteTerminal *t) :
        m_terminal(t),
        m_widget(&t->widget)
{
    /* Inits allocation to 1x1 @ -1,-1 */
    cairo_rectangle_int_t allocation;
    gtk_widget_get_allocation(m_widget, &allocation);
    set_allocated_rect(allocation);

    int i;
    GdkDisplay *display;

    /* NOTE! We allocated zeroed memory, just fill in non-zero stuff. */

    gtk_widget_set_can_focus(m_widget, TRUE);

    /* We do our own redrawing. */
    /* FIXMEchpe still necessary? */
    gtk_widget_set_redraw_on_allocate(m_widget, FALSE);

    m_invalidated_all = false;
    m_update_rects = g_array_sized_new(FALSE /* zero terminated */,
                                        FALSE /* clear */,
                                        sizeof(cairo_rectangle_int_t),
                                        32 /* preallocated size */);

    /* Set an adjustment for the application to use to control scrolling. */
    m_vadjustment = nullptr;
    m_hadjustment = nullptr;

    /* GtkScrollable */
    m_hscroll_policy = GTK_SCROLL_NATURAL;
    m_vscroll_policy = GTK_SCROLL_NATURAL;

    widget_set_hadjustment(nullptr);
    widget_set_vadjustment(nullptr);

    /* Set up dummy metrics, value != 0 to avoid division by 0 */
    m_cell_width = 1;
    m_cell_height = 1;
    m_char_ascent = 1;
    m_char_descent = 1;
    m_char_padding = {0, 0, 0, 0};
    m_line_thickness = 1;
    m_underline_position = 1;
    m_underline_thickness = 1;
    m_double_underline_position = 1;
    m_double_underline_thickness = 1;
    m_undercurl_position = 1.;
    m_undercurl_thickness = 1.;
    m_strikethrough_position = 1;
    m_strikethrough_thickness = 1;
    m_overline_position = 1;
    m_overline_thickness = 1;
    m_regex_underline_position = 1;
    m_regex_underline_thickness = 1;

    m_row_count = VTE_ROWS;
    m_column_count = VTE_COLUMNS;

    /* Initialize the screens and histories. */
    _vte_ring_init (m_alternate_screen.row_data, m_row_count, FALSE);
    m_screen = &m_alternate_screen;
    _vte_ring_init (m_normal_screen.row_data, VTE_SCROLLBACK_INIT, TRUE);
    m_screen = &m_normal_screen;

    reset_default_attributes(true);

    /* Initialize charset modes. */
    m_character_replacements[0] = VTE_CHARACTER_REPLACEMENT_NONE;
    m_character_replacements[1] = VTE_CHARACTER_REPLACEMENT_NONE;
    m_character_replacement = &m_character_replacements[0];

    /* Set up the desired palette. */
    set_colors_default();
    for (i = 0; i < VTE_PALETTE_SIZE; i++) {
        m_palette[i].sources[VTE_COLOR_SOURCE_ESCAPE].is_set = FALSE;
    }

    /* Set up I/O encodings. */
    m_utf8_ambiguous_width = VTE_DEFAULT_UTF8_AMBIGUOUS_WIDTH;
    m_iso2022 = _vte_iso2022_state_new(m_encoding);
    m_incoming = nullptr;
    m_pending = g_array_new(FALSE, TRUE, sizeof(gunichar));
    m_max_input_bytes = VTE_MAX_INPUT_READ;
    m_cursor_blink_tag = 0;
    m_text_blink_tag = 0;
    m_outgoing = _vte_byte_array_new();
    m_outgoing_conv = VTE_INVALID_CONV;
    m_conv_buffer = _vte_byte_array_new();
    set_encoding(nullptr /* UTF-8 */);
    g_assert_cmpstr(m_encoding, ==, "UTF-8");
    m_last_graphic_character = 0;

        /* Set up the emulation. */
    m_keypad_mode = VTE_KEYMODE_NORMAL;
    m_cursor_mode = VTE_KEYMODE_NORMAL;
    m_autowrap = TRUE;
    m_sendrecv_mode = TRUE;
    m_dec_saved = g_hash_table_new(NULL, NULL);
    m_matcher = _vte_matcher_new();
    /* Scrolling options. */
    m_scroll_on_keystroke = TRUE;
    m_alternate_screen_scroll = TRUE;
    m_scrollback_lines = -1; /* force update in vte_terminal_set_scrollback_lines */
    set_scrollback_lines(VTE_SCROLLBACK_INIT);

    /* Selection info. */
    display = gtk_widget_get_display(m_widget);
    m_clipboard[VTE_SELECTION_PRIMARY] = gtk_clipboard_get_for_display(display, GDK_SELECTION_PRIMARY);
    m_clipboard[VTE_SELECTION_CLIPBOARD] = gtk_clipboard_get_for_display(display, GDK_SELECTION_CLIPBOARD);
    m_selection_owned[VTE_SELECTION_PRIMARY] = false;
    m_selection_owned[VTE_SELECTION_CLIPBOARD] = false;

    /* Miscellaneous options. */
    set_backspace_binding(VTE_ERASE_AUTO);
    set_delete_binding(VTE_ERASE_AUTO);
    m_meta_sends_escape = TRUE;
    m_audible_bell = TRUE;
    m_text_blink_mode = VTE_TEXT_BLINK_ALWAYS;
    m_allow_bold = TRUE;
    m_bold_is_bright = TRUE;
    m_deccolm_mode = FALSE;
    m_rewrap_on_resize = TRUE;
    set_default_tabstops();

    m_input_enabled = TRUE;

    /* Cursor shape. */
    m_cursor_shape = VTE_CURSOR_SHAPE_BLOCK;
    m_cursor_aspect_ratio = 0.04;

    /* Cursor blinking. */
    m_cursor_visible = TRUE;
    m_cursor_blink_timeout = 500;
    m_cursor_blinks = FALSE;
    m_cursor_blink_mode = VTE_CURSOR_BLINK_SYSTEM;

    /* DECSCUSR cursor style (shape and blinking possibly overridden
     * via escape sequence) */
    m_cursor_style = VTE_CURSOR_STYLE_TERMINAL_DEFAULT;

    /* Initialize the saved cursor. */
    save_cursor(&m_normal_screen);
    save_cursor(&m_alternate_screen);

    /* Rendering data */
    m_draw = _vte_draw_new();

    /* Set up background information. */
    m_background_alpha = 1.;

    /* Word chars */
    set_word_char_exceptions(WORD_CHAR_EXCEPTIONS_DEFAULT);

    /* Selection */
    m_selection_block_mode = FALSE;
    m_unscaled_font_desc = nullptr;
    m_fontdesc = nullptr;
    m_font_scale = 1.;
    m_cell_width_scale = 1.;
    m_cell_height_scale = 1.;
    m_has_fonts = FALSE;

    /* Hyperlink */
    m_allow_hyperlink = FALSE;
    m_hyperlink_auto_id = 0;

    /* Mouse */
    m_mouse_last_position = vte::view::coords(-1, -1);

    m_padding = default_padding;
    update_view_extents();

#ifdef VTE_DEBUG
    if (g_test_mode) {
        static char const warning[] = "\e[1m\e[31mWARNING:\e[39m Test mode enabled.\e[0m\n\e[G";
        feed(warning, strlen(warning), false);
    }
#endif
}

void VteTerminalPrivate::widget_constructed()
{
   /* Set the style as early as possible, before GTK+ starts
    * invoking various callbacks. This is needed in order to
    * compute the initial geometry correctly in presence of
    * non-default padding, see bug 787710. */
    widget_style_updated();
}

void VteTerminalPrivate::widget_get_preferred_width(int *minimum_width,
                                               int *natural_width)
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_get_preferred_width()\n");

    ensure_font();

#if 0
    refresh_size(); /* FIXME */
#endif
    *minimum_width = m_cell_width * 1;
    *natural_width = m_cell_width * m_column_count;

    *minimum_width += m_padding.left +
                          m_padding.right;
    *natural_width += m_padding.left +
                          m_padding.right;

    _vte_debug_print(VTE_DEBUG_WIDGET_SIZE,
            "[Terminal %p] minimum_width=%d, natural_width=%d for %ldx%ld cells (padding %d,%d;%d,%d).\n",
            m_terminal,
            *minimum_width, *natural_width,
            m_column_count,
            m_row_count,
            m_padding.left, m_padding.right, m_padding.top, m_padding.bottom);
}

void VteTerminalPrivate::widget_get_preferred_height(int *minimum_height,
                                                        int *natural_height)
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_get_preferred_height()\n");

    ensure_font();

#if 0
    refresh_size(); /* FIXME */
#endif

    *minimum_height = m_cell_height * 1;
    *natural_height = m_cell_height * m_row_count;

    *minimum_height += m_padding.top +
               m_padding.bottom;
    *natural_height += m_padding.top +
               m_padding.bottom;

    _vte_debug_print(VTE_DEBUG_WIDGET_SIZE,
            "[Terminal %p] minimum_height=%d, natural_height=%d for %ldx%ld cells (padding %d,%d;%d,%d).\n",
            m_terminal,
            *minimum_height, *natural_height,
            m_column_count,
            m_row_count,
            m_padding.left, m_padding.right, m_padding.top, m_padding.bottom);
}

void VteTerminalPrivate::widget_size_allocate(GtkAllocation *allocation)
{
    glong width, height;
    gboolean repaint, update_scrollback;

    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_size_allocate()\n");

    width = (allocation->width - (m_padding.left + m_padding.right)) / m_cell_width;
    height = (allocation->height - (m_padding.top + m_padding.bottom)) / m_cell_height;
    width = MAX(width, 1);
    height = MAX(height, 1);

    _vte_debug_print(VTE_DEBUG_WIDGET_SIZE,
            "[Terminal %p] Sizing window to %dx%d (%ldx%ld, padding %d,%d;%d,%d).\n",
            m_terminal,
            allocation->width, allocation->height,
            width, height,
            m_padding.left, m_padding.right, m_padding.top, m_padding.bottom);

    auto current_allocation = get_allocated_rect();

    repaint = current_allocation.width != allocation->width
            || current_allocation.height != allocation->height;
    update_scrollback = current_allocation.height != allocation->height;

    /* Set our allocation to match the structure. */
    gtk_widget_set_allocation(m_widget, allocation);
    set_allocated_rect(*allocation);

    if (width != m_column_count
     || height != m_row_count
     || update_scrollback) {
        /* Set the size of the pseudo-terminal. */
        set_size(width, height);

        /* Notify viewers that the contents have changed. */
        queue_contents_changed();
    }

    /* Resize the GDK window. */
    if (widget_realized()) {
        gdk_window_move_resize(m_event_window,
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);
        /* Force a repaint if we were resized. */
        if (repaint) {
            reset_update_rects();
            invalidate_all();
        }
    }
}

void VteTerminalPrivate::widget_unrealize()
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_unrealize()\n");

    /* Deallocate the cursors. */
    m_mouse_cursor_over_widget = FALSE;
    g_object_unref(m_mouse_default_cursor);
    m_mouse_default_cursor = NULL;
    g_object_unref(m_mouse_mousing_cursor);
    m_mouse_mousing_cursor = NULL;
    g_object_unref(m_mouse_hyperlink_cursor);
    m_mouse_hyperlink_cursor = NULL;
    g_object_unref(m_mouse_inviso_cursor);
    m_mouse_inviso_cursor = NULL;

    match_hilite_clear();

    /* Shut down input methods. */
    if (m_im_context != nullptr) {
        g_signal_handlers_disconnect_matched(m_im_context, G_SIGNAL_MATCH_DATA,
                                                    0, 0, NULL, NULL, this);
        im_reset();
        gtk_im_context_set_client_window(m_im_context, NULL);
        g_object_unref(m_im_context);
        m_im_context = nullptr;
    }
    m_im_preedit_active = FALSE;
    if (m_im_preedit != nullptr) {
        g_free(m_im_preedit);
        m_im_preedit = NULL;
    }
    if (m_im_preedit_attrs != NULL) {
        pango_attr_list_unref(m_im_preedit_attrs);
        m_im_preedit_attrs = NULL;
    }
    m_im_preedit_cursor = 0;

    /* Clean up our draw structure. */
    if (m_draw != NULL) {
        _vte_draw_free(m_draw);
        m_draw = NULL;
    }
    m_fontdirty = TRUE;

    /* Unmap the widget if it hasn't been already. */
    /* FIXMEchpe this can't happen */
    if (gtk_widget_get_mapped(m_widget)) {
        gtk_widget_unmap(m_widget);
    }

    /* Remove the cursor blink timeout function. */
    remove_cursor_timeout();

    /* Remove the contents blink timeout function. */
    remove_text_blink_timeout();

    /* Cancel any pending redraws. */
    remove_update_timeout(this);

    /* Cancel any pending signals */
    m_contents_changed_pending = FALSE;
    m_cursor_moved_pending = FALSE;
    m_text_modified_flag = FALSE;
    m_text_inserted_flag = FALSE;
    m_text_deleted_flag = FALSE;

    /* Clear modifiers. */
    m_modifiers = 0;

    /* Destroy the even window */
    gtk_widget_unregister_window(m_widget, m_event_window);
    gdk_window_destroy(m_event_window);
    m_event_window = nullptr;
}

static void vte_terminal_settings_notify_cb (GtkSettings *settings,
                                                GParamSpec *pspec,
                                                VteTerminalPrivate *that)
{
    that->widget_settings_notify();
}

void VteTerminalPrivate::widget_settings_notify()
{
    gboolean blink;
    int blink_time = 1000;
    int blink_timeout = G_MAXINT;

    g_object_get(gtk_widget_get_settings(m_widget),
                    "gtk-cursor-blink", &blink,
                    "gtk-cursor-blink-time", &blink_time,
                    "gtk-cursor-blink-timeout", &blink_timeout,
                    nullptr);

    _vte_debug_print(VTE_DEBUG_MISC,
                        "Cursor blinking settings: blink=%d time=%d timeout=%d\n",
                        blink, blink_time, blink_timeout);

    m_cursor_blink_cycle = blink_time / 2;
    m_cursor_blink_timeout = blink_timeout;

    update_cursor_blinks();

    /* Misuse gtk-cursor-blink-time for text blinking as well. This might change in the future. */
    m_text_blink_cycle = m_cursor_blink_cycle;
    if (m_text_blink_tag != 0) {
       /* The current phase might have changed, and an already installed
        * timer to blink might fire too late. So remove the timer and
        * repaint the contents (which will install a correct new timer). */
        remove_text_blink_timeout();
        invalidate_all();
    }
}

void VteTerminalPrivate::widget_screen_changed (GdkScreen *previous_screen)
{
    GtkSettings *settings;

    auto gdk_screen = gtk_widget_get_screen (m_widget);
    if (previous_screen != NULL &&
        (gdk_screen != previous_screen || gdk_screen == NULL)) {
        settings = gtk_settings_get_for_screen (previous_screen);
        g_signal_handlers_disconnect_matched (settings, G_SIGNAL_MATCH_DATA,
                                                0, 0, NULL, NULL, this);
    }

    if (gdk_screen == previous_screen || gdk_screen == nullptr)
            return;

    widget_settings_notify();

    settings = gtk_widget_get_settings(m_widget);
    g_signal_connect (settings, "notify::gtk-cursor-blink",
                        G_CALLBACK (vte_terminal_settings_notify_cb), this);
    g_signal_connect (settings, "notify::gtk-cursor-blink-time",
                        G_CALLBACK (vte_terminal_settings_notify_cb), this);
    g_signal_connect (settings, "notify::gtk-cursor-blink-timeout",
                        G_CALLBACK (vte_terminal_settings_notify_cb), this);
}

VteTerminalPrivate::~VteTerminalPrivate()
{
    int sel;

    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_finalize()\n");

    /* Free the draw structure. */
    if (m_draw != NULL) {
        _vte_draw_free(m_draw);
    }

    /* The NLS maps. */
    _vte_iso2022_state_free(m_iso2022);

    /* Free the font description. */
    if (m_unscaled_font_desc != NULL) {
            pango_font_description_free(m_unscaled_font_desc);
    }
    if (m_fontdesc != NULL) {
        pango_font_description_free(m_fontdesc);
    }

    /* Free matching data. */
    if (m_match_attributes != NULL) {
        g_array_free(m_match_attributes, TRUE);
    }
    g_free(m_match_contents);

    if (m_search_attrs) {
        g_array_free (m_search_attrs, TRUE);
    }

    /* Disconnect from autoscroll requests. */
    stop_autoscroll();

    /* Cancel pending adjustment change notifications. */
    m_adjustment_changed_pending = FALSE;

    /* Tabstop information. */
    if (m_tabstops) {
        g_hash_table_destroy(m_tabstops);
    }

    /* Free any selected text, but if we currently own the selection,
     * throw the text onto the clipboard without an owner so that it
     * doesn't just disappear. */
    for (sel = VTE_SELECTION_PRIMARY; sel < LAST_VTE_SELECTION; sel++) {
        if (m_selection[sel] != nullptr) {
            if (m_selection_owned[sel]) {
                /* FIXMEchpe we should check m_selection_format[sel]
                 * and also put text/html on if it's VTE_FORMAT_HTML */
                gtk_clipboard_set_text(m_clipboard[sel],
                                        m_selection[sel]->str,
                                        m_selection[sel]->len);
            }
            g_string_free(m_selection[sel], TRUE);
            m_selection[sel] = nullptr;
        }
    }

    /* Clear the output histories. */
    _vte_ring_fini(m_normal_screen.row_data);
    _vte_ring_fini(m_alternate_screen.row_data);

    /* Free conversion descriptors. */
    if (m_outgoing_conv != VTE_INVALID_CONV) {
        _vte_conv_close(m_outgoing_conv);
        m_outgoing_conv = VTE_INVALID_CONV;
    }
    /* Stop processing input. */
    stop_processing(this);

    /* Discard any pending data. */
    _vte_incoming_chunks_release(m_incoming);
    _vte_byte_array_free(m_outgoing);
    g_array_free(m_pending, TRUE);
    _vte_byte_array_free(m_conv_buffer);

    /* Remove hash tables. */
    if (m_dec_saved != NULL) {
        g_hash_table_destroy(m_dec_saved);
    }

    /* Clean up emulation structures. */
    if (m_matcher != NULL) {
        _vte_matcher_free(m_matcher);
    }

    remove_update_timeout(this);

    /* discard title updates */
    g_free(m_window_title);
    g_free(m_window_title_changed);
    g_free(m_icon_title_changed);
    g_free(m_current_directory_uri_changed);
    g_free(m_current_directory_uri);
    g_free(m_current_file_uri_changed);
    g_free(m_current_file_uri);

    /* Word char exceptions */
    g_free(m_word_char_exceptions_string);
    g_free(m_word_char_exceptions);

    /* Free public-facing data. */
    g_free(m_icon_title);
    if (m_vadjustment != NULL) {
        /* Disconnect our signal handlers from this object. */
        g_signal_handlers_disconnect_by_func(m_vadjustment,
                             (void*)vte_terminal_vadjustment_value_changed_cb,
                             this);
        g_object_unref(m_vadjustment);
    }

    g_signal_handlers_disconnect_matched (gtk_widget_get_settings(m_widget), G_SIGNAL_MATCH_DATA,
                                            0, 0, NULL, NULL, this);

    /* Update rects */
    g_array_free(m_update_rects, TRUE /* free segment */);
}

void VteTerminalPrivate::widget_realize()
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_realize()\n");

    auto allocation = get_allocated_rect();

    /* Create the stock cursors. */
    m_mouse_cursor_over_widget = FALSE;  /* We'll receive an enter_notify_event if the window appears under the cursor. */
    m_mouse_default_cursor = widget_cursor_new(VTE_DEFAULT_CURSOR);
    m_mouse_mousing_cursor = widget_cursor_new(VTE_MOUSING_CURSOR);
    if (_vte_debug_on(VTE_DEBUG_HYPERLINK)) {
        /* Differ from the standard regex match cursor in debug mode. */
        m_mouse_hyperlink_cursor = widget_cursor_new(VTE_HYPERLINK_CURSOR_DEBUG);
    } else {
        m_mouse_hyperlink_cursor = widget_cursor_new(VTE_HYPERLINK_CURSOR);
    }
    m_mouse_inviso_cursor = widget_cursor_new(GDK_BLANK_CURSOR);

    /* Create a GDK window for the widget. */
    GdkWindowAttr attributes;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_ONLY;
    attributes.visual = gtk_widget_get_visual(m_widget);
    attributes.event_mask = gtk_widget_get_events(m_widget) |
                            GDK_EXPOSURE_MASK |
                            GDK_FOCUS_CHANGE_MASK |
                            GDK_SMOOTH_SCROLL_MASK |
                            GDK_SCROLL_MASK |
                            GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_BUTTON1_MOTION_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_LEAVE_NOTIFY_MASK |
                            GDK_KEY_PRESS_MASK |
                            GDK_KEY_RELEASE_MASK;
    attributes.cursor = m_mouse_default_cursor;
    guint attributes_mask = GDK_WA_X |
                            GDK_WA_Y |
                            (attributes.visual ? GDK_WA_VISUAL : 0) |
                            GDK_WA_CURSOR;

    m_event_window = gdk_window_new(gtk_widget_get_parent_window (m_widget),
                                        &attributes, attributes_mask);
    gtk_widget_register_window(m_widget, m_event_window);

    /* Create rendering data if this is a re-realise */
    if (m_draw == NULL) {
        m_draw = _vte_draw_new();
    }

        // FIXMEchpe this shouldn't ever be true:
    if (m_im_context != nullptr) {
        g_signal_handlers_disconnect_matched(m_im_context, G_SIGNAL_MATCH_DATA,
                                                    0, 0, NULL, NULL, this);
        im_reset();
        gtk_im_context_set_client_window(m_im_context, nullptr);
        g_object_unref(m_im_context);
        m_im_context = nullptr;
    }
    m_im_preedit_active = FALSE;
    m_im_context = gtk_im_multicontext_new();
    gtk_im_context_set_client_window(m_im_context, m_event_window);
/* FIXME
    g_signal_connect(m_im_context, "commit",
             G_CALLBACK(vte_terminal_im_commit_cb), this);
*/
    g_signal_connect(m_im_context, "preedit-start",
             G_CALLBACK(vte_terminal_im_preedit_start_cb), this);
    g_signal_connect(m_im_context, "preedit-changed",
             G_CALLBACK(vte_terminal_im_preedit_changed_cb), this);
    g_signal_connect(m_im_context, "preedit-end",
             G_CALLBACK(vte_terminal_im_preedit_end_cb), this);
    g_signal_connect(m_im_context, "retrieve-surrounding",
             G_CALLBACK(vte_terminal_im_retrieve_surrounding_cb), this);
    g_signal_connect(m_im_context, "delete-surrounding",
             G_CALLBACK(vte_terminal_im_delete_surrounding_cb), this);
    gtk_im_context_set_use_preedit(m_im_context, TRUE);

    /* Clear modifiers. */
    m_modifiers = 0;

    ensure_font();
}

void VteTerminalPrivate::widget_map()
{
    if (m_event_window) {
        gdk_window_show_unraised(m_event_window);
    }
}

void VteTerminalPrivate::widget_unmap()
{
    if (m_event_window) {
        gdk_window_hide(m_event_window);
    }
}

static inline void swap (guint *a, guint *b)
{
    guint tmp;
    tmp = *a, *a = *b, *b = tmp;
}

/* FIXMEchpe probably @attr should be passed by ref */
void VteTerminalPrivate::determine_colors(VteCellAttr const* attr,
                                            bool is_selected,
                                            bool is_cursor,
                                            guint *pfore,
                                            guint *pback,
                                            guint *pdeco) const
{
    guint fore, back, deco;

    g_assert(attr);

    /* Start with cell colors */
    vte_color_triple_get(attr->colors(), &fore, &back, &deco);

    /* Reverse-mode switches default fore and back colors */
    if (G_UNLIKELY (m_reverse_mode)) {
        if (fore == VTE_DEFAULT_FG) {
            fore = VTE_DEFAULT_BG;
        }
        if (back == VTE_DEFAULT_BG) {
            back = VTE_DEFAULT_FG;
        }
    }

    /* Handle bold by using set bold color or brightening */
    if (attr->bold()) {
        if (fore == VTE_DEFAULT_FG && get_color(VTE_BOLD_FG) != NULL) {
            fore = VTE_BOLD_FG;
        } else if (m_bold_is_bright &&
                    fore >= VTE_LEGACY_COLORS_OFFSET &&
                    fore < VTE_LEGACY_COLORS_OFFSET + VTE_LEGACY_COLOR_SET_SIZE) {
            fore += VTE_COLOR_BRIGHT_OFFSET;
        }
    }

   /* Handle dim colors.  Only apply to palette colors, dimming direct RGB wouldn't make sense.
    * Apply to the foreground color only, but do this before handling reverse/highlight so that
    * those can be used to dim the background instead. */
    if (attr->dim() && !(fore & VTE_RGB_COLOR_MASK(8, 8, 8))) {
        fore |= VTE_DIM_COLOR;
    }

    /* Reverse cell? */
    if (attr->reverse()) {
        swap (&fore, &back);
    }

    /* Selection: use hightlight back/fore, or inverse */
    if (is_selected) {
        /* XXX what if hightlight back is same color as current back? */
        bool do_swap = true;
        if (get_color(VTE_HIGHLIGHT_BG) != NULL) {
            back = VTE_HIGHLIGHT_BG;
            do_swap = false;
        }
        if (get_color(VTE_HIGHLIGHT_FG) != NULL) {
            fore = VTE_HIGHLIGHT_FG;
            do_swap = false;
        }
        if (do_swap) {
            swap (&fore, &back);
        }
    }

    /* Cursor: use cursor back, or inverse */
    if (is_cursor) {
        /* XXX what if cursor back is same color as current back? */
        bool do_swap = true;
        if (get_color(VTE_CURSOR_BG) != NULL) {
            back = VTE_CURSOR_BG;
            do_swap = false;
        }
        if (get_color(VTE_CURSOR_FG) != NULL) {
            fore = VTE_CURSOR_FG;
            do_swap = false;
        }
        if (do_swap) {
            swap (&fore, &back);
        }
    }

   /* Invisible? */
   /* FIXME: This is dead code, this is not where we actually handle invisibile.
    * Instead, draw_cells() is not called from draw_rows().
    * That is required for the foreground to be transparent if so is the background. */
    if (attr->invisible()) {
        fore = back;
        deco = VTE_DEFAULT_FG;
    }

    *pfore = fore;
    *pback = back;
    *pdeco = deco;
}

void VteTerminalPrivate::determine_colors(VteCell const* cell,
                                            bool highlight,
                                            guint *fore,
                                            guint *back,
                                            guint *deco) const
{
    determine_colors(cell ? &cell->attr : &basic_cell.attr,
                        highlight, false /* not cursor */,
                        fore, back, deco);
}

void VteTerminalPrivate::determine_cursor_colors(VteCell const* cell,
                                                    bool highlight,
                                                    guint *fore,
                                                    guint *back,
                                                    guint *deco) const
{
    determine_colors(cell ? &cell->attr : &basic_cell.attr,
                        highlight, true /* cursor */,
                        fore, back, deco);
}

static gboolean invalidate_text_blink_cb(VteTerminalPrivate *that)
{
    that->m_text_blink_tag = 0;
    that->invalidate_all();
    return G_SOURCE_REMOVE;
}

/* Draw a string of characters with similar attributes. */
void VteTerminalPrivate::draw_cells(struct _vte_draw_text_request *items,
                                    gssize n,
                                    uint32_t fore,
                                    uint32_t back,
                                    uint32_t deco,
                                    bool clear,
                                    bool draw_default_bg,
                                    uint32_t attr,
                                    bool hyperlink,
                                    bool hilite,
                                    int column_width,
                                    int row_height)
{
    int i, x, y;
    gint columns = 0;
    vte::color::rgb fg, bg, dc;

    g_assert(n > 0);
#if 0
    _VTE_DEBUG_IF(VTE_DEBUG_CELLS) {
        GString *str = g_string_new (NULL);
        gchar *tmp;
        for (i = 0; i < n; i++) {
            g_string_append_unichar (str, items[i].c);
        }
        tmp = g_string_free (str, FALSE);
                g_printerr ("draw_cells('%s', fore=%d, back=%d, deco=%d, bold=%d,"
                                " ul=%d, strike=%d, ol=%d, blink=%d,"
                                " hyperlink=%d, hilite=%d, boxed=%d)\n",
                                tmp, fore, back, deco, bold,
                                underline, strikethrough, overline, blink,
                                hyperlink, hilite, boxed);
        g_free (tmp);
    }
#endif

    auto bold = (attr & VTE_ATTR_BOLD) != 0;
    rgb_from_index<8, 8, 8>(fore, fg);
    rgb_from_index<8, 8, 8>(back, bg);
    /* FIXMEchpe defer resolving deco color until we actually need to draw an underline? */
    if (deco == VTE_DEFAULT_FG) {
        dc = fg;
    } else {
        rgb_from_index<4, 5, 4>(deco, dc);
    }

    i = 0;
    do {
        columns = 0;
        x = items[i].x;
        y = items[i].y;
        for (; i < n && items[i].y == y; i++) {
            columns += items[i].columns;
        }
        if (clear && (draw_default_bg || back != VTE_DEFAULT_BG)) {
            gint bold_offset = (bold && !_vte_draw_has_bold(m_draw, VTE_DRAW_BOLD)) ? 1 : 0;
            _vte_draw_fill_rectangle(m_draw, x, y,
                    columns * column_width + bold_offset, row_height,
                    &bg, VTE_DRAW_OPAQUE);
        }
    } while (i < n);

    if (attr & VTE_ATTR_BLINK) {
       /* Notify the caller that cells with the "blink" attribute were encountered (regardless of
        * whether they're actually painted or skipped now), so that the caller can set up a timer
        * to make them blink if it wishes to. */
        m_text_to_blink = true;

       /* This is for the "off" state of blinking text. Invisible text could also be handled here,
        * but it's not, it's handled outside by not even calling this method.
        * Setting fg = bg and painting the text would not work for two reasons: it'd be opaque
        * even if the background is translucent, and this method can be called with a continuous
        * run of identical fg, yet different bg colored cells. So we simply bail out. */
        if (!m_text_blink_state) {
            return;
        }
    }

   /* Draw whatever SFX are required. Do this before drawing the letters,
    * so that if the descent of a letter crosses an underline of a different color,
    * it's the letter's color that wins. Other kinds of decorations always have the
    * same color as the text, so the order is irrelevant there. */
    if ((attr & (VTE_ATTR_UNDERLINE_MASK |
                    VTE_ATTR_STRIKETHROUGH_MASK |
                    VTE_ATTR_OVERLINE_MASK |
                    VTE_ATTR_BOXED_MASK)) |
         hyperlink | hilite) {
        i = 0;
        do {
            x = items[i].x;
            y = items[i].y;
            for (columns = 0; i < n && items[i].y == y; i++) {
                columns += items[i].columns;
            }
            switch (vte_attr_get_value(attr, VTE_ATTR_UNDERLINE_VALUE_MASK, VTE_ATTR_UNDERLINE_SHIFT)) {
                case 1:
                    _vte_draw_draw_line(m_draw,
                                        x,
                                        y + m_underline_position,
                                        x + (columns * column_width) - 1,
                                        y + m_underline_position + m_underline_thickness - 1,
                                        VTE_LINE_WIDTH,
                                        &dc, VTE_DRAW_OPAQUE);
                    break;
                case 2:
                    _vte_draw_draw_line(m_draw,
                                        x,
                                        y + m_double_underline_position,
                                        x + (columns * column_width) - 1,
                                        y + m_double_underline_position + m_double_underline_thickness - 1,
                                        VTE_LINE_WIDTH,
                                        &dc, VTE_DRAW_OPAQUE);
                    _vte_draw_draw_line(m_draw,
                                        x,
                                        y + m_double_underline_position + 2 * m_double_underline_thickness,
                                        x + (columns * column_width) - 1,
                                        y + m_double_underline_position + 3 * m_double_underline_thickness - 1,
                                        VTE_LINE_WIDTH,
                                        &dc, VTE_DRAW_OPAQUE);
                    break;
                case 3:
                    _vte_draw_draw_undercurl(m_draw,
                                                x,
                                                y + m_undercurl_position,
                                                m_undercurl_thickness,
                                                columns,
                                                &dc, VTE_DRAW_OPAQUE);
                    break;
            }
            if (attr & VTE_ATTR_STRIKETHROUGH) {
                _vte_draw_draw_line(m_draw,
                                    x,
                                    y + m_strikethrough_position,
                                    x + (columns * column_width) - 1,
                                    y + m_strikethrough_position + m_strikethrough_thickness - 1,
                                    VTE_LINE_WIDTH,
                                    &fg, VTE_DRAW_OPAQUE);
            }
            if (attr & VTE_ATTR_OVERLINE) {
                _vte_draw_draw_line(m_draw,
                                    x,
                                    y + m_overline_position,
                                    x + (columns * column_width) - 1,
                                    y + m_overline_position + m_overline_thickness - 1,
                                    VTE_LINE_WIDTH,
                                    &fg, VTE_DRAW_OPAQUE);
            }
            if (hilite) {
                _vte_draw_draw_line(m_draw,
                                    x,
                                    y + m_regex_underline_position,
                                    x + (columns * column_width) - 1,
                                    y + m_regex_underline_position + m_regex_underline_thickness - 1,
                                    VTE_LINE_WIDTH,
                                    &fg, VTE_DRAW_OPAQUE);
            } else if (hyperlink) {
                for (double j = 1.0 / 6.0; j < columns; j += 0.5) {
                    _vte_draw_fill_rectangle(m_draw,
                                                x + j * column_width,
                                                y + m_regex_underline_position,
                                                MAX(column_width / 6.0, 1.0),
                                                m_regex_underline_thickness,
                                                &fg, VTE_DRAW_OPAQUE);
                }
            }
            if (attr & VTE_ATTR_BOXED) {
                _vte_draw_draw_rectangle(m_draw, x, y,
                                            MAX(0, (columns * column_width)),
                                            MAX(0, row_height),
                                            &fg, VTE_DRAW_OPAQUE);
            }
        } while (i < n);
    }

    _vte_draw_text(m_draw, items, n, &fg, VTE_DRAW_OPAQUE,
                    _vte_draw_get_style(attr & VTE_ATTR_BOLD,
                                        attr & VTE_ATTR_ITALIC));
}

/* FIXME: we don't have a way to tell GTK+ what the default text attributes
 * should be, so for now at least it's assuming white-on-black is the norm and
 * is using "black-on-white" to signify "inverse".  Pick up on that state and
 * fix things.  Do this here, so that if we suddenly get red-on-black, we'll do
 * the right thing. */
void VteTerminalPrivate::fudge_pango_colors(GSList *attributes,
                                            VteCell *cells,
                                            gsize n)
{
    gsize i, sumlen = 0;
    struct _fudge_cell_props{
        gboolean saw_fg, saw_bg;
        vte::color::rgb fg, bg;
        guint index;
    }*props = g_newa (struct _fudge_cell_props, n);

    for (i = 0; i < n; i++) {
        gchar ubuf[7];
        gint len = g_unichar_to_utf8 (cells[i].c, ubuf);
        props[i].index = sumlen;
        props[i].saw_fg = props[i].saw_bg = FALSE;
        sumlen += len;
    }

    while (attributes != NULL) {
        PangoAttribute *attr = (PangoAttribute *)attributes->data;
        PangoAttrColor *color;
        switch (attr->klass->type) {
            case PANGO_ATTR_FOREGROUND:
                for (i = 0; i < n; i++) {
                    if (props[i].index < attr->start_index) {
                        continue;
                    }
                    if (props[i].index >= attr->end_index) {
                        break;
                    }
                    props[i].saw_fg = TRUE;
                    color = (PangoAttrColor*) attr;
                    props[i].fg = color->color;
                }
                break;
            case PANGO_ATTR_BACKGROUND:
                for (i = 0; i < n; i++) {
                    if (props[i].index < attr->start_index) {
                        continue;
                    }
                    if (props[i].index >= attr->end_index) {
                        break;
                    }
                    props[i].saw_bg = TRUE;
                    color = (PangoAttrColor*) attr;
                    props[i].bg = color->color;
                }
                break;
            default:
                break;
        }
        attributes = g_slist_next(attributes);
    }

    for (i = 0; i < n; i++) {
        if (props[i].saw_fg && props[i].saw_bg &&
            (props[i].fg.red == 0xffff) &&
            (props[i].fg.green == 0xffff) &&
            (props[i].fg.blue == 0xffff) &&
            (props[i].bg.red == 0) &&
            (props[i].bg.green == 0) &&
            (props[i].bg.blue == 0)) {
            cells[i].attr.copy_colors(m_color_defaults.attr);
            cells[i].attr.set_reverse(true);
        }
    }
}

/* Apply the attribute given in the PangoAttribute to the list of cells. */
void VteTerminalPrivate::apply_pango_attr(PangoAttribute *attr,
                                            VteCell *cells,
                                            gsize n_cells)
{
    guint i, ival;
    PangoAttrInt *attrint;
    PangoAttrColor *attrcolor;

    switch (attr->klass->type) {
        case PANGO_ATTR_FOREGROUND:
        case PANGO_ATTR_BACKGROUND:
            attrcolor = (PangoAttrColor*) attr;
            ival = VTE_RGB_COLOR(8, 8, 8,
                                ((attrcolor->color.red & 0xFF00) >> 8),
                                ((attrcolor->color.green & 0xFF00) >> 8),
                                ((attrcolor->color.blue & 0xFF00) >> 8));
            for (i = attr->start_index;
                i < attr->end_index && i < n_cells;
                i++) {
                if (attr->klass->type == PANGO_ATTR_FOREGROUND) {
                    cells[i].attr.set_fore(ival);
                }
                if (attr->klass->type == PANGO_ATTR_BACKGROUND) {
                    cells[i].attr.set_back(ival);
                }
            }
            break;
        case PANGO_ATTR_UNDERLINE_COLOR:
            attrcolor = (PangoAttrColor*) attr;
            ival = VTE_RGB_COLOR(4, 5, 4,
                                ((attrcolor->color.red & 0xFF00) >> 8),
                                ((attrcolor->color.green & 0xFF00) >> 8),
                                ((attrcolor->color.blue & 0xFF00) >> 8));
            for (i = attr->start_index;
                i < attr->end_index && i < n_cells;
                i++) {
                if (attr->klass->type == PANGO_ATTR_UNDERLINE) {
                    cells[i].attr.set_deco(ival);
                }
            }
            break;
        case PANGO_ATTR_STRIKETHROUGH:
            attrint = (PangoAttrInt*) attr;
            ival = attrint->value;
            for (i = attr->start_index;
                (i < attr->end_index) && (i < n_cells);
                i++) {
                cells[i].attr.set_strikethrough(ival != FALSE);
            }
            break;
        case PANGO_ATTR_UNDERLINE:
            attrint = (PangoAttrInt*) attr;
            ival = attrint->value;
            for (i = attr->start_index;
                (i < attr->end_index) && (i < n_cells);
                i++) {
                unsigned int underline = 0;
                switch (ival) {
                    case PANGO_UNDERLINE_SINGLE:
                        underline = 1;
                        break;
                    case PANGO_UNDERLINE_DOUBLE:
                        underline = 2;
                        break;
                    case PANGO_UNDERLINE_ERROR:
                        underline = 3; /* wavy */
                        break;
                    case PANGO_UNDERLINE_NONE:
                    case PANGO_UNDERLINE_LOW: /* FIXME */
                        underline = 0;
                        break;
                }
                cells[i].attr.set_underline(underline);
            }
            break;
        case PANGO_ATTR_WEIGHT:
            attrint = (PangoAttrInt*) attr;
            ival = attrint->value;
            for (i = attr->start_index;
                (i < attr->end_index) && (i < n_cells);
                i++) {
                cells[i].attr.set_bold(ival >= PANGO_WEIGHT_BOLD);
            }
            break;
        case PANGO_ATTR_STYLE:
            attrint = (PangoAttrInt*) attr;
            ival = attrint->value;
            for (i = attr->start_index;
                (i < attr->end_index) && (i < n_cells);
                i++) {
                cells[i].attr.set_italic(ival != PANGO_STYLE_NORMAL);
            }
            break;
        default:
            break;
    }
}

/** \brief  Function to avoid function pointer madness
 *
 * \param[in]   attr    pango atrributes
 * \param[in]   unused  unused param, used to provide a GFunc() prototype
 */
static void pango_attr_destroy_glue(PangoAttribute *attr, void *unused)
{
    pango_attribute_destroy(attr);
}


/* Convert a PangoAttrList and a location in that list to settings in a
 * charcell structure.  The cells array is assumed to contain enough items
 * so that all ranges in the attribute list can be mapped into the array, which
 * typically means that the cell array should have the same length as the
 * string (byte-wise) which the attributes describe. */
void VteTerminalPrivate::translate_pango_cells(PangoAttrList *attrs,
                                                VteCell *cells,
                                                gsize n_cells)
{
    PangoAttribute *attr;
    PangoAttrIterator *attriter;
    GSList *list, *listiter;
    guint i;

    for (i = 0; i < n_cells; i++) {
        cells[i] = m_fill_defaults;
    }

    attriter = pango_attr_list_get_iterator(attrs);
    if (attriter != NULL) {
        do {
            list = pango_attr_iterator_get_attrs(attriter);
            if (list != NULL) {
                for (listiter = list;
                     listiter != NULL;
                     listiter = g_slist_next(listiter)) {
                    attr = (PangoAttribute *)listiter->data;
                    apply_pango_attr(attr, cells, n_cells);
                }
                attr = (PangoAttribute *)list->data;
                fudge_pango_colors(
                                 list,
                                 cells +
                                 attr->start_index,
                                 MIN(n_cells, attr->end_index) -
                                 attr->start_index);
                g_slist_foreach(list, (GFunc)pango_attr_destroy_glue, nullptr);
                g_slist_free(list);
            }
        } while (pango_attr_iterator_next(attriter) == TRUE);
        pango_attr_iterator_destroy(attriter);
    }
}

/* Draw the listed items using the given attributes.  Tricky because the
 * attribute string is indexed by byte in the UTF-8 representation of the string
 * of characters.  Because we draw a character at a time, this is slower. */
void VteTerminalPrivate::draw_cells_with_attributes(struct _vte_draw_text_request *items,
                                                    gssize n,
                                                    PangoAttrList *attrs,
                                                    bool draw_default_bg,
                                                    gint column_width,
                                                    gint height)
{
    int i, j, cell_count;
    VteCell *cells;
    char scratch_buf[VTE_UTF8_BPC];
    guint fore, back, deco;

    /* Note: since this function is only called with the pre-edit text,
     * all the items contain gunichar only, not vteunistr. */
    /* FIXMEchpe is that really true for all input methods? */

    uint32_t const attr_mask = m_allow_bold ? ~0 : ~VTE_ATTR_BOLD_MASK;

    for (i = 0, cell_count = 0; i < n; i++) {
        cell_count += g_unichar_to_utf8(items[i].c, scratch_buf);
    }
    cells = g_new(VteCell, cell_count);
    translate_pango_cells(attrs, cells, cell_count);
    for (i = 0, j = 0; i < n; i++) {
        determine_colors(&cells[j], false, &fore, &back, &deco);
        draw_cells(items + i, 1,
                    fore,
                    back,
                    deco,
                    TRUE, draw_default_bg,
                    cells[j].attr.attr & attr_mask,
                    m_allow_hyperlink && cells[j].attr.hyperlink_idx != 0,
                    FALSE, column_width, height);
        j += g_unichar_to_utf8(items[i].c, scratch_buf);
    }
    g_free(cells);
}


/* Paint the contents of a given row at the given location.  Take advantage
 * of multiple-draw APIs by finding runs of characters with identical
 * attributes and bundling them together. */
void VteTerminalPrivate::draw_rows(VteScreen *screen_,
                                    vte::grid::row_t start_row,
                                    vte::grid::row_t end_row,
                                    vte::grid::column_t start_column,
                                    vte::grid::column_t end_column,
                                    gint start_x,
                                    gint start_y,
                                    gint column_width,
                                    gint row_height)
{
    struct _vte_draw_text_request items[4*VTE_DRAW_MAX_LENGTH];
    vte::grid::row_t row, rows;
    vte::grid::column_t i, j;
    long x, y;
    guint fore, nfore, back, nback, deco, ndeco;
    gboolean hyperlink, nhyperlink, hilite, nhilite, selected, nselected;
    guint item_count;
    const VteCell *cell;
    VteRowData const* row_data;

    uint32_t const attr_mask = m_allow_bold ? ~0 : ~VTE_ATTR_BOLD_MASK;

    /* adjust for the absolute start of row */
    start_x -= start_column * column_width;

    /* clear the background */
    x = start_x;
    y = start_y;
    row = start_row;
    rows = end_row - start_row;
    do {
        row_data = find_row_data(row);
        /* Back up in case this is a multicolumn character,
         * making the drawing area a little wider. */
        i = start_column;
        if (row_data != NULL) {
            cell = _vte_row_data_get (row_data, i);
            if (cell != NULL) {
                while (cell->attr.fragment() && i > 0) {
                    cell = _vte_row_data_get (row_data, --i);
                }
            }
            /* Walk the line. */
            do {
                /* Get the character cell's contents. */
                cell = _vte_row_data_get (row_data, i);
                /* Find the colors for this cell. */
                selected = cell_is_selected(i, row);
                determine_colors(cell, selected, &fore, &back, &deco);

                bool bold = cell && cell->attr.bold();
                j = i + (cell ? cell->attr.columns() : 1);

                while (j < end_column){
                    /* Retrieve the cell. */
                    cell = _vte_row_data_get (row_data, j);
                    /* Don't render fragments of multicolumn characters
                     * which have the same attributes as the initial
                     * portions. */
                    if (cell != NULL && cell->attr.fragment()) {
                        j++;
                        continue;
                    }
                    /* Resolve attributes to colors where possible and
                     * compare visual attributes to the first character
                     * in this chunk. */
                    selected = cell_is_selected(j, row);
                    determine_colors(cell, selected, &nfore, &nback, &ndeco);
                    if (nback != back) {
                        break;
                    }
                    bold = cell && cell->attr.bold();
                    j += cell ? cell->attr.columns() : 1;
                }
                if (back != VTE_DEFAULT_BG) {
                    vte::color::rgb bg;
                    gint bold_offset = (bold && !_vte_draw_has_bold(m_draw, VTE_DRAW_BOLD)) ? 1 : 0;
                    rgb_from_index<8, 8, 8>(back, bg);
                    _vte_draw_fill_rectangle (
                            m_draw,
                            x + i * column_width,
                            y,
                            (j - i) * column_width + bold_offset,
                            row_height,
                            &bg, VTE_DRAW_OPAQUE);
                }
                /* We'll need to continue at the first cell which didn't
                 * match the first one in this set. */
                i = j;
            } while (i < end_column);
        } else {
            do {
                selected = cell_is_selected(i, row);
                j = i + 1;
                while (j < end_column){
                    nselected = cell_is_selected(j, row);
                    if (nselected != selected) {
                        break;
                    }
                    j++;
                }
                determine_colors(nullptr, selected, &fore, &back, &deco);
                if (back != VTE_DEFAULT_BG) {
                    vte::color::rgb bg;
                    rgb_from_index<8, 8, 8>(back, bg);
                    _vte_draw_fill_rectangle (m_draw,
                                                x + i *column_width,
                                                y,
                                                (j - i)  * column_width,
                                                row_height,
                                                &bg, VTE_DRAW_OPAQUE);
                }
                i = j;
            } while (i < end_column);
        }
        row++;
        y += row_height;
    } while (--rows);


    /* render the text */
    y = start_y;
    row = start_row;
    rows = end_row - start_row;
    item_count = 1;
    do {
        row_data = find_row_data(row);
        if (row_data == NULL) {
            goto fg_skip_row;
        }
        /* Back up in case this is a multicolumn character,
         * making the drawing area a little wider. */
        i = start_column;
        cell = _vte_row_data_get (row_data, i);
        if (cell == NULL) {
            goto fg_skip_row;
        }
        while (cell->attr.fragment() && i > 0) {
            cell = _vte_row_data_get (row_data, --i);
        }

        /* Walk the line. */
        do {
            /* Get the character cell's contents. */
            cell = _vte_row_data_get (row_data, i);
            if (cell == NULL) {
                goto fg_skip_row;
            }
            while (cell->c == 0 || cell->attr.invisible() ||
                    (cell->c == ' ' &&
                                         cell->attr.has_none(VTE_ATTR_UNDERLINE_MASK |
                                                             VTE_ATTR_STRIKETHROUGH_MASK |
                                                             VTE_ATTR_OVERLINE_MASK) &&
                                         (!m_allow_hyperlink || cell->attr.hyperlink_idx == 0)) ||
                               cell->attr.fragment()) {
                if (++i >= end_column) {
                    goto fg_skip_row;
                }
                cell = _vte_row_data_get (row_data, i);
                if (cell == NULL) {
                    goto fg_skip_row;
                }
            }
            /* Find the colors for this cell. */
            selected = cell_is_selected(i, row);
            determine_colors(cell, selected, &fore, &back, &deco);

            uint32_t const attr = cell->attr.attr;

            hyperlink = (m_allow_hyperlink && cell->attr.hyperlink_idx != 0);
            hilite = (cell->attr.hyperlink_idx != 0 && cell->attr.hyperlink_idx == m_hyperlink_hover_idx) ||
                        (m_hyperlink_hover_idx == 0 && m_match != nullptr && m_match_span.contains(row, i));

            items[0].c = cell->c;
            items[0].columns = cell->attr.columns();
            items[0].x = start_x + i * column_width;
            items[0].y = y;
            j = i + items[0].columns;

            /* Now find out how many cells have the same attributes. */
            do {
                while (j < end_column && item_count < G_N_ELEMENTS(items)) {
                    /* Retrieve the cell. */
                    cell = _vte_row_data_get (row_data, j);
                    if (cell == NULL) {
                        goto fg_next_row;
                    }
                    /* Ignore the attributes on a fragment, the attributes
                        * of the preceding character cell should apply. */
                    if (cell->attr.fragment()) {
                        j++;
                        continue;
                    }
                    if (cell->c == 0){
                        /* only break the run if we
                         * are drawing attributes
                         */
                        if ((attr & (VTE_ATTR_UNDERLINE_MASK |
                                        VTE_ATTR_STRIKETHROUGH_MASK |
                                        VTE_ATTR_OVERLINE_MASK)) |
                            hyperlink | hilite) {
                            break;
                        } else {
                            j++;
                            continue;
                        }
                    }
                    /* Resolve attributes to colors where possible and
                     * compare visual attributes to the first character
                     * in this chunk. */
                    selected = cell_is_selected(j, row);
                    determine_colors(cell, selected, &nfore, &nback, &ndeco);
                    if (nfore != fore) {
                        break;
                    }
                    if (ndeco != deco) {
                        break;
                    }

                    /* Bold, italic, underline, strikethrough,
                     * overline, blink, or invisible differ;
                     * break the run.
                     */
                    if ((cell->attr.attr ^ attr) & (VTE_ATTR_BOLD_MASK |
                                                    VTE_ATTR_ITALIC_MASK |
                                                    VTE_ATTR_UNDERLINE_MASK |
                                                    VTE_ATTR_STRIKETHROUGH_MASK |
                                                    VTE_ATTR_OVERLINE_MASK |
                                                    VTE_ATTR_BLINK_MASK |
                                                    VTE_ATTR_INVISIBLE_MASK)) {
                        break;
                    }

                    nhyperlink = (m_allow_hyperlink && cell->attr.hyperlink_idx != 0);
                    if (nhyperlink != hyperlink) {
                        break;
                    }
                    /* Break up matched/not-matched text. */
                    nhilite = (cell->attr.hyperlink_idx != 0 && cell->attr.hyperlink_idx == m_hyperlink_hover_idx) ||
                                (m_hyperlink_hover_idx == 0 && m_match != nullptr && m_match_span.contains(row, j));
                    if (nhilite != hilite) {
                        break;
                    }
                    /* Add this cell to the draw list. */
                    items[item_count].c = cell->c;
                    items[item_count].columns = cell->attr.columns();
                    items[item_count].x = start_x + j * column_width;
                    items[item_count].y = y;
                    j +=  items[item_count].columns;
                    item_count++;
                }
                /* have we encountered a state change? */
                if (j < end_column) {
                    break;
                }
fg_next_row:
                /* is this the last column, on the last row? */
                do {
                    do {
                        if (!--rows) {
                            goto fg_draw;
                        }

                        /* restart on the next row */
                        row++;
                        y += row_height;
                        row_data = find_row_data(row);
                    } while (row_data == NULL);

                    /* Back up in case this is a
                     * multicolumn character, making the drawing
                     * area a little wider. */
                    j = start_column;
                    cell = _vte_row_data_get (row_data, j);
                } while (cell == NULL);
                while (cell->attr.fragment() && j > 0) {
                    cell = _vte_row_data_get (row_data, --j);
                }
            } while (TRUE);
fg_draw:
            /* Draw the cells. */
            draw_cells(items, item_count, fore, back, deco, FALSE, FALSE,
                        attr & attr_mask, hyperlink, hilite, column_width, row_height);
            item_count = 1;
            /* We'll need to continue at the first cell which didn't
             * match the first one in this set. */
            i = j;
            if (!rows) {
                goto fg_out;
            }
        } while (i < end_column);
fg_skip_row:
        row++;
        y += row_height;
    } while (--rows);
fg_out:
    return;
}

void VteTerminalPrivate::expand_rectangle(cairo_rectangle_int_t& rect) const
{
    /* increase the paint by one pixel on all sides to force the
     * inclusion of neighbouring cells */
    vte::grid::row_t row = pixel_to_row(MAX(0, rect.y - 1));
    /* Both the value given by MIN() and row_stop are exclusive.
        * _vte_terminal_pixel_to_row expects an actual value corresponding
        * to the bottom visible pixel, hence the - 1 + 1 magic. */
    vte::grid::row_t row_stop = pixel_to_row(MIN(rect.height + rect.y + 1, m_view_usable_extents.height()) - 1) + 1;
    if (row_stop <= row) {
            return;
    }

    vte::grid::column_t col = MAX(0, (rect.x - 1) / m_cell_width);
    vte::grid::column_t col_stop = MIN(howmany(rect.width + rect.x + 1, m_cell_width), m_column_count);
    if (col_stop <= col) {
            return;
    }
#if VTE_DEBUG
    cairo_rectangle_int_t old_rect = rect;
#endif
    rect.x = col * m_cell_width;
    rect.width = (col_stop - col) * m_cell_width;
    rect.y = row_to_pixel(row);
    rect.height = (row_stop - row) * m_cell_height;

    _vte_debug_print (VTE_DEBUG_UPDATES,
                        "expand_rectangle"
                        " (%d,%d)x(%d,%d) pixels,"
                        " (%ld,%ld)x(%ld,%ld) cells"
                        " [(%d,%d)x(%d,%d) pixels]\n",
                        old_rect.x, old_rect.y, old_rect.width, old_rect.height,
                        col, row, col_stop - col, row_stop - row,
                        rect.x, rect.y, rect.width, rect.height);
}

void VteTerminalPrivate::paint_area(GdkRectangle const* area)
{
    vte::grid::row_t row, row_stop;
    vte::grid::column_t col, col_stop;

    row = pixel_to_row(MAX(0, area->y));
    /* Both the value given by MIN() and row_stop are exclusive.
        * _vte_terminal_pixel_to_row expects an actual value corresponding
        * to the bottom visible pixel, hence the - 1 + 1 magic. */
    row_stop = pixel_to_row(MIN(area->height + area->y,
                                get_allocated_height() - m_padding.top - m_padding.bottom) - 1) + 1;
    if (row_stop <= row) {
        return;
    }
    col = MAX(0, area->x / m_cell_width);
    col_stop = MIN((area->width + area->x) / m_cell_width,
               m_column_count);
    if (col_stop <= col) {
        return;
    }
    _vte_debug_print (VTE_DEBUG_UPDATES,
            "paint_area"
            "   (%d,%d)x(%d,%d) pixels,"
            " (%ld,%ld)x(%ld,%ld) cells"
            " [(%ld,%ld)x(%ld,%ld) pixels]\n",
            area->x, area->y, area->width, area->height,
            col, row, col_stop - col, row_stop - row,
            col * m_cell_width,
            row * m_cell_height,
            (col_stop - col) * m_cell_width,
            (row_stop - row) * m_cell_height);

    /* Now we're ready to draw the text.  Iterate over the rows we
     * need to draw. */
    draw_rows(m_screen,
                row, row_stop,
                col, col_stop,
                col * m_cell_width,
                row_to_pixel(row),
                m_cell_width,
                m_cell_height);
}

void VteTerminalPrivate::paint_cursor()
{
    struct _vte_draw_text_request item;
        vte::grid::row_t drow;
        vte::grid::column_t col;
        int width, height, cursor_width;
        guint style = 0;
        guint fore, back, deco;
    vte::color::rgb bg;
    int x, y;
    gboolean blink, selected, focus;

    if (!m_cursor_visible) {
        return;
    }

    if (m_im_preedit_active) {
        return;
    }

    col = m_screen->cursor.col;
    drow = m_screen->cursor.row;
    width = m_cell_width;
    height = m_cell_height;

    /* TODOegmont: clamp on rows? tricky... */
    if (CLAMP(col, 0, m_column_count - 1) != col) {
        return;
    }

    focus = m_has_focus;
    blink = m_cursor_blink_state;

    if (focus && !blink) {
        return;
    }

   /* Find the first cell of the character "under" the cursor.
    * This is for CJK.  For TAB, paint the cursor where it really is. */
    auto cell = find_charcell(col, drow);
        while (cell != NULL && cell->attr.fragment() && cell->c != '\t' && col > 0) {
        col--;
        cell = find_charcell(col, drow);
    }

    /* Draw the cursor. */
    item.c = (cell && cell->c) ? cell->c : ' ';
    item.columns = item.c == '\t' ? 1 : cell ? cell->attr.columns() : 1;
    item.x = col * width;
    item.y = row_to_pixel(drow);
    if (cell && cell->c != 0) {
        style = _vte_draw_get_style(cell->attr.bold(), cell->attr.italic());
    }

    selected = cell_is_selected(col, drow);
        determine_cursor_colors(cell, selected, &fore, &back, &deco);
        rgb_from_index<8, 8, 8>(back, bg);

    x = item.x;
    y = item.y;

    switch (decscusr_cursor_shape()) {

        case VTE_CURSOR_SHAPE_IBEAM: 
        {
           /* Draw at the very left of the cell (before the spacing), even in case of CJK.
            * IMO (egmont) not overrunning the letter improves readability, vertical movement
            * looks good (no zigzag even when a somewhat wider glyph that starts filling up
            * the left spacing, or CJK that begins further to the right is encountered),
            * and also this is where it looks good if background colors change, including
            * Shift+arrows highlighting experience in some editors. As per the behavior of
            * word processors, don't increase the height by the line spacing. */
            int stem_width;

            stem_width = (int) (((float) (m_char_ascent + m_char_descent)) * m_cursor_aspect_ratio + 0.5);
            stem_width = CLAMP (stem_width, VTE_LINE_WIDTH, m_cell_width);

            _vte_draw_fill_rectangle(m_draw,
                                        x, y + m_char_padding.top, stem_width, m_char_ascent + m_char_descent,
                                        &bg, VTE_DRAW_OPAQUE);
            break;
        }

        case VTE_CURSOR_SHAPE_UNDERLINE: 
        {
           /* The width is at least the overall width of the cell (or two cells) minus the two
            * half spacings on the two edges. That is, underlines under a CJK are more than twice
            * as wide as narrow characters in case of letter spacing. Plus, if necessary, the width
            * is increased to span under the entire glyph. Vertical position is not affected by
            * line spacing. */

            int line_height, left, right;

            /* use height (not width) so underline and ibeam will
             * be equally visible */
            line_height = (int) (((float) (m_char_ascent + m_char_descent)) * m_cursor_aspect_ratio + 0.5);
            line_height = CLAMP (line_height, VTE_LINE_WIDTH, m_char_ascent + m_char_descent);

            left = m_char_padding.left;
            right = item.columns * m_cell_width - m_char_padding.right;

            if (cell && cell->c != 0 && cell->c != ' ' && cell->c != '\t') {
                int l, r;
                _vte_draw_get_char_edges (m_draw, cell->c, cell->attr.columns(), style, &l, &r);
                left = MIN(left, l);
                right = MAX(right, r);
            }

            _vte_draw_fill_rectangle(m_draw,
                                        x + left, y + m_cell_height - m_char_padding.bottom - line_height,
                                        right - left, line_height,
                                        &bg, VTE_DRAW_OPAQUE);
            break;
        }

        case VTE_CURSOR_SHAPE_BLOCK:
           /* Include the spacings in the cursor, see bug 781479 comments 39-44.
            * Make the cursor even wider if the glyph is wider. */

            cursor_width = item.columns * width;
            if (cell && cell->c != 0 && cell->c != ' ' && cell->c != '\t') {
                int r;
                _vte_draw_get_char_edges (m_draw, cell->c, cell->attr.columns(), style, NULL, &r);
                cursor_width = MAX(cursor_width, r);
            }

            uint32_t const attr_mask = m_allow_bold ? ~0 : ~VTE_ATTR_BOLD_MASK;

            if (focus) {
                /* just reverse the character under the cursor */
                _vte_draw_fill_rectangle(m_draw, x, y, cursor_width, height,
                                            &bg, VTE_DRAW_OPAQUE);

                if (cell && cell->c != 0 && cell->c != ' ' && cell->c != '\t') {
                    draw_cells(&item, 1,
                                fore, back, deco, TRUE, FALSE,
                                cell->attr.attr & attr_mask,
                                m_allow_hyperlink && cell->attr.hyperlink_idx != 0,
                                FALSE,
                                width,
                                height);
                }

            } else {
                /* draw a box around the character */
                _vte_draw_draw_rectangle(m_draw,
                                            x - VTE_LINE_WIDTH,
                                            y - VTE_LINE_WIDTH,
                                            cursor_width + 2*VTE_LINE_WIDTH,
                                            height + 2*VTE_LINE_WIDTH,
                                            &bg, VTE_DRAW_OPAQUE);
            }

            break;
    }
}

void VteTerminalPrivate::paint_im_preedit_string()
{
    int col, columns;
    long width, height;
    int i, len;

    if (!m_im_preedit) {
        return;
    }

    /* Keep local copies of rendering information. */
    width = m_cell_width;
    height = m_cell_height;

    /* Find out how many columns the pre-edit string takes up. */
    columns = get_preedit_width(false);
    len = get_preedit_length(false);

    /* If the pre-edit string won't fit on the screen if we start
     * drawing it at the cursor's position, move it left. */
    col = m_screen->cursor.col;
    if (col + columns > m_column_count) {
        col = MAX(0, m_column_count - columns);
    }

    /* Draw the preedit string, boxed. */
    if (len > 0) {
        struct _vte_draw_text_request *items;
        const char *preedit = m_im_preedit;
        int preedit_cursor;

        items = g_new(struct _vte_draw_text_request, len);
        for (i = columns = 0; i < len; i++) {
            items[i].c = g_utf8_get_char(preedit);
            items[i].columns = _vte_unichar_width(items[i].c, m_utf8_ambiguous_width);
            items[i].x = (col + columns) * width;
            items[i].y = row_to_pixel(m_screen->cursor.row);
            columns += items[i].columns;
            preedit = g_utf8_next_char(preedit);
        }
        if (G_LIKELY(m_clear_background)) {
            _vte_draw_clear(m_draw,
                            col * width,
                            row_to_pixel(m_screen->cursor.row),
                            width * columns,
                            height,
                            get_color(VTE_DEFAULT_BG), m_background_alpha);
        }
        draw_cells_with_attributes(
                                    items, len,
                                    m_im_preedit_attrs,
                                    TRUE,
                                    width, height);
        preedit_cursor = m_im_preedit_cursor;

        if (preedit_cursor >= 0 && preedit_cursor < len) {
            uint32_t fore, back, deco;
            vte_color_triple_get(m_color_defaults.attr.colors(), &fore, &back, &deco);

            /* Cursored letter in reverse. */
            draw_cells(
                        &items[preedit_cursor], 1,
                                                fore, back, deco,
                                                TRUE,  /* clear */
                                                TRUE,  /* draw_default_bg */
                                                VTE_ATTR_NONE | VTE_ATTR_BOXED,
                                                FALSE, /* hyperlink */
                                                FALSE, /* hilite */
                        width, height);
        }
        g_free(items);
    }
}

void VteTerminalPrivate::widget_draw(cairo_t *cr)
{
    cairo_rectangle_int_t clip_rect;
    cairo_region_t *region;
    int allocated_width, allocated_height;
    int extra_area_for_cursor;
    bool text_blink_enabled_now;
    gint64 now = 0;

    if (!gdk_cairo_get_clip_rectangle (cr, &clip_rect)) {
        return;
    }

    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_draw()\n");
    _vte_debug_print (VTE_DEBUG_WORK, "+");
    _vte_debug_print (VTE_DEBUG_UPDATES, "Draw (%d,%d)x(%d,%d)\n",
                        clip_rect.x, clip_rect.y,
                        clip_rect.width, clip_rect.height);

    region = vte_cairo_get_clip_region (cr);
    if (region == NULL) {
        return;
    }

    allocated_width = get_allocated_width();
    allocated_height = get_allocated_height();

    /* Designate the start of the drawing operation and clear the area. */
    _vte_draw_set_cairo(m_draw, cr);

    if (G_LIKELY(m_clear_background)) {
        _vte_draw_clear (m_draw, 0, 0,
                            allocated_width, allocated_height,
                            get_color(VTE_DEFAULT_BG), m_background_alpha);
    }

    /* Clip vertically, for the sake of smooth scrolling. We want the top and bottom paddings to be unused.
     * Don't clip horizontally so that antialiasing can legally overflow to the right padding. */
    cairo_save(cr);
    cairo_rectangle(cr, 0, m_padding.top, allocated_width, allocated_height - m_padding.top - m_padding.bottom);
    cairo_clip(cr);

    cairo_translate(cr, m_padding.left, m_padding.top);

    /* Transform to view coordinates */
    cairo_region_translate(region, -m_padding.left, -m_padding.top);

    cairo_rectangle_int_t *rectangles;
    int n, n_rectangles;
    n_rectangles = cairo_region_num_rectangles (region);
    rectangles = g_new(cairo_rectangle_int_t, n_rectangles);
    for (n = 0; n < n_rectangles; n++) {
        cairo_region_get_rectangle (region, n, &rectangles[n]);
    }

    /* don't bother to enlarge an invalidate all */
    if (!(n_rectangles == 1
          && rectangles[0].width == allocated_width
          && rectangles[0].height == allocated_height)) {
        cairo_region_t *rr = cairo_region_create ();
       /* Expand the rectangles so that they cover whole cells,
        * to avoid overlapping XY bands.
        */
        for (n = 0; n < n_rectangles; n++) {
            expand_rectangle(rectangles[n]);
            cairo_region_union_rectangle(rr, &rectangles[n]);
        }
        g_free(rectangles);

        n_rectangles = cairo_region_num_rectangles (rr);
        rectangles = g_new (cairo_rectangle_int_t, n_rectangles);
        for (n = 0; n < n_rectangles; n++) {
            cairo_region_get_rectangle(rr, n, &rectangles[n]);
        }
        cairo_region_destroy(rr);
    }

    /* Whether blinking text should be visible now */
    m_text_blink_state = true;
    text_blink_enabled_now = m_text_blink_mode & (m_has_focus ? VTE_TEXT_BLINK_FOCUSED : VTE_TEXT_BLINK_UNFOCUSED);
    if (text_blink_enabled_now) {
        now = g_get_monotonic_time() / 1000;
        if (now % (m_text_blink_cycle * 2) >= m_text_blink_cycle) {
            m_text_blink_state = false;
        }
    }
    /* Painting will flip this if it encounters any cell with blink attribute */
    m_text_to_blink = false;

    /* and now paint them */
    for (n = 0; n < n_rectangles; n++) {
        paint_area(&rectangles[n]);
    }
    g_free (rectangles);

    paint_im_preedit_string();

    cairo_restore(cr);

    /* Re-clip, allowing 1 more pixel row for the outline cursor. */
    /* TODOegmont: It's really ugly to do it here. */
    cairo_save(cr);
    extra_area_for_cursor = (decscusr_cursor_shape() == VTE_CURSOR_SHAPE_BLOCK && !m_has_focus) ? 1 : 0;
    cairo_rectangle(cr, 0, m_padding.top - extra_area_for_cursor, allocated_width, allocated_height - m_padding.top - m_padding.bottom + 2 * extra_area_for_cursor);
    cairo_clip(cr);

    cairo_translate(cr, m_padding.left, m_padding.top);

    paint_cursor();

    cairo_restore(cr);

    /* Done with various structures. */
    _vte_draw_set_cairo(m_draw, NULL);

    cairo_region_destroy (region);

   /* If painting encountered any cell with blink attribute, we might need to set up a timer.
    * Blinking is implemented using a one-shot (not repeating) timer that keeps getting reinstalled
    * here as long as blinking cells are encountered during (re)painting. This way there's no need
    * for an explicit step to stop the timer when blinking cells are no longer present, this happens
    * implicitly by the timer not getting reinstalled anymore (often after a final unnecessary but
    * harmless repaint). */
    if (G_UNLIKELY (m_text_to_blink && text_blink_enabled_now && m_text_blink_tag == 0)) {
        m_text_blink_tag = g_timeout_add_full(G_PRIORITY_LOW,
                                                m_text_blink_cycle - now % m_text_blink_cycle,
                                                (GSourceFunc)invalidate_text_blink_cb,
                                                this,
                                                NULL);
    }

    m_invalidated_all = FALSE;
}

/* Handle an expose event by painting the exposed area. */
static cairo_region_t *vte_cairo_get_clip_region (cairo_t *cr)
{
    cairo_rectangle_list_t *list;
    cairo_region_t *region;
    int i;

    list = cairo_copy_clip_rectangle_list (cr);
    if (list->status == CAIRO_STATUS_CLIP_NOT_REPRESENTABLE) {
        cairo_rectangle_int_t clip_rect;

        cairo_rectangle_list_destroy (list);

        if (!gdk_cairo_get_clip_rectangle (cr, &clip_rect)) {
            return NULL;
        }
        return cairo_region_create_rectangle (&clip_rect);
    }


    region = cairo_region_create ();
    for (i = list->num_rectangles - 1; i >= 0; --i) {
        cairo_rectangle_t *rect = &list->rectangles[i];
        cairo_rectangle_int_t clip_rect;

        clip_rect.x = floor (rect->x);
        clip_rect.y = floor (rect->y);
        clip_rect.width = ceil (rect->x + rect->width) - clip_rect.x;
        clip_rect.height = ceil (rect->y + rect->height) - clip_rect.y;

        if (cairo_region_union_rectangle (region, &clip_rect) != CAIRO_STATUS_SUCCESS) {
            cairo_region_destroy (region);
            region = NULL;
            break;
        }
    }

    cairo_rectangle_list_destroy (list);
    return region;
}

void VteTerminalPrivate::widget_scroll(GdkEventScroll *event)
{
    gdouble delta_x, delta_y;
    gdouble v;
    gint cnt, i;
    int button;

    GdkEvent *base_event = reinterpret_cast<GdkEvent*>(event);
    auto rowcol = confined_grid_coords_from_event(base_event);

    read_modifiers(base_event);

    switch (event->direction) {
        case GDK_SCROLL_UP:
            m_mouse_smooth_scroll_delta -= 1.;
            _vte_debug_print(VTE_DEBUG_EVENTS, "Scroll up\n");
            break;
        case GDK_SCROLL_DOWN:
            m_mouse_smooth_scroll_delta += 1.;
            _vte_debug_print(VTE_DEBUG_EVENTS, "Scroll down\n");
            break;
        case GDK_SCROLL_SMOOTH:
            gdk_event_get_scroll_deltas ((GdkEvent*) event, &delta_x, &delta_y);
            m_mouse_smooth_scroll_delta += delta_y;
            _vte_debug_print(VTE_DEBUG_EVENTS,
                    "Smooth scroll by %f, delta now at %f\n",
                    delta_y, m_mouse_smooth_scroll_delta);
            break;
        default:
            break;
    }

    /* If we're running a mouse-aware application, map the scroll event
     * to a button press on buttons four and five. */
    if (m_mouse_tracking_mode) {
        cnt = m_mouse_smooth_scroll_delta;
        if (cnt == 0) {
            return;
        }
        m_mouse_smooth_scroll_delta -= cnt;
        _vte_debug_print(VTE_DEBUG_EVENTS,
                "Scroll application by %d lines, smooth scroll delta set back to %f\n",
                cnt, m_mouse_smooth_scroll_delta);

        button = cnt > 0 ? 5 : 4;
        if (cnt < 0) {
            cnt = -cnt;
        }
        for (i = 0; i < cnt; i++) {
            /* Encode the parameters and send them to the app. */
            feed_mouse_event(rowcol,
                                button,
                                false /* not drag */,
                                false /* not release */);
        }
        return;
    }

    v = MAX (1., ceil (gtk_adjustment_get_page_increment (m_vadjustment) / 10.));
    _vte_debug_print(VTE_DEBUG_EVENTS,
            "Scroll speed is %d lines per non-smooth scroll unit\n",
            (int) v);
    if (m_screen == &m_alternate_screen && m_alternate_screen_scroll) {
        char *normal;
        gssize normal_length;

        cnt = v * m_mouse_smooth_scroll_delta;
        if (cnt == 0) {
            return;
        }
        m_mouse_smooth_scroll_delta -= cnt / v;
        _vte_debug_print(VTE_DEBUG_EVENTS,
                "Scroll by %d lines, smooth scroll delta set back to %f\n",
                cnt, m_mouse_smooth_scroll_delta);

        /* In the alternate screen there is no scrolling,
         * so fake a few cursor keystrokes. */

        _vte_keymap_map (
                cnt > 0 ? GDK_KEY_Down : GDK_KEY_Up,
                m_modifiers,
                m_cursor_mode == VTE_KEYMODE_APPLICATION,
                m_keypad_mode == VTE_KEYMODE_APPLICATION,
                &normal,
                &normal_length);
        if (cnt < 0) {
            cnt = -cnt;
        }
#if 0
        for (i = 0; i < cnt; i++) {
            feed_child_using_modes(normal, normal_length);
        }
#endif
        g_free (normal);
    } else {
        /* Perform a history scroll. */
        double dcnt = m_screen->scroll_delta + v * m_mouse_smooth_scroll_delta;
        queue_adjustment_value_changed_clamped(dcnt);
        m_mouse_smooth_scroll_delta = 0;
    }
}

bool VteTerminalPrivate::set_audible_bell(bool setting)
{
    if (setting == m_audible_bell) {
        return false;
    }

    m_audible_bell = setting;
    return true;
}

bool VteTerminalPrivate::set_text_blink_mode(VteTextBlinkMode setting)
{
    if (setting == m_text_blink_mode) {
        return false;
    }

    m_text_blink_mode = setting;
    invalidate_all();

    return true;
}

bool VteTerminalPrivate::set_allow_bold(bool setting)
{
    if (setting == m_allow_bold) {
        return false;
    }

    m_allow_bold = setting;
    invalidate_all();

    return true;
}

bool VteTerminalPrivate::set_bold_is_bright(bool setting)
{
    if (setting == m_bold_is_bright) {
        return false;
    }

    m_bold_is_bright = setting;
    invalidate_all();

    return true;
}

bool VteTerminalPrivate::set_allow_hyperlink(bool setting)
{
    if (setting == m_allow_hyperlink) {
        return false;
    }

    if (setting == false) {
        m_hyperlink_hover_idx = _vte_ring_get_hyperlink_at_position(m_screen->row_data, -1, -1, true, NULL);
        g_assert (m_hyperlink_hover_idx == 0);
        m_hyperlink_hover_uri = NULL;
        emit_hyperlink_hover_uri_changed(NULL);  /* FIXME only emit if really changed */
        m_defaults.attr.hyperlink_idx = _vte_ring_get_hyperlink_idx(m_screen->row_data, NULL);
        g_assert (m_defaults.attr.hyperlink_idx == 0);
    }

    m_allow_hyperlink = setting;
    invalidate_all();

    return true;
}

bool VteTerminalPrivate::set_scroll_on_output(bool scroll)
{
    if (scroll == m_scroll_on_output) {
        return false;
    }

    m_scroll_on_output = scroll;
    return true;
}

bool VteTerminalPrivate::set_scroll_on_keystroke(bool scroll)
{
    if (scroll == m_scroll_on_keystroke) {
        return false;
    }

    m_scroll_on_keystroke = scroll;
    return true;
}

bool VteTerminalPrivate::set_rewrap_on_resize(bool rewrap)
{
    if (rewrap == m_rewrap_on_resize) {
        return false;
    }

    m_rewrap_on_resize = rewrap;
    return true;
}

void VteTerminalPrivate::update_cursor_blinks()
{
    bool blink = false;

    switch (decscusr_cursor_blink()) {
        case VTE_CURSOR_BLINK_SYSTEM:
            gboolean v;
            g_object_get(gtk_widget_get_settings(m_widget),
                                                    "gtk-cursor-blink",
                                                    &v, nullptr);
            blink = v != FALSE;
            break;
        case VTE_CURSOR_BLINK_ON:
            blink = true;
            break;
        case VTE_CURSOR_BLINK_OFF:
            blink = false;
            break;
    }

    if (m_cursor_blinks == blink) {
        return;
    }

    m_cursor_blinks = blink;
    check_cursor_blink();
}

bool VteTerminalPrivate::set_cursor_blink_mode(VteCursorBlinkMode mode)
{
    if (mode == m_cursor_blink_mode) {
        return false;
    }

    m_cursor_blink_mode = mode;
    update_cursor_blinks();

    return true;
}

bool VteTerminalPrivate::set_cursor_shape(VteCursorShape shape)
{
    if (shape == m_cursor_shape) {
        return false;
    }

    m_cursor_shape = shape;
    invalidate_cursor_once();

    return true;
}

/* DECSCUSR set cursor style */
bool VteTerminalPrivate::set_cursor_style(VteCursorStyle style)
{
    if (m_cursor_style == style) {
        return false;
    }

    m_cursor_style = style;
    update_cursor_blinks();
    /* and this will also make cursor shape match the DECSCUSR style */
    invalidate_cursor_once();

    return true;
}

/*
 * VteTerminalPrivate::decscusr_cursor_blink:
 *
 * Returns the cursor blink mode set by DECSCUSR. If DECSCUSR was never
 * called, or it set the blink mode to terminal default, this returns the
 * value set via API or in dconf. Internal use only.
 *
 * Return value: cursor blink mode
 */
VteCursorBlinkMode VteTerminalPrivate::decscusr_cursor_blink()
{
    switch (m_cursor_style) {
        default:
        case VTE_CURSOR_STYLE_TERMINAL_DEFAULT:
            return m_cursor_blink_mode;
        case VTE_CURSOR_STYLE_BLINK_BLOCK:
        case VTE_CURSOR_STYLE_BLINK_UNDERLINE:
        case VTE_CURSOR_STYLE_BLINK_IBEAM:
            return VTE_CURSOR_BLINK_ON;
        case VTE_CURSOR_STYLE_STEADY_BLOCK:
        case VTE_CURSOR_STYLE_STEADY_UNDERLINE:
        case VTE_CURSOR_STYLE_STEADY_IBEAM:
            return VTE_CURSOR_BLINK_OFF;
    }
}

/*
 * VteTerminalPrivate::decscusr_cursor_shape:
 * @terminal: a #VteTerminal
 *
 * Returns the cursor shape set by DECSCUSR. If DECSCUSR was never called,
 * or it set the cursor shape to terminal default, this returns the value
 * set via API. Internal use only.
 *
 * Return value: cursor shape
 */
VteCursorShape VteTerminalPrivate::decscusr_cursor_shape()
{
    switch (m_cursor_style) {
        default:
        case VTE_CURSOR_STYLE_TERMINAL_DEFAULT:
            return m_cursor_shape;
        case VTE_CURSOR_STYLE_BLINK_BLOCK:
        case VTE_CURSOR_STYLE_STEADY_BLOCK:
            return VTE_CURSOR_SHAPE_BLOCK;
        case VTE_CURSOR_STYLE_BLINK_UNDERLINE:
        case VTE_CURSOR_STYLE_STEADY_UNDERLINE:
            return VTE_CURSOR_SHAPE_UNDERLINE;
        case VTE_CURSOR_STYLE_BLINK_IBEAM:
        case VTE_CURSOR_STYLE_STEADY_IBEAM:
            return VTE_CURSOR_SHAPE_IBEAM;
    }
}

bool VteTerminalPrivate::set_scrollback_lines(long lines)
{
    glong low, high, next;
    double scroll_delta;
    VteScreen *scrn;

    if (lines < 0) {
        lines = G_MAXLONG;
    }

#if 0
        /* FIXME: this breaks the scrollbar range, bug #562511 */
        if (lines == m_scrollback_lines)
                return false;
#endif

    _vte_debug_print (VTE_DEBUG_MISC,
            "Setting scrollback lines to %ld\n", lines);

    m_scrollback_lines = lines;

    /* The main screen gets the full scrollback buffer. */
    scrn = &m_normal_screen;
    lines = MAX (lines, m_row_count);
    next = MAX (m_screen->cursor.row + 1,
                _vte_ring_next (scrn->row_data));
    _vte_ring_resize (scrn->row_data, lines);
    low = _vte_ring_delta (scrn->row_data);
    high = lines + MIN (G_MAXLONG - lines, low - m_row_count + 1);
    scrn->insert_delta = CLAMP (scrn->insert_delta, low, high);
    scrn->scroll_delta = CLAMP (scrn->scroll_delta, low, scrn->insert_delta);
    next = MIN (next, scrn->insert_delta + m_row_count);
    if (_vte_ring_next (scrn->row_data) > next){
        _vte_ring_shrink (scrn->row_data, next - low);
    }

    /* The alternate scrn isn't allowed to scroll at all. */
    scrn = &m_alternate_screen;
    _vte_ring_resize (scrn->row_data, m_row_count);
    scrn->scroll_delta = _vte_ring_delta (scrn->row_data);
    scrn->insert_delta = _vte_ring_delta (scrn->row_data);
    if (_vte_ring_next (scrn->row_data) > scrn->insert_delta + m_row_count){
        _vte_ring_shrink (scrn->row_data, m_row_count);
    }

    /* Adjust the scrollbar to the new location. */
    /* Hack: force a change in scroll_delta even if the value remains, so that
       vte_term_q_adj_val_changed() doesn't shortcut to no-op, see bug 676075. */
    scroll_delta = m_screen->scroll_delta;
    m_screen->scroll_delta = -1;
    queue_adjustment_value_changed(scroll_delta);
    adjust_adjustments_full();

    return true;
}

bool VteTerminalPrivate::set_backspace_binding(VteEraseBinding binding)
{
    if (binding == m_backspace_binding) {
        return false;
    }

    m_backspace_binding = binding;
    return true;
}

bool VteTerminalPrivate::set_delete_binding(VteEraseBinding binding)
{
    if (binding == m_delete_binding) {
        return false;
    }

    m_delete_binding = binding;
    return true;
}

bool VteTerminalPrivate::set_mouse_autohide(bool autohide)
{
    if (autohide == m_mouse_autohide) {
        return false;
    }

    m_mouse_autohide = autohide;

    if (m_mouse_cursor_autohidden) {
        hyperlink_hilite_update();
        apply_mouse_cursor();
    }
    return true;
}

/*
 * VteTerminalPrivate::reset:
 * @clear_tabstops: whether to reset tabstops
 * @clear_history: whether to empty the terminal's scrollback buffer
 *
 * Resets as much of the terminal's internal state as possible, discarding any
 * unprocessed input data, resetting character attributes, cursor state,
 * national character set state, status line, terminal modes (insert/delete),
 * selection state, and encoding.
 *
 */
void VteTerminalPrivate::reset(bool clear_tabstops,
                                bool clear_history,
                                bool from_api)
{
    if (from_api && !m_input_enabled) {
        return;
    }

    GObject *object = G_OBJECT(m_terminal);
    g_object_freeze_notify(object);

    m_bell_pending = false;

    /* Clear the output buffer. */
    _vte_byte_array_clear(m_outgoing);
    /* Reset charset substitution state. */
    _vte_iso2022_state_free(m_iso2022);
    m_iso2022 = _vte_iso2022_state_new(nullptr);
    _vte_iso2022_state_set_codeset(m_iso2022, m_encoding);
    m_last_graphic_character = 0;
    /* Reset keypad/cursor key modes. */
    m_keypad_mode = VTE_KEYMODE_NORMAL;
    m_cursor_mode = VTE_KEYMODE_NORMAL;
    /* Enable autowrap. */
    m_autowrap = TRUE;
    /* Enable meta-sends-escape. */
    m_meta_sends_escape = TRUE;
    /* Disable DECCOLM mode. */
    m_deccolm_mode = FALSE;
    /* Reset saved settings. */
    if (m_dec_saved != NULL) {
        g_hash_table_destroy(m_dec_saved);
        m_dec_saved = g_hash_table_new(NULL, NULL);
    }
    /* Reset the color palette. Only the 256 indexed colors, not the special ones, as per xterm. */
    for (int i = 0; i < 256; i++) {
        m_palette[i].sources[VTE_COLOR_SOURCE_ESCAPE].is_set = FALSE;
    }
    /* Reset the default attributes.  Reset the alternate attribute because
     * it's not a real attribute, but we need to treat it as one here. */
    reset_default_attributes(true);
    /* Reset charset modes. */
    m_character_replacements[0] = VTE_CHARACTER_REPLACEMENT_NONE;
    m_character_replacements[1] = VTE_CHARACTER_REPLACEMENT_NONE;
    m_character_replacement = &m_character_replacements[0];
    /* Clear the scrollback buffers and reset the cursors. Switch to normal screen. */
    if (clear_history) {
        m_screen = &m_normal_screen;
        m_normal_screen.scroll_delta = m_normal_screen.insert_delta =
                _vte_ring_reset(m_normal_screen.row_data);
        m_normal_screen.cursor.row = m_normal_screen.insert_delta;
        m_normal_screen.cursor.col = 0;
        m_alternate_screen.scroll_delta = m_alternate_screen.insert_delta =
                _vte_ring_reset(m_alternate_screen.row_data);
        m_alternate_screen.cursor.row = m_alternate_screen.insert_delta;
        m_alternate_screen.cursor.col = 0;
        /* Adjust the scrollbar to the new location. */
        /* Hack: force a change in scroll_delta even if the value remains, so that
            vte_term_q_adj_val_changed() doesn't shortcut to no-op, see bug 730599. */
        m_screen->scroll_delta = -1;
        queue_adjustment_value_changed(m_screen->insert_delta);
        adjust_adjustments_full();
    }
    /* DECSCUSR cursor style */
    set_cursor_style(VTE_CURSOR_STYLE_TERMINAL_DEFAULT);
    /* Do more stuff we refer to as a "full" reset. */
    if (clear_tabstops) {
        set_default_tabstops();
    }
    /* Reset restricted scrolling regions, leave insert mode, make
     * the cursor visible again. */
    m_scrolling_restricted = FALSE;
    m_sendrecv_mode = TRUE;
    m_insert_mode = FALSE;
    m_linefeed_mode = FALSE;
    m_origin_mode = FALSE;
    m_reverse_mode = FALSE;
    m_cursor_visible = TRUE;
    /* For some reason, xterm doesn't reset alternateScroll, but we do. */
    m_alternate_screen_scroll = TRUE;
    /* Reset the visual bits of selection on hard reset, see bug 789954. */
    if (clear_history) {
        deselect_all();
        m_has_selection = FALSE;
        m_selecting = FALSE;
        m_selecting_restart = FALSE;
        m_selecting_had_delta = FALSE;
        memset(&m_selection_origin, 0, sizeof(m_selection_origin));
        memset(&m_selection_last, 0, sizeof(m_selection_last));
        memset(&m_selection_start, 0, sizeof(m_selection_start));
        memset(&m_selection_end, 0, sizeof(m_selection_end));
    }

    /* Reset mouse motion events. */
    m_mouse_tracking_mode = MOUSE_TRACKING_NONE;
    apply_mouse_cursor();
    m_mouse_pressed_buttons = 0;
    m_mouse_handled_buttons = 0;
    m_mouse_xterm_extension = FALSE;
    m_mouse_urxvt_extension = FALSE;
    m_mouse_smooth_scroll_delta = 0.;
    /* Reset focus tracking */
    m_focus_tracking_mode = FALSE;
    /* Clear modifiers. */
    m_modifiers = 0;
    /* Reset miscellaneous stuff. */
    m_bracketed_paste_mode = FALSE;
    /* Reset the saved cursor. */
    save_cursor(&m_normal_screen);
    save_cursor(&m_alternate_screen);
    /* Cause everything to be redrawn (or cleared). */
    invalidate_all();

    g_object_thaw_notify(object);
}

/* We need this bit of glue to ensure that accessible objects will always
 * get signals. */
void VteTerminalPrivate::subscribe_accessible_events()
{
    m_accessible_emit = true;
}

void VteTerminalPrivate::select_text(vte::grid::column_t start_col,
                                        vte::grid::row_t start_row,
                                        vte::grid::column_t end_col,
                                        vte::grid::row_t end_row)
{
    deselect_all();

    m_selection_type = selection_type_char;
    m_selecting_had_delta = true;
    m_selection_start.col = start_col;
    m_selection_start.row = start_row;
    m_selection_end.col = end_col;
    m_selection_end.row = end_row;
    widget_copy(VTE_SELECTION_PRIMARY, VTE_FORMAT_TEXT);
    emit_selection_changed();

    invalidate_region(MIN (start_col, end_col), MAX (start_col, end_col),
                          MIN (start_row, end_row), MAX (start_row, end_row),
                          false);
}

void VteTerminalPrivate::select_empty(vte::grid::column_t col,
                                        vte::grid::row_t row)
{
        select_text(col, row, col - 1, row);
}

static void remove_process_timeout_source(void)
{
    if (process_timeout_tag == 0) {
        return;
    }

    _vte_debug_print(VTE_DEBUG_TIMEOUT, "Removing process timeout\n");
    g_source_remove (process_timeout_tag);
    process_timeout_tag = 0;
}

static void add_update_timeout(VteTerminalPrivate *that)
{
    if (update_timeout_tag == 0) {
        _vte_debug_print (VTE_DEBUG_TIMEOUT, "Starting update timeout\n");
        update_timeout_tag = g_timeout_add_full (GDK_PRIORITY_REDRAW,
                                                    VTE_UPDATE_TIMEOUT,
                                                    update_timeout, NULL,
                                                    NULL);
    }
    if (!in_process_timeout) {
                remove_process_timeout_source();
    }
    if (that->m_active_terminals_link == nullptr) {
        _vte_debug_print (VTE_DEBUG_TIMEOUT, "Adding terminal to active list\n");
        that->m_active_terminals_link = g_active_terminals =
                                    g_list_prepend(g_active_terminals, that);
    }
}

void VteTerminalPrivate::reset_update_rects()
{
    g_array_set_size(m_update_rects, 0);
    m_invalidated_all = FALSE;
}

static bool remove_from_active_list(VteTerminalPrivate *that)
{
    if (that->m_active_terminals_link == nullptr || that->m_update_rects->len != 0) {
        return false;
    }

    _vte_debug_print(VTE_DEBUG_TIMEOUT, "Removing terminal from active list\n");
    g_active_terminals = g_list_delete_link(g_active_terminals, that->m_active_terminals_link);
    that->m_active_terminals_link = nullptr;
    return true;
}

static void stop_processing(VteTerminalPrivate *that)
{
    if (!remove_from_active_list(that)) {
        return;
    }

    if (g_active_terminals != nullptr) {
        return;
    }

    if (!in_process_timeout) {
        remove_process_timeout_source();
    }
    if (in_update_timeout == FALSE && update_timeout_tag != 0) {
        _vte_debug_print(VTE_DEBUG_TIMEOUT, "Removing update timeout\n");
        g_source_remove (update_timeout_tag);
        update_timeout_tag = 0;
    }
}

static void remove_update_timeout(VteTerminalPrivate *that)
{
    that->reset_update_rects();
    stop_processing(that);
}

static void add_process_timeout(VteTerminalPrivate *that)
{
    _vte_debug_print(VTE_DEBUG_TIMEOUT, "Adding terminal to active list\n");
    that->m_active_terminals_link = g_active_terminals =
                                    g_list_prepend(g_active_terminals, that);
    if (update_timeout_tag == 0 && process_timeout_tag == 0) {
        _vte_debug_print(VTE_DEBUG_TIMEOUT, "Starting process timeout\n");
        process_timeout_tag = g_timeout_add (VTE_DISPLAY_TIMEOUT,
                                                process_timeout, NULL);
    }
}

void VteTerminalPrivate::start_processing()
{
    if (!is_processing()) {
        add_process_timeout(this);
    }
}

void VteTerminalPrivate::emit_pending_signals()
{
    GObject *object = G_OBJECT(m_terminal);
    g_object_freeze_notify(object);
    gboolean really_changed;

    emit_adjustment_changed();

    if (m_window_title_changed) {
        really_changed = (g_strcmp0(m_window_title, m_window_title_changed) != 0);
        g_free (m_window_title);
        m_window_title = m_window_title_changed;
        m_window_title_changed = NULL;

        if (really_changed) {
            _vte_debug_print(VTE_DEBUG_SIGNALS,
                                "Emitting `window-title-changed'.\n");
            g_signal_emit(object, signals[SIGNAL_WINDOW_TITLE_CHANGED], 0);
            g_object_notify_by_pspec(object, pspecs[PROP_WINDOW_TITLE]);
        }
    }

    if (m_icon_title_changed) {
        really_changed = (g_strcmp0(m_icon_title, m_icon_title_changed) != 0);
        g_free (m_icon_title);
        m_icon_title = m_icon_title_changed;
        m_icon_title_changed = NULL;

        if (really_changed) {
            _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `icon-title-changed'.\n");
            g_signal_emit(object, signals[SIGNAL_ICON_TITLE_CHANGED], 0);
            g_object_notify_by_pspec(object, pspecs[PROP_ICON_TITLE]);
        }
    }

    if (m_current_directory_uri_changed) {
        really_changed = (g_strcmp0(m_current_directory_uri, m_current_directory_uri_changed) != 0);
        g_free (m_current_directory_uri);
        m_current_directory_uri = m_current_directory_uri_changed;
        m_current_directory_uri_changed = NULL;

        if (really_changed) {
            _vte_debug_print(VTE_DEBUG_SIGNALS,
                                "Emitting `current-directory-uri-changed'.\n");
            g_signal_emit(object, signals[SIGNAL_CURRENT_DIRECTORY_URI_CHANGED], 0);
            g_object_notify_by_pspec(object, pspecs[PROP_CURRENT_DIRECTORY_URI]);
        }
    }

    if (m_current_file_uri_changed) {
        really_changed = (g_strcmp0(m_current_file_uri, m_current_file_uri_changed) != 0);
        g_free (m_current_file_uri);
        m_current_file_uri = m_current_file_uri_changed;
        m_current_file_uri_changed = NULL;

        if (really_changed) {
            _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `current-file-uri-changed'.\n");
            g_signal_emit(object, signals[SIGNAL_CURRENT_FILE_URI_CHANGED], 0);
            g_object_notify_by_pspec(object, pspecs[PROP_CURRENT_FILE_URI]);
        }
    }

    /* Flush any pending "inserted" signals. */

    if (m_cursor_moved_pending) {
        _vte_debug_print(VTE_DEBUG_SIGNALS,
                            "Emitting `cursor-moved'.\n");
        g_signal_emit(object, signals[SIGNAL_CURSOR_MOVED], 0);
        m_cursor_moved_pending = false;
    }
    if (m_text_modified_flag) {
        _vte_debug_print(VTE_DEBUG_SIGNALS,
                            "Emitting buffered `text-modified'.\n");
        emit_text_modified();
        m_text_modified_flag = false;
    }
    if (m_text_inserted_flag) {
        _vte_debug_print(VTE_DEBUG_SIGNALS,
                            "Emitting buffered `text-inserted'\n");
        emit_text_inserted();
        m_text_inserted_flag = false;
    }
    if (m_text_deleted_flag) {
        _vte_debug_print(VTE_DEBUG_SIGNALS,
                            "Emitting buffered `text-deleted'\n");
        emit_text_deleted();
        m_text_deleted_flag = false;
    }
    if (m_contents_changed_pending) {
        /* Update hyperlink and dingus match set. */
        match_contents_clear();
        if (m_mouse_cursor_over_widget) {
            hyperlink_hilite_update();
        }

        _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `contents-changed'.\n");
        g_signal_emit(m_terminal, signals[SIGNAL_CONTENTS_CHANGED], 0);
        m_contents_changed_pending = false;
    }
    if (m_bell_pending) {
        auto const timestamp = g_get_monotonic_time();
        if ((timestamp - m_bell_timestamp) >= VTE_BELL_MINIMUM_TIME_DIFFERENCE) {
            beep();
            emit_bell();

            m_bell_timestamp = timestamp;
        }

        m_bell_pending = false;
    }

    g_object_thaw_notify(object);
}

void VteTerminalPrivate::time_process_incoming()
{
    g_timer_reset(process_timer);
    process_incoming();
    auto elapsed = g_timer_elapsed(process_timer, NULL) * 1000;
    gssize target = VTE_MAX_PROCESS_TIME / elapsed * m_input_bytes;
    m_max_input_bytes = (m_max_input_bytes + target) / 2;
}

bool VteTerminalPrivate::process(bool emit_adj_changed)
{
    bool is_active;
    if (emit_adj_changed) {
        emit_adjustment_changed();
    }
    is_active = _vte_incoming_chunks_length(m_incoming) != 0;
    if (is_active) {
        if (VTE_MAX_PROCESS_TIME) {
            time_process_incoming();
        } else {
            process_incoming();
        }
        m_input_bytes = 0;
    } else {
        emit_pending_signals();
    }

    return is_active;
}

/* This function is called after DISPLAY_TIMEOUT ms.
 * It makes sure initial output is never delayed by more than DISPLAY_TIMEOUT
 */
static gboolean process_timeout (gpointer data)
{
    GList *l, *next;
    gboolean again;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_enter();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    in_process_timeout = TRUE;

    _vte_debug_print (VTE_DEBUG_WORK, "<");
    _vte_debug_print (VTE_DEBUG_TIMEOUT,
                          "Process timeout:  %d active\n",
                          g_list_length(g_active_terminals));

    for (l = g_active_terminals; l != NULL; l = next) {
        VteTerminalPrivate *that = reinterpret_cast<VteTerminalPrivate*>(l->data);
        bool active;

        next = l->next;

        if (l != g_active_terminals) {
            _vte_debug_print (VTE_DEBUG_WORK, "T");
        }

        /* FIXMEchpe find out why we don't emit_adjustment_changed() here!! */
        active = that->process(false);

        if (!active) {
            remove_from_active_list(that);
        }
    }

    _vte_debug_print (VTE_DEBUG_WORK, ">");

    if (g_active_terminals != nullptr && update_timeout_tag == 0) {
        again = TRUE;
    } else {
        _vte_debug_print(VTE_DEBUG_TIMEOUT, "Stopping process timeout\n");
        process_timeout_tag = 0;
        again = FALSE;
    }

    in_process_timeout = FALSE;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_leave();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    if (again) {
        /* Force us to relinquish the CPU as the child is running
         * at full tilt and making us run to keep up...
         */
        g_usleep (0);
    } else if (update_timeout_tag == 0) {
        /* otherwise free up memory used to capture incoming data */
        prune_chunks (10);
    }

    return again;
}

bool VteTerminalPrivate::invalidate_dirty_rects_and_process_updates()
{
    if (G_UNLIKELY(!widget_realized())) {
        return false;
    }

    if (G_UNLIKELY (!m_update_rects->len)) {
        return false;
    }

    auto region = cairo_region_create();
    auto n_rects = m_update_rects->len;
    for (guint i = 0; i < n_rects; i++) {
        cairo_rectangle_int_t *rect = &g_array_index(m_update_rects, cairo_rectangle_int_t, i);
        cairo_region_union_rectangle(region, rect);
    }
    g_array_set_size(m_update_rects, 0);
    m_invalidated_all = false;

    auto allocation = get_allocated_rect();
    cairo_region_translate(region,
                            allocation.x + m_padding.left,
                            allocation.y + m_padding.top);

    /* and perform the merge with the window visible area */
    gtk_widget_queue_draw_region(m_widget, region);
    cairo_region_destroy (region);

    return true;
}

static gboolean update_repeat_timeout (gpointer data)
{
    GList *l, *next;
    bool again;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_enter();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    in_update_timeout = TRUE;

    _vte_debug_print (VTE_DEBUG_WORK, "[");
    _vte_debug_print (VTE_DEBUG_TIMEOUT,
                          "Repeat timeout:  %d active\n",
                          g_list_length(g_active_terminals));

    for (l = g_active_terminals; l != NULL; l = next) {
        VteTerminalPrivate *that = reinterpret_cast<VteTerminalPrivate*>(l->data);

        next = l->next;

        if (l != g_active_terminals) {
            _vte_debug_print (VTE_DEBUG_WORK, "T");
        }

        that->process(true);

        again = that->invalidate_dirty_rects_and_process_updates();
        if (!again) {
            remove_from_active_list(that);
        }
    }

    _vte_debug_print (VTE_DEBUG_WORK, "]");

   /* We only stop the timer if no update request was received in this
    * past cycle.  Technically, always stop this timer object and maybe
    * reinstall a new one because we need to delay by the amount of time
    * it took to repaint the screen: bug 730732.
    */
    if (g_active_terminals == nullptr) {
        _vte_debug_print(VTE_DEBUG_TIMEOUT, "Stopping update timeout\n");
        update_timeout_tag = 0;
        again = false;
    } else {
        update_timeout_tag =
                g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                                    VTE_UPDATE_REPEAT_TIMEOUT,
                                    update_repeat_timeout, NULL,
                                    NULL);
        again = true;
    }

    in_update_timeout = FALSE;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_leave();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    if (again) {
        /* Force us to relinquish the CPU as the child is running
         * at full tilt and making us run to keep up...
         */
        g_usleep (0);
    } else {
        /* otherwise free up memory used to capture incoming data */
        prune_chunks (10);
    }

    return FALSE;  /* If we need to go again, we already have a new timer for that. */
}

static gboolean update_timeout (gpointer data)
{
    GList *l, *next;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_enter();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    in_update_timeout = TRUE;

    _vte_debug_print (VTE_DEBUG_WORK, "{");
    _vte_debug_print (VTE_DEBUG_TIMEOUT,
                          "Update timeout:  %d active\n",
                          g_list_length(g_active_terminals));

    remove_process_timeout_source();

    for (l = g_active_terminals; l != NULL; l = next) {
        VteTerminalPrivate *that = reinterpret_cast<VteTerminalPrivate*>(l->data);

        next = l->next;

        if (l != g_active_terminals) {
            _vte_debug_print (VTE_DEBUG_WORK, "T");
        }

        that->process(true);

        that->invalidate_dirty_rects_and_process_updates();
    }

    _vte_debug_print (VTE_DEBUG_WORK, "}");

    /* Set a timer such that we do not invalidate for a while. */
    /* This limits the number of times we draw to ~40fps. */
    update_timeout_tag =
        g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                    VTE_UPDATE_REPEAT_TIMEOUT,
                    update_repeat_timeout, NULL,
                    NULL);
    in_update_timeout = FALSE;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    gdk_threads_leave();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    return FALSE;
}

bool VteTerminalPrivate::write_contents_sync (GOutputStream *stream,
                                                VteWriteFlags flags,
                                                GCancellable *cancellable,
                                                GError **error)
{
    return _vte_ring_write_contents (m_screen->row_data,
                                        stream, flags,
                                        cancellable, error);
}

/*
 * Buffer search
 */

/* TODO Add properties & signals */


/*
 * VteTerminalPrivate::set_input_enabled:
 * @enabled: whether to enable user input
 *
 * Enables or disables user input. When user input is disabled,
 * the terminal's child will not receive any key press, or mouse button
 * press or motion events sent to it.
 *
 * Returns: %true iff the setting changed
 */
bool VteTerminalPrivate::set_input_enabled (bool enabled)
{
    if (enabled == m_input_enabled) {
        return false;
    }

    m_input_enabled = enabled;

    auto context = gtk_widget_get_style_context(m_widget);

    /* FIXME: maybe hide cursor when input disabled, too? */

    if (enabled) {
        if (gtk_widget_has_focus(m_widget)) {
            gtk_im_context_focus_in(m_im_context);
        }

        gtk_style_context_remove_class (context, GTK_STYLE_CLASS_READ_ONLY);
    } else {
        im_reset();
        if (gtk_widget_has_focus(m_widget)) {
            gtk_im_context_focus_out(m_im_context);
        }
        _vte_byte_array_clear(m_outgoing);

        gtk_style_context_add_class (context, GTK_STYLE_CLASS_READ_ONLY);
    }

    return true;
}

bool VteTerminalPrivate::process_word_char_exceptions(char const *str,
                                                        gunichar **arrayp,
                                                        gsize *lenp)
{
    const char *p;
    gunichar *array, c;
    gsize len, i;

    if (str == NULL) {
        str = WORD_CHAR_EXCEPTIONS_DEFAULT;
    }

    len = g_utf8_strlen(str, -1);
    array = g_new(gunichar, len);
    i = 0;

    for (p = str; *p; p = g_utf8_next_char(p)) {
        c = g_utf8_get_char(p);

       /* For forward compatibility reasons, we skip
        * characters that aren't supposed to be here,
        * instead of erroring out.
        */
        /* '-' must only be used*  at the start of the string */
        if (c == (gunichar)'-' && p != str) {
            continue;
        }
        if (!g_unichar_isgraph(c)) {
            continue;
        }
        if (g_unichar_isspace(c)) {
            continue;
        }
        if (g_unichar_isalnum(c)) {
            continue;
        }

        array[i++] = g_utf8_get_char(p);
    }

    g_assert(i <= len);
    len = i; /* we may have skipped some characters */

    /* Sort the result since we want to use bsearch on it */
    qsort(array, len, sizeof(gunichar), compare_unichar_p);

    /* Check that no character occurs twice */
    for (i = 1; i < len; i++) {
        if (array[i-1] != array[i]) {
            continue;
        }

        g_free(array);
        return false;
    }

#if 0
    /* Debug */
    for (i = 0; i < len; i++) {
        char utf[7];
        c = array[i];
        utf[g_unichar_to_utf8(c, utf)] = '\0';
        g_printerr("Word char exception: U+%04X %s\n", c, utf);
    }
#endif

    *lenp = len;
    *arrayp = array;
    return true;
}

/*
 * VteTerminalPrivate::set_word_char_exceptions:
 * @exceptions: a string of ASCII punctuation characters, or %nullptr
 *
 * With this function you can provide a set of characters which will
 * be considered parts of a word when doing word-wise selection, in
 * addition to the default which only considers alphanumeric characters
 * part of a word.
 *
 * The characters in @exceptions must be non-alphanumeric, each character
 * must occur only once, and if @exceptions contains the character
 * U+002D HYPHEN-MINUS, it must be at the start of the string.
 *
 * Use %nullptr to reset the set of exception characters to the default.
 *
 * Returns: %true if the word char exceptions changed
 */
bool VteTerminalPrivate::set_word_char_exceptions(char const* exceptions)
{
    gunichar *array;
    gsize len;

    if (g_strcmp0(exceptions, m_word_char_exceptions_string) == 0) {
        return false;
    }

    if (!process_word_char_exceptions(exceptions, &array, &len)) {
        return false;
    }

    g_free(m_word_char_exceptions_string);
    m_word_char_exceptions_string = g_strdup(exceptions);

    g_free(m_word_char_exceptions);
    m_word_char_exceptions = array;
    m_word_char_exceptions_len = len;

    return true;
}

void VteTerminalPrivate::set_clear_background(bool setting)
{
    if (m_clear_background == setting) {
        return;
    }

    m_clear_background = setting;
    invalidate_all();
}
