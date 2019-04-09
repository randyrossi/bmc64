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

#pragma once

#include <glib-object.h>

class VteTerminalPrivate;
namespace vte { namespace parser { struct Params; } }

typedef void (VteTerminalPrivate::* sequence_handler_t)(vte::parser::Params const&);

struct _vte_matcher;

struct _vte_matcher_impl {
    const struct _vte_matcher_class *klass;
    /* private */
};

typedef enum {
        VTE_MATCHER_RESULT_NO_MATCH,
        VTE_MATCHER_RESULT_MATCH,
        VTE_MATCHER_RESULT_PARTIAL
} vte_matcher_result_t;

struct vte_matcher_entry_t {
        char seq[20];
        sequence_handler_t handler;
};

vte_matcher_entry_t const* _vte_get_matcher_entries(unsigned int* n_entries);

typedef struct _vte_matcher_impl *(*_vte_matcher_create_func)(void);
typedef vte_matcher_result_t (*_vte_matcher_match_func)(struct _vte_matcher_impl *impl,
                                                        const gunichar *pattern,
                                                        gssize length,
                                                        sequence_handler_t *handler,
                                                        const gunichar **consumed,
                                                        GValueArray **array);
typedef void (*_vte_matcher_add_func)(struct _vte_matcher_impl *impl,
                                      const char *pattern, gssize length,
                                      sequence_handler_t handler);
typedef void (*_vte_matcher_print_func)(struct _vte_matcher_impl *impl);
typedef void (*_vte_matcher_destroy_func)(struct _vte_matcher_impl *impl);
struct _vte_matcher_class{
    _vte_matcher_create_func create;
    _vte_matcher_add_func add;
    _vte_matcher_print_func print;
    _vte_matcher_match_func match;
    _vte_matcher_destroy_func destroy;
};

/* Create and init matcher. */
struct _vte_matcher *_vte_matcher_new(void);

/* Free a matcher. */
void _vte_matcher_free(struct _vte_matcher *matcher);

/* Check if a string matches a sequence the matcher knows about. */
vte_matcher_result_t _vte_matcher_match(struct _vte_matcher *matcher,
                                        const gunichar *pattern,
                                        gssize length,
                                        sequence_handler_t *handler,
                                        const gunichar **consumed,
                                        GValueArray **array);

/* Dump out the contents of a matcher, mainly for debugging. */
void _vte_matcher_print(struct _vte_matcher *matcher);

/* Free a parameter array. */
void _vte_matcher_free_params_array(struct _vte_matcher *matcher, GValueArray *params);
