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

#ifndef novte_iso2022_h_included
#define novte_iso2022_h_included


#include <glib.h>
#include <glib-object.h>
#include "buffer.h"
#include "matcher.hh"

G_BEGIN_DECLS

struct _vte_iso2022_state;
struct _vte_iso2022_state *_vte_iso2022_state_new(const char *native_codeset);
void _vte_iso2022_state_set_codeset(struct _vte_iso2022_state *state, const char *codeset);
const char *_vte_iso2022_state_get_codeset(struct _vte_iso2022_state *state);
gsize _vte_iso2022_process(struct _vte_iso2022_state *state,
                           const guchar *input, gsize length, GArray *gunichars);
void _vte_iso2022_state_free(struct _vte_iso2022_state *);

G_END_DECLS

#endif
