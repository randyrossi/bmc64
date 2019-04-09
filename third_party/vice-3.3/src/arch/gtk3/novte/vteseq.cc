/*
 * Copyright (C) 2001-2004 Red Hat, Inc.
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

#include "vice.h"

#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef HAVE_SYS_SYSLIMITS_H
#include <sys/syslimits.h>
#endif

#include <glib.h>

#include "novte.h"
#include "vteinternal.hh"
#include "vtegtk.hh"
#include "vteutils.h"  /* for strchrnul on non-GNU systems */
#include "caps.hh"
#include "debug.h"

#define BEL "\007"
#define ST _VTE_CAP_ST

#include <algorithm>

void vte::parser::Params::print() const
{
#ifdef VTE_DEBUG
    g_printerr("(");
    auto n_params = size();
    for (unsigned int i = 0; i < n_params; i++) {
        auto value = value_at_unchecked(i);
        if (i > 0) {
            g_printerr(", ");
        }
        if (G_VALUE_HOLDS_LONG(value)) {
            auto l = g_value_get_long(value);
            g_printerr("LONG(%ld)", l);
        } else if (G_VALUE_HOLDS_STRING(value)) {
            auto const s = g_value_get_string(value);
            g_printerr("STRING(\"%s\")", s);
        } else if (G_VALUE_HOLDS_POINTER(value)) {
            auto w = (const gunichar *)g_value_get_pointer(value);
            g_printerr("WSTRING(\"%ls\")", (const wchar_t*) w);
        } else if (G_VALUE_HOLDS_BOXED(value)) {
            vte::parser::Params subparams{(GValueArray*)g_value_get_boxed(value)};
            subparams.print();
        }
    }
    g_printerr(")\n");
#endif
}

/* A couple are duplicated from vte.c, to keep them static... */

/* Check how long a string of unichars is.  Slow version. */
static gsize vte_unichar_strlen(gunichar const* c)
{
    gsize i;
    for (i = 0; c[i] != 0; i++) {} ;
    return i;
}

/* Convert a wide character string to a multibyte string */
/* Simplified from glib's g_ucs4_to_utf8() to simply allocate the maximum
 * length instead of walking the input twice.
 */
char*vte::parser::Params::ucs4_to_utf8(gunichar const* str) const
{
    auto len = vte_unichar_strlen(str);
    auto outlen = (len * VTE_UTF8_BPC) + 1;

    auto result = (char*)g_try_malloc(outlen);
    if (result == nullptr) {
        return nullptr;
    }

    auto end = str + len;
    auto p = result;
    for (auto i = str; i < end; i++) {
        p += g_unichar_to_utf8(*i, p);
    }
    *p = '\0';

    return result;
}

/* Emit a "bell" signal. */
void VteTerminalPrivate::emit_bell()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `bell'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_BELL], 0);
}


/* Emit a "deiconify-window" signal. */
void VteTerminalPrivate::emit_deiconify_window()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `deiconify-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_DEICONIFY_WINDOW], 0);
}

/* Emit a "iconify-window" signal. */
void VteTerminalPrivate::emit_iconify_window()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `iconify-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_ICONIFY_WINDOW], 0);
}

/* Emit a "raise-window" signal. */
void VteTerminalPrivate::emit_raise_window()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `raise-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_RAISE_WINDOW], 0);
}

/* Emit a "lower-window" signal. */
void VteTerminalPrivate::emit_lower_window()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `lower-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_LOWER_WINDOW], 0);
}

/* Emit a "maximize-window" signal. */
void VteTerminalPrivate::emit_maximize_window()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `maximize-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_MAXIMIZE_WINDOW], 0);
}

/* Emit a "refresh-window" signal. */
void VteTerminalPrivate::emit_refresh_window()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `refresh-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_REFRESH_WINDOW], 0);
}

/* Emit a "restore-window" signal. */
void VteTerminalPrivate::emit_restore_window()
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `restore-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_RESTORE_WINDOW], 0);
}

/* Emit a "move-window" signal.  (Pixels.) */
void VteTerminalPrivate::emit_move_window(guint x, guint y)
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `move-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_MOVE_WINDOW], 0, x, y);
}

/* Emit a "resize-window" signal.  (Grid size.) */
void VteTerminalPrivate::emit_resize_window(guint columns, guint rows)
{
    _vte_debug_print(VTE_DEBUG_SIGNALS, "Emitting `resize-window'.\n");
    g_signal_emit(m_terminal, signals[SIGNAL_RESIZE_WINDOW], 0, columns, rows);
}


/* Some common functions */

void VteTerminalPrivate::seq_checksum_rectangular_area(vte::parser::Params const& params)
{
    /*
    * DECRQCRA - request checksum of rectangular area
    * Computes a simple checksum of the characters in the rectangular
    * area. args[0] is an identifier, which the response must use.
    * args[1] is the page number; if it's 0 or default then the
    * checksum is computed over all pages; if it's greater than the
    * number of pages, then the checksum is computed only over the
    * last page. args[2]..args[5] describe the area to compute the
    * checksum from, denoting the top, left, bottom, right, resp
    * (1-based). It's required that top ≤ bottom, and left ≤ right.
    * These coordinates are interpreted according to origin mode.
    *
    * NOTE: Since this effectively allows to read the screen
    * (by using a 1x1 rectangle on each cell), we normally only
    * send a dummy reply, and only reply with the actual checksum
    * when in test mode.
    *
    * Defaults:
    *   args[0]: no default
    *   args[1]: 0
    *   args[2]: 1
    *   args[3]: no default (?)
    *   args[4]: height of current page
    *   args[5]: width of current page
    *
    * Reply: DECCKSR
    *   @args[0]: the identifier from the request
    *   DATA: the checksum as a 4-digit hex number
    *
    * References: VT525
    *             XTERM
    */

    char buf[32];
    /* gsize len; */

    int id = params.number_or_default_at(0, 0);

#ifndef VTE_DEBUG
    /* Send a dummy reply */
    /* len = */ g_snprintf(buf, sizeof(buf), "\eP%d!~%04X\e\\", id, 0);
    /* feed_child(buf, len); */ /* FIXME: removed */
#else

    /* Not in test mode? Send a dummy reply */
    if (!g_test_mode) {
        /* len = */ g_snprintf(buf, sizeof(buf), "\eP%d!~%04X\e\\", id, 0);
        /* feed_child(buf, len); */ /* FIXME: removed */
        return;
    }

    /* We only support 1 'page', so ignore args[1] */

    int top = params.number_or_default_at(2, 1);
    int left = params.number_or_default_at(3, 1); /* use 1 as default here */
    int bottom = params.number_or_default_at(4, m_row_count);
    int right = params.number_or_default_at(5, m_column_count);

    top = CLAMP(top, 1, m_row_count);
    left = CLAMP(left, 1, m_column_count);
    bottom = CLAMP(bottom, 1, m_row_count);
    right = CLAMP (right, 1, m_column_count);

    if (m_origin_mode && m_scrolling_restricted) {
        top += m_scrolling_region.start;

        bottom += m_scrolling_region.start;
        bottom = MIN(bottom, m_scrolling_region.end);
    }

    unsigned int checksum;
    if (bottom < top || right < left) {
        checksum = 0; /* empty area */
    } else {
        checksum = checksum_area(top -1 + m_screen->insert_delta,
                                    left - 1,
                                    bottom - 1 + m_screen->insert_delta,
                                    right - 1);
    }

    /* len = */ g_snprintf(buf, sizeof(buf), "\eP%d!~%04X\e\\", id, checksum);
    /* feed_child(buf, len); */ /* FIXME: removed */
#endif /* VTE_DEBUG */
}

/* In Xterm, upon printing a character in the last column the cursor doesn't
 * advance.  It's special cased that printing the following letter will first
 * wrap to the next row.
 *
 * As a rule of thumb, escape sequences that move the cursor (e.g. cursor up)
 * or immediately update the visible contents (e.g. clear in line) disable
 * this special mode, whereas escape sequences with no immediate visible
 * effect (e.g. color change) leave this special mode on.  There are
 * exceptions of course (e.g. scroll up).
 *
 * In VTE, a different technical approach is used.  The cursor is advanced to
 * the invisible column on the right, but it's set back to the visible
 * rightmost column whenever necessary (that is, before handling any of the
 * sequences that disable the special cased mode in xterm).  (Bug 731155.)
 */
void VteTerminalPrivate::ensure_cursor_is_onscreen()
{
    if (G_UNLIKELY (m_screen->cursor.col >= m_column_count)) {
        m_screen->cursor.col = m_column_count - 1;
    }
}

void VteTerminalPrivate::home_cursor()
{
    set_cursor_coords(0, 0);
}

void VteTerminalPrivate::clear_screen()
{
    auto row = m_screen->cursor.row - m_screen->insert_delta;
    auto initial = _vte_ring_next(m_screen->row_data);
    /* Add a new screen's worth of rows. */
    for (auto i = 0; i < m_row_count; i++) {
        ring_append(true);
    }
    /* Move the cursor and insertion delta to the first line in the
     * newly-cleared area and scroll if need be. */
    m_screen->insert_delta = initial;
    m_screen->cursor.row = row + m_screen->insert_delta;
    adjust_adjustments();
    /* Redraw everything. */
    invalidate_all();
    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Clear the current line. */
void VteTerminalPrivate::clear_current_line()
{
    VteRowData *rowdata;

    /* If the cursor is actually on the screen, clear data in the row
     * which corresponds to the cursor. */
    if (_vte_ring_next(m_screen->row_data) > m_screen->cursor.row) {
        /* Get the data for the row which the cursor points to. */
        rowdata = _vte_ring_index_writable(m_screen->row_data, m_screen->cursor.row);
        g_assert(rowdata != NULL);
        /* Remove it. */
        _vte_row_data_shrink (rowdata, 0);
        /* Add enough cells to the end of the line to fill out the row. */
        _vte_row_data_fill (rowdata, &m_fill_defaults, m_column_count);
        rowdata->attr.soft_wrapped = 0;
        /* Repaint this row. */
        invalidate_cells(0, m_column_count, m_screen->cursor.row, 1);
    }

    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Clear above the current line. */
void VteTerminalPrivate::clear_above_current()
{
    /* If the cursor is actually on the screen, clear data in the row
     * which corresponds to the cursor. */
    for (auto i = m_screen->insert_delta; i < m_screen->cursor.row; i++) {
        if (_vte_ring_next(m_screen->row_data) > i) {
            /* Get the data for the row we're erasing. */
            auto rowdata = _vte_ring_index_writable(m_screen->row_data, i);
            g_assert(rowdata != NULL);
            /* Remove it. */
            _vte_row_data_shrink (rowdata, 0);
            /* Add new cells until we fill the row. */
            _vte_row_data_fill (rowdata, &m_fill_defaults, m_column_count);
            rowdata->attr.soft_wrapped = 0;
            /* Repaint the row. */
            invalidate_cells(0, m_column_count, i, 1);
        }
    }
    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Scroll the text, but don't move the cursor.  Negative = up, positive = down. */
void VteTerminalPrivate::scroll_text(vte::grid::row_t scroll_amount)
{
    vte::grid::row_t start, end;
    if (m_scrolling_restricted) {
        start = m_screen->insert_delta + m_scrolling_region.start;
        end = m_screen->insert_delta + m_scrolling_region.end;
    } else {
        start = m_screen->insert_delta;
        end = start + m_row_count - 1;
    }

    while (_vte_ring_next(m_screen->row_data) <= end) {
        ring_append(false);
    }

    if (scroll_amount > 0) {
        for (auto i = 0; i < scroll_amount; i++) {
            ring_remove(end);
            ring_insert(start, true);
        }
    } else {
        for (auto i = 0; i < -scroll_amount; i++) {
            ring_remove(start);
            ring_insert(end, true);
        }
    }

    /* Update the display. */
    scroll_region(start, end - start + 1, scroll_amount);

    /* Adjust the scrollbars if necessary. */
    adjust_adjustments();

    /* We've modified the display.  Make a note of it. */
    m_text_inserted_flag = TRUE;
    m_text_deleted_flag = TRUE;
}

/* Restore cursor. */
void VteTerminalPrivate::seq_restore_cursor(vte::parser::Params const& params)
{
    restore_cursor();
}

void VteTerminalPrivate::restore_cursor()
{
    restore_cursor(m_screen);
    ensure_cursor_is_onscreen();
}

/* Save cursor. */
void VteTerminalPrivate::seq_save_cursor(vte::parser::Params const& params)
{
    save_cursor();
}

void VteTerminalPrivate::save_cursor()
{
    save_cursor(m_screen);
}

/* Switch to normal screen. */
void VteTerminalPrivate::switch_normal_screen()
{
    switch_screen(&m_normal_screen);
}

void VteTerminalPrivate::switch_screen(VteScreen *new_screen)
{
    /* if (new_screen == m_screen) return; ? */

   /* The two screens use different hyperlink pools, so carrying on the idx
    * wouldn't make sense and could lead to crashes.
    * Ideally we'd carry the target URI itself, but I'm just lazy.
    * Also, run a GC before we switch away from that screen. */
    m_hyperlink_hover_idx = _vte_ring_get_hyperlink_at_position(m_screen->row_data, -1, -1, true, NULL);
    g_assert (m_hyperlink_hover_idx == 0);
    m_hyperlink_hover_uri = NULL;
    emit_hyperlink_hover_uri_changed(NULL);  /* FIXME only emit if really changed */
    m_defaults.attr.hyperlink_idx = _vte_ring_get_hyperlink_idx(m_screen->row_data, NULL);
    g_assert (m_defaults.attr.hyperlink_idx == 0);

    /* cursor.row includes insert_delta, adjust accordingly */
    auto cr = m_screen->cursor.row - m_screen->insert_delta;
    m_screen = new_screen;
    m_screen->cursor.row = cr + m_screen->insert_delta;

    /* Make sure the ring is large enough */
    ensure_row();
}

/* Switch to alternate screen. */
void VteTerminalPrivate::switch_alternate_screen()
{
    switch_screen(&m_alternate_screen);
}

/* Switch to normal screen and restore cursor (in this order). */
void VteTerminalPrivate::switch_normal_screen_and_restore_cursor()
{
    switch_normal_screen();
    restore_cursor();
}

/* Save cursor and switch to alternate screen (in this order). */
void VteTerminalPrivate::save_cursor_and_switch_alternate_screen()
{
    save_cursor();
    switch_alternate_screen();
}

/* Set icon/window titles. */
void VteTerminalPrivate::set_title_internal(vte::parser::Params const& params,
                                            bool change_icon_title,
                                            bool change_window_title)
{
    if (change_icon_title == FALSE && change_window_title == FALSE) {
        return;
    }

    /* Get the string parameter's value. */
    char* title;
    if (!params.string_at(0, title)) {
        return;
    }

    char *p, *validated;
    const char *end;

    /* FIXMEchpe why? it's guaranteed UTF-8 already */
    /* Validate the text. */
    g_utf8_validate(title, strlen(title), &end);
    validated = g_strndup(title, end - title);

    /* No control characters allowed. */
    for (p = validated; *p != '\0'; p++) {
        if ((*p & 0x1f) == *p) {
            *p = ' ';
        }
    }

    /* Emit the signal */
    if (change_window_title) {
        g_free(m_window_title_changed);
        m_window_title_changed = g_strdup(validated);
    }

    if (change_icon_title) {
        g_free(m_icon_title_changed);
        m_icon_title_changed = g_strdup(validated);
    }

    g_free (validated);

    g_free(title);
}

/* Toggle a terminal mode. */
void VteTerminalPrivate::set_mode(vte::parser::Params const& params, bool value)
{
    auto n_params = params.size();
    if (n_params == 0) {
        return;
    }

    for (unsigned int i = 0; i < n_params; i++) {
        long setting;
        if (!params.number_at_unchecked(i, setting)) {
            continue;
        }

        switch (setting) {
            case 2:        /* keyboard action mode (?) */
                break;
            case 4:        /* insert/overtype mode */
                m_insert_mode = value;
                break;
            case 12:    /* send/receive mode (local echo) */
                m_sendrecv_mode = value;
                break;
            case 20:    /* automatic newline / normal linefeed mode */
                m_linefeed_mode = value;
                break;
            default:
                break;
        }
    }
}

void VteTerminalPrivate::reset_mouse_smooth_scroll_delta()
{
    m_mouse_smooth_scroll_delta = 0.0;
}

typedef void (VteTerminalPrivate::* decset_handler_t)();

struct decset_t {
    gint16 setting;
    /* offset in VteTerminalPrivate (> 0) or VteScreen (< 0) */
    gint16 boffset;
    gint16 ioffset;
    gint16 poffset;
    gint16 fvalue;
    gint16 tvalue;
    decset_handler_t reset, set;
};

static int decset_cmp(const void *va, const void *vb)
{
    const struct decset_t *a = (const struct decset_t *)va;
    const struct decset_t *b = (const struct decset_t *)vb;

    return a->setting < b->setting ? -1 : a->setting > b->setting;
}

/* Manipulate certain terminal attributes. */
void VteTerminalPrivate::decset(vte::parser::Params const& params,
                                bool restore, bool save, bool set)
{

    auto n_params = params.size();
    for (unsigned int i = 0; i < n_params; i++) {
        long setting;

        if (!params.number_at(i, setting)) {
            continue;
        }

        decset(setting, restore, save, set);
    }
}

void VteTerminalPrivate::decset(long setting,
                                bool restore,
                                bool save,
                                bool set)
{
    static const struct decset_t settings[] = {
#define PRIV_OFFSET(member) (G_STRUCT_OFFSET(VteTerminalPrivate, member))
#define SCREEN_OFFSET(member) (-G_STRUCT_OFFSET(VteScreen, member))
        /* 1: Application/normal cursor keys. */
        {1, 0, PRIV_OFFSET(m_cursor_mode), 0,
         VTE_KEYMODE_NORMAL,
         VTE_KEYMODE_APPLICATION,
         nullptr, nullptr,},
        /* 2: disallowed, we don't do VT52. */
        {2, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 3: DECCOLM set/reset to and from 132/80 columns */
        {3, 0, 0, 0,
            FALSE,
            TRUE,
            nullptr, nullptr,},
        /* 5: Reverse video. */
        {5, PRIV_OFFSET(m_reverse_mode), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
        /* 6: Origin mode: when enabled, cursor positioning is
         * relative to the scrolling region. */
        {6, PRIV_OFFSET(m_origin_mode), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
        /* 7: Wraparound mode. */
        {7, PRIV_OFFSET(m_autowrap), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
        /* 8: disallowed, keyboard repeat is set by user. */
        {8, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 9: Send-coords-on-click. */
        {9, 0, PRIV_OFFSET(m_mouse_tracking_mode), 0,
         0,
         MOUSE_TRACKING_SEND_XY_ON_CLICK,
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,},
        /* 12: disallowed, cursor blinks is set by user. */
        {12, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 18: print form feed. */
        /* 19: set print extent to full screen. */
        /* 25: Cursor visible. */
        {25, PRIV_OFFSET(m_cursor_visible), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
        /* 30/rxvt: disallowed, scrollbar visibility is set by user. */
        {30, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 35/rxvt: disallowed, fonts set by user. */
        {35, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 38: enter Tektronix mode. */
        /* 40: Enable DECCOLM mode. */
        {40, PRIV_OFFSET(m_deccolm_mode), 0, 0,
            FALSE,
            TRUE,
            nullptr, nullptr,},
        /* 41: more(1) fix. */
        /* 42: Enable NLS replacements. */
        /* 44: Margin bell. */
        /* 47: Alternate screen. */
        {47, 0, 0, 0,
            0,
            0,
            &VteTerminalPrivate::switch_normal_screen,
            &VteTerminalPrivate::switch_alternate_screen,},
        /* 66: Keypad mode. */
        {66, PRIV_OFFSET(m_keypad_mode), 0, 0,
         VTE_KEYMODE_NORMAL,
         VTE_KEYMODE_APPLICATION,
         nullptr, nullptr,},
        /* 67: disallowed, backspace key policy is set by user. */
        {67, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 1000: Send-coords-on-button. */
        {1000, 0, PRIV_OFFSET(m_mouse_tracking_mode), 0,
         0,
         MOUSE_TRACKING_SEND_XY_ON_BUTTON,
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,},
        /* 1001: Hilite tracking. */
        {1001, 0, PRIV_OFFSET(m_mouse_tracking_mode), 0,
         (0),
         (MOUSE_TRACKING_HILITE_TRACKING),
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,},
        /* 1002: Cell motion tracking. */
        {1002, 0, PRIV_OFFSET(m_mouse_tracking_mode), 0,
         (0),
         (MOUSE_TRACKING_CELL_MOTION_TRACKING),
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,},
        /* 1003: All motion tracking. */
        {1003, 0, PRIV_OFFSET(m_mouse_tracking_mode), 0,
         (0),
         (MOUSE_TRACKING_ALL_MOTION_TRACKING),
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,
                 &VteTerminalPrivate::reset_mouse_smooth_scroll_delta,},
        /* 1004: Focus tracking. */
        {1004, PRIV_OFFSET(m_focus_tracking_mode), 0, 0,
         FALSE,
         TRUE,
         nullptr,
         &VteTerminalPrivate::feed_focus_event_initial,},
        /* 1006: Extended mouse coordinates. */
        {1006, PRIV_OFFSET(m_mouse_xterm_extension), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
        /* 1007: Alternate screen scroll. */
        {1007, PRIV_OFFSET(m_alternate_screen_scroll), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
        /* 1010/rxvt: disallowed, scroll-on-output is set by user. */
        {1010, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 1011/rxvt: disallowed, scroll-on-keypress is set by user. */
        {1011, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 1015/urxvt: Extended mouse coordinates. */
        {1015, PRIV_OFFSET(m_mouse_urxvt_extension), 0, 0,
         FALSE,
         TRUE,
        nullptr, nullptr,},
        /* 1035: disallowed, don't know what to do with it. */
        {1035, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 1036: Meta-sends-escape. */
        {1036, PRIV_OFFSET(m_meta_sends_escape), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
        /* 1037: disallowed, delete key policy is set by user. */
        {1037, 0, 0, 0, 0, 0, nullptr, nullptr,},
        /* 1047: Use alternate screen buffer. */
        {1047, 0, 0, 0,
            0,
            0,
            &VteTerminalPrivate::switch_normal_screen,
            &VteTerminalPrivate::switch_alternate_screen,},
        /* 1048: Save/restore cursor position. */
        {1048, 0, 0, 0,
         0,
         0,
        &VteTerminalPrivate::restore_cursor,
        &VteTerminalPrivate::save_cursor,},
        /* 1049: Use alternate screen buffer, saving the cursor
         * position. */
        {1049, 0, 0, 0,
            0,
            0,
            &VteTerminalPrivate::switch_normal_screen_and_restore_cursor,
            &VteTerminalPrivate::save_cursor_and_switch_alternate_screen,},
        /* 2004: Bracketed paste mode. */
        {2004, PRIV_OFFSET(m_bracketed_paste_mode), 0, 0,
         FALSE,
         TRUE,
         nullptr, nullptr,},
#undef PRIV_OFFSET
#undef SCREEN_OFFSET
    };
    struct decset_t key;
    struct decset_t *found;

    /* Handle the setting. */
    key.setting = setting;
    found = (struct decset_t *)bsearch(&key, settings, G_N_ELEMENTS(settings), sizeof(settings[0]), decset_cmp);
    if (!found) {
    _vte_debug_print (VTE_DEBUG_MISC,
                "DECSET/DECRESET mode %ld not recognized, ignoring.\n",
                setting);
        return;
    }

    key = *found;
    do {
        gboolean *bvalue = NULL;
        gint *ivalue = NULL;
        gpointer *pvalue = NULL, pfvalue = NULL, ptvalue = NULL;
        gpointer p;

        /* Handle settings we want to ignore. */
        if ((key.fvalue == key.tvalue) &&
            (!key.set) &&
            (!key.reset)) {
            break;
        }

#define STRUCT_MEMBER_P(type,total_offset) \
                (type) (total_offset >= 0 ? G_STRUCT_MEMBER_P(this, total_offset) : G_STRUCT_MEMBER_P(m_screen, -total_offset))

        if (key.boffset) {
                bvalue = STRUCT_MEMBER_P(gboolean*, key.boffset);
        } else if (key.ioffset) {
                ivalue = STRUCT_MEMBER_P(int*, key.ioffset);
        } else if (key.poffset) {
                pvalue = STRUCT_MEMBER_P(gpointer*, key.poffset);
                pfvalue = STRUCT_MEMBER_P(gpointer, key.fvalue);
                ptvalue = STRUCT_MEMBER_P(gpointer, key.tvalue);
        }
#undef STRUCT_MEMBER_P

        /* Read the old setting. */
        if (restore) {
            p = g_hash_table_lookup(m_dec_saved, GINT_TO_POINTER(setting));
            set = (p != NULL);
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Setting %ld was %s.\n",
                                setting, set ? "set" : "unset");
        }
        /* Save the current setting. */
        if (save) {
            if (bvalue) {
                set = *(bvalue) != FALSE;
            } else if (ivalue) {
                set = *(ivalue) == (int)key.tvalue;
            } else if (pvalue) {
                set = *(pvalue) == ptvalue;
            }
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Setting %ld is %s, saving.\n",
                                setting, set ? "set" : "unset");
            g_hash_table_insert(m_dec_saved,
                                GINT_TO_POINTER(setting),
                                GINT_TO_POINTER(set));
        }
        /* Change the current setting to match the new/saved value. */
        if (!save) {
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Setting %ld to %s.\n",
                                setting, set ? "set" : "unset");
            if (key.set && set) {
                (this->*key.set)();
            }
            if (bvalue) {
                *(bvalue) = set;
            } else if (ivalue) {
                *(ivalue) = set ? (int)key.tvalue : (int)key.fvalue;
            } else if (pvalue) {
                *(pvalue) = set ? ptvalue : pfvalue;
            }
            if (key.reset && !set) {
                (this->*key.reset)();
            }
        }
    } while (0);

    /* Do whatever's necessary when the setting changes. */
    switch (setting) {
        case 1:
            _vte_debug_print(VTE_DEBUG_KEYBOARD, set ?
                    "Entering application cursor mode.\n" :
                    "Leaving application cursor mode.\n");
            break;
        case 3:
            /* 3: DECCOLM set/reset to 132/80 columns mode, clear screen and cursor home */
            if (m_deccolm_mode) {
                emit_resize_window(set ? 132 : 80, m_row_count);
                clear_screen();
                home_cursor();
            }
            break;
        case 5:
            /* Repaint everything in reverse mode. */
            invalidate_all();
            break;
        case 6:
            /* Reposition the cursor in its new home position. */
            home_cursor();
            break;
        case 47:
        case 1047:
        case 1049:
            /* Clear the alternate screen if we're switching to it */
            if (set) {
                clear_screen();
            }
            /* Reset scrollbars and repaint everything. */
            gtk_adjustment_set_value(m_vadjustment, m_screen->scroll_delta);
            set_scrollback_lines(m_scrollback_lines);
            queue_contents_changed();
            invalidate_all();
            break;
        case 9:
        case 1000:
        case 1001:
        case 1002:
        case 1003:
            /* Mouse pointer might change. */
            apply_mouse_cursor();
            break;
        case 66:
            _vte_debug_print(VTE_DEBUG_KEYBOARD, set ?
                    "Entering application keypad mode.\n" :
                    "Leaving application keypad mode.\n");
            break;
        default:
            break;
    }
}

/* THE HANDLERS */

/* Do nothing. */
void VteTerminalPrivate::seq_nop(vte::parser::Params const& params)
{
}

void VteTerminalPrivate::set_character_replacements(unsigned slot,
                                               VteCharacterReplacement replacement)
{
    g_assert(slot < G_N_ELEMENTS(m_character_replacements));
    m_character_replacements[slot] = replacement;
}

/* G0 character set is a pass-thru (no mapping). */
void VteTerminalPrivate::seq_designate_g0_plain(vte::parser::Params const& params)
{
    set_character_replacements(0, VTE_CHARACTER_REPLACEMENT_NONE);
}

/* G0 character set is DEC Special Character and Line Drawing Set. */
void VteTerminalPrivate::seq_designate_g0_line_drawing(vte::parser::Params const& params)
{
    set_character_replacements(0, VTE_CHARACTER_REPLACEMENT_LINE_DRAWING);
}

/* G0 character set is British (# is converted to £). */
void VteTerminalPrivate::seq_designate_g0_british(vte::parser::Params const& params)
{
    set_character_replacements(0, VTE_CHARACTER_REPLACEMENT_BRITISH);
}

/* G1 character set is a pass-thru (no mapping). */
void VteTerminalPrivate::seq_designate_g1_plain(vte::parser::Params const& params)
{
    set_character_replacements(1, VTE_CHARACTER_REPLACEMENT_NONE);
}

/* G1 character set is DEC Special Character and Line Drawing Set. */
void VteTerminalPrivate::seq_designate_g1_line_drawing(vte::parser::Params const& params)
{
    set_character_replacements(1, VTE_CHARACTER_REPLACEMENT_LINE_DRAWING);
}

/* G1 character set is British (# is converted to £). */
void VteTerminalPrivate::seq_designate_g1_british(vte::parser::Params const& params)
{
    set_character_replacements(1, VTE_CHARACTER_REPLACEMENT_BRITISH);
}

void VteTerminalPrivate::set_character_replacement(unsigned slot)
{
    g_assert(slot < G_N_ELEMENTS(m_character_replacements));
    m_character_replacement = &m_character_replacements[slot];
}

/* SI (shift in): switch to G0 character set. */
void VteTerminalPrivate::seq_shift_in(vte::parser::Params const& params)
{
    set_character_replacement(0);
}

/* SO (shift out): switch to G1 character set. */
void VteTerminalPrivate::seq_shift_out(vte::parser::Params const& params)
{
    set_character_replacement(1);
}

/* Beep. */
void VteTerminalPrivate::seq_bell(vte::parser::Params const& params)
{
    m_bell_pending = true;
}

/* Backtab. */
void VteTerminalPrivate::seq_cursor_back_tab(vte::parser::Params const& params)
{
    /* Calculate which column is the previous tab stop. */
    auto newcol = m_screen->cursor.col;

    if (m_tabstops) {
        /* Find the next tabstop. */
        while (newcol > 0) {
            newcol--;
            if (get_tabstop(newcol % m_column_count)) {
                break;
            }
        }
    }

    /* Warp the cursor. */
    _vte_debug_print(VTE_DEBUG_PARSE,
                        "Moving cursor to column %ld.\n", (long)newcol);
    set_cursor_column(newcol);
}

/* Clear from the cursor position (inclusive!) to the beginning of the line. */
void VteTerminalPrivate::clear_to_bol()
{
    ensure_cursor_is_onscreen();

    /* Get the data for the row which the cursor points to. */
    auto rowdata = ensure_row();
    /* Clean up Tab/CJK fragments. */
    cleanup_fragments(0, m_screen->cursor.col + 1);
    /* Clear the data up to the current column with the default
     * attributes.  If there is no such character cell, we need
     * to add one. */
    vte::grid::column_t i;
    for (i = 0; i <= m_screen->cursor.col; i++) {
        if (i < (glong) _vte_row_data_length (rowdata)) {
            /* Muck with the cell in this location. */
            auto pcell = _vte_row_data_get_writable(rowdata, i);
            *pcell = m_color_defaults;
        } else {
            /* Add new cells until we have one here. */
            _vte_row_data_append (rowdata, &m_color_defaults);
        }
    }
    /* Repaint this row. */
    invalidate_cells(0, m_screen->cursor.col+1,
                         m_screen->cursor.row, 1);

    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Clear to the right of the cursor and below the current line. */
void VteTerminalPrivate::clear_below_current()
{
    ensure_cursor_is_onscreen();

    /* If the cursor is actually on the screen, clear the rest of the
     * row the cursor is on and all of the rows below the cursor. */
    VteRowData *rowdata;
    auto i = m_screen->cursor.row;
    if (i < _vte_ring_next(m_screen->row_data)) {
        /* Get the data for the row we're clipping. */
        rowdata = _vte_ring_index_writable(m_screen->row_data, i);
        /* Clean up Tab/CJK fragments. */
        if ((glong) _vte_row_data_length(rowdata) > m_screen->cursor.col) {
            cleanup_fragments(m_screen->cursor.col, _vte_row_data_length(rowdata)); 
        }
        /* Clear everything to the right of the cursor. */
        if (rowdata) {
            _vte_row_data_shrink(rowdata, m_screen->cursor.col);
        }
    }
    /* Now for the rest of the lines. */
    for (i = m_screen->cursor.row + 1;
        i < _vte_ring_next(m_screen->row_data);
        i++) {
        /* Get the data for the row we're removing. */
        rowdata = _vte_ring_index_writable(m_screen->row_data, i);
        /* Remove it. */
        if (rowdata) {
            _vte_row_data_shrink (rowdata, 0);
        }
    }
    /* Now fill the cleared areas. */
    bool const not_default_bg = (m_fill_defaults.attr.back() != VTE_DEFAULT_BG);

    for (i = m_screen->cursor.row;
        i < m_screen->insert_delta + m_row_count;
        i++) {
        /* Retrieve the row's data, creating it if necessary. */
        if (_vte_ring_contains(m_screen->row_data, i)) {
            rowdata = _vte_ring_index_writable (m_screen->row_data, i);
            g_assert(rowdata != NULL);
        } else {
            rowdata = ring_append(false);
        }
        /* Pad out the row. */
        if (not_default_bg) {
            _vte_row_data_fill(rowdata, &m_fill_defaults, m_column_count);
        }
        rowdata->attr.soft_wrapped = 0;
        /* Repaint this row. */
        invalidate_cells(0, m_column_count, i, 1);
    }

    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Clear from the cursor position to the end of the line. */
void VteTerminalPrivate::clear_to_eol()
{
    /* If we were to strictly emulate xterm, we'd ensure the cursor is onscreen.
     * But due to https://bugzilla.gnome.org/show_bug.cgi?id=740789 we intentionally
     * deviate and do instead what konsole does. This way emitting a \e[K doesn't
     * influence the text flow, and serves as a perfect workaround against a new line
     * getting painted with the active background color (except for a possible flicker).
     */
    /* ensure_cursor_is_onscreen(); */

    /* Get the data for the row which the cursor points to. */
    auto rowdata = ensure_row();
    g_assert(rowdata != NULL);
    if ((glong) _vte_row_data_length(rowdata) > m_screen->cursor.col) {
        /* Clean up Tab/CJK fragments. */
        cleanup_fragments(m_screen->cursor.col, _vte_row_data_length(rowdata));
        /* Remove the data at the end of the array until the current column
         * is the end of the array. */
        _vte_row_data_shrink(rowdata, m_screen->cursor.col);
        /* We've modified the display.  Make a note of it. */
        m_text_deleted_flag = TRUE;
    }
    bool const not_default_bg = (m_fill_defaults.attr.back() != VTE_DEFAULT_BG);

    if (not_default_bg) {
        /* Add enough cells to fill out the row. */
        _vte_row_data_fill(rowdata, &m_fill_defaults, m_column_count);
    }
    rowdata->attr.soft_wrapped = 0;
    /* Repaint this row. */
    invalidate_cells(m_screen->cursor.col, m_column_count - m_screen->cursor.col,
                         m_screen->cursor.row, 1);
}

/* Move the cursor to the given column (horizontal position), 1-based. */
void VteTerminalPrivate::seq_cursor_character_absolute(vte::parser::Params const& params)
{
    auto value = params.number_or_default_at(0, 1) - 1;
    set_cursor_column(value);
}

/*
 * VteTerminalPrivate::set_cursor_column:
 * @col: the column. 0-based from 0 to m_column_count - 1
 *
 * Sets the cursor column to @col, clamped to the range 0..m_column_count-1.
 */
void VteTerminalPrivate::set_cursor_column(vte::grid::column_t col)
{
    m_screen->cursor.col = CLAMP(col, 0, m_column_count - 1);
}

/*
 * VteTerminalPrivate::set_cursor_row:
 * @row: the row. 0-based and relative to the scrolling region
 *
 * Sets the cursor row to @row. @row is relative to the scrolling region
 * (0 if restricted scrolling is off).
 */
void VteTerminalPrivate::set_cursor_row(vte::grid::row_t row)
{
    vte::grid::row_t start_row, end_row;
    if (m_origin_mode && m_scrolling_restricted) {
        start_row = m_scrolling_region.start;
        end_row = m_scrolling_region.end;
    } else {
        start_row = 0;
        end_row = m_row_count - 1;
    }
    row += start_row;
    row = CLAMP(row, start_row, end_row);

    m_screen->cursor.row = row + m_screen->insert_delta;
}

/*
 * VteTerminalPrivate::get_cursor_row:
 *
 * Returns: the relative cursor row, 0-based and relative to the scrolling region
 * if set (regardless of origin mode).
 */
vte::grid::row_t VteTerminalPrivate::get_cursor_row() const
{
    auto row = m_screen->cursor.row - m_screen->insert_delta;
    /* Note that we do NOT check m_origin_mode here! */
    if (m_scrolling_restricted) {
        row -= m_scrolling_region.start;
    }
    return row;
}

vte::grid::column_t VteTerminalPrivate::get_cursor_column() const
{
    return m_screen->cursor.col;
}

/*
 * VteTerminalPrivate::set_cursor_coords:
 * @row: the row. 0-based and relative to the scrolling region
 * @col: the column. 0-based from 0 to m_column_count - 1
 *
 * Sets the cursor row to @row. @row is relative to the scrolling region
 * (0 if restricted scrolling is off).
 *
 * Sets the cursor column to @col, clamped to the range 0..m_column_count-1.
 */
void VteTerminalPrivate::set_cursor_coords(vte::grid::row_t row,
                                            vte::grid::column_t column)
{
    set_cursor_column(column);
    set_cursor_row(row);
}

/* Move the cursor to the given position, 1-based. */
void VteTerminalPrivate::seq_cursor_position(vte::parser::Params const& params)
{
    /* The first is the row, the second is the column. */
    auto rowval = params.number_or_default_at(0, 1) - 1;
    auto colval = params.number_or_default_at(1, 1) - 1;
    set_cursor_coords(rowval, colval);
}

/* Carriage return. */
void VteTerminalPrivate::seq_carriage_return(vte::parser::Params const& params)
{
    set_cursor_column(0);
}

void VteTerminalPrivate::reset_scrolling_region()
{
    m_scrolling_restricted = FALSE;
    home_cursor();
}

/* Restrict scrolling and updates to a subset of the visible lines. */
void VteTerminalPrivate::seq_set_scrolling_region(vte::parser::Params const& params)
{
    /* We require two parameters.  Anything less is a reset. */
    if (params.size() < 2) {
        return reset_scrolling_region();
    }

    auto start = params.number_or_default_at_unchecked(0) - 1;
    auto end = params.number_or_default_at_unchecked(1) - 1;
    set_scrolling_region(start, end);
}

void VteTerminalPrivate::set_scrolling_region(vte::grid::row_t start /* relative */,
                                                vte::grid::row_t end /* relative */)
{
    /* A (1-based) value of 0 means default. */
    if (start == -1) {
        start = 0;
    }
    if (end == -1) {
        end = m_row_count - 1;
    }
    /* Bail out on garbage, require at least 2 rows, as per xterm. */
    if (start < 0 || start >= m_row_count - 1 || end < start + 1) {
        return;
    }
    if (end >= m_row_count) {
        end = m_row_count - 1;
    }

    /* Set the right values. */
    m_scrolling_region.start = start;
    m_scrolling_region.end = end;
    m_scrolling_restricted = TRUE;
    if (m_scrolling_region.start == 0 &&
        m_scrolling_region.end == m_row_count - 1) {
        /* Special case -- run wild, run free. */
        m_scrolling_restricted = FALSE;
    } else {
        /* Maybe extend the ring -- bug 710483 */
        while (_vte_ring_next(m_screen->row_data) < m_screen->insert_delta + m_row_count) {
                _vte_ring_insert(m_screen->row_data, _vte_ring_next(m_screen->row_data));
        }
    }
    home_cursor();
}

/* Move the cursor to the beginning of the Nth next line, no scrolling. */
void VteTerminalPrivate::seq_cursor_next_line(vte::parser::Params const& params)
{
    set_cursor_column(0);
    seq_cursor_down(params);
}

/* Move the cursor to the beginning of the Nth previous line, no scrolling. */
void VteTerminalPrivate::seq_cursor_preceding_line(vte::parser::Params const& params)
{
    set_cursor_column(0);
    seq_cursor_up(params);
}

/* Move the cursor to the given row (vertical position), 1-based. */
void VteTerminalPrivate::seq_line_position_absolute(vte::parser::Params const& params)
{
    /* FIXMEchpe shouldn't we ensure_cursor_is_onscreen AFTER setting the new cursor row? */
    ensure_cursor_is_onscreen();

    auto val = params.number_or_default_at(0, 1) - 1;
    set_cursor_row(val);
}

/* Delete a character at the current cursor position. */
void VteTerminalPrivate::delete_character()
{
    VteRowData *rowdata;
    long col;

    ensure_cursor_is_onscreen();

    if (_vte_ring_next(m_screen->row_data) > m_screen->cursor.row) {
        long len;
        /* Get the data for the row which the cursor points to. */
        rowdata = _vte_ring_index_writable(m_screen->row_data, m_screen->cursor.row);
        g_assert(rowdata != NULL);
        col = m_screen->cursor.col;
        len = _vte_row_data_length (rowdata);
        /* Remove the column. */
        if (col < len) {
            /* Clean up Tab/CJK fragments. */
            cleanup_fragments(col, col + 1);
            _vte_row_data_remove (rowdata, col);
            bool const not_default_bg = (m_fill_defaults.attr.back() != VTE_DEFAULT_BG);

            if (not_default_bg) {
                _vte_row_data_fill(rowdata, &m_fill_defaults, m_column_count);
                len = m_column_count;
            }
            rowdata->attr.soft_wrapped = 0;
            /* Repaint this row. */
            invalidate_cells(col, len - col, m_screen->cursor.row, 1);
        }
    }

    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Delete N characters at the current cursor position. */
void VteTerminalPrivate::seq_delete_characters(vte::parser::Params const& params)
{
    auto val = std::max(std::min(params.number_or_default_at(0, 1),
                                    m_column_count - m_screen->cursor.col), long(1));
    for (auto i = 0; i < val; i++) {
        delete_character();
    }
}

/* Cursor down N lines, no scrolling. */
void VteTerminalPrivate::seq_cursor_down(vte::parser::Params const& params)
{
    auto val = params.number_or_default_at(0, 1);
    move_cursor_down(val);
}

void VteTerminalPrivate::move_cursor_down(vte::grid::row_t rows)
{
    rows = CLAMP(rows, 1, m_row_count);

    /* FIXMEchpe why not do this afterwards? */
    ensure_cursor_is_onscreen();

    vte::grid::row_t end;
    /* FIXMEchpe why not check m_origin_mode here? */
    if (m_scrolling_restricted) {
        end = m_screen->insert_delta + m_scrolling_region.end;
    } else {
        end = m_screen->insert_delta + m_row_count - 1;
    }
    m_screen->cursor.row = MIN(m_screen->cursor.row + rows, end);
}

/* Erase characters starting at the cursor position (overwriting N with
 * spaces, but not moving the cursor). */
void VteTerminalPrivate::seq_erase_characters(vte::parser::Params const& params)
{
    /* If we got a parameter, use it. */
    auto count = std::min(params.number_or_default_at(0, 1), long(65535));
    erase_characters(count);
}

void VteTerminalPrivate::erase_characters(long count)
{
    VteCell *cell;
    long col, i;

    ensure_cursor_is_onscreen();

    /* Clear out the given number of characters. */
    auto rowdata = ensure_row();
    if (_vte_ring_next(m_screen->row_data) > m_screen->cursor.row) {
        g_assert(rowdata != NULL);
        /* Clean up Tab/CJK fragments. */
        cleanup_fragments(m_screen->cursor.col, m_screen->cursor.col + count);
        /* Write over the characters.  (If there aren't enough, we'll
         * need to create them.) */
        for (i = 0; i < count; i++) {
            col = m_screen->cursor.col + i;
            if (col >= 0) {
                if (col < (glong) _vte_row_data_length (rowdata)) {
                    /* Replace this cell with the current
                     * defaults. */
                    cell = _vte_row_data_get_writable (rowdata, col);
                    *cell = m_color_defaults;
                } else {
                    /* Add new cells until we have one here. */
                    _vte_row_data_fill (rowdata, &m_color_defaults, col + 1);
                }
            }
        }
        /* Repaint this row. */
        invalidate_cells(m_screen->cursor.col, count, m_screen->cursor.row, 1);
    }

    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Form-feed / next-page. */
void VteTerminalPrivate::seq_form_feed(vte::parser::Params const& params)
{
    line_feed();
}

/* Insert a blank character. */
void VteTerminalPrivate::insert_blank_character()
{
    ensure_cursor_is_onscreen();

    auto save = m_screen->cursor;
    insert_char(' ', true, true);
    m_screen->cursor = save;
}

/* Insert N blank characters. */
/* TODOegmont: Insert them in a single run, so that we call cleanup_fragments only once. */
void VteTerminalPrivate::seq_insert_blank_characters(vte::parser::Params const& params)
{
    auto val = std::max(std::min(params.number_or_default_at(0, 1),
                                    m_column_count - m_screen->cursor.col), long(1));
    for (auto i = 0; i < val; i++) {
        insert_blank_character();
    }
}

/* REP: Repeat the last graphic character n times. */
void VteTerminalPrivate::seq_repeat(vte::parser::Params const& params)
{
    auto val = std::min(params.number_or_default_at(0, 1), long(65535)); 
    /* FIXMEchpe maybe limit more, to m_column_count - m_screen->cursor.col ? */
    for (auto i = 0; i < val; i++) {
        /* FIXMEchpe can't we move that check out of the loop? */
        if (m_last_graphic_character == 0) {
            break;
        }
        insert_char(m_last_graphic_character, false, true);
    }
}

/* Cursor down 1 line, with scrolling. */
void VteTerminalPrivate::seq_index(vte::parser::Params const& params)
{
    line_feed();
}

/* Cursor left. */
void VteTerminalPrivate::seq_backspace(vte::parser::Params const& params)
{
    ensure_cursor_is_onscreen();

    if (m_screen->cursor.col > 0) {
        /* There's room to move left, so do so. */
        m_screen->cursor.col--;
    }
}

/* Cursor left N columns. */
void VteTerminalPrivate::seq_cursor_backward(vte::parser::Params const& params)
{
    auto val = params.number_or_default_at(0, 1);
    move_cursor_backward(val);
}

void VteTerminalPrivate::move_cursor_backward(vte::grid::column_t columns)
{
    ensure_cursor_is_onscreen();

    auto col = get_cursor_column();
    columns = CLAMP(columns, 1, col);
    set_cursor_column(col - columns);
}

/* Cursor right N columns. */
void VteTerminalPrivate::seq_cursor_forward(vte::parser::Params const& params)
{
    auto val = params.number_or_default_at(0, 1);
    move_cursor_forward(val);
}

void VteTerminalPrivate::move_cursor_forward(vte::grid::column_t columns)
{
    columns = CLAMP(columns, 1, m_column_count);

    ensure_cursor_is_onscreen();

    /* The cursor can be further to the right, don't move in that case. */
    auto col = get_cursor_column();
    if (col < m_column_count) {
        /* There's room to move right. */
        set_cursor_column(col + columns);
    }
}

/* Move the cursor to the beginning of the next line, scrolling if necessary. */
void VteTerminalPrivate::seq_next_line(vte::parser::Params const& params)
{
    set_cursor_column(0);
    cursor_down(true);
}

/* Scroll the text down N lines, but don't move the cursor. */
void VteTerminalPrivate::seq_scroll_down(vte::parser::Params const& params)
{
    /* No ensure_cursor_is_onscreen() here as per xterm */
    auto val = std::max(params.number_or_default_at(0, 1), long(1));
    scroll_text(val);
}

/* Internal helper for changing color in the palette */
void VteTerminalPrivate::change_color(vte::parser::Params const& params,
                                        const char *terminator)
{
    char **pairs;
    {
        char* str;
        if (!params.string_at(0, str)) {
            return;
        }

        pairs = g_strsplit (str, ";", 0);
        g_free(str);
    }

    if (!pairs) {
        return;
    }

    vte::color::rgb color;
    guint idx, i;

    for (i = 0; pairs[i] && pairs[i + 1]; i += 2) {
        idx = strtoul (pairs[i], (char **) NULL, 10);

        if (idx >= VTE_DEFAULT_FG && idx != 256) {
            continue;
        }

        if (color.parse(pairs[i + 1])) {
            set_color(idx == 256 ? VTE_BOLD_FG : idx, VTE_COLOR_SOURCE_ESCAPE, color);
        } else if (strcmp (pairs[i + 1], "?") == 0) {
            gchar buf[128];
            auto c = get_color(idx == 256 ? VTE_BOLD_FG : idx);
            if (c == NULL && idx == 256) {
                c = get_color(VTE_DEFAULT_FG);
            }
            g_assert(c != NULL);
            g_snprintf (buf, sizeof (buf),
                            _VTE_CAP_OSC "4;%u;rgb:%04x/%04x/%04x%s",
                            idx, c->red, c->green, c->blue, terminator);
            /*feed_child(buf, -1);*/ /* FIXME: removed */
        }
    }

    g_strfreev (pairs);

    /* emit the refresh as the palette has changed and previous
     * renders need to be updated. */
    emit_refresh_window();
}

/* Change color in the palette, BEL terminated */
void VteTerminalPrivate::seq_change_color_bel(vte::parser::Params const& params)
{
    change_color(params, BEL);
}

/* Change color in the palette, ST terminated */
void VteTerminalPrivate::seq_change_color_st(vte::parser::Params const& params)
{
    change_color(params, ST);
}

/* Reset color in the palette */
void VteTerminalPrivate::seq_reset_color(vte::parser::Params const& params)
{
    auto n_params = params.size();
    if (n_params) {
        for (unsigned int i = 0; i < n_params; i++) {
            long value;
            if (!params.number_at_unchecked(i, value)) {
                    continue;
            }

            if ((value < 0 || value >= VTE_DEFAULT_FG) && value != 256) {
                    continue;
            }

            reset_color(value == 256 ? VTE_BOLD_FG : value, VTE_COLOR_SOURCE_ESCAPE);
        }
    } else {
        for (unsigned int idx = 0; idx < VTE_DEFAULT_FG; idx++) {
            reset_color(idx, VTE_COLOR_SOURCE_ESCAPE);
        }
    }
}

/* Scroll the text up N lines, but don't move the cursor. */
void VteTerminalPrivate::seq_scroll_up(vte::parser::Params const& params)
{
    /* No ensure_cursor_is_onscreen() here as per xterm */

    auto val = std::max(params.number_or_default_at(0, 1), long(1));
    scroll_text(-val);
}

/* Cursor down 1 line, with scrolling. */
void VteTerminalPrivate::seq_line_feed(vte::parser::Params const& params)
{
    line_feed();
}

void VteTerminalPrivate::line_feed()
{
    ensure_cursor_is_onscreen();
    cursor_down(true);
}

/* Cursor up 1 line, with scrolling. */
void VteTerminalPrivate::seq_reverse_index(vte::parser::Params const& params)
{
    ensure_cursor_is_onscreen();

    vte::grid::row_t start, end;
    if (m_scrolling_restricted) {
        start = m_scrolling_region.start + m_screen->insert_delta;
        end = m_scrolling_region.end + m_screen->insert_delta;
    } else {
        start = m_screen->insert_delta;
        end = start + m_row_count - 1;
    }

    if (m_screen->cursor.row == start) {
        /* If we're at the top of the scrolling region, add a
            * line at the top to scroll the bottom off. */
        ring_remove(end);
        ring_insert(start, true);
        /* Update the display. */
        scroll_region(start, end - start + 1, 1);
        invalidate_cells(0, m_column_count, start, 2);
    } else {
        /* Otherwise, just move the cursor up. */
        m_screen->cursor.row--;
    }
    /* Adjust the scrollbars if necessary. */
    adjust_adjustments();
    /* We modified the display, so make a note of it. */
    m_text_modified_flag = TRUE;
}

/* Set tab stop in the current column. */
void VteTerminalPrivate::seq_tab_set(vte::parser::Params const& params)
{
    if (m_tabstops == NULL) {
        m_tabstops = g_hash_table_new(NULL, NULL);
    }
    set_tabstop(m_screen->cursor.col);
}

/* Tab. */
void VteTerminalPrivate::seq_tab(vte::parser::Params const& params)
{
    move_cursor_tab();
}

void
VteTerminalPrivate::move_cursor_tab()
{
    long old_len;
    vte::grid::column_t newcol, col;

    /* Calculate which column is the next tab stop. */
    newcol = col = m_screen->cursor.col;

    g_assert (col >= 0);

    if (m_tabstops != NULL) {
        /* Find the next tabstop. */
        for (newcol++; newcol < VTE_TAB_MAX; newcol++) {
            if (get_tabstop(newcol)) {
                break;
            }
        }
    }

    /* If we have no tab stops or went past the end of the line, stop
     * at the right-most column. */
    if (newcol >= m_column_count) {
        newcol = m_column_count - 1;
    }

    /* but make sure we don't move cursor back (bug #340631) */
    if (col < newcol) {
        VteRowData *rowdata = ensure_row();

        /* Smart tab handling: bug 353610
         *
         * If we currently don't have any cells in the space this
         * tab creates, we try to make the tab character copyable,
         * by appending a single tab char with lots of fragment
         * cells following it.
         *
         * Otherwise, just append empty cells that will show up
         * as a space each.
         */

        old_len = _vte_row_data_length (rowdata);
        _vte_row_data_fill (rowdata, &basic_cell, newcol);

        /* Insert smart tab if there's nothing in the line after
         * us, not even empty cells (with non-default background
         * color for example).
         *
         * Notable bugs here: 545924, 597242, 764330 */
        if (col >= old_len && newcol - col <= VTE_TAB_WIDTH_MAX) {
            glong i;
            VteCell *cell = _vte_row_data_get_writable (rowdata, col);
            VteCell tab = *cell;
            tab.attr.set_columns(newcol - col);
            tab.c = '\t';
            /* Save tab char */
            *cell = tab;
            /* And adjust the fragments */
            for (i = col + 1; i < newcol; i++) {
                cell = _vte_row_data_get_writable (rowdata, i);
                cell->c = '\t';
                cell->attr.set_columns(1);
                cell->attr.set_fragment(true);
            }
        }

        invalidate_cells(m_screen->cursor.col, newcol - m_screen->cursor.col,
                            m_screen->cursor.row, 1);
        m_screen->cursor.col = newcol;
    }
}

void VteTerminalPrivate::seq_cursor_forward_tabulation(vte::parser::Params const& params)
{
    auto val = std::max(std::min(params.number_or_default_at(0, 1),
                                    m_column_count - m_screen->cursor.col), long(1));
    for (auto i = 0; i < val; i++) {
        move_cursor_tab();
    }
}

/* Clear tabs selectively. */
void VteTerminalPrivate::seq_tab_clear(vte::parser::Params const& params)
{
    auto param = params.number_or_default_at(0, 0);

    if (param == 0) {
        clear_tabstop(m_screen->cursor.col);
    } else if (param == 3) {
        if (m_tabstops != nullptr) {
            g_hash_table_destroy(m_tabstops);
            m_tabstops = nullptr;
        }
    }
}

/* Cursor up N lines, no scrolling. */
void VteTerminalPrivate::seq_cursor_up(vte::parser::Params const& params)
{
    auto val = params.number_or_default_at(0, 1);
    move_cursor_up(val);
}

void VteTerminalPrivate::move_cursor_up(vte::grid::row_t rows)
{
    rows = CLAMP(rows, 1, m_row_count);

    /* FIXMEchpe why not do this afterward? */
    ensure_cursor_is_onscreen();

    vte::grid::row_t start;
    /* FIXMEchpe why not check m_origin_mode here? */
    if (m_scrolling_restricted) {
        start = m_screen->insert_delta + m_scrolling_region.start;
    } else {
        start = m_screen->insert_delta;
    }

    m_screen->cursor.row = MAX(m_screen->cursor.row - rows, start);
}

/* Vertical tab. */
void VteTerminalPrivate::seq_vertical_tab(vte::parser::Params const& params)
{
    line_feed();
}

/* Parse parameters of SGR 38, 48 or 58, starting at @index within @params.
 * If @might_contain_color_space_id, a true color sequence sequence is started, and after
 * its leading number "2" at least 4 more parameters are present, then there's an (ignored)
 * color_space_id before the three color components. See the comment below in
 * seq_character_attributes() to understand the different accepted formats.
 * Returns the color index, or -1 on error.
 * Increments @index to point to the last consumed parameter (not beyond). */

template<unsigned int redbits, unsigned int greenbits, unsigned int bluebits>
int32_t
VteTerminalPrivate::parse_sgr_38_48_parameters(vte::parser::Params const& params,
                                               unsigned int *index,
                                               bool might_contain_color_space_id)
{
    auto n_params = params.size();
    if (*index < n_params) {
        long param0;
        if (G_UNLIKELY(!params.number_at_unchecked(*index, param0))) {
            return -1;
        }

        switch (param0) {
            case 2: 
            {
                if (G_UNLIKELY(*index + 3 >= n_params)) {
                    return -1;
                }
                if (might_contain_color_space_id && *index + 5 <= n_params) {
                    *index += 1;
                }

                long param1, param2, param3;
                if (G_UNLIKELY(!params.number_at_unchecked(*index + 1, param1) ||
                               !params.number_at_unchecked(*index + 2, param2) ||
                               !params.number_at_unchecked(*index + 3, param3))) {
                    return -1;
                }

                if (G_UNLIKELY (param1 < 0 || param1 >= 256 || param2 < 0 || param2 >= 256 || param3 < 0 || param3 >= 256)) {
                    return -1;
                }
                *index += 3;

                return VTE_RGB_COLOR(redbits, greenbits, bluebits, param1, param2, param3);
            }
            case 5: 
            {
                long param1;
                if (G_UNLIKELY(!params.number_at(*index + 1, param1))) {
                    return -1;
                }

                if (G_UNLIKELY(param1 < 0 || param1 >= 256)) {
                    return -1;
                }
                *index += 1;
                return param1;
            }
        }
    }
    return -1;
}

/* Handle ANSI color setting and related stuffs (SGR).
 * @params contains the values split at semicolons, with sub arrays splitting at colons
 * wherever colons were encountered. */
void VteTerminalPrivate::seq_character_attributes(vte::parser::Params const& params)
{
    /* Step through each numeric parameter. */
    auto n_params = params.size();
    unsigned int i;
    for (i = 0; i < n_params; i++) {
        /* If this parameter is an array, it can be a fully colon separated 38 or 48
         * (see below for details). */
        if (G_UNLIKELY(params.has_subparams_at_unchecked(i))) {
            auto subparams = params.subparams_at_unchecked(i);

            long param0, param1;
            if (G_UNLIKELY(!subparams.number_at(0, param0))) {
                    continue;
            }

            switch (param0) {
                case 4:
                    if (subparams.number_at(1, param1) && param1 >= 0 && param1 <= 3) {
                        m_defaults.attr.set_underline(param1);
                    }
                    break;
                case 38: 
                {
                    unsigned int index = 1;
                    auto color = parse_sgr_38_48_parameters<8, 8, 8>(subparams, &index, true);
                    if (G_LIKELY (color != -1)) {
                        m_defaults.attr.set_fore(color);
                    }
                    break;
                }
                case 48: 
                {
                    unsigned int index = 1;
                    auto color = parse_sgr_38_48_parameters<8, 8, 8>(subparams, &index, true);
                    if (G_LIKELY (color != -1)) {
                        m_defaults.attr.set_back(color);
                    }
                    break;
                }
                case 58: 
                {
                    unsigned int index = 1;
                    auto color = parse_sgr_38_48_parameters<4, 5, 4>(subparams, &index, true);
                    if (G_LIKELY (color != -1)) {
                        m_defaults.attr.set_deco(color);
                    }
                    break;
                }
            }

            continue;
        }
        /* If this parameter is not a number either, skip it. */
        long param;
        if (!params.number_at_unchecked(i, param)) {
            continue;
        }

        switch (param) {
            case 0:
                reset_default_attributes(false);
                break;
            case 1:
                m_defaults.attr.set_bold(true);
                break;
            case 2:
                m_defaults.attr.set_dim(true);
                break;
            case 3:
                m_defaults.attr.set_italic(true);
                break;
            case 4:
                m_defaults.attr.set_underline(1);
                break;
            case 5:
                m_defaults.attr.set_blink(true);
                break;
            case 7:
                m_defaults.attr.set_reverse(true);
                break;
            case 8:
                m_defaults.attr.set_invisible(true);
                break;
            case 9:
                m_defaults.attr.set_strikethrough(true);
                break;
            case 21:
                m_defaults.attr.set_underline(2);
                break;
            case 22: /* ECMA 48. */
                m_defaults.attr.unset(VTE_ATTR_BOLD_MASK | VTE_ATTR_DIM_MASK);
                break;
            case 23:
                m_defaults.attr.set_italic(false);
                break;
            case 24:
                m_defaults.attr.set_underline(0);
                break;
            case 25:
                m_defaults.attr.set_blink(false);
                break;
            case 27:
                m_defaults.attr.set_reverse(false);
                break;
            case 28:
                m_defaults.attr.set_invisible(false);
                break;
            case 29:
                m_defaults.attr.set_strikethrough(false);
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
                m_defaults.attr.set_fore(VTE_LEGACY_COLORS_OFFSET + (param - 30));
                break;
            case 38:
            case 48:
            case 58:
            {
               /* The format looks like:
                * - 256 color indexed palette:
                *   - ^[[38:5:INDEXm  (de jure standard: ITU-T T.416 / ISO/IEC 8613-6; we also allow and ignore further parameters)
                *   - ^[[38;5;INDEXm  (de facto standard, understood by probably all terminal emulators that support 256 colors)
                * - true colors:
                *   - ^[[38:2:[id]:RED:GREEN:BLUE[:...]m  (de jure standard: ITU-T T.416 / ISO/IEC 8613-6)
                *   - ^[[38:2:RED:GREEN:BLUEm             (common misinterpretation of the standard, FIXME: stop supporting it at some point)
                *   - ^[[38;2;RED;GREEN;BLUEm             (de facto standard, understood by probably all terminal emulators that support true colors)
                * See bugs 685759 and 791456 for details.
                * The colon version was handled above separately.
                * This branch here is reached when the separators are semicolons. */
                if ((i + 1) < n_params) {
                    ++i;
                    int32_t color;
                    switch (param) {
                        case 38:
                            color = parse_sgr_38_48_parameters<8 ,8 ,8>(params, &i, false);
                            if (G_LIKELY (color != -1)) {
                                m_defaults.attr.set_fore(color);
                            }
                            break;
                        case 48:
                            color = parse_sgr_38_48_parameters<8, 8, 8>(params, &i, false);
                            if (G_LIKELY (color != -1)) {
                                m_defaults.attr.set_back(color);
                            }
                            break;
                        case 58:
                            color = parse_sgr_38_48_parameters<4, 5, 4>(params, &i, false);
                            g_printerr("Parsed semicoloned deco colour: %x\n", color);
                            if (G_LIKELY (color != -1)) {
                                m_defaults.attr.set_deco(color);
                            }
                            break;
                    }
                }
                break;
            }
            case 39:
                /* default foreground */
                m_defaults.attr.set_fore(VTE_DEFAULT_FG);
                break;
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
                m_defaults.attr.set_back(VTE_LEGACY_COLORS_OFFSET + (param - 40));
                break;
            /* case 48: was handled above at 38 to avoid code duplication */
            case 49:
                /* default background */
                m_defaults.attr.set_back(VTE_DEFAULT_BG);
                break;
            case 53:
                m_defaults.attr.set_overline(true);
                break;
            case 55:
                m_defaults.attr.set_overline(false);
                break;
                /* case 58: was handled above at 38 to avoid code duplication */
            case 59:
                /* default decoration color, that is, same as the cell's foreground */
                m_defaults.attr.set_deco(VTE_DEFAULT_FG);
                break;
            case 90:
            case 91:
            case 92:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97:
                m_defaults.attr.set_fore(VTE_LEGACY_COLORS_OFFSET + (param - 90) +
                                            VTE_COLOR_BRIGHT_OFFSET);
                break;
            case 100:
            case 101:
            case 102:
            case 103:
            case 104:
            case 105:
            case 106:
            case 107:
                m_defaults.attr.set_back(VTE_LEGACY_COLORS_OFFSET + (param - 100) +
                                            VTE_COLOR_BRIGHT_OFFSET);
                break;
        }
    }
    /* If we had no parameters, default to the defaults. */
    if (i == 0) {
        reset_default_attributes(false);
    }
    /* Save the new colors. */
    m_color_defaults.attr.copy_colors(m_defaults.attr);
    m_fill_defaults.attr.copy_colors(m_defaults.attr);
}

/* Move the cursor to the given column in the top row, 1-based. */
void VteTerminalPrivate::seq_cursor_position_top_row(vte::parser::Params const& params)
{
    auto colval = params.number_or_default_at(0, 1) - 1;
    set_cursor_coords(0, colval);

}

/* Request terminal attributes. */
void VteTerminalPrivate::seq_request_terminal_parameters(vte::parser::Params const& params)
{
    /* removed: feed child */
}

/* Request terminal attributes. */
void VteTerminalPrivate::seq_return_terminal_status(vte::parser::Params const& params)
{
    /* removed: feed child */
}

/* Send primary device attributes. */
void VteTerminalPrivate::seq_send_primary_device_attributes(vte::parser::Params const& params)
{
    /* Claim to be a VT220 with only national character set support. */
    /* removed: feed child */
}

/* Send terminal ID. */
void VteTerminalPrivate::seq_return_terminal_id(vte::parser::Params const& params)
{
    seq_send_primary_device_attributes(params);
}

/* Send secondary device attributes. */
void VteTerminalPrivate::seq_send_secondary_device_attributes(vte::parser::Params const& params)
{
    /* Claim to be a VT220, more or less.  The '>' in the response appears
     * to be undocumented. */
    /* removed: feed child */
}

/* Set one or the other. */
void VteTerminalPrivate::seq_set_icon_title(vte::parser::Params const& params)
{
    set_title_internal(params, true, false);
}

void VteTerminalPrivate::seq_set_window_title(vte::parser::Params const& params)
{
    set_title_internal(params, false, true);
}

/* Set both the window and icon titles to the same string. */
void VteTerminalPrivate::seq_set_icon_and_window_title(vte::parser::Params const& params)
{
    set_title_internal(params, true, true);
}

void VteTerminalPrivate::seq_set_current_directory_uri(vte::parser::Params const& params)
{
    char* uri = nullptr;
    if (params.string_at(0, uri)) {
        /* Validate URI */
        if (uri[0]) {
            auto filename = g_filename_from_uri (uri, nullptr, nullptr);
            if (filename == nullptr) {
                /* invalid URI */
                g_free (uri);
                uri = nullptr;
            } else {
                g_free (filename);
            }
        } else {
            g_free(uri);
            uri = nullptr;
        }
    }

    g_free(m_current_directory_uri_changed);
    m_current_directory_uri_changed = uri /* adopt */;
}

void VteTerminalPrivate::seq_set_current_file_uri(vte::parser::Params const& params)
{
    char* uri = nullptr;
    if (params.string_at(0, uri)) {
        /* Validate URI */
        if (uri[0]) {
            auto filename = g_filename_from_uri (uri, nullptr, nullptr);
            if (filename == nullptr) {
                /* invalid URI */
                g_free (uri);
                uri = nullptr;
            } else {
                g_free (filename);
            }
        } else {
            g_free(uri);
            uri = nullptr;
        }
    }

    g_free(m_current_file_uri_changed);
    m_current_file_uri_changed = uri /* adopt */;
}

/* Handle OSC 8 hyperlinks.
 * See bug 779734 and https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda. */
void VteTerminalPrivate::seq_set_current_hyperlink(vte::parser::Params const& params)
{

    char* hyperlink_params = nullptr;
    char* uri = nullptr;
    if (params.size() >= 2) {
        params.string_at_unchecked(0, hyperlink_params);
        params.string_at_unchecked(1, uri);
    }

    set_current_hyperlink(hyperlink_params, uri);
}

void VteTerminalPrivate::set_current_hyperlink(char *hyperlink_params /* adopted */,
                                                char* uri /* adopted */)
{
    guint idx;
    char *id = NULL;
    char idbuf[24];

    if (!m_allow_hyperlink) {
        return;
    }

    /* Get the "id" parameter */
    if (hyperlink_params) {
        if (strncmp(hyperlink_params, "id=", 3) == 0) {
            id = hyperlink_params + 3;
        } else {
            id = strstr(hyperlink_params, ":id=");
            if (id) {
                id += 4;
            }
        }
    }
    if (id) {
        /* *strchrnul(id, ':') = '\0'; */
        char *p = id;
        while (*p && (*p != ':')) {
            ++p;
        }
        *p = 0;
    }
    _vte_debug_print (VTE_DEBUG_HYPERLINK,
                        "OSC 8: id=\"%s\" uri=\"%s\"\n",
                        id, uri);

    if (uri && strlen(uri) > VTE_HYPERLINK_URI_LENGTH_MAX) {
        _vte_debug_print (VTE_DEBUG_HYPERLINK,
                            "Overlong URI ignored: \"%s\"\n",
                            uri);
        uri[0] = '\0';
    }

    if (id && strlen(id) > VTE_HYPERLINK_ID_LENGTH_MAX) {
        _vte_debug_print (VTE_DEBUG_HYPERLINK,
                            "Overlong \"id\" ignored: \"%s\"\n",
                            id);
        id[0] = '\0';
    }

    if (uri && uri[0]) {
        /* The hyperlink, as we carry around and store in the streams, is "id;uri" */
        char *hyperlink;

        if (!id || !id[0]) {
            /* Automatically generate a unique ID string. The colon makes sure
                * it cannot conflict with an explicitly specified one. */
            sprintf(idbuf, ":%ld", m_hyperlink_auto_id++);
            id = idbuf;
            _vte_debug_print (VTE_DEBUG_HYPERLINK,
                                "Autogenerated id=\"%s\"\n",
                                id);
        }
        hyperlink = g_strdup_printf("%s;%s", id, uri);
        idx = _vte_ring_get_hyperlink_idx(m_screen->row_data, hyperlink);
        g_free (hyperlink);
    } else {
        /* idx = 0; also remove the previous current_idx so that it can be GC'd now. */
        idx = _vte_ring_get_hyperlink_idx(m_screen->row_data, NULL);
    }

    m_defaults.attr.hyperlink_idx = idx;

    g_free(hyperlink_params);
    g_free(uri);
}

/* Restrict the scrolling region. */
void VteTerminalPrivate::seq_set_scrolling_region_from_start(vte::parser::Params const& params)
{
    /* We require a parameters.  Anything less is a reset. */
    if (params.size() < 1) {
        return reset_scrolling_region();
    }

    auto end = params.number_or_default_at(1) - 1;
    set_scrolling_region(-1, end);
}

void VteTerminalPrivate::seq_set_scrolling_region_to_end(vte::parser::Params const& params)
{
    /* We require a parameters.  Anything less is a reset. */
    if (params.size() < 1) {
        return reset_scrolling_region();
    }

    auto start = params.number_or_default_at(0) - 1;
    set_scrolling_region(start, -1);
}

void VteTerminalPrivate::set_keypad_mode(VteKeymode mode)
{
    m_keypad_mode = mode;
}

/* Set the application or normal keypad. */
void VteTerminalPrivate::seq_application_keypad(vte::parser::Params const& params)
{
    _vte_debug_print(VTE_DEBUG_KEYBOARD, "Entering application keypad mode.\n");
    set_keypad_mode(VTE_KEYMODE_APPLICATION);
}

void VteTerminalPrivate::seq_normal_keypad(vte::parser::Params const& params)
{
    _vte_debug_print(VTE_DEBUG_KEYBOARD, "Leaving application keypad mode.\n");
    set_keypad_mode(VTE_KEYMODE_NORMAL);
}

/* Same as cursor_character_absolute, not widely supported. */
void VteTerminalPrivate::seq_character_position_absolute(vte::parser::Params const& params)
{
    seq_cursor_character_absolute (params);
}

/* Set certain terminal attributes. */
void VteTerminalPrivate::seq_set_mode(vte::parser::Params const& params)
{
    set_mode(params, true);
}

/* Unset certain terminal attributes. */
void VteTerminalPrivate::seq_reset_mode(vte::parser::Params const& params)
{
    set_mode(params, false);
}

/* Set certain terminal attributes. */
void VteTerminalPrivate::seq_decset(vte::parser::Params const& params)
{
    decset(params, false, false, true);
}

/* Unset certain terminal attributes. */
void VteTerminalPrivate::seq_decreset(vte::parser::Params const& params)
{
    decset(params, false, false, false);
}

/* Erase certain lines in the display. */
void VteTerminalPrivate::seq_erase_in_display(vte::parser::Params const& params)
{
    /* The default parameter is 0. */
    long param = 0;
    /* Pull out the first parameter. */
    /* FIXMEchpe why this weird taking of the first number param, not the actual first param? */
    auto n_params = params.size();
    for (unsigned int i = 0; i < n_params; i++) {
        if (params.number_at_unchecked(i, param)) {
            break;
        }
    }

    erase_in_display(param);
}

void VteTerminalPrivate::erase_in_display(long param)
{
    /* Clear the right area. */
    switch (param) {
        case 0:
            /* Clear below the current line. */
            clear_below_current();
            break;
        case 1:
            /* Clear above the current line. */
            clear_above_current();
            /* Clear everything to the left of the cursor, too. */
            /* FIXME: vttest. */
            clear_to_bol();
            break;
        case 2:
            /* Clear the entire screen. */
            clear_screen();
            break;
        case 3:
            /* Drop the scrollback. */
            drop_scrollback();
            break;
        default:
            break;
    }
    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Erase certain parts of the current line in the display. */
void VteTerminalPrivate::seq_erase_in_line(vte::parser::Params const& params)
{
    /* The default parameter is 0. */
    long param = 0;
    /* Pull out the first parameter. */
    /* FIXMEchpe why this weird taking of the first number param, not the actual first param? */
    auto n_params = params.size();
    for (unsigned int i = 0; i < n_params; i++) {
        if (params.number_at_unchecked(i, param)) {
            break;
        }
    }

    erase_in_line(param);
}

void VteTerminalPrivate::erase_in_line(long param)
{
    /* Clear the right area. */
    switch (param) {
        case 0:
            /* Clear to end of the line. */
            clear_to_eol();
            break;
        case 1:
            /* Clear to start of the line. */
            clear_to_bol();
            break;
        case 2:
            /* Clear the entire line. */
            clear_current_line();
            break;
        default:
            break;
    }
    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Perform a full-bore reset. */
void VteTerminalPrivate::seq_full_reset(vte::parser::Params const& params)
{
    reset(true, true);
}

/* Insert a certain number of lines below the current cursor. */
void VteTerminalPrivate::seq_insert_lines(vte::parser::Params const& params)
{
    /* The default is one. */
    auto param = params.number_or_default_at(0, 1);
    insert_lines(param);
}

void VteTerminalPrivate::insert_lines(vte::grid::row_t param)
{
    vte::grid::row_t end, i;

    /* Find the region we're messing with. */
    auto row = m_screen->cursor.row;
    if (m_scrolling_restricted) {
        end = m_screen->insert_delta + m_scrolling_region.end;
    } else {
        end = m_screen->insert_delta + m_row_count - 1;
    }

    /* Only allow to insert as many lines as there are between this row
     * and the end of the scrolling region. See bug #676090.
     */
    auto limit = end - row + 1;
    param = MIN (param, limit);

    for (i = 0; i < param; i++) {
        /* Clear a line off the end of the region and add one to the
         * top of the region. */
        ring_remove(end);
        ring_insert(row, true);
    }
    m_screen->cursor.col = 0;
    /* Update the display. */
    scroll_region(row, end - row + 1, param);
    /* Adjust the scrollbars if necessary. */
    adjust_adjustments();
    /* We've modified the display.  Make a note of it. */
    m_text_inserted_flag = TRUE;
}

/* Delete certain lines from the scrolling region. */
void VteTerminalPrivate::seq_delete_lines(vte::parser::Params const& params)
{
    /* The default is one. */
    auto param = params.number_or_default_at(0, 1);
    delete_lines(param);
}

void VteTerminalPrivate::delete_lines(vte::grid::row_t param)
{
    vte::grid::row_t end, i;

    /* Find the region we're messing with. */
    auto row = m_screen->cursor.row;
    if (m_scrolling_restricted) {
        end = m_screen->insert_delta + m_scrolling_region.end;
    } else {
        end = m_screen->insert_delta + m_row_count - 1;
    }

   /* Only allow to delete as many lines as there are between this row
    * and the end of the scrolling region. See bug #676090.
    */
    auto limit = end - row + 1;
    param = MIN (param, limit);

    /* Clear them from below the current cursor. */
    for (i = 0; i < param; i++) {
        /* Insert a line at the end of the region and remove one from
         * the top of the region. */
        ring_remove(row);
        ring_insert(end, true);
    }
    m_screen->cursor.col = 0;
    /* Update the display. */
    scroll_region(row, end - row + 1, -param);
    /* Adjust the scrollbars if necessary. */
    adjust_adjustments();
    /* We've modified the display.  Make a note of it. */
    m_text_deleted_flag = TRUE;
}

/* Device status reports. The possible reports are the cursor position and
 * whether or not we're okay. */
void VteTerminalPrivate::seq_device_status_report(vte::parser::Params const& params)
{
    /* removed */
}

/* DEC-style device status reports. */
void VteTerminalPrivate::seq_dec_device_status_report(vte::parser::Params const& params)
{
    /* removed */
}

/* Restore a certain terminal attribute. */
void VteTerminalPrivate::seq_restore_mode(vte::parser::Params const& params)
{
    decset(params, true, false, false);
}

/* Save a certain terminal attribute. */
void VteTerminalPrivate::seq_save_mode(vte::parser::Params const& params)
{
    decset(params, false, true, false);
}

/* Perform a screen alignment test -- fill all visible cells with the
 * letter "E". */
void VteTerminalPrivate::seq_screen_alignment_test(vte::parser::Params const& params)
{
    for (auto row = m_screen->insert_delta;
         row < m_screen->insert_delta + m_row_count;
         row++) {
        /* Find this row. */
        while (_vte_ring_next(m_screen->row_data) <= row) {
            ring_append(false);
        }
        adjust_adjustments();
        auto rowdata = _vte_ring_index_writable (m_screen->row_data, row);
        g_assert(rowdata != NULL);
        /* Clear this row. */
        _vte_row_data_shrink (rowdata, 0);

        emit_text_deleted();
        /* Fill this row. */
        VteCell cell;
        cell.c = 'E';
        cell.attr = basic_cell.attr;
        cell.attr.set_columns(1);
        _vte_row_data_fill(rowdata, &cell, m_column_count);
        emit_text_inserted();
    }
        invalidate_all();

    /* We modified the display, so make a note of it for completeness. */
    m_text_modified_flag = TRUE;
}

/* DECSCUSR set cursor style */
void VteTerminalPrivate::seq_set_cursor_style(vte::parser::Params const& params)
{
    auto n_params = params.size();
    if (n_params > 1) {
        return;
    }

    long style;
    if (n_params == 0) {
        /* no parameters means default (according to vt100.net) */
        style = VTE_CURSOR_STYLE_TERMINAL_DEFAULT;
    } else {
        if (!params.number_at(0, style)) {
            return;
        }
        if (style < 0 || style > 6) {
            return;
        }
    }

    set_cursor_style(VteCursorStyle(style));
}

/* Perform a soft reset. */
void VteTerminalPrivate::seq_soft_reset(vte::parser::Params const& params)
{
    reset(false, false);
}

/* Window manipulation control sequences.  Most of these are considered
 * bad ideas, but they're implemented as signals which the application
 * is free to ignore, so they're harmless.  Handle at most one action,
 * see bug 741402. */
void VteTerminalPrivate::seq_window_manipulation(vte::parser::Params const& params)
{
    auto n_params = params.size();
    if (n_params < 1) {
        return;
    }

    long param;
    if (!params.number_at_unchecked(0, param)) {
        return;
    }

    long arg1 = -1;
    long arg2 = -1;
    if (n_params >= 2) {
        params.number_at_unchecked(1, arg1);
    }
    if (n_params >= 3) {
        params.number_at_unchecked(2, arg2);
    }

    /*GdkScreen *gscreen;*/ /* FIXME */
    char buf[128];
    int width, height;

    switch (param) {
        case 1:
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Deiconifying window.\n");
            emit_deiconify_window();
            break;
        case 2:
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Iconifying window.\n");
            emit_iconify_window();
            break;
        case 3:
            if ((arg1 != -1) && (arg2 != -1)) {
                _vte_debug_print(VTE_DEBUG_PARSE,
                                    "Moving window to "
                                    "%ld,%ld.\n", arg1, arg2);
                emit_move_window(arg1, arg2);
            }
            break;
        case 4:
            if ((arg1 != -1) && (arg2 != -1)) {
                _vte_debug_print(VTE_DEBUG_PARSE,
                                    "Resizing window "
                                    "(to %ldx%ld pixels, grid size %ldx%ld).\n",
                                    arg2, arg1,
                                    arg2 / m_cell_width,
                                    arg1 / m_cell_height);
                emit_resize_window(arg2 / m_cell_width, arg1 / m_cell_height);
            }
            break;
        case 5:
            _vte_debug_print(VTE_DEBUG_PARSE, "Raising window.\n");
            emit_raise_window();
            break;
        case 6:
            _vte_debug_print(VTE_DEBUG_PARSE, "Lowering window.\n");
            emit_lower_window();
            break;
        case 7:
            _vte_debug_print(VTE_DEBUG_PARSE, "Refreshing window.\n");
            invalidate_all();
            emit_refresh_window();
            break;
        case 8:
            if ((arg1 != -1) && (arg2 != -1)) {
                _vte_debug_print(VTE_DEBUG_PARSE,
                                    "Resizing window "
                                    "(to %ld columns, %ld rows).\n",
                                    arg2, arg1);
                emit_resize_window(arg2, arg1);
            }
            break;
        case 9:
            switch (arg1) {
                case 0:
                    _vte_debug_print(VTE_DEBUG_PARSE,
                                        "Restoring window.\n");
                    emit_restore_window();
                    break;
                case 1:
                    _vte_debug_print(VTE_DEBUG_PARSE,
                                        "Maximizing window.\n");
                    emit_maximize_window();
                    break;
                default:
                    break;
            }
            break;
        case 11:
            /* If we're unmapped, then we're iconified. */
            g_snprintf(buf, sizeof(buf),
                        _VTE_CAP_CSI "%dt",
                        1 + !gtk_widget_get_mapped(m_widget));
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Reporting window state %s.\n",
                                gtk_widget_get_mapped(m_widget) ?
                                "non-iconified" : "iconified");
            /*feed_child(buf, -1);*/ /*FIXME: removed*/
            break;
        case 13:
            /* Send window location, in pixels. */
            gdk_window_get_origin(gtk_widget_get_window(m_widget),
                                    &width, &height);
            g_snprintf(buf, sizeof(buf),
                        _VTE_CAP_CSI "3;%d;%dt",
                        width + m_padding.left,
                        height + m_padding.top);
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Reporting window location"
                                "(%d++,%d++).\n",
                                width, height);
            /*feed_child(buf, -1);*/ /*FIXME:removed*/
            break;
        case 14:
            /* Send window size, in pixels. */
            g_snprintf(buf, sizeof(buf),
                        _VTE_CAP_CSI "4;%d;%dt",
                        (int)(m_row_count * m_cell_height),
                        (int)(m_column_count * m_cell_width));
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Reporting window size "
                                "(%dx%d)\n",
                                (int)(m_row_count * m_cell_height),
                                (int)(m_column_count * m_cell_width));

            /*feed_child(buf, -1);*//*FIXME: removed*/
            break;
        case 18:
            /* Send widget size, in cells. */
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Reporting widget size.\n");
            g_snprintf(buf, sizeof(buf),
                        _VTE_CAP_CSI "8;%ld;%ldt",
                        m_row_count,
                        m_column_count);
            /*eed_child(buf, -1);*//*FIXME: removed */
            break;
        case 19:
#if 0 /* FIXME */
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Reporting screen size.\n");
            gscreen = gtk_widget_get_screen(m_widget);
            height = gdk_screen_get_height(gscreen);
            width = gdk_screen_get_width(gscreen);
            g_snprintf(buf, sizeof(buf),
                        _VTE_CAP_CSI "9;%ld;%ldt",
                        height / m_cell_height,
                        width / m_cell_width);
            feed_child(buf, -1);
#endif
            break;
        case 20:
            /* Report a static icon title, since the real
                icon title should NEVER be reported, as it
                creates a security vulnerability.  See
                http://marc.info/?l=bugtraq&m=104612710031920&w=2
                and CVE-2003-0070. */
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Reporting fake icon title.\n");
            /* never use m_icon_title here! */
            g_snprintf (buf, sizeof (buf),
                        _VTE_CAP_OSC "LTerminal" _VTE_CAP_ST);
            /*feed_child(buf, -1);*//*FIXME: removed */
            break;
        case 21:
            /* Report a static window title, since the real
                window title should NEVER be reported, as it
                creates a security vulnerability.  See
                http://marc.info/?l=bugtraq&m=104612710031920&w=2
                and CVE-2003-0070. */
            _vte_debug_print(VTE_DEBUG_PARSE,
                                "Reporting fake window title.\n");
            /* never use m_window_title here! */
            g_snprintf (buf, sizeof (buf),
                        _VTE_CAP_OSC "lTerminal" _VTE_CAP_ST);
            /*feed_child(buf, -1);*//*FIXME: removed */
            break;
        default:
            if (param >= 24) {
                _vte_debug_print(VTE_DEBUG_PARSE,
                                    "Resizing to %ld rows.\n",
                                    param);
                /* Resize to the specified number of
                 * rows. */
                emit_resize_window(m_column_count, param);
            }
            break;
    }
}

/* Internal helper for setting/querying special colors */
void VteTerminalPrivate::change_special_color(vte::parser::Params const& params,
                                                int index,
                                                int index_fallback,
                                                const char *osc,
                                                const char *terminator)
{
    char* name;
    if (!params.string_at(0, name)) {
        return;
    }

    vte::color::rgb color;

    if (color.parse(name)) {
        set_color(index, VTE_COLOR_SOURCE_ESCAPE, color);
    } else if (strcmp (name, "?") == 0) {
        gchar buf[128];
        auto c = get_color(index);
        if (c == NULL && index_fallback != -1) {
            c = get_color(index_fallback);
        }
        g_assert(c != NULL);
        g_snprintf (buf, sizeof (buf),
                _VTE_CAP_OSC "%s;rgb:%04x/%04x/%04x%s",
                osc, c->red, c->green, c->blue, terminator);
        /*feed_child(buf, -1);*//*FIXME: removed */
    }
}

/* Change the bold color, BEL terminated */
void VteTerminalPrivate::seq_change_bold_color_bel(vte::parser::Params const& params)
{
    change_special_color(params, VTE_BOLD_FG, VTE_DEFAULT_FG, "5;0", BEL);
}

/* Change the bold color, ST terminated */
void
VteTerminalPrivate::seq_change_bold_color_st(vte::parser::Params const& params)
{
    change_special_color(params, VTE_BOLD_FG, VTE_DEFAULT_FG, "5;0", ST);
}

/* Reset the bold color */
void VteTerminalPrivate::seq_reset_bold_color(vte::parser::Params const& params)
{
    reset_color(VTE_BOLD_FG, VTE_COLOR_SOURCE_ESCAPE);
}

/* Change the default foreground cursor, BEL terminated */
void VteTerminalPrivate::seq_change_foreground_color_bel(vte::parser::Params const& params)
{
    change_special_color(params, VTE_DEFAULT_FG, -1, "10", BEL);
}

/* Change the default foreground cursor, ST terminated */
void VteTerminalPrivate::seq_change_foreground_color_st(vte::parser::Params const& params)
{
    change_special_color(params, VTE_DEFAULT_FG, -1, "10", ST);
}

/* Reset the default foreground color */
void VteTerminalPrivate::seq_reset_foreground_color(vte::parser::Params const& params)
{
    reset_color(VTE_DEFAULT_FG, VTE_COLOR_SOURCE_ESCAPE);
}

/* Change the default background cursor, BEL terminated */
void VteTerminalPrivate::seq_change_background_color_bel(vte::parser::Params const& params)
{
    change_special_color(params, VTE_DEFAULT_BG, -1, "11", BEL);
}

/* Change the default background cursor, ST terminated */
void VteTerminalPrivate::seq_change_background_color_st(vte::parser::Params const& params)
{
    change_special_color(params, VTE_DEFAULT_BG, -1, "11", ST);
}

/* Reset the default background color */
void VteTerminalPrivate::seq_reset_background_color(vte::parser::Params const& params)
{
    reset_color(VTE_DEFAULT_BG, VTE_COLOR_SOURCE_ESCAPE);
}

/* Change the color of the cursor background, BEL terminated */
void VteTerminalPrivate::seq_change_cursor_background_color_bel(vte::parser::Params const& params)
{
    change_special_color(params, VTE_CURSOR_BG, VTE_DEFAULT_FG, "12", BEL);
}

/* Change the color of the cursor background, ST terminated */
void VteTerminalPrivate::seq_change_cursor_background_color_st(vte::parser::Params const& params)
{
    change_special_color(params, VTE_CURSOR_BG, VTE_DEFAULT_FG, "12", ST);
}

/* Reset the color of the cursor */
void VteTerminalPrivate::seq_reset_cursor_background_color(vte::parser::Params const& params)
{
    reset_color(VTE_CURSOR_BG, VTE_COLOR_SOURCE_ESCAPE);
}

/* Change the highlight background color, BEL terminated */
void VteTerminalPrivate::seq_change_highlight_background_color_bel(vte::parser::Params const& params)
{
    change_special_color(params, VTE_HIGHLIGHT_BG, VTE_DEFAULT_FG, "17", BEL);
}

/* Change the highlight background color, ST terminated */
void VteTerminalPrivate::seq_change_highlight_background_color_st(vte::parser::Params const& params)
{
    change_special_color(params, VTE_HIGHLIGHT_BG, VTE_DEFAULT_FG, "17", ST);
}

/* Reset the highlight background color */
void VteTerminalPrivate::seq_reset_highlight_background_color(vte::parser::Params const& params)
{
    reset_color(VTE_HIGHLIGHT_BG, VTE_COLOR_SOURCE_ESCAPE);
}

/* Change the highlight foreground color, BEL terminated */
void VteTerminalPrivate::seq_change_highlight_foreground_color_bel(vte::parser::Params const& params)
{
    change_special_color(params, VTE_HIGHLIGHT_FG, VTE_DEFAULT_BG, "19", BEL);
}

/* Change the highlight foreground color, ST terminated */
void VteTerminalPrivate::seq_change_highlight_foreground_color_st(vte::parser::Params const& params)
{
    change_special_color(params, VTE_HIGHLIGHT_FG, VTE_DEFAULT_BG, "19", ST);
}

/* Reset the highlight foreground color */
void VteTerminalPrivate::seq_reset_highlight_foreground_color(vte::parser::Params const& params)
{
    reset_color(VTE_HIGHLIGHT_FG, VTE_COLOR_SOURCE_ESCAPE);
}

/* URXVT generic OSC 777 */

void VteTerminalPrivate::seq_urxvt_777(vte::parser::Params const& params)
{
    /* Accept but ignore this for compatibility with downstream-patched vte (bug #711059)*/
}

/* iterm2 OSC 133 & 1337 */

void VteTerminalPrivate::seq_iterm2_133(vte::parser::Params const& params)
{
   /* Accept but ignore this for compatibility when sshing to an osx host
    * where the iterm2 integration is loaded even when not actually using
    * iterm2.
    */
}

void VteTerminalPrivate::seq_iterm2_1337(vte::parser::Params const& params)
{
   /* Accept but ignore this for compatibility when sshing to an osx host
    * where the iterm2 integration is loaded even when not actually using
    * iterm2.
    */
}

#define UNIMPLEMENTED_SEQUENCE_HANDLER(name) \
        void \
        VteTerminalPrivate::seq_ ## name (vte::parser::Params const& params) \
        { \
                static bool warned = false; \
                if (!warned) { \
                        _vte_debug_print(VTE_DEBUG_PARSE, \
                                         "Unimplemented handler for control sequence `%s'.\n", \
                                         "name"); \
                        warned = true; \
                } \
        }

UNIMPLEMENTED_SEQUENCE_HANDLER(ansi_conformance_level_1)
UNIMPLEMENTED_SEQUENCE_HANDLER(ansi_conformance_level_2)
UNIMPLEMENTED_SEQUENCE_HANDLER(ansi_conformance_level_3)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_font_name)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_font_number)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_logfile)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_mouse_cursor_background_color_bel)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_mouse_cursor_background_color_st)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_mouse_cursor_foreground_color_bel)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_mouse_cursor_foreground_color_st)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_tek_background_color_bel)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_tek_background_color_st)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_tek_cursor_color_bel)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_tek_cursor_color_st)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_tek_foreground_color_bel)
UNIMPLEMENTED_SEQUENCE_HANDLER(change_tek_foreground_color_st)
UNIMPLEMENTED_SEQUENCE_HANDLER(cursor_lower_left)
UNIMPLEMENTED_SEQUENCE_HANDLER(dec_media_copy)
UNIMPLEMENTED_SEQUENCE_HANDLER(default_character_set)
UNIMPLEMENTED_SEQUENCE_HANDLER(device_control_string)
UNIMPLEMENTED_SEQUENCE_HANDLER(double_height_bottom_half)
UNIMPLEMENTED_SEQUENCE_HANDLER(double_height_top_half)
UNIMPLEMENTED_SEQUENCE_HANDLER(double_width)
UNIMPLEMENTED_SEQUENCE_HANDLER(eight_bit_controls)
UNIMPLEMENTED_SEQUENCE_HANDLER(enable_filter_rectangle)
UNIMPLEMENTED_SEQUENCE_HANDLER(enable_locator_reporting)
UNIMPLEMENTED_SEQUENCE_HANDLER(end_of_guarded_area)
UNIMPLEMENTED_SEQUENCE_HANDLER(initiate_hilite_mouse_tracking)
UNIMPLEMENTED_SEQUENCE_HANDLER(invoke_g1_character_set_as_gr)
UNIMPLEMENTED_SEQUENCE_HANDLER(invoke_g2_character_set)
UNIMPLEMENTED_SEQUENCE_HANDLER(invoke_g2_character_set_as_gr)
UNIMPLEMENTED_SEQUENCE_HANDLER(invoke_g3_character_set)
UNIMPLEMENTED_SEQUENCE_HANDLER(invoke_g3_character_set_as_gr)
UNIMPLEMENTED_SEQUENCE_HANDLER(linux_console_cursor_attributes)
UNIMPLEMENTED_SEQUENCE_HANDLER(media_copy)
UNIMPLEMENTED_SEQUENCE_HANDLER(memory_lock)
UNIMPLEMENTED_SEQUENCE_HANDLER(memory_unlock)
UNIMPLEMENTED_SEQUENCE_HANDLER(request_locator_position)
UNIMPLEMENTED_SEQUENCE_HANDLER(reset_mouse_cursor_foreground_color)
UNIMPLEMENTED_SEQUENCE_HANDLER(reset_mouse_cursor_background_color)
UNIMPLEMENTED_SEQUENCE_HANDLER(reset_tek_background_color)
UNIMPLEMENTED_SEQUENCE_HANDLER(reset_tek_cursor_color)
UNIMPLEMENTED_SEQUENCE_HANDLER(reset_tek_foreground_color)
UNIMPLEMENTED_SEQUENCE_HANDLER(select_character_protection)
UNIMPLEMENTED_SEQUENCE_HANDLER(select_locator_events)
UNIMPLEMENTED_SEQUENCE_HANDLER(selective_erase_in_display)
UNIMPLEMENTED_SEQUENCE_HANDLER(selective_erase_in_line)
UNIMPLEMENTED_SEQUENCE_HANDLER(send_tertiary_device_attributes)
UNIMPLEMENTED_SEQUENCE_HANDLER(set_conformance_level)
UNIMPLEMENTED_SEQUENCE_HANDLER(set_text_property_21)
UNIMPLEMENTED_SEQUENCE_HANDLER(set_text_property_2L)
UNIMPLEMENTED_SEQUENCE_HANDLER(set_xproperty)
UNIMPLEMENTED_SEQUENCE_HANDLER(seven_bit_controls)
UNIMPLEMENTED_SEQUENCE_HANDLER(single_shift_g2)
UNIMPLEMENTED_SEQUENCE_HANDLER(single_shift_g3)
UNIMPLEMENTED_SEQUENCE_HANDLER(single_width)
UNIMPLEMENTED_SEQUENCE_HANDLER(start_of_guarded_area)
UNIMPLEMENTED_SEQUENCE_HANDLER(start_or_end_of_string)
UNIMPLEMENTED_SEQUENCE_HANDLER(utf_8_character_set)

#undef UNIMPLEMENTED_UNIMPLEMENTED_SEQUENCE_HANDLER

vte_matcher_entry_t const* _vte_get_matcher_entries(unsigned int* n_entries)
{
#include "caps-list.hh"
    *n_entries = G_N_ELEMENTS (entries);
    return entries;
}
