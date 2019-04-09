/*
 * Copyright (C) 2002,2003 Red Hat, Inc.
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

/*
 * This file used to contain a full iso2022 decoder which was removed for
 * version 0.40. Now it only performs input conversion from the given
 * character encoding. TODO: probably this layer could be removed completely.
 */

#include <vice.h>

#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include "debug.h"
#include "buffer.h"
#include "iso2022.h"
#include "vteconv.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <gdk/gdkkeysyms.h>

/* An invalid codepoint. */
#define INVALID_CODEPOINT 0xFFFD

struct _vte_iso2022_state {
    const gchar *codeset, *native_codeset, *utf8_codeset, *target_codeset;
    VteConv conv;
    VteByteArray *buffer;
};

struct _vte_iso2022_state *_vte_iso2022_state_new(const char *native_codeset)
{
    struct _vte_iso2022_state *state;

    state = g_slice_new0(struct _vte_iso2022_state);
    state->native_codeset = state->codeset = g_intern_string(native_codeset);
    if (native_codeset == NULL) {
        const char *codeset;
        g_get_charset(&codeset);
        state->native_codeset = state->codeset = g_intern_string(codeset);
    }
    state->utf8_codeset = g_intern_string("UTF-8");
    state->target_codeset = VTE_CONV_GUNICHAR_TYPE;
    _vte_debug_print(VTE_DEBUG_SUBSTITUTION, "Native codeset \"%s\", currently %s\n",
                                                state->native_codeset, state->codeset);
    state->conv = _vte_conv_open(state->target_codeset, state->codeset);
    state->buffer = _vte_byte_array_new();
    if (state->conv == VTE_INVALID_CONV) {
        g_warning("Unable to convert characters from %s to %s.",
                    state->codeset, state->target_codeset);
        _vte_debug_print(VTE_DEBUG_SUBSTITUTION, "Using UTF-8 instead.\n");
        state->codeset = state->utf8_codeset;
        state->conv = _vte_conv_open(state->target_codeset, state->codeset);
        if (state->conv == VTE_INVALID_CONV) {
            g_error("Unable to convert characters from %s to %s.",
                        state->codeset, state->target_codeset);
        }
    }
    return state;
}

void _vte_iso2022_state_free(struct _vte_iso2022_state *state)
{
    _vte_byte_array_free(state->buffer);
    if (state->conv != VTE_INVALID_CONV) {
        _vte_conv_close(state->conv);
    }
    g_slice_free(struct _vte_iso2022_state, state);
}

void _vte_iso2022_state_set_codeset(struct _vte_iso2022_state *state,
                   const char *codeset)
{
    VteConv conv;

    g_return_if_fail(state != NULL);
    g_return_if_fail(codeset != NULL);
    g_return_if_fail(strlen(codeset) > 0);

    _vte_debug_print(VTE_DEBUG_SUBSTITUTION, "%s\n", codeset);
    conv = _vte_conv_open(state->target_codeset, codeset);
    if (conv == VTE_INVALID_CONV) {
        g_warning("Unable to convert characters from %s to %s.",
                    codeset, state->target_codeset);
        return;
    }
    if (state->conv != VTE_INVALID_CONV) {
        _vte_conv_close(state->conv);
    }
    state->codeset = g_intern_string (codeset);
    state->conv = conv;
}

const char *_vte_iso2022_state_get_codeset(struct _vte_iso2022_state *state)
{
    return state->codeset;
}

gsize _vte_iso2022_process(struct _vte_iso2022_state *state,
                     const guchar *cdata, gsize length,
                     GArray *gunichars)
{
    glong processed = 0;
    gsize converted;
    const guchar *inbuf;
    gunichar *outbuf, *buf;
    gsize inbytes, outbytes;
    guint i, j;
    gunichar c;
    gboolean stop;

    inbuf = cdata;
    inbytes = length;
    _vte_byte_array_set_minimum_size(state->buffer, sizeof(gunichar) * length * 2);
    buf = (gunichar *)state->buffer->data;
    outbuf = buf;
    outbytes = sizeof(gunichar) * length * 2;
    do {
        converted = _vte_conv_cu(state->conv, &inbuf, &inbytes, &outbuf, &outbytes);
        stop = FALSE;
        switch (converted) {
            case ((gsize)-1):
                switch (errno) {
                    case EILSEQ:
                        /* Munge the input. */
                        inbuf++;
                        inbytes--;
                        *outbuf++ = INVALID_CODEPOINT;
                        outbytes -= sizeof(gunichar);
                        break;
                    case EINVAL:
                        /* Incomplete. Save for later. */
                        stop = TRUE;
                        break;
                    case E2BIG:
                        /* Should never happen. */
                        g_assert_not_reached();
                        break;
                    default:
                        /* Should never happen. */
                        g_assert_not_reached();
                        break;
                }
            default:
                break;
        }
    } while ((inbytes > 0) && !stop);

    /* skip blanks -- TODOegmont: why here? */
    j = gunichars->len;
    g_array_set_size(gunichars, gunichars->len + outbuf-buf);
    for (i = 0; buf + i < outbuf; i++) {
        c = buf[i];
        if (G_UNLIKELY (c == '\0')) {
            /* Skip the padding character. */
            continue;
        }
        g_array_index(gunichars, gunichar, j++) = c;
    }
    gunichars->len = j;

    /* Done. */
    processed = length - inbytes;

    _vte_debug_print(VTE_DEBUG_SUBSTITUTION, "Consuming %ld bytes.\n", (long) processed);
    return processed;
}
