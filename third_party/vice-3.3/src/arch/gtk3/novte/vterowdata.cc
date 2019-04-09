/*
 * Copyright (C) 2002,2009 Red Hat, Inc.
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
 * Red Hat Author(s): Nalin Dahyabhai, Behdad Esfahbod
 */

#include <vice.h>

#include "debug.h"
#include "vterowdata.h"

#include <string.h>


/*
 * VteCells: A row's cell array
 */

typedef struct _VteCells VteCells;
struct _VteCells {
    guint32 alloc_len;
    VteCell cells[1];
};

static inline VteCells *_vte_cells_for_cell_array (VteCell *cells)
{
    if (G_UNLIKELY (!cells)) {
        return NULL;
    }

    return (VteCells *) (((guchar *) cells) - G_STRUCT_OFFSET (VteCells, cells));
}

static VteCells *_vte_cells_realloc (VteCells *cells, guint32 len)
{
    guint32 alloc_len = (1 << g_bit_storage (MAX (len, 80))) - 1;

    _vte_debug_print(VTE_DEBUG_RING, "Enlarging cell array of %d cells to %d cells\n", cells ? cells->alloc_len : 0, alloc_len);
    cells = (VteCells *)g_realloc (cells, G_STRUCT_OFFSET (VteCells, cells) + alloc_len * sizeof (cells->cells[0]));
    cells->alloc_len = alloc_len;

    return cells;
}

static void _vte_cells_free (VteCells *cells)
{
    _vte_debug_print(VTE_DEBUG_RING, "Freeing cell array of %d cells\n", cells->alloc_len);
    g_free (cells);
}


/*
 * VteRowData: A row's data
 */

void _vte_row_data_init (VteRowData *row)
{
    memset (row, 0, sizeof (*row));
}

void _vte_row_data_clear (VteRowData *row)
{
    VteCell *cells = row->cells;
    _vte_row_data_init (row);
    row->cells = cells;
}

void _vte_row_data_fini (VteRowData *row)
{
    if (row->cells) {
        _vte_cells_free (_vte_cells_for_cell_array (row->cells));
    }
    row->cells = NULL;
}

static inline gboolean _vte_row_data_ensure (VteRowData *row, gulong len)
{
    VteCells *cells = _vte_cells_for_cell_array (row->cells);
    if (G_LIKELY (cells && len <= cells->alloc_len)) {
        return TRUE;
    }

    if (G_UNLIKELY (len >= 0xFFFF)) {
        return FALSE;
    }

    row->cells = _vte_cells_realloc (cells, len)->cells;

    return TRUE;
}

void _vte_row_data_insert (VteRowData *row, gulong col, const VteCell *cell)
{
    gulong i;

    if (G_UNLIKELY (!_vte_row_data_ensure (row, row->len + 1))) {
        return;
    }

    for (i = row->len; i > col; i--) {
        row->cells[i] = row->cells[i - 1];
    }

    row->cells[col] = *cell;
    row->len++;
}

void _vte_row_data_append (VteRowData *row, const VteCell *cell)
{
    if (G_UNLIKELY (!_vte_row_data_ensure (row, row->len + 1))) {
        return;
    }

    row->cells[row->len] = *cell;
    row->len++;
}

void _vte_row_data_remove (VteRowData *row, gulong col)
{
    gulong i;

    for (i = col + 1; i < row->len; i++) {
        row->cells[i - 1] = row->cells[i];
    }

    if (G_LIKELY (row->len)) {
        row->len--;
    }
}

void _vte_row_data_fill (VteRowData *row, const VteCell *cell, gulong len)
{
    if (row->len < len) {
        gulong i;

        if (G_UNLIKELY (!_vte_row_data_ensure (row, len))) {
            return;
        }

        for (i = row->len; i < len; i++) {
            row->cells[i] = *cell;
        }

        row->len = len;
    }
}

void _vte_row_data_shrink (VteRowData *row, gulong max_len)
{
    if (max_len < row->len) {
        row->len = max_len;
    }
}

