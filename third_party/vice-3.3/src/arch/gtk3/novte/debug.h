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

#ifndef novte_debug_h_included
#define novte_debug_h_included

#include <vice.h>

#include <glib.h>

#ifndef VTE_COMPILATION
#define _vte_debug_flags _vte_external_debug_flags
#define _vte_debug_init  _vte_external_debug_init
#define _vte_debug_on    _vte_external_debug_on
#if !defined(__GNUC__) || !G_HAVE_GNUC_VARARGS
#define _vte_debug_print _vte_external_debug_print
#endif
#endif

G_BEGIN_DECLS

typedef enum {
    VTE_DEBUG_MISC          = 1 << 0,
    VTE_DEBUG_PARSE         = 1 << 1,
    VTE_DEBUG_IO            = 1 << 2,
    VTE_DEBUG_UPDATES       = 1 << 3,
    VTE_DEBUG_EVENTS        = 1 << 4,
    VTE_DEBUG_SIGNALS       = 1 << 5,
    VTE_DEBUG_SELECTION     = 1 << 6,
    VTE_DEBUG_SUBSTITUTION  = 1 << 7,
    VTE_DEBUG_RING          = 1 << 8,
    VTE_DEBUG_PTY           = 1 << 9,
    VTE_DEBUG_CURSOR        = 1 << 10,
    VTE_DEBUG_KEYBOARD      = 1 << 11,
    VTE_DEBUG_LIFECYCLE     = 1 << 12,
    VTE_DEBUG_MATCHER       = 1 << 13,
    VTE_DEBUG_WORK          = 1 << 14,
    VTE_DEBUG_CELLS         = 1 << 15,
    VTE_DEBUG_TIMEOUT       = 1 << 16,
    VTE_DEBUG_DRAW          = 1 << 17,
    VTE_DEBUG_ALLY          = 1 << 18,
    VTE_DEBUG_ADJ           = 1 << 19,
    VTE_DEBUG_PANGOCAIRO    = 1 << 20,
    VTE_DEBUG_WIDGET_SIZE   = 1 << 21,
    VTE_DEBUG_STYLE         = 1 << 22,
    VTE_DEBUG_RESIZE        = 1 << 23,
    VTE_DEBUG_REGEX         = 1 << 24,
    VTE_DEBUG_HYPERLINK     = 1 << 25,
} VteDebugFlags;

void _vte_debug_init(void);
const char *_vte_debug_sequence_to_string(const char *str,
                                          gssize length);

extern guint _vte_debug_flags;
static inline gboolean _vte_debug_on(guint flags) G_GNUC_CONST G_GNUC_UNUSED;

static inline gboolean
_vte_debug_on(guint flags)
{
    return (_vte_debug_flags & flags) != 0;
}

#ifdef VTE_DEBUG
#define _VTE_DEBUG_IF(flags) if (G_UNLIKELY (_vte_debug_on (flags)))
#else
#define _VTE_DEBUG_IF(flags) if (0)
#endif

#ifdef VTE_DEBUG
#if defined(__GNUC__) && G_HAVE_GNUC_VARARGS
#define _vte_debug_print(flags, fmt, ...) \
    G_STMT_START { _VTE_DEBUG_IF(flags) g_printerr(fmt, ##__VA_ARGS__); } G_STMT_END
#else
#include <stdarg.h>
static void _vte_debug_print(guint flags, const char *fmt, ...)
{
    _VTE_DEBUG_IF(flags) {
        va_list  ap;
        va_start (ap, fmt);
        g_vfprintf (stderr, fmt, ap);
        va_end (ap);
    }
}
#endif
#else
#define _vte_debug_print(args...) do { } while(0)
#endif /* VTE_DEBUG */

G_END_DECLS

#endif
