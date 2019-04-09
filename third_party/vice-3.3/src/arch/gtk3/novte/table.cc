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
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include "debug.h"
#include "iso2022.h"
#include "table.hh"

/* Table info. */
#define VTE_TABLE_MAX_LITERAL (128 + 32)
#define _vte_table_map_literal(__c) \
    (((__c) < (VTE_TABLE_MAX_LITERAL)) ? (__c) : 0)
#define _vte_table_is_numeric(__c) \
    (((__c) >= '0') && ((__c) <= '9'))
#define _vte_table_is_numeric_list(__c) \
    ((((__c) >= '0') && ((__c) <= '9')) || (__c) == ';' || (__c) == ':')

struct _vte_table {
    struct _vte_matcher_impl impl;
    sequence_handler_t handler;
    unsigned char *original;
    gssize original_length;
    struct _vte_table *table_string;
    struct _vte_table *table_number;
    struct _vte_table *table_number_list;
    struct _vte_table **table;
};

/* Argument info. */
enum _vte_table_argtype {
    _vte_table_arg_number=0,
    _vte_table_arg_string,
    _vte_table_arg_char
};
struct _vte_table_arginfo {
    const gunichar *start;
    struct _vte_table_arginfo *next;
    guint type:2;
    guint length:30;
};
#define MAX_STACK 16
struct _vte_table_arginfo_head {
    guint stack_allocated;
    struct _vte_table_arginfo *list;
    struct _vte_table_arginfo stack[MAX_STACK];
};

static void _vte_table_arginfo_head_init(struct _vte_table_arginfo_head *head)
{
    head->list = NULL;
    head->stack_allocated = 0;
}
static inline struct _vte_table_arginfo* _vte_table_arginfo_alloc(struct _vte_table_arginfo_head *head)
{
    struct _vte_table_arginfo *info;
    if (G_LIKELY (head->stack_allocated < G_N_ELEMENTS(head->stack))) {
        info = &head->stack[head->stack_allocated++];
    } else {
        info = g_slice_new (struct _vte_table_arginfo);
    }
    info->next = head->list;
    head->list = info;
    return info;
}
static void _vte_table_arginfo_head_revert(struct _vte_table_arginfo_head *head, struct _vte_table_arginfo *last)
{
    struct _vte_table_arginfo *info;
    info = head->list;
    head->list = last->next;
    if (last >= &head->stack[0] && last < &head->stack[G_N_ELEMENTS(head->stack)]){
        head->stack_allocated = last - &head->stack[0];
    }
    do {
        struct _vte_table_arginfo *next = info->next;
        if (info >= &head->stack[0] && info < &head->stack[G_N_ELEMENTS(head->stack)]){
            break;
        }
        g_slice_free(struct _vte_table_arginfo, info);
        if (info == last) {
            break;
        }
        info = next;
    }while (TRUE);
}
static struct _vte_table_arginfo *_vte_table_arginfo_head_reverse(struct _vte_table_arginfo_head *head)
{
    struct _vte_table_arginfo *prev = NULL;
    while (head->list) {
        struct _vte_table_arginfo *next = head->list->next;

        head->list->next = prev;

        prev = head->list;
        head->list = next;
    }
    return prev;
}
static void _vte_table_arginfo_head_finalize(struct _vte_table_arginfo_head *head)
{
    struct _vte_table_arginfo *info, *next;
    for (info = head->list; info != NULL; info = next) {
        next = info->next;
        if (info >= &head->stack[0] && info < &head->stack[G_N_ELEMENTS(head->stack)]){
            continue;
        }
        g_slice_free(struct _vte_table_arginfo, info);
    }
}

/* Create an empty, one-level table. */
struct _vte_table *_vte_table_new(void)
{
    struct _vte_table * ret;
    ret = g_slice_new0(struct _vte_table);
    ret->impl.klass = &_vte_matcher_table;
    return ret;
}

static struct _vte_table **_vte_table_literal_new(void)
{
    return g_new0(struct _vte_table *, VTE_TABLE_MAX_LITERAL);
}

/* Free a table. */
void _vte_table_free(struct _vte_table *table)
{
    unsigned int i;
    if (table->table != NULL) {
        for (i = 0; i < VTE_TABLE_MAX_LITERAL; i++) {
            if (table->table[i] != NULL) {
                _vte_table_free(table->table[i]);
            }
        }
        g_free(table->table);
    }
    if (table->table_string != NULL) {
        _vte_table_free(table->table_string);
    }
    if (table->table_number != NULL) {
        _vte_table_free(table->table_number);
    }
    if (table->table_number_list != NULL) {
        _vte_table_free(table->table_number_list);
    }
    if (table->original_length == 0) {
        g_assert(table->original == NULL);
    } else {
        g_assert(table->original != NULL);
    }
    if (table->original != NULL) {
        g_free(table->original);
    }
    g_slice_free(struct _vte_table, table);
}

static void _vte_table_addi(struct _vte_table *table,
        const unsigned char *original,
        gssize original_length,
        const char *pattern,
        gssize length,
        sequence_handler_t handler)
{
    int i;
    guint8 check;
    struct _vte_table *subtable;

    if (original_length == -1) {
        original_length = strlen((char *) original);
    }
    if (length == -1) {
        length = strlen(pattern);
    }

    /* If this is the terminal node, set the result. */
    if (length == 0) {
        if (table->handler) {
            _VTE_DEBUG_IF (VTE_DEBUG_PARSE) {
                    g_printerr ("'%s'",
                                _vte_debug_sequence_to_string ((const char *)table->original,
                                                                table->original_length));
                    g_printerr (" and '%s' are indistinguishable.\n",
                                _vte_debug_sequence_to_string ((const char *)original,
                                                                original_length));
            }
        }
        table->handler = handler;
        g_free(table->original);
        table->original = (unsigned char *) g_memdup(original, original_length);
        table->original_length = original_length;
        return;
    }

    /* All of the interesting arguments begin with '%'. */
    if (pattern[0] == '%') {
        /* Handle numeric parameters. */
        if (pattern[1] == 'd') {
            /* Create a new subtable. */
            if (table->table_number == NULL) {
                subtable = _vte_table_new();
                table->table_number = subtable;
            } else {
                subtable = table->table_number;
            }
            /* Add the rest of the string to the subtable. */
            _vte_table_addi(subtable, original, original_length,
                                pattern + 2, length - 2, handler);
            return;
        }

        /* Handle variable-length parameters. */
        if (pattern[1] == 'm') {
            /* Build the "new" original using the initial portion
             * of the original string and what's left after this
             * specifier. */
            {
                int initial;
                GByteArray *b;

                initial = original_length - length;
                b = g_byte_array_new();
                g_byte_array_set_size(b, 0);
                g_byte_array_append(b, original, initial);
                g_byte_array_append(b, (const guint8*)pattern + 2, length - 2);
                _vte_table_addi(table, b->data, b->len, (const char *)b->data + initial,
                                b->len - initial, handler);
                g_byte_array_free(b, TRUE);
            }
            /* Create a new subtable. */
            if (table->table_number_list == NULL) {
                subtable = _vte_table_new();
                table->table_number_list = subtable;
            } else {
                subtable = table->table_number_list;
            }
            /* Add the rest of the string to the subtable. */
            _vte_table_addi(subtable, original, original_length,
                            pattern + 2, length - 2, handler);
            return;
        }

        /* Handle string parameters. */
        if (pattern[1] == 's') {
            /* It must have a terminator. */
            g_assert(length >= 3);
            /* Create a new subtable. */
            if (table->table_string == NULL) {
                subtable = _vte_table_new();
                table->table_string = subtable;
            } else {
                subtable = table->table_string;
            }
            /* Add the rest of the string to the subtable. */
            _vte_table_addi(subtable, original, original_length,
                            pattern + 2, length - 2, handler);
            return;
        }

        /* Handle an escaped '%'. */
        if (pattern[1] == '%') {
            /* Create a new subtable. */
            if (table->table == NULL) {
                table->table = _vte_table_literal_new();
                subtable = _vte_table_new();
                table->table['%'] = subtable;
            } else
            if (table->table['%'] == NULL) {
                subtable = _vte_table_new();
                table->table['%'] = subtable;
            } else {
                subtable = table->table['%'];
            }
            /* Add the rest of the string to the subtable. */
            _vte_table_addi(subtable, original, original_length,
                            pattern + 2, length - 2, handler);
            return;
        }

        /* Handle a parameter character. */
        if (pattern[1] == '+') {
            /* It must have an addend. */
            g_assert(length >= 3);
            /* Fill in all of the table entries above the given
             * character value. */
            for (i = pattern[2]; i < VTE_TABLE_MAX_LITERAL; i++) {
                /* Create a new subtable. */
                if (table->table == NULL) {
                    table->table = _vte_table_literal_new();
                    subtable = _vte_table_new();
                    table->table[i] = subtable;
                } else if (table->table[i] == NULL) {
                    subtable = _vte_table_new();
                    table->table[i] = subtable;
                } else {
                    subtable = table->table[i];
                }
                /* Add the rest of the string to the subtable. */
                _vte_table_addi(subtable, original, original_length,
                                pattern + 3, length - 3, handler);
            }
            /* Also add a subtable for higher characters. */
            if (table->table == NULL) {
                table->table = _vte_table_literal_new();
                subtable = _vte_table_new();
                table->table[0] = subtable;
            } else if (table->table[0] == NULL) {
                subtable = _vte_table_new();
                table->table[0] = subtable;
            } else {
                subtable = table->table[0];
            }
            /* Add the rest of the string to the subtable. */
            _vte_table_addi(subtable, original, original_length,
                            pattern + 3, length - 3, handler);
            return;
        }
    }

    /* A literal (or an unescaped '%', which is also a literal). */
    check = (guint8) pattern[0];
    g_assert(check < VTE_TABLE_MAX_LITERAL);
    if (table->table == NULL) {
        table->table = _vte_table_literal_new();
        subtable = _vte_table_new();
        table->table[check] = subtable;
    } else if (table->table[check] == NULL) {
        subtable = _vte_table_new();
        table->table[check] = subtable;
    } else {
        subtable = table->table[check];
    }

    /* Add the rest of the string to the subtable. */
    _vte_table_addi(subtable, original, original_length,
                    pattern + 1, length - 1, handler);
}

/* Add a string to the matching tree. */
void _vte_table_add(struct _vte_table *table, const char *pattern,
                    gssize length, sequence_handler_t handler)
{
    _vte_table_addi(table, (const unsigned char *) pattern, length,
                        pattern, length, handler);
}

/* Match a string in a subtree. */
static vte_matcher_result_t _vte_table_matchi(struct _vte_table *table,
                                                const gunichar *candidate,
                                                gssize length,
                                                sequence_handler_t *handler,
                                                const gunichar **consumed,
                                                unsigned char **original,
                                                gssize *original_length,
                                                struct _vte_table_arginfo_head *params)
{
    int i = 0;
    struct _vte_table *subtable = NULL;
    struct _vte_table_arginfo *arginfo;

    /* Check if this is a result node. */
    if (table->handler) {
        *consumed = candidate;
        *original = table->original;
        *original_length = table->original_length;
        *handler = table->handler;
        return VTE_MATCHER_RESULT_MATCH;
    }

    /* If we're out of data, but we still have children, return the empty
     * string. */
    if (G_UNLIKELY (length == 0)) {
        *consumed = candidate;
        *handler = nullptr;
        return VTE_MATCHER_RESULT_PARTIAL;
    }

    /* Check if this node has a string disposition. */
    if (table->table_string != NULL) {
        /* Iterate over all non-terminator values. */
        subtable = table->table_string;
        for (i = 0; i < length; i++) {
            if ((subtable->table != NULL) &&
                (subtable->table[_vte_table_map_literal(candidate[i])] != NULL)) {
                break;
            }
        }
        /* Save the parameter info. */
        arginfo = _vte_table_arginfo_alloc(params);
        arginfo->type = _vte_table_arg_string;
        arginfo->start = candidate;
        arginfo->length = i;
        /* Continue. */
        return _vte_table_matchi(subtable, candidate + i, length - i,
                     handler, consumed, original, original_length, params);
    }

    /* Check if this could be a list. */
    if ((_vte_table_is_numeric_list(candidate[0])) &&
        (table->table_number_list != NULL)) {

        subtable = table->table_number_list;
        /* Iterate over all numeric characters, ';' and ':'. */
        for (i = 1; i < length; i++) {
            if (!_vte_table_is_numeric_list(candidate[i])) {
                break;
            }
        }
        /* Save the parameter info. */
        arginfo = _vte_table_arginfo_alloc(params);
        arginfo->type = _vte_table_arg_number;
        arginfo->start = candidate;
        arginfo->length = i;

        /* Try and continue. */
        auto local_result = _vte_table_matchi(subtable,
                                                candidate + i, length - i,
                                                handler, consumed,
                                                original, original_length,
                                                params);
        if (local_result != VTE_MATCHER_RESULT_NO_MATCH) {
            return local_result;
        }
        _vte_table_arginfo_head_revert (params, arginfo);

        /* try again */
    }

    /* Check if this could be a number. */
    if ((_vte_table_is_numeric(candidate[0])) &&
        (table->table_number != NULL)) {
        subtable = table->table_number;
        /* Iterate over all numeric characters. */
        for (i = 1; i < length; i++) {
            if (!_vte_table_is_numeric(candidate[i])) {
                break;
            }
        }
        /* Save the parameter info. */
        arginfo = _vte_table_arginfo_alloc(params);
        arginfo->type = _vte_table_arg_number;
        arginfo->start = candidate;
        arginfo->length = i;
        /* Continue. */
        return _vte_table_matchi(subtable, candidate + i, length - i,
                     handler, consumed, original, original_length, params);
    }

    /* Check for an exact match. */
    if ((table->table != NULL) &&
        (table->table[_vte_table_map_literal(candidate[0])] != NULL)) {
        subtable = table->table[_vte_table_map_literal(candidate[0])];
        /* Save the parameter info. */
        arginfo = _vte_table_arginfo_alloc(params);
        arginfo->type = _vte_table_arg_char;
        arginfo->start = candidate;
        arginfo->length = 1;
        /* Continue. */
        return _vte_table_matchi(subtable, candidate + 1, length - 1,
                     handler, consumed, original, original_length, params);
    }

    /* If there's nothing else to do, then we can't go on.  Keep track of
     * where we are. */
    *consumed = candidate;
    *handler = nullptr;
    return VTE_MATCHER_RESULT_NO_MATCH;
}

static void _vte_table_extract_numbers(GValueArray **array, struct _vte_table_arginfo *arginfo)
{
    if (G_UNLIKELY (*array == NULL)) {
        *array = g_value_array_new(1);
    }

    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_LONG);
    gssize i = 0;
    GValueArray *subarray = nullptr;
    do {
        long total = 0;
        for (; i < arginfo->length && arginfo->start[i] != ';' && arginfo->start[i] != ':'; i++) {
            gint v = g_unichar_digit_value (arginfo->start[i]);
            total *= 10;
            total += v == -1 ?  0 : v;
        }
        g_value_set_long(&value, CLAMP (total, 0, G_MAXUSHORT));
        if (i < arginfo->length && arginfo->start[i] == ':') {
            if (subarray == NULL) {
                subarray = g_value_array_new(2);
            }
            g_value_array_append(subarray, &value);
        } else {
            if (subarray == NULL) {
                g_value_array_append(*array, &value);
            } else {
                g_value_array_append(subarray, &value);

                GValue subvalue = G_VALUE_INIT;
                g_value_init(&subvalue, G_TYPE_VALUE_ARRAY);
                g_value_take_boxed(&subvalue, subarray);
                g_value_array_append(*array, &subvalue);
                g_value_unset(&subvalue);

                subarray = NULL;
            }
        }
    } while (i++ < arginfo->length);
    g_value_unset(&value);
    if (subarray != nullptr) {
        g_value_array_free(subarray);
    }
}

static void _vte_table_extract_string(GValueArray **array, struct _vte_table_arginfo *arginfo)
{
    GValue value = G_VALUE_INIT;
    gunichar *ptr;

    ptr = g_new(gunichar, arginfo->length + 1);
    memcpy(ptr, arginfo->start, sizeof(gunichar) * arginfo->length);
    ptr[arginfo->length] = '\0';
    g_value_init(&value, G_TYPE_POINTER);
    g_value_set_pointer(&value, ptr);

    if (G_UNLIKELY (*array == NULL)) {
        *array = g_value_array_new(1);
    }
    g_value_array_append(*array, &value);
    g_value_unset(&value);
}

/* Check if a string matches something in the tree. */
vte_matcher_result_t _vte_table_match(struct _vte_table *table,
                                        const gunichar *candidate,
                                        gssize length,
                                        sequence_handler_t *handler,
                                        const gunichar **consumed,
                                        GValueArray **array)
{
    struct _vte_table *head;
    unsigned char *original, *p;
    gssize original_length;
    int i;
    struct _vte_table_arginfo_head params;
    struct _vte_table_arginfo *arginfo;

    g_assert_nonnull(handler);
    g_assert_nonnull(consumed);
    *consumed = candidate;

    /* Provide a fast path for the usual "not a sequence" cases. */
    if (G_LIKELY (length == 0 || candidate == NULL)) {
        *handler = nullptr;
        return VTE_MATCHER_RESULT_NO_MATCH;
    }

    /* If there's no literal path, and no generic path, and the numeric
     * path isn't available, then it's not a sequence, either. */
    if (table->table == NULL ||
        table->table[_vte_table_map_literal(candidate[0])] == NULL) {
        if (table->table_string == NULL) {
            if (table->table_number == NULL ||
                    !_vte_table_is_numeric(candidate[0])){
                if (table->table_number_list == NULL ||
                    !_vte_table_is_numeric_list(candidate[0])){
                    /* No match. */
                    *handler = nullptr;
                    return VTE_MATCHER_RESULT_NO_MATCH;
                }
            }
        }
    }

    /* Check for a literal match. */
    for (i = 0, head = table; i < length && head != NULL; i++) {
        if (head->table == NULL) {
            head = NULL;
        } else {
            head = head->table[_vte_table_map_literal(candidate[i])];
        }
    }
    if (head != NULL && head->handler) {
        /* Got a literal match. */
        *consumed = candidate + i;
        *handler = head->handler;
        return VTE_MATCHER_RESULT_MATCH;
    }

    _vte_table_arginfo_head_init (&params);

    /* Check for a pattern match. */
    auto ret = _vte_table_matchi(table, candidate, length, handler, consumed,
                                     &original, &original_length, &params);

    /* If we got a match, extract the parameters. */
    if (ret == VTE_MATCHER_RESULT_MATCH && array != nullptr) {
        g_assert_nonnull(original);
        p = original;
        arginfo = _vte_table_arginfo_head_reverse (&params);
        do {
            /* All of the interesting arguments begin with '%'. */
            if (p[0] == '%') {
                /* Handle an escaped '%'. */
                if (p[1] == '%') {
                    p++;
                }
                /* Handle numeric parameters. */
                else if ((p[1] == 'd') || (p[1] == 'm')) {
                    _vte_table_extract_numbers(array, arginfo);
                    p++;
                }
                /* Handle string parameters. */
                else if (p[1] == 's') {
                    _vte_table_extract_string(array, arginfo);
                    p++;
                } else {
                    _vte_debug_print (VTE_DEBUG_PARSE, "Invalid sequence %s\n", original);
                }
            } /* else Literal. */
            arginfo = arginfo->next;
        } while (++p < original + original_length && arginfo);
    }

    /* Clean up extracted parameters. */
    _vte_table_arginfo_head_finalize (&params);

    return ret;
}

static void _vte_table_printi(struct _vte_table *table, const char *lead, int *count)
{
    unsigned int i;
    char *newlead = NULL;

    (*count)++;

    /* Result? */
    if (table->handler) {
        g_printerr("%s => result\n", _vte_debug_sequence_to_string(lead, -1));
    }

    /* Literal? */
    for (i = 1; i < VTE_TABLE_MAX_LITERAL; i++) {
        if ((table->table != NULL) && (table->table[i] != NULL)) {
            if (i < 32) {
                newlead = g_strdup_printf("%s^%c", _vte_debug_sequence_to_string(lead, -1), i + 64);
            } else {
                newlead = g_strdup_printf("%s%c", _vte_debug_sequence_to_string(lead, -1), i);
            }
            _vte_table_printi(table->table[i], newlead, count);
            g_free(newlead);
        }
    }

    /* String? */
    if (table->table_string != NULL) {
        newlead = g_strdup_printf("%s{string}", _vte_debug_sequence_to_string(lead, -1));
        _vte_table_printi(table->table_string, newlead, count);
        g_free(newlead);
    }

    /* Number(+)? */
    if (table->table_number != NULL) {
        newlead = g_strdup_printf("%s{number}", _vte_debug_sequence_to_string(lead, -1));
        _vte_table_printi(table->table_number, newlead, count);
        g_free(newlead);
    }
}

/* Dump out the contents of a tree. */
void _vte_table_print(struct _vte_table *table)
{
    int count = 0;
    _vte_table_printi(table, "", &count);
    g_printerr("%d nodes = %lu bytes.\n",
            count, (unsigned long)(count * sizeof(struct _vte_table)));
}

const struct _vte_matcher_class _vte_matcher_table = {
    (_vte_matcher_create_func)_vte_table_new,
    (_vte_matcher_add_func)_vte_table_add,
    (_vte_matcher_print_func)_vte_table_print,
    (_vte_matcher_match_func)_vte_table_match,
    (_vte_matcher_destroy_func)_vte_table_free
};
