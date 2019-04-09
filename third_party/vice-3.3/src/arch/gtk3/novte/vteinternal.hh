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

#pragma once

#include <glib.h>

#include "vtedefines.hh"
#include "vtetypes.hh"

#include "ring.h"
#include "vteconv.h"
#include "buffer.h"
#include "matcher.hh"

/* The order is important */
typedef enum {
    MOUSE_TRACKING_NONE,
    MOUSE_TRACKING_SEND_XY_ON_CLICK,
    MOUSE_TRACKING_SEND_XY_ON_BUTTON,
    MOUSE_TRACKING_HILITE_TRACKING,
    MOUSE_TRACKING_CELL_MOTION_TRACKING,
    MOUSE_TRACKING_ALL_MOTION_TRACKING
} MouseTrackingMode;

typedef enum _VteCharacterReplacement {
    VTE_CHARACTER_REPLACEMENT_NONE,
    VTE_CHARACTER_REPLACEMENT_LINE_DRAWING,
    VTE_CHARACTER_REPLACEMENT_BRITISH
} VteCharacterReplacement;

/* The terminal's keypad/cursor state.  A terminal can either be using the
 * normal keypad, or the "application" keypad. */
typedef enum _VteKeymode {
    VTE_KEYMODE_NORMAL,
    VTE_KEYMODE_APPLICATION
} VteKeymode;

typedef struct _VtePaletteColor {
    struct {
        vte::color::rgb color;
        gboolean is_set;
    } sources[2];
} VtePaletteColor;

/* These correspond to the parameters for DECSCUSR (Set cursor style). */
typedef enum _VteCursorStyle {
    /* We treat 0 and 1 differently, assuming that the VT510 does so too.
     *
     * See, according to the "VT510 Video Terminal Programmer Information",
     * from vt100.net, paragraph "2.5.7 Cursor Display", there was a menu
     * item in the "Terminal Set-Up" to set the cursor's style. It looks
     * like that defaulted to blinking block. So it makes sense for 0 to
     * mean "set cursor style to default (set by Set-Up)" and 1 to mean
     * "set cursor style to blinking block", since that default need not be
     * blinking block. Access to a VT510 is needed to test this theory,
     * but it seems plausible. And, anyhow, we can even decide we know
     * better than the VT510 designers! */
    VTE_CURSOR_STYLE_TERMINAL_DEFAULT = 0,
    VTE_CURSOR_STYLE_BLINK_BLOCK      = 1,
    VTE_CURSOR_STYLE_STEADY_BLOCK     = 2,
    VTE_CURSOR_STYLE_BLINK_UNDERLINE  = 3,
    VTE_CURSOR_STYLE_STEADY_UNDERLINE = 4,
    /* *_IBEAM are xterm extensions */
    VTE_CURSOR_STYLE_BLINK_IBEAM      = 5,
    VTE_CURSOR_STYLE_STEADY_IBEAM     = 6
} VteCursorStyle;

typedef struct _vte_incoming_chunk _vte_incoming_chunk_t;
struct _vte_incoming_chunk{
    _vte_incoming_chunk_t *next;
    guint len;
    guchar dataminusone;    /* Hack: Keep it right before data, so that data[-1] is valid and usable */
    guchar data[VTE_INPUT_CHUNK_SIZE - 2 * sizeof(void *) - 1];
};

typedef struct _VteScreen VteScreen;
struct _VteScreen {
    VteRing row_data[1];    /* buffer contents */
    VteVisualPosition cursor;  /* absolute value, from the beginning of the terminal history */
    double scroll_delta;    /* scroll offset */
    long insert_delta;    /* insertion offset */

    /* Stuff saved along with the cursor */
    struct {
        VteVisualPosition cursor;  /* onscreen coordinate, that is, relative to insert_delta */
        gboolean reverse_mode;
        gboolean origin_mode;
        gboolean sendrecv_mode;
        gboolean insert_mode;
        gboolean linefeed_mode;
        VteCell defaults;
        VteCell color_defaults;
        VteCell fill_defaults;
        VteCharacterReplacement character_replacements[2];
        VteCharacterReplacement *character_replacement;
    } saved;
};

enum vte_selection_type {
        selection_type_char,
        selection_type_word,
        selection_type_line
};

/* Until the selection can be generated on demand, let's not enable this on stable */
#undef HTML_SELECTION

/* For unified handling of PRIMARY and CLIPBOARD selection */
typedef enum {
    VTE_SELECTION_PRIMARY,
    VTE_SELECTION_CLIPBOARD,
    LAST_VTE_SELECTION
} VteSelection;

/* Used in the GtkClipboard API, to distinguish requests for HTML and TEXT
 * contents of a clipboard */
typedef enum {
    VTE_TARGET_TEXT,
    VTE_TARGET_HTML,
    LAST_VTE_TARGET
} VteSelectionTarget;

struct vte_scrolling_region {
    int start, end;
};

template <class T>
class ClipboardTextRequestGtk {
public:
    typedef void (T::* Callback)(char const*);

    ClipboardTextRequestGtk() : m_request(nullptr) { }
    ~ClipboardTextRequestGtk() { cancel(); }

    void request_text(GtkClipboard *clipboard,
                        Callback callback,
                        T* that)
    {
        cancel();
        new Request(clipboard, callback, that, &m_request);
    }

private:

    class Request {
    public:
        Request(GtkClipboard *clipboard,
                Callback callback,
                T* that,
                Request** location) :
                m_callback(callback),
                m_that(that),
                m_location(location)
        {
            /* We need to store this here instead of doing it after the |new| above,
                * since gtk_clipboard_request_text may dispatch the callback
                * immediately or only later, with no way to know this beforehand.
                */
            *m_location = this;
            gtk_clipboard_request_text(clipboard, text_received, this);
        }

        ~Request()
        {
            invalidate();
        }

        void cancel()
        {
            invalidate();
            m_that = nullptr;
            m_location = nullptr;
        }

    private:
        Callback m_callback;
        T *m_that;
        Request** m_location;

        void invalidate()
        {
            if (m_that && m_location) {
                    *m_location = nullptr;
            }
        }

        void dispatch(char const *text)
        {
            if (m_that) {
                    g_assert(m_location == nullptr || *m_location == this);

                    (m_that->*m_callback)(text);
            }
        }

        static void text_received(GtkClipboard *clipboard, char const* text, gpointer data) {
            Request* request = reinterpret_cast<Request*>(data);
            request->dispatch(text);
            delete request;
        }
    };

private:
    void cancel()
    {
        if (m_request) {
                m_request->cancel();
        }
        g_assert(m_request == nullptr);
    }

    Request *m_request;
};

namespace vte {
namespace parser {

struct Params {

    typedef long number;

    char* ucs4_to_utf8(gunichar const* str) const;

    void print() const;

    inline unsigned int size() const
    {
        return G_LIKELY(m_values != nullptr) ? m_values->n_values : 0;
    }

    inline GValue* value_at_unchecked(unsigned int position) const
    {
        return g_value_array_get_nth(m_values, position);
    }

    inline bool has_number_at_unchecked(unsigned int position) const
    {
        return G_UNLIKELY(G_VALUE_HOLDS_LONG(value_at_unchecked(position)));
    }

    inline bool number_at_unchecked(unsigned int position, number& v) const
    {
        auto value = value_at_unchecked(position);
        if (G_UNLIKELY(!G_VALUE_HOLDS(value, G_TYPE_LONG))) {
            return false;
        }

        v = g_value_get_long(value);
        return true;
    }

    inline bool number_at(unsigned int position, number& v) const
    {
        if (G_UNLIKELY(position >= size())) {
            return false;
        }

        return number_at_unchecked(position, v);
    }

    inline number number_or_default_at_unchecked(unsigned int position, number default_v = 0) const
    {
        number v;
        if (G_UNLIKELY(!number_at_unchecked(position, v))) {
            v = default_v;
        }
        return v;
    }


    inline number number_or_default_at(unsigned int position, number default_v = 0) const
    {
        number v;
        if (G_UNLIKELY(!number_at(position, v))) {
            v = default_v;
        }
        return v;
    }

    inline bool string_at_unchecked(unsigned int position, char*& str) const
    {
        auto value = value_at_unchecked(position);
        if (G_LIKELY(G_VALUE_HOLDS_POINTER(value))) {
            str = ucs4_to_utf8((gunichar const*)g_value_get_pointer (value));
            return str != nullptr;
        }
        if (G_VALUE_HOLDS_STRING(value)) {
            /* Copy the string into the buffer. */
            str = g_value_dup_string(value);
            return str != nullptr;
        }
        if (G_VALUE_HOLDS_LONG(value)) {
            /* Convert the long to a string. */
            str = g_strdup_printf("%ld", g_value_get_long(value));
            return true;
        }
        return false;
    }

    inline bool string_at(unsigned int position, char*& str) const
    {
        if (G_UNLIKELY(position >= size())) {
                return false;
        }

        return string_at_unchecked(position, str);
    }

    inline bool has_subparams_at_unchecked(unsigned int position) const
    {
        return G_UNLIKELY(G_VALUE_HOLDS_BOXED(value_at_unchecked(position)));
    }

    inline Params subparams_at_unchecked(unsigned int position) const
    {
        return {(GValueArray*)g_value_get_boxed(value_at_unchecked(position))};
    }

    inline void recycle(struct _vte_matcher *matcher)
    {
        if (G_LIKELY(m_values != nullptr)) {
            _vte_matcher_free_params_array(matcher, m_values);
        }
    }

    GValueArray *m_values;
};

} // namespace parser
} // namespace vte


/* Terminal private data. */
class VteTerminalPrivate {
public:
    VteTerminalPrivate(VteTerminal *t);
    ~VteTerminalPrivate();

public:
    VteTerminal *m_terminal;
    GtkWidget *m_widget;

    /* Event window */
    GdkWindow *m_event_window;

    /* Metric and sizing data: dimensions of the window */
    vte::grid::row_t m_row_count;
    vte::grid::column_t m_column_count;

    /* Emulation setup data. */
    struct _vte_matcher *m_matcher;   /* control sequence matcher */
    gboolean m_autowrap;              /* auto wraparound at right margin */
    int m_keypad_mode, m_cursor_mode; /* these would be VteKeymodes, but we
                                            need to guarantee its type */
    GHashTable *m_dec_saved;

    /* Input data queues. */
    const char *m_encoding;            /* the pty's encoding */
    int m_utf8_ambiguous_width;
    struct _vte_iso2022_state *m_iso2022;
    _vte_incoming_chunk_t *m_incoming; /* pending bytestream */
    GArray *m_pending;                 /* pending characters */
    gunichar m_last_graphic_character; /* for REP */
    /* Array of dirty rectangles in view coordinates; need to
     * add allocation origin and padding when passing to gtk.
     */
    GArray *m_update_rects;
    gboolean m_invalidated_all;       /* pending refresh of entire terminal */
    /* If non-nullptr, contains the GList element for @this in g_active_terminals
     * and means that this terminal is processing data.
     */
    GList *m_active_terminals_link;
    /* FIXMEchpe should these two be g[s]size ? */
    glong m_input_bytes;
    glong m_max_input_bytes;

    /* Output data queue. */
    VteByteArray *m_outgoing; /* pending input characters */
    VteConv m_outgoing_conv;

    /* IConv buffer. */
    VteByteArray *m_conv_buffer;

    /* Screen data.  We support the normal screen, and an alternate
     * screen, which seems to be a DEC-specific feature. */
    struct _VteScreen m_normal_screen, m_alternate_screen, *m_screen;

    /* Values we save along with the cursor */
    gboolean m_reverse_mode;  /* reverse mode */
    gboolean m_origin_mode;   /* origin mode */
    gboolean m_sendrecv_mode; /* sendrecv mode */
    gboolean m_insert_mode;   /* insert mode */
    gboolean m_linefeed_mode; /* linefeed mode */
    VteCell m_defaults;       /* default characteristics for insertion of any new characters */
    VteCell m_color_defaults; /* original defaults plus the current fore/back */
    VteCell m_fill_defaults;  /* original defaults plus the current fore/back with no character data */
    VteCharacterReplacement m_character_replacements[2];  /* charsets in the G0 and G1 slots */
    VteCharacterReplacement *m_character_replacement;     /* pointer to the active one */

    /* Word chars */
    char *m_word_char_exceptions_string;
    gunichar *m_word_char_exceptions;
    gsize m_word_char_exceptions_len;

    /* Selection information. */
    gboolean m_has_selection;
    gboolean m_selecting;
    gboolean m_selecting_after_threshold;
    gboolean m_selecting_restart;
    gboolean m_selecting_had_delta;
    gboolean m_selection_block_mode;
    enum vte_selection_type m_selection_type;
    vte::view::coords m_selection_origin, m_selection_last;
    VteVisualPosition m_selection_start, m_selection_end;

    /* Clipboard data information. */
    /* FIXMEchpe check if this can make m_has_selection obsolete! */
    bool m_selection_owned[LAST_VTE_SELECTION];
    VteFormat m_selection_format[LAST_VTE_SELECTION];
    bool m_changing_selection;
    GString *m_selection[LAST_VTE_SELECTION];
    GtkClipboard *m_clipboard[LAST_VTE_SELECTION];

    ClipboardTextRequestGtk<VteTerminalPrivate> m_paste_request;

    /* Miscellaneous options. */
    VteEraseBinding m_backspace_binding;
    VteEraseBinding m_delete_binding;
    gboolean m_meta_sends_escape;
    gboolean m_audible_bell;
    gboolean m_allow_bold;
    gboolean m_bold_is_bright;
    gboolean m_deccolm_mode; /* DECCOLM allowed */
    GHashTable *m_tabstops;
    gboolean m_text_modified_flag;
    gboolean m_text_inserted_flag;
    gboolean m_text_deleted_flag;
    gboolean m_rewrap_on_resize;
    gboolean m_bracketed_paste_mode;

    /* Scrolling options. */
    gboolean m_scroll_on_output;
    gboolean m_scroll_on_keystroke;
    gboolean m_alternate_screen_scroll;
    vte::grid::row_t m_scrollback_lines;

    /* Restricted scrolling */
    struct vte_scrolling_region m_scrolling_region;     /* the region we scroll in */
    gboolean m_scrolling_restricted;

    /* Cursor shape, as set via API */
    VteCursorShape m_cursor_shape;
    double m_cursor_aspect_ratio;

    /* Cursor blinking, as set in dconf. */
    VteCursorBlinkMode m_cursor_blink_mode;
    gboolean m_cursor_blink_state;
    guint m_cursor_blink_tag;           /* cursor blinking timeout ID */
    gint m_cursor_blink_cycle;          /* gtk-cursor-blink-time / 2 */
    gint m_cursor_blink_timeout;        /* gtk-cursor-blink-timeout */
    gboolean m_cursor_blinks;           /* whether the cursor is actually blinking */
    gint64 m_cursor_blink_time;         /* how long the cursor has been blinking yet */
    gboolean m_cursor_visible;
    gboolean m_has_focus;               /* is the terminal window focused */

    /* Contents blinking */
    VteTextBlinkMode m_text_blink_mode;
    gint m_text_blink_cycle;  /* gtk-cursor-blink-time / 2 */
    bool m_text_blink_state;  /* whether blinking text should be visible at this very moment */
    bool m_text_to_blink;     /* drawing signals here if it encounters any cell with blink attribute */
    guint m_text_blink_tag;   /* timeout ID for redrawing due to blinking */

    /* DECSCUSR cursor style (shape and blinking possibly overridden via escape sequence) */
    VteCursorStyle m_cursor_style;

    /* Input device options. */
    gboolean m_input_enabled;
    time_t m_last_keypress_time;

    int m_mouse_tracking_mode;  /* this is of type MouseTrackingMode,
                                   but we need to guarantee its type. */
    guint m_mouse_pressed_buttons;      /* bits 0, 1, 2 resp. for buttons 1, 2, 3 */
    guint m_mouse_handled_buttons;      /* similar bitmap for buttons we handled ourselves */
    /* The last known position the mouse pointer from an event. We don't store
     * this in grid coordinates because we want also to check if they were outside
     * the viewable area, and also want to catch in-cell movements if they make the pointer visible.
     */
    vte::view::coords m_mouse_last_position;
    guint m_mouse_autoscroll_tag;
    gboolean m_mouse_xterm_extension;
    gboolean m_mouse_urxvt_extension;
    double m_mouse_smooth_scroll_delta;

    gboolean m_focus_tracking_mode;

    /* State variables for handling match checks. */
    char* m_match_contents;
    GArray* m_match_attributes;
    GArray* m_match_regexes;
    char* m_match;
    int m_match_tag;
    /* If m_match non-null, then m_match_span contains the region of the match.
     * If m_match is null, and m_match_span is not .empty(), then it contains
     * the minimal region around the last checked coordinates that don't contain
     * a match for any of the dingu regexes.
     */
    vte::grid::span m_match_span;

    /* Search data. */
    gboolean m_search_wrap_around;
    GArray* m_search_attrs; /* Cache attrs */

    /* Data used when rendering the text which does not require server
     * resources and which can be kept after unrealizing. */
    PangoFontDescription *m_unscaled_font_desc;
    PangoFontDescription *m_fontdesc;
    gdouble m_font_scale;
    gboolean m_fontdirty;

   /* First, the dimensions of ASCII characters are measured. The result
    * could probably be called char_{width,height} or font_{width,height}
    * but these aren't stored directly here, not to accidentally be confused
    * with m_cell_{width_height}. The values are stored in vtedraw's font_info.
    *
    * Then in case of nondefault m_cell_{width,height}_scale an additional
    * m_char_padding is added, resulting in m_cell_{width,height} which are
    * hence potentially larger than the characters. This is to implement
    * line spacing and letter spacing, primarly for accessibility (bug 781479).
    *
    * Char width/height, if really needed, can be computed by subtracting
    * the char padding from the cell dimensions. Char height can also be
    * reconstructed from m_char_{ascent,descent}, one of which is redundant,
    * stored for convenience only.
    */
    long m_char_ascent;
    long m_char_descent;
    double m_cell_width_scale;
    double m_cell_height_scale;
    GtkBorder m_char_padding;
    glong m_cell_width;
    glong m_cell_height;

    /* Data used when rendering the text which reflects server resources
     * and data, which should be dropped when unrealizing and (re)created
     * when realizing. */
    struct _vte_draw *m_draw;
    bool m_clear_background{true};

    VtePaletteColor m_palette[VTE_PALETTE_SIZE];

    /* Mouse cursors. */
    gboolean m_mouse_cursor_over_widget; /* as per enter and leave events */
    gboolean m_mouse_autohide;           /* the API setting */
    gboolean m_mouse_cursor_autohidden;  /* whether the autohiding logic wants to hide it; even if autohiding is disabled via API */
    GdkCursor* m_mouse_default_cursor;
    GdkCursor* m_mouse_mousing_cursor;
    GdkCursor* m_mouse_hyperlink_cursor;
    GdkCursor* m_mouse_inviso_cursor;

    /* Input method support. */
    GtkIMContext *m_im_context;
    gboolean m_im_preedit_active;
    char *m_im_preedit;
    PangoAttrList *m_im_preedit_attrs;
    int m_im_preedit_cursor;

    gboolean m_accessible_emit;

    /* Adjustment updates pending. */
    gboolean m_adjustment_changed_pending;
    gboolean m_adjustment_value_changed_pending;
    gboolean m_cursor_moved_pending;
    gboolean m_contents_changed_pending;

    /* window name changes */
    char* m_window_title;
    char* m_window_title_changed;
    char* m_icon_title;
    char* m_icon_title_changed;
    char* m_current_directory_uri;
    char* m_current_directory_uri_changed;
    char* m_current_file_uri;
    char* m_current_file_uri_changed;

    /* Background */
    double m_background_alpha;

    /* Bell */
    int64_t m_bell_timestamp;
    bool m_bell_pending{false};

    /* Key modifiers. */
    guint m_modifiers;

    /* Font stuff. */
    gboolean m_has_fonts;
    long m_line_thickness;
    long m_underline_position;
    long m_underline_thickness;
    long m_double_underline_position;
    long m_double_underline_thickness;
    double m_undercurl_position;
    double m_undercurl_thickness;
    long m_strikethrough_position;
    long m_strikethrough_thickness;
    long m_overline_position;
    long m_overline_thickness;
    long m_regex_underline_position;
    long m_regex_underline_thickness;

    /* Style stuff */
    GtkBorder m_padding;

    /* GtkScrollable impl */
    GtkAdjustment* m_hadjustment; /* unused */
    GtkAdjustment* m_vadjustment;
    guint m_hscroll_policy : 1; /* unused */
    guint m_vscroll_policy : 1;

    /* Hyperlinks */
    gboolean m_allow_hyperlink;
    hyperlink_idx_t m_hyperlink_hover_idx;
    const char *m_hyperlink_hover_uri; /* data is owned by the ring */
    long m_hyperlink_auto_id;

public:

    // FIXMEchpe inline!
    /* inline */ VteRowData* ring_insert(vte::grid::row_t position,
                                    bool fill);
    /* inline */ VteRowData* ring_append(bool fill);
    /* inline */ void ring_remove(vte::grid::row_t position);
    inline VteRowData const* find_row_data(vte::grid::row_t row) const;
    inline VteRowData* find_row_data_writable(vte::grid::row_t row) const;
    inline VteCell const* find_charcell(vte::grid::column_t col,
                                        vte::grid::row_t row) const;
    inline vte::grid::column_t find_start_column(vte::grid::column_t col,
                                                    vte::grid::row_t row) const;
    inline vte::grid::column_t find_end_column(vte::grid::column_t col,
                                                vte::grid::row_t row) const;

    inline vte::view::coord_t scroll_delta_pixel() const;
    inline vte::grid::row_t pixel_to_row(vte::view::coord_t y) const;
    inline vte::view::coord_t row_to_pixel(vte::grid::row_t row) const;
    inline vte::grid::row_t first_displayed_row() const;
    inline vte::grid::row_t last_displayed_row() const;

    inline VteRowData *insert_rows (guint cnt);
    VteRowData *ensure_row();
    VteRowData *ensure_cursor();
    void update_insert_delta();

    void cleanup_fragments(long start,
                            long end);

    void cursor_down(bool explicit_sequence);
    void drop_scrollback();

    void restore_cursor(VteScreen *screen__);
    void save_cursor(VteScreen *screen__);

    bool insert_char(gunichar c,
                        bool insert,
                        bool invalidate_now);

    void invalidate(vte::grid::span const& s, bool block = false);
    void invalidate_match_span();
    void invalidate_cell(vte::grid::column_t column, vte::grid::row_t row);
    void invalidate_cells(vte::grid::column_t sc, int cc,
                            vte::grid::row_t sr, int rc);
    void invalidate_region(vte::grid::column_t sc, vte::grid::column_t ec,
                            vte::grid::row_t sr, vte::grid::row_t er,
                            bool block = false);
    void invalidate_selection();
    void invalidate_all();

    void reset_update_rects();
    bool invalidate_dirty_rects_and_process_updates();
    void time_process_incoming();
    void process_incoming();
    bool process(bool emit_adj_changed);
    inline bool is_processing() const { return m_active_terminals_link != nullptr; }
    void start_processing();

    gssize get_preedit_width(bool left_only);
    gssize get_preedit_length(bool left_only);

    void invalidate_cursor_once(bool periodic = false);
    void invalidate_cursor_periodic();
    void check_cursor_blink();
    void add_cursor_timeout();
    void remove_cursor_timeout();
    void update_cursor_blinks();
    VteCursorBlinkMode decscusr_cursor_blink();
    VteCursorShape decscusr_cursor_shape();

    void remove_text_blink_timeout();

    /* The allocation of the widget */
    cairo_rectangle_int_t m_allocated_rect;
    /* The usable view area. This is the allocation, minus the padding, but
     * including additional right/bottom area if the allocation is not grid aligned.
     */
    vte::view::extents m_view_usable_extents;

    void set_allocated_rect(cairo_rectangle_int_t const& r) { m_allocated_rect = r; update_view_extents(); }
    void update_view_extents() {
        m_view_usable_extents =
            vte::view::extents(m_allocated_rect.width - m_padding.left - m_padding.right,
                                m_allocated_rect.height - m_padding.top - m_padding.bottom);
    }

    inline bool widget_realized() const { return gtk_widget_get_realized(m_widget); }
    inline cairo_rectangle_int_t const& get_allocated_rect() const { return m_allocated_rect; }
    inline vte::view::coord_t get_allocated_width() const { return m_allocated_rect.width; }
    inline vte::view::coord_t get_allocated_height() const { return m_allocated_rect.height; }

    vte::view::coords view_coords_from_event(GdkEvent const* event) const;
    vte::grid::coords grid_coords_from_event(GdkEvent const* event) const;

    vte::view::coords view_coords_from_grid_coords(vte::grid::coords const& rowcol) const;
    vte::grid::coords grid_coords_from_view_coords(vte::view::coords const& pos) const;

    bool view_coords_visible(vte::view::coords const& pos) const;
    bool grid_coords_visible(vte::grid::coords const& rowcol) const;

    inline bool grid_coords_in_scrollback(vte::grid::coords const& rowcol) const { return rowcol.row() < m_screen->insert_delta; }

    vte::grid::coords confine_grid_coords(vte::grid::coords const& rowcol) const;
    vte::grid::coords confined_grid_coords_from_event(GdkEvent const* event) const;
    vte::grid::coords confined_grid_coords_from_view_coords(vte::view::coords const& pos) const;

    void confine_coordinates(long *xp, long *yp);

    void widget_paste(GdkAtom board);
    void widget_copy(VteSelection sel,
                        VteFormat format);
    void widget_paste_received(char const* text);
    void widget_clipboard_cleared(GtkClipboard *clipboard);
    void widget_clipboard_requested(GtkClipboard *target_clipboard,
                                    GtkSelectionData *data,
                                    guint info);

    void widget_set_hadjustment(GtkAdjustment *adjustment);
    void widget_set_vadjustment(GtkAdjustment *adjustment);

    GdkCursor *widget_cursor_new(GdkCursorType cursor_type) const;

    void widget_constructed();
    void widget_realize();
    void widget_unrealize();
    void widget_map();
    void widget_unmap();
    void widget_style_updated();
    void widget_focus_in(GdkEventFocus *event);
    void widget_focus_out(GdkEventFocus *event);
    bool widget_key_press(GdkEventKey *event);
    bool widget_key_release(GdkEventKey *event);
    bool widget_button_press(GdkEventButton *event);
    bool widget_button_release(GdkEventButton *event);
    void widget_enter(GdkEventCrossing *event);
    void widget_leave(GdkEventCrossing *event);
    void widget_scroll(GdkEventScroll *event);
    bool widget_motion_notify(GdkEventMotion *event);
    void widget_draw(cairo_t *cr);
    void widget_screen_changed (GdkScreen *previous_screen);
    void widget_get_preferred_width(int *minimum_width, int *natural_width);
    void widget_get_preferred_height(int *minimum_height, int *natural_height);
    void widget_size_allocate(GtkAllocation *allocation);

    void widget_settings_notify();

    void expand_rectangle(cairo_rectangle_int_t& rect) const;
    void paint_area(GdkRectangle const* area);
    void paint_cursor();
    void paint_im_preedit_string();
    void draw_cells(struct _vte_draw_text_request *items,
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
                    int row_height);
    void fudge_pango_colors(GSList *attributes, VteCell *cells, gsize n);
    void apply_pango_attr(PangoAttribute *attr, VteCell *cells, gsize n_cells);
    void translate_pango_cells(PangoAttrList *attrs, VteCell *cells, gsize n_cells);
    void draw_cells_with_attributes(struct _vte_draw_text_request *items,
                                    gssize n,
                                    PangoAttrList *attrs,
                                    bool draw_default_bg,
                                    int column_width,
                                    int height);
    void draw_rows(VteScreen *screen,
                    vte::grid::row_t start_row,
                    long row_count,
                    vte::grid::column_t start_column,
                    long column_count,
                    gint start_x,
                    gint start_y,
                    gint column_width,
                    gint row_height);

    bool autoscroll();
    void start_autoscroll();
    void stop_autoscroll();

    void scroll_region (long row, long count, long delta);

    void connect_pty_read();
    void disconnect_pty_read();

    void connect_pty_write();
    void disconnect_pty_write();

    void pty_termios_changed();
    void pty_scroll_lock_changed(bool locked);

    void pty_channel_eof();
    bool pty_io_read(GIOChannel *channel, GIOCondition condition);
    bool pty_io_write(GIOChannel *channel, GIOCondition condition);

    void feed_chunks(struct _vte_incoming_chunk *chunks);
    void send_child(char const* data,
                    gssize length,
                    bool local_echo,
                    bool newline_stuff);
    void feed_child_using_modes(char const* data, gssize length);

    void watch_child (GPid child_pid);
    void child_watch_done(GPid pid, int status);

    void im_commit(char const* text);
    void im_preedit_start();
    void im_preedit_end();
    void im_preedit_changed();
    bool im_retrieve_surrounding();
    bool im_delete_surrounding(int offset, int n_chars);
    void im_reset();
    void im_update_cursor();

    void reset(bool clear_tabstops, bool clear_history, bool from_api = false);
    void feed(char const* data, gssize length, bool start_processsing_ = true);
#if 0 /* FIXME: removed */
    void feed_child(char const *text, gssize length);
    void feed_child_binary(guint8 const* data, gsize length);
#endif
    bool is_word_char(gunichar c) const;
    bool is_same_class(vte::grid::column_t acol,
                        vte::grid::row_t arow,
                        vte::grid::column_t bcol,
                        vte::grid::row_t brow) const;

    inline bool line_is_wrappable(vte::grid::row_t row) const;

    GString* get_text(vte::grid::row_t start_row,
                        vte::grid::column_t start_col,
                        vte::grid::row_t end_row,
                        vte::grid::column_t end_col,
                        bool block,
                        bool wrap,
                        bool include_trailing_spaces,
                        GArray* attributes = nullptr);

    GString* get_text_displayed(bool wrap,
                                bool include_trailing_spaces,
                                GArray* attributes = nullptr);

    GString* get_text_displayed_a11y(bool wrap,
                                        bool include_trailing_spaces,
                                        GArray* attributes = nullptr);

    GString* get_selected_text(GArray* attributes = nullptr);

    template<unsigned int redbits, unsigned int greenbits, unsigned int bluebits>
    inline void rgb_from_index(guint index, vte::color::rgb& color) const;
    inline void determine_colors(VteCellAttr const* attr,
                                    bool selected,
                                    bool cursor,
                                    guint *pfore,
                                    guint *pback,
                                    guint *pdeco) const;
    inline void determine_colors(VteCell const* cell,
                                    bool selected,
                                    guint *pfore,
                                    guint *pback,
                                    guint *pdeco) const;
    inline void determine_cursor_colors(VteCell const* cell,
                                        bool selected,
                                        guint *pfore,
                                        guint *pback,
                                        guint *pdeco) const;

    char *cellattr_to_html(VteCellAttr const* attr, char const* text) const;
    VteCellAttr const* char_to_cell_attr(VteCharAttributes const* attr) const;

    GString* attributes_to_html(GString* text_string, GArray* attrs);

    void start_selection(long x, long y, enum vte_selection_type selection_type);
    bool maybe_end_selection();

    void extend_selection_expand();
    void extend_selection(long x, long y, bool always_grow, bool force);

    void select_all();
    void deselect_all();

    bool cell_is_selected(vte::grid::column_t col, vte::grid::row_t) const;

    void reset_default_attributes(bool reset_hyperlink);

    void ensure_font();
    void update_font();
    void apply_font_metrics(int cell_width,
                            int cell_height,
                            int char_ascent,
                            int char_descent,
                            GtkBorder char_spacing);

    void refresh_size();
    void screen_set_size(VteScreen *screen_,
                            long old_columns,
                            long old_rows,
                            bool do_rewrap);

    void vadjustment_value_changed();

    void read_modifiers(GdkEvent *event);
    guint translate_ctrlkey(GdkEventKey *event);

    void apply_mouse_cursor();
    void set_pointer_autohidden(bool autohidden);

    void beep();

    void emit_adjustment_changed();
    void emit_commit(char const* text, gssize length);
    void emit_eof();
    void emit_selection_changed();
    void queue_adjustment_changed();
    void queue_adjustment_value_changed(double v);
    void queue_adjustment_value_changed_clamped(double v);
    void adjust_adjustments();
    void adjust_adjustments_full();

    void scroll_lines(long lines);
    void scroll_pages(long pages) { scroll_lines(pages * m_row_count); }
    void maybe_scroll_to_top();
    void maybe_scroll_to_bottom();

    void queue_cursor_moved();
    void queue_contents_changed();
    void queue_eof();

    void emit_text_deleted();
    void emit_text_inserted();
    void emit_text_modified();
    void emit_text_scrolled(long delta);
    void emit_pending_signals();
    void emit_char_size_changed(int width,
                                int height);
    void emit_increase_font_size();
    void emit_decrease_font_size();
    void emit_bell();
    void emit_deiconify_window();
    void emit_iconify_window();
    void emit_raise_window();
    void emit_lower_window();
    void emit_maximize_window();
    void emit_refresh_window();
    void emit_restore_window();
    void emit_move_window(guint x, guint y);
    void emit_resize_window(guint columns, guint rows);
    void emit_copy_clipboard();
    void emit_paste_clipboard();
    void emit_hyperlink_hover_uri_changed(const GdkRectangle *bbox);

    void clear_tabstop(int column); // FIXMEchpe vte::grid::column_t ?
    bool get_tabstop(int column);
    void set_tabstop(int column);
    void set_default_tabstops();

    void hyperlink_invalidate_and_get_bbox(hyperlink_idx_t idx, GdkRectangle *bbox);
    void hyperlink_hilite_update();

    void match_contents_clear();
    void match_contents_refresh();
    void set_cursor_from_regex_match(struct vte_match_regex *regex);
    void match_hilite_clear();
    void match_hilite_update();

    bool rowcol_from_event(GdkEvent *event, long *column, long *row);

    char *hyperlink_check(GdkEvent *event);

    bool feed_mouse_event(vte::grid::coords const& unconfined_rowcol,
                            int button,
                            bool is_drag,
                            bool is_release);
    bool maybe_send_mouse_button(vte::grid::coords const& rowcol,
                                    GdkEventType event_type,
                                    int event_button);
    bool maybe_send_mouse_drag(vte::grid::coords const& rowcol,
                                GdkEventType event_type);

    void feed_focus_event(bool in);
    void feed_focus_event_initial();
    void maybe_feed_focus_event(bool in);

    bool search_find(bool backward);
    bool search_set_wrap_around(bool wrap);

    void set_size(long columns, long rows);

    bool process_word_char_exceptions(char const *str, gunichar **arrayp, gsize *lenp);

    long get_cell_height() { ensure_font(); return m_cell_height; }
    long get_cell_width()  { ensure_font(); return m_cell_width;  }

    vte::color::rgb const* get_color(int entry) const;
    void set_color(int entry, int source, vte::color::rgb const& proposed);
    void reset_color(int entry, int source);

    bool set_audible_bell(bool setting);
    bool set_text_blink_mode(VteTextBlinkMode setting);
    bool set_allow_bold(bool setting);
    bool set_allow_hyperlink(bool setting);
    bool set_backspace_binding(VteEraseBinding binding);
    bool set_background_alpha(double alpha);
    bool set_bold_is_bright(bool setting);
    bool set_cell_height_scale(double scale);
    bool set_cell_width_scale(double scale);
    bool set_cjk_ambiguous_width(int width);
    void set_color_background(vte::color::rgb const &color);
    void set_color_bold(vte::color::rgb const& color);
    void reset_color_bold();
    void set_color_cursor_background(vte::color::rgb const& color);
    void reset_color_cursor_background();
    void set_color_cursor_foreground(vte::color::rgb const& color);
    void reset_color_cursor_foreground();
    void set_color_foreground(vte::color::rgb const& color);
    void set_color_highlight_background(vte::color::rgb const& color);
    void reset_color_highlight_background();
    void set_color_highlight_foreground(vte::color::rgb const& color);
    void reset_color_highlight_foreground();
    void set_colors(vte::color::rgb const *foreground,
                    vte::color::rgb const *background,
                    vte::color::rgb const *palette,
                    gsize palette_size);
    void set_colors_default();
    bool set_cursor_blink_mode(VteCursorBlinkMode mode);
    bool set_cursor_shape(VteCursorShape shape);
    bool set_cursor_style(VteCursorStyle style);
    bool set_delete_binding(VteEraseBinding binding);
    bool set_encoding(char const* codeset);
    bool set_font_desc(PangoFontDescription const* desc);
    bool set_font_scale(double scale);
    bool set_input_enabled(bool enabled);
    bool set_mouse_autohide(bool autohide);
    bool set_rewrap_on_resize(bool rewrap);
    bool set_scrollback_lines(long lines);
    bool set_scroll_on_keystroke(bool scroll);
    bool set_scroll_on_output(bool scroll);
    bool set_word_char_exceptions(char const* exceptions);
    void set_clear_background(bool setting);

    bool write_contents_sync (GOutputStream *stream,
                                VteWriteFlags flags,
                                GCancellable *cancellable,
                                GError **error);

    /* Sequence handlers and their helper functions */
    void handle_sequence(char const* match,
                            vte::parser::Params const& params);

    inline void ensure_cursor_is_onscreen();
    inline void home_cursor();
    inline void clear_screen();
    inline void clear_current_line();
    inline void clear_above_current();
    inline void scroll_text(vte::grid::row_t scroll_amount);
    inline void switch_screen(VteScreen *new_screen);
    inline void switch_normal_screen();
    inline void switch_alternate_screen();
    inline void save_cursor();
    inline void restore_cursor();
    inline void switch_normal_screen_and_restore_cursor();
    inline void save_cursor_and_switch_alternate_screen();
    void set_title_internal(vte::parser::Params const& params,
                            bool icon_title,
                            bool window_title);
    inline void set_mode(vte::parser::Params const& params, bool value);
    inline void reset_mouse_smooth_scroll_delta();
    inline void enter_focus_tracking_mode();
    inline void decset(long setting, bool restore, bool save, bool set);
    inline void decset(vte::parser::Params const& params, bool restore, bool save, bool set);
    inline void set_character_replacements(unsigned slot, VteCharacterReplacement replacement);
    inline void set_character_replacement(unsigned slot);
    inline void clear_to_bol();
    inline void clear_below_current();
    inline void clear_to_eol();
    inline void delete_character();
    inline void set_cursor_column(vte::grid::column_t col);
    inline void set_cursor_row(vte::grid::row_t row /* relative to scrolling region */);
    inline void set_cursor_coords(vte::grid::row_t row /* relative to scrolling region */,
                                    vte::grid::column_t column);
    inline vte::grid::row_t get_cursor_row() const;
    inline vte::grid::column_t get_cursor_column() const;
    inline void reset_scrolling_region();
    inline void set_scrolling_region(vte::grid::row_t start /* relative */,
                                        vte::grid::row_t end /* relative */);
    inline void move_cursor_up(vte::grid::row_t rows);
    inline void move_cursor_down(vte::grid::row_t rows);
    inline void erase_characters(long count);
    inline void insert_blank_character();

    template<unsigned int redbits, unsigned int greenbits, unsigned int bluebits>
    inline int32_t parse_sgr_38_48_parameters(vte::parser::Params const& params,
                                                unsigned int *index,
                                                bool might_contain_color_space_id);

    inline void move_cursor_backward(vte::grid::column_t columns);
    inline void move_cursor_forward(vte::grid::column_t columns);
    inline void move_cursor_tab();
    inline void change_color(vte::parser::Params const& params, char const* terminator);
    inline void line_feed();
    inline void set_current_hyperlink(char* hyperlink_params /* adopted */, char* uri /* adopted */);
    inline void set_keypad_mode(VteKeymode mode);
    inline void erase_in_display(long param);
    inline void erase_in_line(long param);
    inline void insert_lines(vte::grid::row_t param);
    inline void delete_lines(vte::grid::row_t param);
    inline void change_special_color(vte::parser::Params const& params,
                                        int index,
                                        int index_fallback,
                                        const char *osc,
                                        char const *terminator);

    unsigned int checksum_area(vte::grid::row_t start_row,
                                vte::grid::column_t start_col,
                                vte::grid::row_t end_row,
                                vte::grid::column_t end_col);

    void subscribe_accessible_events();
    void select_text(vte::grid::column_t start_col,
                        vte::grid::row_t start_row,
                        vte::grid::column_t end_col,
                        vte::grid::row_t end_row);
    void select_empty(vte::grid::column_t col,
                        vte::grid::row_t row);

#define SEQUENCE_HANDLER(name) \
    inline void seq_ ## name (vte::parser::Params const& params);
#include "vteseq-list.hh"
#undef SEQUENCE_HANDLER
};

extern GTimer *process_timer;

VteTerminalPrivate *_vte_terminal_get_impl(VteTerminal *terminal);

static inline bool _vte_double_equal(double a, double b)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
    return a == b;
#pragma GCC diagnostic pop
}

extern bool g_test_mode;
