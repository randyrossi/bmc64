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

#define GLIB_DISABLE_DEPRECATION_WARNINGS /* FIXME */

#include <vice.h>

#include <sys/types.h>
#include <string.h>
#include <glib-object.h>
#include "debug.h"
#include "caps.hh"
#include "matcher.hh"
#include "table.hh"

struct _vte_matcher {
    _vte_matcher_match_func match; /* shortcut to the most common op */
    struct _vte_matcher_impl *impl;
    GValueArray *free_params;
};

static GMutex _vte_matcher_mutex;
static struct _vte_matcher *_vte_matcher_singleton = NULL;
static int _vte_matcher_ref_count = 0;

static struct _vte_matcher_impl dummy_vte_matcher_table = {
    &_vte_matcher_table
};

/* Add a string to the matcher. */
static void _vte_matcher_add(const struct _vte_matcher *matcher,
                             const char *pattern,
                             gssize length,
                             sequence_handler_t handler)
{
    matcher->impl->klass->add(matcher->impl, pattern, length, handler);
}

/* Loads all sequences into matcher */
static void _vte_matcher_init(struct _vte_matcher *matcher)
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "_vte_matcher_init()\n");

    unsigned int n_entries;
    auto entries = _vte_get_matcher_entries(&n_entries);

    for (unsigned int e = 0; e < n_entries; e++) {
        char const* code = entries[e].seq;

        /* Escape sequences from \e@ to \e_ have a C1 counterpart
            * with the eighth bit set instead of a preceding '\x1b'.
            * This is encoded in the current encoding, e.g. in UTF-8
            * the C1 CSI (U+009B) becomes \xC2\x9B.
            *
            * When matching, the bytestream is already decoded to
            * Unicode codepoints.  In the "code" string, each byte
            * is interpreted as a Unicode codepoint (in other words,
            * Latin-1 is assumed).  So '\x80' .. '\x9F' bytes
            * (e.g. the byte '\x9B' for CSI) are the right choice here.
            *
            * For each sequence containing N occurrences of \e@ to \e_,
            * we create 2^N variants, by replacing every subset of them
            * with their C1 counterpart.
            */
        int variants = 1;
        for (int i = 0; code[i] != '\0'; i++) {
            if (code[i] == '\x1B' && code[i + 1] >= '@' && code[i + 1] <= '_') {
                variants <<= 1;
            }
        }
        for (int n = 0; n < variants; n++) {
            char* c1 = g_strdup(code);
            int k = 0;
            for (int i = 0; c1[i] != '\0'; i++) {
                if (c1[i] == '\x1B' && c1[i + 1] >= '@' && c1[i + 1] <= '_') {
                    if (n & (1 << k)) {
                        memmove(c1 + i, c1 + i + 1, strlen(c1 + i + 1) + 1);
                        c1[i] += 0x40;
                    }
                    k++;
                }
            }
            _vte_matcher_add(matcher, c1, strlen(c1), entries[e].handler);
            g_free(c1);
        }
    }

    _VTE_DEBUG_IF(VTE_DEBUG_MATCHER) {
        g_printerr("Matcher contents:\n");
        _vte_matcher_print(matcher);
        g_printerr("\n");
    }
}

/* Allocates new matcher structure. */
static struct _vte_matcher *_vte_matcher_create(void)
{
    struct _vte_matcher *ret = NULL;

    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "_vte_matcher_create()\n");
    ret = g_slice_new(struct _vte_matcher);
    ret->impl = &dummy_vte_matcher_table;
    ret->match = NULL;
    ret->free_params = NULL;

    return ret;
}

/* Noone uses this matcher, free it. */
static void _vte_matcher_destroy(struct _vte_matcher *matcher)
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "_vte_matcher_destroy()\n");
    if (matcher->free_params != NULL) {
        g_value_array_free (matcher->free_params);
    }
    if (matcher->match != NULL) {
        /* do not call destroy on dummy values */
        matcher->impl->klass->destroy(matcher->impl);
    }
    g_slice_free(struct _vte_matcher, matcher);
}

/* Create and init matcher. */
struct _vte_matcher *_vte_matcher_new(void)
{
    struct _vte_matcher *ret = NULL;

    g_mutex_lock(&_vte_matcher_mutex);

    if (_vte_matcher_ref_count++ == 0) {
        g_assert(_vte_matcher_singleton == NULL);
        ret = _vte_matcher_create();

        if (ret->match == NULL) {
            ret->impl = ret->impl->klass->create();
            ret->match = ret->impl->klass->match;
            _vte_matcher_init(ret);
        }
        _vte_matcher_singleton = ret;
    }

    g_mutex_unlock(&_vte_matcher_mutex);
    return _vte_matcher_singleton;
}

/* Free a matcher. */
void _vte_matcher_free(struct _vte_matcher *matcher)
{
    g_assert(_vte_matcher_singleton != NULL);
    g_mutex_lock(&_vte_matcher_mutex);
    if (--_vte_matcher_ref_count == 0) {
        _vte_matcher_destroy(matcher);
        _vte_matcher_singleton = NULL;
    }
    g_mutex_unlock(&_vte_matcher_mutex);
}

/* Check if a string matches a sequence the matcher knows about. */
vte_matcher_result_t _vte_matcher_match(struct _vte_matcher *matcher,
                                        const gunichar *pattern,
                                        gssize length,
                                        sequence_handler_t *handler,
                                        const gunichar **consumed,
                                        GValueArray **array)
{
    if (G_UNLIKELY (array != NULL && matcher->free_params != NULL)) {
        *array = matcher->free_params;
        matcher->free_params = NULL;
    }
    return matcher->match(matcher->impl, pattern, length, handler, consumed, array);
}

/* Dump out the contents of a matcher, mainly for debugging. */
void _vte_matcher_print(struct _vte_matcher *matcher)
{
    matcher->impl->klass->print(matcher->impl);
}

/* Free a parameter array.  Most of the GValue elements can clean up after
 * themselves, but we're using gpointers to hold unicode character strings, and
 * we need to free those ourselves. */
void _vte_matcher_free_params_array(struct _vte_matcher *matcher, GValueArray *params)
{
    guint i;
    for (i = 0; i < params->n_values; i++) {
        auto value = g_value_array_get_nth(params, i);
        if (G_UNLIKELY (G_VALUE_HOLDS_POINTER(value))) {
            g_free(g_value_get_pointer(value));
        } else if (G_UNLIKELY (G_VALUE_HOLDS_BOXED(value))) {
            g_value_array_free((GValueArray*)g_value_get_boxed(value));
        }
    }
    if (G_UNLIKELY (matcher == NULL || matcher->free_params != NULL)) {
        g_value_array_free (params);
    } else {
        matcher->free_params = params;
        params->n_values = 0;
    }
}
