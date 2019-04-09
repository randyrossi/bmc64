/*
 * Copyright (C) 2002 Red Hat, Inc.
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

/* The interfaces in this file are subject to change at any time. */

#ifndef novterowdata_h_included
#define novterowdata_h_included

#include <string.h>

#include "vteunistr.h"
#include "vtemacros.h"
#include "vtedefines.hh"

#include "attr.hh"
#include "color-triple.hh"

G_BEGIN_DECLS

#define VTE_TAB_WIDTH_MAX       ((1 << VTE_ATTR_COLUMNS_BITS) - 1)

#define VTE_CELL_ATTR_COMMON_BYTES      12  /* The number of common bytes in VteCellAttr and VteStreamCellAttr */

/*
 * VteCellAttr: A single cell style attributes
 *
 * When adding new attributes, keep in sync with VteStreamCellAttr and
 * update VTE_CELL_ATTR_COMMON_BYTES accordingly.
 * Also don't forget to update basic_cell below!
 */

#define CELL_ATTR_BOOL(lname,uname) \
        inline void set_##lname(bool value) \
        { \
                vte_attr_set_bool(&attr, VTE_ATTR_##uname##_MASK, value); \
        } \
        \
        inline constexpr bool lname() const \
        { \
                return vte_attr_get_bool(attr, VTE_ATTR_##uname##_SHIFT); \
        }

#define CELL_ATTR_UINT(lname,uname) \
        inline void set_##lname(unsigned int value) \
        { \
                vte_attr_set_value(&attr, VTE_ATTR_##uname##_MASK, VTE_ATTR_##uname##_SHIFT, value); \
        } \
        \
        inline constexpr uint32_t lname() const \
        { \
                return vte_attr_get_value(attr, VTE_ATTR_##uname##_VALUE_MASK, VTE_ATTR_##uname##_SHIFT); \
        }

typedef struct _VTE_GNUC_PACKED VteCellAttr {
    uint32_t attr;

    /* 4-byte boundary (8-byte boundary in VteCell) */
    uint64_t m_colors;      /* fore, back and deco (underline) colour */

    /* 12-byte boundary (16-byte boundary in VteCell) */
    uint32_t hyperlink_idx; /* a unique hyperlink index at a time for the ring's cells,
                               0 means not a hyperlink, VTE_HYPERLINK_IDX_TARGET_IN_STREAM
                               means the target is irrelevant/unknown at the moment.
                               If bitpacking, choose a size big enough to hold a different idx
                               for every cell in the ring but not yet in the stream
                               (currently the height rounded up to the next power of two, times width)
                               for supported VTE sizes, and update VTE_HYPERLINK_IDX_TARGET_IN_STREAM. */

    /* Methods */

    inline constexpr uint64_t colors() const { return m_colors; }

    inline void copy_colors(VteCellAttr const& other)
    {
            m_colors = vte_color_triple_copy(other.colors());
    }

#define CELL_ATTR_COLOR(name) \
        inline void set_##name(uint32_t value) \
        { \
                vte_color_triple_set_##name(&m_colors, value); \
        } \
        \
        inline constexpr uint32_t name() const \
        { \
                return vte_color_triple_get_##name(m_colors); \
        }

    CELL_ATTR_COLOR(fore)
    CELL_ATTR_COLOR(back)
    CELL_ATTR_COLOR(deco)
#undef CELL_ATTR_COLOR

    inline constexpr bool has_any(uint32_t mask) const
    {
            return !!(attr & mask);
    }

    inline constexpr bool has_all(uint32_t mask) const
    {
            return (attr & mask) == mask;
    }

    inline constexpr bool has_none(uint32_t mask) const
    {
            return !(attr & mask);
    }

    inline void unset(uint32_t mask)
    {
            attr &= ~mask;
    }

    CELL_ATTR_UINT(columns, COLUMNS)
    CELL_ATTR_BOOL(fragment, FRAGMENT)
    CELL_ATTR_BOOL(bold, BOLD)
    CELL_ATTR_BOOL(italic, ITALIC)
    CELL_ATTR_UINT(underline, UNDERLINE)
    CELL_ATTR_BOOL(strikethrough, STRIKETHROUGH)
    CELL_ATTR_BOOL(overline, OVERLINE)
    CELL_ATTR_BOOL(reverse, REVERSE)
    CELL_ATTR_BOOL(blink, BLINK)
    CELL_ATTR_BOOL(dim, DIM)
    CELL_ATTR_BOOL(invisible, INVISIBLE)
    /* ATTR_BOOL(boxed, BOXED) */
} VteCellAttr;
#if 0 /* FIXME */
G_STATIC_ASSERT (sizeof (VteCellAttr) == 16);
G_STATIC_ASSERT (offsetof (VteCellAttr, hyperlink_idx) == VTE_CELL_ATTR_COMMON_BYTES);
#endif

/*
 * VteStreamCellAttr: Variant of VteCellAttr to be stored in attr_stream.
 *
 * When adding new attributes, keep in sync with VteCellAttr and
 * update VTE_CELL_ATTR_COMMON_BYTES accordingly.
 */

typedef struct _VTE_GNUC_PACKED _VteStreamCellAttr {
    uint32_t attr; /* Same as VteCellAttr. We only access columns
                    * and fragment, however.
                    */
    /* 4-byte boundary */
    uint64_t colors;
    /* 12-byte boundary */
    guint16 hyperlink_length;       /* make sure it fits VTE_HYPERLINK_TOTAL_LENGTH_MAX */

    /* Methods */
    CELL_ATTR_UINT(columns, COLUMNS)
    CELL_ATTR_BOOL(fragment, FRAGMENT)
} VteStreamCellAttr;
#if 0 /* FIXME */
G_STATIC_ASSERT (sizeof (VteStreamCellAttr) == 14);
G_STATIC_ASSERT (offsetof (VteStreamCellAttr, hyperlink_length) == VTE_CELL_ATTR_COMMON_BYTES);
#endif

#undef CELL_ATTR_BOOL
#undef CELL_ATTR_UINT

/*
 * VteCell: A single cell's data
 */

typedef struct _VTE_GNUC_PACKED _VteCell {
    vteunistr c;
    VteCellAttr attr;
} VteCell;
#if 0 /* FIXME */
G_STATIC_ASSERT (sizeof (VteCell) == 20);
#endif

static const VteCell basic_cell = {
    0,
    {
        VTE_ATTR_DEFAULT, /* attr */
        VTE_COLOR_TRIPLE_INIT_DEFAULT, /* colors */
        0, /* hyperlink_idx */
    }
};


/*
 * VteRowAttr: A single row's attributes
 */

typedef struct _VteRowAttr {
    guint8 soft_wrapped: 1;
} VteRowAttr;
G_STATIC_ASSERT (sizeof (VteRowAttr) == 1);

/*
 * VteRowData: A single row's data
 */

typedef struct _VteRowData {
    VteCell *cells;
    guint16 len;
    VteRowAttr attr;
} VteRowData;


#define _vte_row_data_length(__row)         ((__row)->len + 0)

static inline const VteCell *_vte_row_data_get (const VteRowData *row, gulong col)
{
    if (G_UNLIKELY (row->len <= col)) {
        return NULL;
    }

    return &row->cells[col];
}

static inline VteCell *_vte_row_data_get_writable (VteRowData *row, gulong col)
{
    if (G_UNLIKELY (row->len <= col)) {
        return NULL;
    }

    return &row->cells[col];
}

/*
 * Copy the common attributes from VteCellAttr to VteStreamCellAttr or vice versa.
 */
static inline void _attrcpy (void *dst, void *src)
{
    memcpy(dst, src, VTE_CELL_ATTR_COMMON_BYTES);
}

void _vte_row_data_init (VteRowData *row);
void _vte_row_data_clear (VteRowData *row);
void _vte_row_data_fini (VteRowData *row);
void _vte_row_data_insert (VteRowData *row, gulong col, const VteCell *cell);
void _vte_row_data_append (VteRowData *row, const VteCell *cell);
void _vte_row_data_remove (VteRowData *row, gulong col);
void _vte_row_data_fill (VteRowData *row, const VteCell *cell, gulong len);
void _vte_row_data_shrink (VteRowData *row, gulong max_len);


G_END_DECLS

#endif
