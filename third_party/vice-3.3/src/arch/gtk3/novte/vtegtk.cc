/*
 * Copyright (C) 2001-2004,2009,2010 Red Hat, Inc.
 * Copyright © 2008, 2009, 2010, 2015 Christian Persch
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION: vte-terminal
 * @short_description: A terminal widget implementation
 *
 * A NoVteTerminal is a terminal emulator implemented as a GTK3 widget.
 *
 * Note that altough #NoVteTerminal implements the #GtkScrollable interface,
 * you should not place a #NoVteTerminal inside a #GtkScrolledWindow
 * container, since they are incompatible. Instead, pack the terminal in
 * a horizontal #GtkBox together with a #GtkScrollbar which uses the
 * #GtkAdjustment returned from gtk_scrollable_get_vadjustment().
 */

#define GLIB_DISABLE_DEPRECATION_WARNINGS /* FIXME */

#include "vice.h"

#include <new> /* placement new */

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <glib.h>

#include <gtk/gtk.h>
#include "vteenums.h"

#include "vteterminal.h"
#include "vtetypebuiltins.h"

#include "debug.h"
#include "marshal.h"
#include "vtedefines.hh"
#include "vteinternal.hh"
#include "vteaccess.h"

#include "vtegtk.hh"

#if !GLIB_CHECK_VERSION(2, 42, 0)
#define G_PARAM_EXPLICIT_NOTIFY 0
#endif

#define I_(string) (g_intern_static_string(string))

#define VTE_TERMINAL_CSS_NAME "vte-terminal"

struct _VteTerminalClassPrivate {
        GtkStyleProvider *style_provider;
};

#ifdef VTE_DEBUG
G_DEFINE_TYPE_WITH_CODE(NoVteTerminal, vte_terminal, GTK_TYPE_WIDGET,
                        G_ADD_PRIVATE(NoVteTerminal)
                        g_type_add_class_private (g_define_type_id, sizeof (VteTerminalClassPrivate));
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, NULL)
                        if (_vte_debug_on(VTE_DEBUG_LIFECYCLE)) {
                                g_printerr("vte_terminal_get_type()\n");
                        })
#else
G_DEFINE_TYPE_WITH_CODE(VteTerminal, vte_terminal, GTK_TYPE_WIDGET,
                        G_ADD_PRIVATE(VteTerminal)
                        g_type_add_class_private (g_define_type_id, sizeof (VteTerminalClassPrivate));
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_SCROLLABLE, NULL))
#endif

/* VOID:INT,INT (marshal.list:1) */
G_GNUC_INTERNAL void _vte_marshal_VOID__INT_INT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data)
{
}

/* VOID:OBJECT,OBJECT (marshal.list:2) */
G_GNUC_INTERNAL void _vte_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data)
{
}

/* VOID:STRING,BOXED (marshal.list:3) */
G_GNUC_INTERNAL void _vte_marshal_VOID__STRING_BOXED (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data)
{
}

/* VOID:STRING,UINT (marshal.list:4) */
G_GNUC_INTERNAL void _vte_marshal_VOID__STRING_UINT (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data)
{
}

/* VOID:UINT,UINT (marshal.list:5) */
G_GNUC_INTERNAL void _vte_marshal_VOID__UINT_UINT (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data)
{
}


#define IMPL(t) (reinterpret_cast<VteTerminalPrivate*>(vte_terminal_get_instance_private(t)))

guint signals[LAST_SIGNAL];
GParamSpec *pspecs[LAST_PROP];
GTimer *process_timer;
bool g_test_mode = false;

static bool valid_color(GdkRGBA const* color)
{
    return color->red >= 0. && color->red <= 1. &&
            color->green >= 0. && color->green <= 1. &&
            color->blue >= 0. && color->blue <= 1. &&
            color->alpha >= 0. && color->alpha <= 1.;
}

VteTerminalPrivate *_vte_terminal_get_impl(NoVteTerminal *terminal)
{
    return IMPL(terminal);
}

static void vte_terminal_set_hadjustment(NoVteTerminal *terminal,
                                            GtkAdjustment *adjustment)
{
    g_return_if_fail(adjustment == nullptr || GTK_IS_ADJUSTMENT(adjustment));
    IMPL(terminal)->widget_set_hadjustment(adjustment);
}

static void vte_terminal_set_vadjustment(NoVteTerminal *terminal,
                                            GtkAdjustment *adjustment)
{
    g_return_if_fail(adjustment == nullptr || GTK_IS_ADJUSTMENT(adjustment));
    IMPL(terminal)->widget_set_vadjustment(adjustment);
}

static void vte_terminal_set_hscroll_policy(NoVteTerminal *terminal,
                                            GtkScrollablePolicy policy)
{
    IMPL(terminal)->m_hscroll_policy = policy;
    gtk_widget_queue_resize_no_redraw (GTK_WIDGET (terminal));
}


static void vte_terminal_set_vscroll_policy(NoVteTerminal *terminal,
                                            GtkScrollablePolicy policy)
{
    IMPL(terminal)->m_vscroll_policy = policy;
    gtk_widget_queue_resize_no_redraw (GTK_WIDGET (terminal));
}

static void vte_terminal_real_copy_clipboard(NoVteTerminal *terminal)
{
    IMPL(terminal)->widget_copy(VTE_SELECTION_CLIPBOARD, VTE_FORMAT_TEXT);
}

static void vte_terminal_real_paste_clipboard(NoVteTerminal *terminal)
{
    IMPL(terminal)->widget_paste(GDK_SELECTION_CLIPBOARD);
}

static void vte_terminal_style_updated (GtkWidget *widget)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);

    GTK_WIDGET_CLASS (vte_terminal_parent_class)->style_updated (widget);

    IMPL(terminal)->widget_style_updated();
}

static gboolean vte_terminal_key_press(GtkWidget *widget, GdkEventKey *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);

    /* We do NOT want chain up to GtkWidget::key-press-event, since that would
     * cause GtkWidget's keybindings to be handled and consumed. However we'll
     * have to handle the one sane binding (Shift-F10 or MenuKey, to pop up the
     * context menu) ourself, so for now we simply skip the offending keybinding
     * in class_init.
     */

    /* First, check if GtkWidget's behavior already does something with
     * this key. */
    if (GTK_WIDGET_CLASS(vte_terminal_parent_class)->key_press_event) {
        if ((GTK_WIDGET_CLASS(vte_terminal_parent_class))->key_press_event(widget, event)) {
            return TRUE;
        }
    }

    return IMPL(terminal)->widget_key_press(event);
}

static gboolean vte_terminal_key_release(GtkWidget *widget, GdkEventKey *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    return IMPL(terminal)->widget_key_release(event);
}

static gboolean vte_terminal_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    return IMPL(terminal)->widget_motion_notify(event);
}

static gboolean vte_terminal_button_press(GtkWidget *widget, GdkEventButton *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    return IMPL(terminal)->widget_button_press(event);
}

static gboolean vte_terminal_button_release(GtkWidget *widget, GdkEventButton *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    return IMPL(terminal)->widget_button_release(event);
}

static gboolean vte_terminal_scroll(GtkWidget *widget, GdkEventScroll *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_scroll(event);
    return TRUE;
}

static gboolean vte_terminal_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_focus_in(event);
    return FALSE;
}

static gboolean vte_terminal_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_focus_out(event);
    return FALSE;
}

static gboolean vte_terminal_enter(GtkWidget *widget, GdkEventCrossing *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    gboolean ret = FALSE;

    if (GTK_WIDGET_CLASS (vte_terminal_parent_class)->enter_notify_event) {
        ret = GTK_WIDGET_CLASS (vte_terminal_parent_class)->enter_notify_event (widget, event);
    }

    IMPL(terminal)->widget_enter(event);

    return ret;
}

static gboolean vte_terminal_leave(GtkWidget *widget, GdkEventCrossing *event)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    gboolean ret = FALSE;

    if (GTK_WIDGET_CLASS (vte_terminal_parent_class)->leave_notify_event) {
        ret = GTK_WIDGET_CLASS (vte_terminal_parent_class)->leave_notify_event (widget, event);
    }

    IMPL(terminal)->widget_leave(event);

    return ret;
}

static void vte_terminal_get_preferred_width(GtkWidget *widget,
                                                int       *minimum_width,
                                                int       *natural_width)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_get_preferred_width(minimum_width, natural_width);
}

static void vte_terminal_get_preferred_height(GtkWidget *widget,
                                                int       *minimum_height,
                                                int       *natural_height)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_get_preferred_height(minimum_height, natural_height);
}

static void vte_terminal_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_size_allocate(allocation);
}

static gboolean vte_terminal_draw(GtkWidget *widget,
                  cairo_t *cr)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL (widget);
    IMPL(terminal)->widget_draw(cr);
    return FALSE;
}

static void vte_terminal_realize(GtkWidget *widget)
{
    GTK_WIDGET_CLASS(vte_terminal_parent_class)->realize(widget);

    NoVteTerminal *terminal= NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_realize();
}

static void vte_terminal_unrealize(GtkWidget *widget)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL (widget);
    IMPL(terminal)->widget_unrealize();

    GTK_WIDGET_CLASS(vte_terminal_parent_class)->unrealize(widget);
}

static void vte_terminal_map(GtkWidget *widget)
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_map()\n");

    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    GTK_WIDGET_CLASS(vte_terminal_parent_class)->map(widget);

    IMPL(terminal)->widget_map();
}

static void vte_terminal_unmap(GtkWidget *widget)
{
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_unmap()\n");

    NoVteTerminal *terminal = NOVTE_TERMINAL(widget);
    IMPL(terminal)->widget_unmap();

    GTK_WIDGET_CLASS(vte_terminal_parent_class)->unmap(widget);
}

static void vte_terminal_screen_changed (GtkWidget *widget,
                                            GdkScreen *previous_screen)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL (widget);

    if (GTK_WIDGET_CLASS (vte_terminal_parent_class)->screen_changed) {
            GTK_WIDGET_CLASS (vte_terminal_parent_class)->screen_changed (widget, previous_screen);
    }

    IMPL(terminal)->widget_screen_changed(previous_screen);
}

static void vte_terminal_constructed (GObject *object)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL (object);

    G_OBJECT_CLASS (vte_terminal_parent_class)->constructed (object);

    IMPL(terminal)->widget_constructed();
}

static void vte_terminal_init(NoVteTerminal *terminal)
{
    void *place;
    GtkStyleContext *context;

    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_init()\n");

    context = gtk_widget_get_style_context(&terminal->widget);
    gtk_style_context_add_provider (context,
                                    VTE_TERMINAL_GET_CLASS (terminal)->priv->style_provider,
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    /* Initialize private data. NOTE: place is zeroed */
    place = vte_terminal_get_instance_private(terminal);
    new (place) VteTerminalPrivate(terminal);

    gtk_widget_set_has_window(&terminal->widget, FALSE);
}

static void vte_terminal_finalize(GObject *object)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL (object);

    IMPL(terminal)->~VteTerminalPrivate();

    /* Call the inherited finalize() method. */
    G_OBJECT_CLASS(vte_terminal_parent_class)->finalize(object);
}

static void vte_terminal_get_property (GObject *object,
                                        guint prop_id,
                                        GValue *value,
                                        GParamSpec *pspec)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL (object);
    auto impl = IMPL(terminal);

    switch (prop_id) {
        case PROP_HADJUSTMENT:
            g_value_set_object (value, impl->m_hadjustment);
            break;
        case PROP_VADJUSTMENT:
            g_value_set_object (value, impl->m_vadjustment);
            break;
        case PROP_HSCROLL_POLICY:
            g_value_set_enum (value, impl->m_hscroll_policy);
            break;
        case PROP_VSCROLL_POLICY:
            g_value_set_enum (value, impl->m_vscroll_policy);
            break;
        case PROP_ALLOW_BOLD:
            g_value_set_boolean (value, vte_terminal_get_allow_bold (terminal));
            break;
        case PROP_ALLOW_HYPERLINK:
            g_value_set_boolean (value, vte_terminal_get_allow_hyperlink (terminal));
            break;
        case PROP_AUDIBLE_BELL:
            g_value_set_boolean (value, vte_terminal_get_audible_bell (terminal));
            break;
#if 0 /* FIXME: disabled */
        case PROP_BACKSPACE_BINDING:
            g_value_set_enum (value, impl->m_backspace_binding);
            break;
#endif
        case PROP_BOLD_IS_BRIGHT:
            g_value_set_boolean (value, vte_terminal_get_bold_is_bright (terminal));
            break;
        case PROP_CELL_HEIGHT_SCALE:
            g_value_set_double (value, vte_terminal_get_cell_height_scale (terminal));
            break;
        case PROP_CELL_WIDTH_SCALE:
            g_value_set_double (value, vte_terminal_get_cell_width_scale (terminal));
            break;
        case PROP_CJK_AMBIGUOUS_WIDTH:
            g_value_set_int (value, vte_terminal_get_cjk_ambiguous_width (terminal));
            break;
        case PROP_CURSOR_BLINK_MODE:
            g_value_set_enum (value, vte_terminal_get_cursor_blink_mode (terminal));
            break;
        case PROP_CURRENT_DIRECTORY_URI:
            g_value_set_string (value, vte_terminal_get_current_directory_uri (terminal));
            break;
        case PROP_CURRENT_FILE_URI:
            g_value_set_string (value, vte_terminal_get_current_file_uri (terminal));
            break;
        case PROP_CURSOR_SHAPE:
            g_value_set_enum (value, vte_terminal_get_cursor_shape (terminal));
            break;
#if 0 /* FIXME: disabled */
        case PROP_DELETE_BINDING:
            g_value_set_enum (value, impl->m_delete_binding);
            break;
#endif
        case PROP_ENCODING:
            g_value_set_string (value, vte_terminal_get_encoding (terminal));
            break;
        case PROP_FONT_DESC:
            g_value_set_boxed (value, vte_terminal_get_font (terminal));
            break;
        case PROP_FONT_SCALE:
            g_value_set_double (value, vte_terminal_get_font_scale (terminal));
            break;
        case PROP_HYPERLINK_HOVER_URI:
            g_value_set_string (value, impl->m_hyperlink_hover_uri);
            break;
        case PROP_ICON_TITLE:
            g_value_set_string (value, vte_terminal_get_icon_title (terminal));
            break;
        case PROP_INPUT_ENABLED:
            g_value_set_boolean (value, vte_terminal_get_input_enabled (terminal));
            break;
        case PROP_MOUSE_POINTER_AUTOHIDE:
            g_value_set_boolean (value, vte_terminal_get_mouse_autohide (terminal));
            break;
#if 0 /* FIXME: disabled */
        case PROP_PTY:
            g_value_set_object (value, vte_terminal_get_pty(terminal));
            break;
#endif
        case PROP_REWRAP_ON_RESIZE:
            g_value_set_boolean (value, vte_terminal_get_rewrap_on_resize (terminal));
            break;
        case PROP_SCROLLBACK_LINES:
            g_value_set_uint (value, vte_terminal_get_scrollback_lines(terminal));
            break;
        case PROP_SCROLL_ON_KEYSTROKE:
            g_value_set_boolean (value, vte_terminal_get_scroll_on_keystroke(terminal));
            break;
        case PROP_SCROLL_ON_OUTPUT:
            g_value_set_boolean (value, vte_terminal_get_scroll_on_output(terminal));
            break;
        case PROP_TEXT_BLINK_MODE:
            g_value_set_enum (value, vte_terminal_get_text_blink_mode (terminal));
            break;
        case PROP_WINDOW_TITLE:
            g_value_set_string (value, vte_terminal_get_window_title (terminal));
            break;
        case PROP_WORD_CHAR_EXCEPTIONS:
            g_value_set_string (value, vte_terminal_get_word_char_exceptions (terminal));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            return;
    }
}

static void vte_terminal_set_property (GObject *object,
                                        guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec)
{
    NoVteTerminal *terminal = NOVTE_TERMINAL (object);

    switch (prop_id) {
        case PROP_HADJUSTMENT:
            vte_terminal_set_hadjustment (terminal, (GtkAdjustment *)g_value_get_object (value));
            break;
        case PROP_VADJUSTMENT:
            vte_terminal_set_vadjustment (terminal, (GtkAdjustment *)g_value_get_object (value));
            break;
        case PROP_HSCROLL_POLICY:
            vte_terminal_set_hscroll_policy(terminal, (GtkScrollablePolicy)g_value_get_enum(value));
            break;
        case PROP_VSCROLL_POLICY:
            vte_terminal_set_vscroll_policy(terminal, (GtkScrollablePolicy)g_value_get_enum(value));
            break;
        case PROP_ALLOW_BOLD:
            vte_terminal_set_allow_bold (terminal, g_value_get_boolean (value));
            break;
        case PROP_ALLOW_HYPERLINK:
            vte_terminal_set_allow_hyperlink (terminal, g_value_get_boolean (value));
            break;
        case PROP_AUDIBLE_BELL:
            vte_terminal_set_audible_bell (terminal, g_value_get_boolean (value));
            break;
#if 0 /* FIXME: disabled */
        case PROP_BACKSPACE_BINDING:
            vte_terminal_set_backspace_binding (terminal, (VteEraseBinding)g_value_get_enum (value));
            break;
#endif
        case PROP_BOLD_IS_BRIGHT:
            vte_terminal_set_bold_is_bright (terminal, g_value_get_boolean (value));
            break;
        case PROP_CELL_HEIGHT_SCALE:
            vte_terminal_set_cell_height_scale (terminal, g_value_get_double (value));
            break;
        case PROP_CELL_WIDTH_SCALE:
            vte_terminal_set_cell_width_scale (terminal, g_value_get_double (value));
            break;
        case PROP_CJK_AMBIGUOUS_WIDTH:
            vte_terminal_set_cjk_ambiguous_width (terminal, g_value_get_int (value));
            break;
        case PROP_CURSOR_BLINK_MODE:
            vte_terminal_set_cursor_blink_mode (terminal, (VteCursorBlinkMode)g_value_get_enum (value));
            break;
        case PROP_CURSOR_SHAPE:
            vte_terminal_set_cursor_shape (terminal, (VteCursorShape)g_value_get_enum (value));
            break;
#if 0 /* FIXME: disabled */
        case PROP_DELETE_BINDING:
            vte_terminal_set_delete_binding (terminal, (VteEraseBinding)g_value_get_enum (value));
            break;
#endif
        case PROP_ENCODING:
            vte_terminal_set_encoding (terminal, g_value_get_string (value), NULL);
            break;
        case PROP_FONT_DESC:
            vte_terminal_set_font (terminal, (PangoFontDescription *)g_value_get_boxed (value));
            break;
        case PROP_FONT_SCALE:
            vte_terminal_set_font_scale (terminal, g_value_get_double (value));
            break;
        case PROP_INPUT_ENABLED:
            vte_terminal_set_input_enabled (terminal, g_value_get_boolean (value));
            break;
        case PROP_MOUSE_POINTER_AUTOHIDE:
            vte_terminal_set_mouse_autohide (terminal, g_value_get_boolean (value));
            break;
#if 0 /* FIXME: disabled */
        case PROP_PTY:
            vte_terminal_set_pty (terminal, (VtePty *)g_value_get_object (value));
            break;
#endif
        case PROP_REWRAP_ON_RESIZE:
            vte_terminal_set_rewrap_on_resize (terminal, g_value_get_boolean (value));
            break;
        case PROP_SCROLLBACK_LINES:
            novte_terminal_set_scrollback_lines (terminal, g_value_get_uint (value));
            break;
        case PROP_SCROLL_ON_KEYSTROKE:
            vte_terminal_set_scroll_on_keystroke(terminal, g_value_get_boolean (value));
            break;
        case PROP_SCROLL_ON_OUTPUT:
            novte_terminal_set_scroll_on_output (terminal, g_value_get_boolean (value));
            break;
        case PROP_TEXT_BLINK_MODE:
            vte_terminal_set_text_blink_mode (terminal, (VteTextBlinkMode)g_value_get_enum (value));
            break;
        case PROP_WORD_CHAR_EXCEPTIONS:
            vte_terminal_set_word_char_exceptions (terminal, g_value_get_string (value));
            break;

                        /* Not writable */
        case PROP_CURRENT_DIRECTORY_URI:
        case PROP_CURRENT_FILE_URI:
        case PROP_HYPERLINK_HOVER_URI:
        case PROP_ICON_TITLE:
        case PROP_WINDOW_TITLE:
            g_assert_not_reached ();
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            return;
    }
}

static void vte_terminal_class_init(VteTerminalClass *klass)
{
    GObjectClass *gobject_class;
    GtkWidgetClass *widget_class;
    GtkBindingSet  *binding_set;

#ifdef VTE_DEBUG
    {
    _vte_debug_init();
    _vte_debug_print(VTE_DEBUG_LIFECYCLE, "vte_terminal_class_init()\n");
    /* print out the legend */
    _vte_debug_print(VTE_DEBUG_WORK,
                                "Debugging work flow (top input to bottom output):\n"
                                "  .  _vte_terminal_process_incoming\n"
                                "  <  start process_timeout\n"
                                "  {[ start update_timeout  [ => rate limited\n"
                                "  T  start of terminal in update_timeout\n"
                                "  (  start _vte_terminal_process_incoming\n"
                                "  ?  _vte_invalidate_cells (call)\n"
                                "  !  _vte_invalidate_cells (dirty)\n"
                                "  *  _vte_invalidate_all\n"
                                "  )  end _vte_terminal_process_incoming\n"
                                "  =  vte_terminal_paint\n"
                                "  ]} end update_timeout\n"
                                "  >  end process_timeout\n");

    char const* test_env = g_getenv("VTE_TEST");
    if (test_env != nullptr) {
        g_test_mode = g_str_equal(test_env, "1");
        g_unsetenv("VTE_TEST");
    }
    }
#endif

#if 0 /* FIXME: deprecated */
    _VTE_DEBUG_IF (VTE_DEBUG_UPDATES) gdk_window_set_debug_updates(TRUE);
#endif

    gobject_class = G_OBJECT_CLASS(klass);
    widget_class = GTK_WIDGET_CLASS(klass);

    /* Override some of the default handlers. */
    gobject_class->constructed = vte_terminal_constructed;
    gobject_class->finalize = vte_terminal_finalize;
    gobject_class->get_property = vte_terminal_get_property;
    gobject_class->set_property = vte_terminal_set_property;
    widget_class->realize = vte_terminal_realize;
    widget_class->unrealize = vte_terminal_unrealize;
    widget_class->map = vte_terminal_map;
    widget_class->unmap = vte_terminal_unmap;
    widget_class->scroll_event = vte_terminal_scroll;
    widget_class->draw = vte_terminal_draw;
    widget_class->key_press_event = vte_terminal_key_press;
    widget_class->key_release_event = vte_terminal_key_release;
    widget_class->button_press_event = vte_terminal_button_press;
    widget_class->button_release_event = vte_terminal_button_release;
    widget_class->motion_notify_event = vte_terminal_motion_notify;
    widget_class->enter_notify_event = vte_terminal_enter;
    widget_class->leave_notify_event = vte_terminal_leave;
    widget_class->focus_in_event = vte_terminal_focus_in;
    widget_class->focus_out_event = vte_terminal_focus_out;
    widget_class->style_updated = vte_terminal_style_updated;
    widget_class->get_preferred_width = vte_terminal_get_preferred_width;
    widget_class->get_preferred_height = vte_terminal_get_preferred_height;
    widget_class->size_allocate = vte_terminal_size_allocate;
    widget_class->screen_changed = vte_terminal_screen_changed;

/* FIXME: can we remove this? */
#if GTK_CHECK_VERSION(3, 19, 5)
    gtk_widget_class_set_css_name(widget_class, VTE_TERMINAL_CSS_NAME);
#else
    /* Bug #763538 */
    if (gtk_check_version(3, 19, 5) == nullptr) {
        g_printerr("NOVTE needs to be recompiled against a newer gtk+ version.\n");
    }
#endif

    /* Initialize default handlers. */
    klass->eof = NULL;
    klass->child_exited = NULL;
    klass->encoding_changed = NULL;
    klass->char_size_changed = NULL;
    klass->window_title_changed = NULL;
    klass->icon_title_changed = NULL;
    klass->selection_changed = NULL;
    klass->contents_changed = NULL;
    klass->cursor_moved = NULL;
    klass->commit = NULL;

    klass->deiconify_window = NULL;
    klass->iconify_window = NULL;
    klass->raise_window = NULL;
    klass->lower_window = NULL;
    klass->refresh_window = NULL;
    klass->restore_window = NULL;
    klass->maximize_window = NULL;
    klass->resize_window = NULL;
    klass->move_window = NULL;

    klass->increase_font_size = NULL;
    klass->decrease_font_size = NULL;

    klass->text_modified = NULL;
    klass->text_inserted = NULL;
    klass->text_deleted = NULL;
    klass->text_scrolled = NULL;

    klass->copy_clipboard = vte_terminal_real_copy_clipboard;
    klass->paste_clipboard = vte_terminal_real_paste_clipboard;

    klass->bell = NULL;

    /* GtkScrollable interface properties */
    g_object_class_override_property (gobject_class, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property (gobject_class, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property (gobject_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property (gobject_class, PROP_VSCROLL_POLICY, "vscroll-policy");

    /* Register some signals of our own. */

    /*
     * NoVteTerminal::eof:
     * @vteterminal: the object which received the signal
     *
     * Emitted when the terminal receives an end-of-file from a child which
     * is running in the terminal.  This signal is frequently (but not
     * always) emitted with a #NoVteTerminal::child-exited signal.
     */
    signals[SIGNAL_EOF] =
            g_signal_new(I_("eof"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, eof),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::child-exited:
     * @vteterminal: the object which received the signal
     * @status: the child's exit status
     *
     * This signal is emitted when the terminal detects that a child
     * watched using vte_terminal_watch_child() has exited.
     */
    signals[SIGNAL_CHILD_EXITED] =
            g_signal_new(I_("child-exited"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, child_exited),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__INT,
                            G_TYPE_NONE,
                            1, G_TYPE_INT);

    /*
     * NoVteTerminal::window-title-changed:
     * @vteterminal: the object which received the signal
     *
     * Emitted when the terminal's %window_title field is modified.
     */
    signals[SIGNAL_WINDOW_TITLE_CHANGED] =
            g_signal_new(I_("window-title-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, window_title_changed),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::icon-title-changed:
     * @vteterminal: the object which received the signal
     *
     * Emitted when the terminal's %icon_title field is modified.
     */
    signals[SIGNAL_ICON_TITLE_CHANGED] =
            g_signal_new(I_("icon-title-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, icon_title_changed),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::current-directory-uri-changed:
     * @vteterminal: the object which received the signal
     *
     * Emitted when the current directory URI is modified.
     */
    signals[SIGNAL_CURRENT_DIRECTORY_URI_CHANGED] =
            g_signal_new(I_("current-directory-uri-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            0,
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::current-file-uri-changed:
     * @vteterminal: the object which received the signal
     *
     * Emitted when the current file URI is modified.
     */
    signals[SIGNAL_CURRENT_FILE_URI_CHANGED] =
            g_signal_new(I_("current-file-uri-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            0,
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::hyperlink-hover-uri-changed:
     * @vteterminal: the object which received the signal
     * @uri: the nonempty target URI under the mouse, or NULL
     * @bbox: the bounding box of the hyperlink anchor text, or NULL
     *
     * Emitted when the hovered hyperlink changes.
     *
     * @uri and @bbox are owned by VTE, must not be modified, and might
     * change after the signal handlers returns.
     *
     * The signal is not re-emitted when the bounding box changes for the
     * same hyperlink. This might change in a future VTE version without notice.
     *
     * Since: 0.50
     */
    signals[SIGNAL_HYPERLINK_HOVER_URI_CHANGED] =
            g_signal_new(I_("hyperlink-hover-uri-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            0,
                            NULL,
                            NULL,
                            _vte_marshal_VOID__STRING_BOXED,
                            G_TYPE_NONE,
                            2, G_TYPE_STRING, GDK_TYPE_RECTANGLE | G_SIGNAL_TYPE_STATIC_SCOPE);

    /*
     * NoVteTerminal::encoding-changed:
     * @vteterminal: the object which received the signal
     *
     * Emitted whenever the terminal's current encoding has changed, either
     * as a result of receiving a control sequence which toggled between the
     * local and UTF-8 encodings, or at the parent application's request.
     */
    signals[SIGNAL_ENCODING_CHANGED] =
            g_signal_new(I_("encoding-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, encoding_changed),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::commit:
     * @vteterminal: the object which received the signal
     * @text: a string of text
     * @size: the length of that string of text
     *
     * Emitted whenever the terminal receives input from the user and
     * prepares to send it to the child process.  The signal is emitted even
     * when there is no child process.
     */
    signals[SIGNAL_COMMIT] =
            g_signal_new(I_("commit"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, commit),
                            NULL,
                            NULL,
                            _vte_marshal_VOID__STRING_UINT,
                            G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_UINT);

    /*
     * NoVteTerminal::char-size-changed:
     * @vteterminal: the object which received the signal
     * @width: the new character cell width
     * @height: the new character cell height
     *
     * Emitted whenever the cell size changes, e.g. due to a change in
     * font, font-scale or cell-width/height-scale.
     *
     * Note that this signal should rather be called "cell-size-changed".
     */
    signals[SIGNAL_CHAR_SIZE_CHANGED] =
            g_signal_new(I_("char-size-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, char_size_changed),
                            NULL,
                            NULL,
                            _vte_marshal_VOID__UINT_UINT,
                            G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

    /*
     * NoVteTerminal::selection-changed:
     * @vteterminal: the object which received the signal
     *
     * Emitted whenever the contents of terminal's selection changes.
     */
    signals[SIGNAL_SELECTION_CHANGED] =
            g_signal_new (I_("selection-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, selection_changed),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::contents-changed:
     * @vteterminal: the object which received the signal
     *
     * Emitted whenever the visible appearance of the terminal has changed.
     * Used primarily by #VteTerminalAccessible.
     */
    signals[SIGNAL_CONTENTS_CHANGED] =
            g_signal_new(I_("contents-changed"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, contents_changed),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::cursor-moved:
     * @vteterminal: the object which received the signal
     *
     * Emitted whenever the cursor moves to a new character cell.  Used
     * primarily by #VteTerminalAccessible.
     */
    signals[SIGNAL_CURSOR_MOVED] =
            g_signal_new(I_("cursor-moved"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, cursor_moved),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::deiconify-window:
     * @vteterminal: the object which received the signal
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_DEICONIFY_WINDOW] =
            g_signal_new(I_("deiconify-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, deiconify_window),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::iconify-window:
     * @vteterminal: the object which received the signal
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_ICONIFY_WINDOW] =
            g_signal_new(I_("iconify-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, iconify_window),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::raise-window:
     * @vteterminal: the object which received the signal
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_RAISE_WINDOW] =
            g_signal_new(I_("raise-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, raise_window),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::lower-window:
     * @vteterminal: the object which received the signal
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_LOWER_WINDOW] =
            g_signal_new(I_("lower-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, lower_window),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::refresh-window:
     * @vteterminal: the object which received the signal
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_REFRESH_WINDOW] =
            g_signal_new(I_("refresh-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, refresh_window),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::restore-window:
     * @vteterminal: the object which received the signal
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_RESTORE_WINDOW] =
            g_signal_new(I_("restore-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, restore_window),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::maximize-window:
     * @vteterminal: the object which received the signal
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_MAXIMIZE_WINDOW] =
            g_signal_new(I_("maximize-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, maximize_window),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::resize-window:
     * @vteterminal: the object which received the signal
     * @width: the desired number of columns
     * @height: the desired number of rows
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_RESIZE_WINDOW] =
            g_signal_new(I_("resize-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, resize_window),
                            NULL,
                            NULL,
                            _vte_marshal_VOID__UINT_UINT,
                            G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

    /*
     * NoVteTerminal::move-window:
     * @vteterminal: the object which received the signal
     * @x: the terminal's desired location, X coordinate
     * @y: the terminal's desired location, Y coordinate
     *
     * Emitted at the child application's request.
     */
    signals[SIGNAL_MOVE_WINDOW] =
            g_signal_new(I_("move-window"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, move_window),
                            NULL,
                            NULL,
                            _vte_marshal_VOID__UINT_UINT,
                            G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

    /*
     * NoVteTerminal::increase-font-size:
     * @vteterminal: the object which received the signal
     *
     * Emitted when the user hits the '+' key while holding the Control key.
     */
    signals[SIGNAL_INCREASE_FONT_SIZE] =
            g_signal_new(I_("increase-font-size"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, increase_font_size),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::decrease-font-size:
     * @vteterminal: the object which received the signal
     *
     * Emitted when the user hits the '-' key while holding the Control key.
     */
    signals[SIGNAL_DECREASE_FONT_SIZE] =
            g_signal_new(I_("decrease-font-size"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, decrease_font_size),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::text-modified:
     * @vteterminal: the object which received the signal
     *
     * An internal signal used for communication between the terminal and
     * its accessibility peer. May not be emitted under certain
     * circumstances.
     */
    signals[SIGNAL_TEXT_MODIFIED] =
            g_signal_new(I_("text-modified"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, text_modified),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::text-inserted:
     * @vteterminal: the object which received the signal
     *
     * An internal signal used for communication between the terminal and
     * its accessibility peer. May not be emitted under certain
     * circumstances.
     */
    signals[SIGNAL_TEXT_INSERTED] =
            g_signal_new(I_("text-inserted"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, text_inserted),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::text-deleted:
     * @vteterminal: the object which received the signal
     *
     * An internal signal used for communication between the terminal and
     * its accessibility peer. May not be emitted under certain
     * circumstances.
     */
    signals[SIGNAL_TEXT_DELETED] =
            g_signal_new(I_("text-deleted"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, text_deleted),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::text-scrolled:
     * @vteterminal: the object which received the signal
     * @delta: the number of lines scrolled
     *
     * An internal signal used for communication between the terminal and
     * its accessibility peer. May not be emitted under certain
     * circumstances.
     */
    signals[SIGNAL_TEXT_SCROLLED] =
            g_signal_new(I_("text-scrolled"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, text_scrolled),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__INT,
                            G_TYPE_NONE, 1, G_TYPE_INT);

    /*
     * NoVteTerminal::copy-clipboard:
     * @vteterminal: the object which received the signal
     *
     * Emitted whenever novte_terminal_copy_clipboard() is called.
     */
    signals[SIGNAL_COPY_CLIPBOARD] =
            g_signal_new(I_("copy-clipboard"),
                            G_OBJECT_CLASS_TYPE(klass),
                            (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                            G_STRUCT_OFFSET(VteTerminalClass, copy_clipboard),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::paste-clipboard:
     * @vteterminal: the object which received the signal
     *
     * Emitted whenever vte_terminal_paste_clipboard() is called.
     */
    signals[SIGNAL_PASTE_CLIPBOARD] =
            g_signal_new(I_("paste-clipboard"),
                            G_OBJECT_CLASS_TYPE(klass),
                            (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                            G_STRUCT_OFFSET(VteTerminalClass, paste_clipboard),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal::bell:
     * @vteterminal: the object which received the signal
     *
     * This signal is emitted when the a child sends a bell request to the
     * terminal.
     */
    signals[SIGNAL_BELL] =
            g_signal_new(I_("bell"),
                            G_OBJECT_CLASS_TYPE(klass),
                            G_SIGNAL_RUN_LAST,
                            G_STRUCT_OFFSET(VteTerminalClass, bell),
                            NULL,
                            NULL,
                            g_cclosure_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);

    /*
     * NoVteTerminal:allow-bold:
     *
     * Controls whether or not the terminal will attempt to draw bold text.
     * This may happen either by using a bold font variant, or by
     * repainting text with a different offset.
     */
    pspecs[PROP_ALLOW_BOLD] =
            g_param_spec_boolean ("allow-bold", NULL, NULL,
                                    TRUE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:allow-hyperlink:
     *
     * Controls whether or not hyperlinks (OSC 8 escape sequence) are recognized and displayed.
     *
     * Since: 0.50
     */
    pspecs[PROP_ALLOW_HYPERLINK] =
            g_param_spec_boolean ("allow-hyperlink", NULL, NULL,
                                    FALSE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:audible-bell:
     *
     * Controls whether or not the terminal will beep when the child outputs the
     * "bl" sequence.
     */
    pspecs[PROP_AUDIBLE_BELL] =
            g_param_spec_boolean ("audible-bell", NULL, NULL,
                                    TRUE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
#if 0
    /*
     * NoVteTerminal:backspace-binding:
     *
     * Controls what string or control sequence the terminal sends to its child
     * when the user presses the backspace key.
     */
    pspecs[PROP_BACKSPACE_BINDING] =
            g_param_spec_enum ("backspace-binding", NULL, NULL,
                                VTE_TYPE_ERASE_BINDING,
                                VTE_ERASE_AUTO,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
#endif

    /*
     * NoVteTerminal:bold-is-bright:
     *
     * Whether the SGR 1 attribute also switches to the bright counterpart
     * of the first 8 palette colors, in addition to making them bold (legacy behavior)
     * or if SGR 1 only enables bold and leaves the color intact.
     *
     * Since: 0.52
     */
    pspecs[PROP_BOLD_IS_BRIGHT] =
            g_param_spec_boolean ("bold-is-bright", NULL, NULL,
                                    TRUE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:cell-height-scale:
     *
     * Scale factor for the cell height, to increase line spacing. (The font's height is not affected.)
     *
     * Since: 0.52
     */
    pspecs[PROP_CELL_HEIGHT_SCALE] =
            g_param_spec_double ("cell-height-scale", NULL, NULL,
                                    VTE_CELL_SCALE_MIN,
                                    VTE_CELL_SCALE_MAX,
                                    1.,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:cell-width-scale:
     *
     * Scale factor for the cell width, to increase letter spacing. (The font's width is not affected.)
     *
     * Since: 0.52
     */
    pspecs[PROP_CELL_WIDTH_SCALE] =
            g_param_spec_double ("cell-width-scale", NULL, NULL,
                                    VTE_CELL_SCALE_MIN,
                                    VTE_CELL_SCALE_MAX,
                                    1.,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:cjk-ambiguous-width:
     *
     * This setting controls whether ambiguous-width characters are narrow or wide
     * when using the UTF-8 encoding (vte_terminal_set_encoding()). In all other encodings,
     * the width of ambiguous-width characters is fixed.
     *
     * This setting only takes effect the next time the terminal is reset, either
     * via escape sequence or with vte_terminal_reset().
     */
    pspecs[PROP_CJK_AMBIGUOUS_WIDTH] =
            g_param_spec_int ("cjk-ambiguous-width", NULL, NULL,
                                1, 2, VTE_DEFAULT_UTF8_AMBIGUOUS_WIDTH,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
#if 0 /* FIXME: removed */
    /*
     * NoVteTerminal:cursor-blink-mode:
     *
     * Sets whether or not the cursor will blink. Using %VTE_CURSOR_BLINK_SYSTEM
     * will use the #GtkSettings::gtk-cursor-blink setting.
     */
    pspecs[PROP_CURSOR_BLINK_MODE] =
            g_param_spec_enum ("cursor-blink-mode", NULL, NULL,
                                /* VTE_TYPE_CURSOR_BLINK_MODE, */
                                VTE_CURSOR_BLINK_ON,
                                VTE_CURSOR_BLINK_SYSTEM,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
#endif
#if 0 /* FIXME: removed */
    /*
     * NoVteTerminal:cursor-shape:
     *
     * Controls the shape of the cursor.
     */
    pspecs[PROP_CURSOR_SHAPE] =
            g_param_spec_enum ("cursor-shape", NULL, NULL,
                                /* VTE_TYPE_CURSOR_SHAPE, */
                                VTE_CURSOR_SHAPE_BLOCK,
                                VTE_CURSOR_SHAPE_BLOCK,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
#endif
#if 0 /* FIXME: removed */
    /*
     * NoVteTerminal:delete-binding:
     *
     * Controls what string or control sequence the terminal sends to its child
     * when the user presses the delete key.
     */
    pspecs[PROP_DELETE_BINDING] =
            g_param_spec_enum ("delete-binding", NULL, NULL,
                                VTE_TYPE_ERASE_BINDING,
                                VTE_ERASE_AUTO,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
#endif
    /*
     * NoVteTerminal:font-scale:
     *
     * The terminal's font scale.
     */
    pspecs[PROP_FONT_SCALE] =
            g_param_spec_double ("font-scale", NULL, NULL,
                                    VTE_FONT_SCALE_MIN,
                                    VTE_FONT_SCALE_MAX,
                                    1.,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:encoding:
     *
     * Controls the encoding the terminal will expect data from the child to
     * be encoded with.  For certain terminal types, applications executing in the
     * terminal can change the encoding.  The default is defined by the
     * application's locale settings.
     */
    pspecs[PROP_ENCODING] =
            g_param_spec_string ("encoding", NULL, NULL,
                                    NULL,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:font-desc:
     *
     * Specifies the font used for rendering all text displayed by the terminal,
     * overriding any fonts set using gtk_widget_modify_font().  The terminal
     * will immediately attempt to load the desired font, retrieve its
     * metrics, and attempt to resize itself to keep the same number of rows
     * and columns.
     */
    pspecs[PROP_FONT_DESC] =
            g_param_spec_boxed ("font-desc", NULL, NULL,
                                PANGO_TYPE_FONT_DESCRIPTION,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:icon-title:
     *
     * The terminal's so-called icon title, or %NULL if no icon title has been set.
     */
    pspecs[PROP_ICON_TITLE] =
            g_param_spec_string ("icon-title", NULL, NULL,
                                    NULL,
                                    (GParamFlags) (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:input-enabled:
     *
     * Controls whether the terminal allows user input. When user input is disabled,
     * key press and mouse button press and motion events are not sent to the
     * terminal's child.
     */
    pspecs[PROP_INPUT_ENABLED] =
            g_param_spec_boolean ("input-enabled", NULL, NULL,
                                    TRUE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:pointer-autohide:
     *
     * Controls the value of the terminal's mouse autohide setting.  When autohiding
     * is enabled, the mouse cursor will be hidden when the user presses a key and
     * shown when the user moves the mouse.
     */
    pspecs[PROP_MOUSE_POINTER_AUTOHIDE] =
            g_param_spec_boolean ("pointer-autohide", NULL, NULL,
                                    FALSE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
    /*
     * NoVteTerminal:rewrap-on-resize:
     *
     * Controls whether or not the terminal will rewrap its contents, including
     * the scrollback buffer, whenever the terminal's width changes.
     */
    pspecs[PROP_REWRAP_ON_RESIZE] =
            g_param_spec_boolean ("rewrap-on-resize", NULL, NULL,
                                    TRUE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:scrollback-lines:
     *
     * The length of the scrollback buffer used by the terminal.  The size of
     * the scrollback buffer will be set to the larger of this value and the number
     * of visible rows the widget can display, so 0 can safely be used to disable
     * scrollback.  Note that this setting only affects the normal screen buffer.
     * For terminal types which have an alternate screen buffer, no scrollback is
     * allowed on the alternate screen buffer.
     */
    pspecs[PROP_SCROLLBACK_LINES] =
            g_param_spec_uint ("scrollback-lines", NULL, NULL,
                                0, G_MAXUINT,
                                VTE_SCROLLBACK_INIT,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:scroll-on-keystroke:
     *
     * Controls whether or not the terminal will forcibly scroll to the bottom of
     * the viewable history when the user presses a key.  Modifier keys do not
     * trigger this behavior.
     */
    pspecs[PROP_SCROLL_ON_KEYSTROKE] =
            g_param_spec_boolean ("scroll-on-keystroke", NULL, NULL,
                                    FALSE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:scroll-on-output:
     *
     * Controls whether or not the terminal will forcibly scroll to the bottom of
     * the viewable history when the new data is received from the child.
     */
    pspecs[PROP_SCROLL_ON_OUTPUT] =
            g_param_spec_boolean ("scroll-on-output", NULL, NULL,
                                    TRUE,
                                    (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

#if 0 /* FIXME: removed */
    /*
     * NoVteTerminal:text-blink-mode:
     *
     * Controls whether or not the terminal will allow blinking text.
     *
     * Since: 0.52
     */
    pspecs[PROP_TEXT_BLINK_MODE] =
            g_param_spec_enum ("text-blink-mode", NULL, NULL,
                                /* VTE_TYPE_TEXT_BLINK_MODE, */
                                VTE_TEXT_BLINK_NEVER,
                                VTE_TEXT_BLINK_ALWAYS,
                                (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));
#endif

    /*
     * NoVteTerminal:window-title:
     *
     * The terminal's title.
     */
    pspecs[PROP_WINDOW_TITLE] =
            g_param_spec_string ("window-title", NULL, NULL,
                                    NULL,
                                    (GParamFlags) (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:current-directory-uri:
     *
     * The current directory URI, or %NULL if unset.
     */
    pspecs[PROP_CURRENT_DIRECTORY_URI] =
            g_param_spec_string ("current-directory-uri", NULL, NULL,
                                    NULL,
                                    (GParamFlags) (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:current-file-uri:
     *
     * The current file URI, or %NULL if unset.
     */
    pspecs[PROP_CURRENT_FILE_URI] =
            g_param_spec_string ("current-file-uri", NULL, NULL,
                                    NULL,
                                    (GParamFlags) (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    /*
     * NoVteTerminal:hyperlink-hover-uri:
     *
     * The currently hovered hyperlink URI, or %NULL if unset.
     *
     * Since: 0.50
     */
    pspecs[PROP_HYPERLINK_HOVER_URI] =
            g_param_spec_string ("hyperlink-hover-uri", NULL, NULL,
                                    NULL,
                                    (GParamFlags) (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

#if 0 /* FIXME: GTK warnings */
    /*
     * NoVteTerminal:word-char-exceptions:
     *
     * The set of characters which will be considered parts of a word
     * when doing word-wise selection, in addition to the default which only
     * considers alphanumeric characters part of a word.
     *
     * If %NULL, a built-in set is used.
     *
     * Since: 0.40
     */
    pspecs[PROP_WORD_CHAR_EXCEPTIONS] =
            g_param_spec_string ("word-char-exceptions", NULL, NULL,
                                    NULL,
                                    (GParamFlags) (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

    g_object_class_install_properties(gobject_class, LAST_PROP, pspecs);
#endif

    /* Disable GtkWidget's keybindings except for Shift-F10 and MenuKey
     * which pop up the context menu.
     */
    binding_set = gtk_binding_set_by_class(vte_terminal_parent_class);
    gtk_binding_entry_skip(binding_set, GDK_KEY_F1, GDK_CONTROL_MASK);
    gtk_binding_entry_skip(binding_set, GDK_KEY_F1, GDK_SHIFT_MASK);
    gtk_binding_entry_skip(binding_set, GDK_KEY_KP_F1, GDK_CONTROL_MASK);
    gtk_binding_entry_skip(binding_set, GDK_KEY_KP_F1, GDK_SHIFT_MASK);

    process_timer = g_timer_new();

    klass->priv = G_TYPE_CLASS_GET_PRIVATE (klass, VTE_TYPE_TERMINAL, VteTerminalClassPrivate);

    klass->priv->style_provider = GTK_STYLE_PROVIDER (gtk_css_provider_new ());
    gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (klass->priv->style_provider),
                                        "VteTerminal, " VTE_TERMINAL_CSS_NAME " {\n"
                                        "padding: 1px 1px 1px 1px;\n"
                                        "background-color: @theme_base_color;\n"
                                        "color: @theme_fg_color;\n"
                                        "}\n",
                                        -1, NULL);

    /* a11y */
    gtk_widget_class_set_accessible_type(widget_class, VTE_TYPE_TERMINAL_ACCESSIBLE);
}

/* NoVteTerminal public API */

/**
 * novte_terminal_new:
 *
 * Creates a new terminal widget.
 *
 * Returns: (transfer none) (type Vte.Terminal): a new #NoVteTerminal object
 */
GtkWidget *novte_terminal_new(void)
{
    return (GtkWidget *)g_object_new(VTE_TYPE_TERMINAL, nullptr);
}

/**
 * novte_terminal_copy_clipboard:
 * @terminal: a #NoVteTerminal
 *
 * Places the selected text in the terminal in the #GDK_SELECTION_CLIPBOARD
 * selection.
 *
 * Deprecated: 0.50: Use vte_terminal_copy_clipboard_format() with %VTE_FORMAT_TEXT
 *   instead.
 */
void novte_terminal_copy_clipboard(NoVteTerminal *terminal)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    IMPL(terminal)->emit_copy_clipboard();
}


/**
 * vte_terminal_copy_clipboard_format:
 * @terminal: a #NoVteTerminal
 * @format: a #VteFormat
 *
 * Places the selected text in the terminal in the #GDK_SELECTION_CLIPBOARD
 * selection in the form specified by @format.
 *
 * For all formats, the selection data (see #GtkSelectionData) will include the
 * text targets (see gtk_target_list_add_text_targets() and
 * gtk_selection_data_targets_includes_text()). For %VTE_FORMAT_HTML,
 * the selection will also include the "text/html" target, which when requested,
 * returns the HTML data in UTF-16 with a U+FEFF BYTE ORDER MARK character at
 * the start.
 *
 * Since: 0.50
 */
void vte_terminal_copy_clipboard_format(NoVteTerminal *terminal,
                                   VteFormat format)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(format == VTE_FORMAT_TEXT || format == VTE_FORMAT_HTML);

    IMPL(terminal)->widget_copy(VTE_SELECTION_CLIPBOARD, format);
}

/**
 * vte_terminal_copy_primary:
 * @terminal: a #NoVteTerminal
 *
 * Places the selected text in the terminal in the #GDK_SELECTION_PRIMARY
 * selection.
 */
void vte_terminal_copy_primary(NoVteTerminal *terminal)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    _vte_debug_print(VTE_DEBUG_SELECTION, "Copying to PRIMARY.\n");
    IMPL(terminal)->widget_copy(VTE_SELECTION_PRIMARY, VTE_FORMAT_TEXT);
}

/**
 * vte_terminal_paste_clipboard:
 * @terminal: a #NoVteTerminal
 *
 * Sends the contents of the #GDK_SELECTION_CLIPBOARD selection to the
 * terminal's child.  If necessary, the data is converted from UTF-8 to the
 * terminal's current encoding. It's called on paste menu item, or when
 * user presses Shift+Insert.
 */
void vte_terminal_paste_clipboard(NoVteTerminal *terminal)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    IMPL(terminal)->emit_paste_clipboard();
}

/**
 * vte_terminal_paste_primary:
 * @terminal: a #NoVteTerminal
 *
 * Sends the contents of the #GDK_SELECTION_PRIMARY selection to the terminal's
 * child.  If necessary, the data is converted from UTF-8 to the terminal's
 * current encoding.  The terminal will call also paste the
 * #GDK_SELECTION_PRIMARY selection when the user clicks with the the second
 * mouse button.
 */
void vte_terminal_paste_primary(NoVteTerminal *terminal)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    _vte_debug_print(VTE_DEBUG_SELECTION, "Pasting PRIMARY.\n");
    IMPL(terminal)->widget_paste(GDK_SELECTION_PRIMARY);
}

/**
 * vte_terminal_hyperlink_check_event:
 * @terminal: a #NoVteTerminal
 * @event: a #GdkEvent
 *
 * Returns a nonempty string: the target of the explicit hyperlink (printed using the OSC 8
 * escape sequence) at the position of the event, or %NULL.
 *
 * Proper use of the escape sequence should result in URI-encoded URIs with a proper scheme
 * like "http://", "https://", "file://", "mailto:" etc. This is, however, not enforced by VTE.
 * The caller must tolerate the returned string potentially not being a valid URI.
 *
 * Returns: (transfer full): a newly allocated string containing the target of the hyperlink
 *
 * Since: 0.50
 */
char *vte_terminal_hyperlink_check_event(NoVteTerminal *terminal, GdkEvent *event)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->hyperlink_check(event);
}

/**
 * vte_terminal_search_get_wrap_around:
 * @terminal: a #NoVteTerminal
 *
 * Returns: whether searching will wrap around
 */
gboolean vte_terminal_search_get_wrap_around (NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);

    return IMPL(terminal)->m_search_wrap_around;
}


/**
 * vte_terminal_select_all:
 * @terminal: a #NoVteTerminal
 *
 * Selects all text within the terminal (including the scrollback buffer).
 */
void vte_terminal_select_all (NoVteTerminal *terminal)
{
    g_return_if_fail (VTE_IS_TERMINAL (terminal));

    IMPL(terminal)->select_all();
}

/**
 * vte_terminal_unselect_all:
 * @terminal: a #NoVteTerminal
 *
 * Clears the current selection.
 */
void vte_terminal_unselect_all(NoVteTerminal *terminal)
{
    g_return_if_fail (VTE_IS_TERMINAL (terminal));

    IMPL(terminal)->deselect_all();
}

/**
 * vte_terminal_get_cursor_position:
 * @terminal: a #NoVteTerminal
 * @column: (out) (allow-none): a location to store the column, or %NULL
 * @row: (out) (allow-none): a location to store the row, or %NULL
 *
 * Reads the location of the insertion cursor and returns it.  The row
 * coordinate is absolute.
 */
void vte_terminal_get_cursor_position(NoVteTerminal *terminal,
                                        long *column, long *row)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    auto impl = IMPL(terminal);
    if (column) {
        *column = impl->m_screen->cursor.col;
    }
    if (row) {
        *row = impl->m_screen->cursor.row;
    }
}

/**
 * novte_terminal_feed:
 * @terminal: a #NoVteTerminal
 * @data: (array length=length) (element-type guint8) (allow-none): a string in the terminal's current encoding
 * @length: the length of the string, or -1 to use the full length or a nul-terminated string
 *
 * Interprets @data as if it were data received from a child process.  This
 * can either be used to drive the terminal without a child process, or just
 * to mess with your users.
 */
void novte_terminal_feed(NoVteTerminal *terminal,
                            const char *data,
                            gssize length)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(length == 0 || data != NULL);

    IMPL(terminal)->feed(data, length);
}

/**
 * VteSelectionFunc:
 * @terminal: terminal in which the cell is.
 * @column: column in which the cell is.
 * @row: row in which the cell is.
 * @data: (closure): user data.
 *
 * Specifies the type of a selection function used to check whether
 * a cell has to be selected or not.
 *
 * Returns: %TRUE if cell has to be selected; %FALSE if otherwise.
 */

static void warn_if_callback(VteSelectionFunc func)
{
    if (!func) {
        return;
    }

#ifndef VTE_DEBUG
    static gboolean warned = FALSE;
    if (warned) {
        return;
    }
    warned = TRUE;
#endif
    g_warning ("VteSelectionFunc callback ignored.\n");
}

/**
 * vte_terminal_get_text:
 * @terminal: a #NoVteTerminal
 * @is_selected: (scope call) (allow-none): a #VteSelectionFunc callback
 * @user_data: (closure): user data to be passed to the callback
 * @attributes: (out caller-allocates) (transfer full) (array) (element-type Vte.CharAttributes): location for storing text attributes
 *
 * Extracts a view of the visible part of the terminal.  If @is_selected is not
 * %NULL, characters will only be read if @is_selected returns %TRUE after being
 * passed the column and row, respectively.  A #VteCharAttributes structure
 * is added to @attributes for each byte added to the returned string detailing
 * the character's position, colors, and other characteristics.
 *
 * Returns: (transfer full): a newly allocated text string, or %NULL.
 */
char *vte_terminal_get_text(NoVteTerminal *terminal,
                            VteSelectionFunc is_selected,
                            gpointer user_data,
                            GArray *attributes)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);
    warn_if_callback(is_selected);
    auto text = IMPL(terminal)->get_text_displayed(true /* wrap */,
                                                    false /* include trailing whitespace */,
                                                    attributes);
    if (text == nullptr) {
        return nullptr;
    }
    return (char*)g_string_free(text, FALSE);
}

/**
 * vte_terminal_get_text_include_trailing_spaces:
 * @terminal: a #NoVteTerminal
 * @is_selected: (scope call) (allow-none): a #VteSelectionFunc callback
 * @user_data: (closure): user data to be passed to the callback
 * @attributes: (out caller-allocates) (transfer full) (array) (element-type Vte.CharAttributes): location for storing text attributes
 *
 * Extracts a view of the visible part of the terminal.  If @is_selected is not
 * %NULL, characters will only be read if @is_selected returns %TRUE after being
 * passed the column and row, respectively.  A #VteCharAttributes structure
 * is added to @attributes for each byte added to the returned string detailing
 * the character's position, colors, and other characteristics. This function
 * differs from vte_terminal_get_text() in that trailing spaces at the end of
 * lines are included.
 *
 * Returns: (transfer full): a newly allocated text string, or %NULL.
 */
char *vte_terminal_get_text_include_trailing_spaces(NoVteTerminal *terminal,
                                                    VteSelectionFunc is_selected,
                                                    gpointer user_data,
                                                    GArray *attributes)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);
    warn_if_callback(is_selected);
    auto text = IMPL(terminal)->get_text_displayed(true /* wrap */,
                                                    true /* include trailing whitespace */,
                                                    attributes);
    if (text == nullptr) {
        return nullptr;
    }
    return (char*)g_string_free(text, FALSE);
}

/**
 * vte_terminal_get_text_range:
 * @terminal: a #NoVteTerminal
 * @start_row: first row to search for data
 * @start_col: first column to search for data
 * @end_row: last row to search for data
 * @end_col: last column to search for data
 * @is_selected: (scope call) (allow-none): a #VteSelectionFunc callback
 * @user_data: (closure): user data to be passed to the callback
 * @attributes: (out caller-allocates) (transfer full) (array) (element-type Vte.CharAttributes): location for storing text attributes
 *
 * Extracts a view of the visible part of the terminal.  If @is_selected is not
 * %NULL, characters will only be read if @is_selected returns %TRUE after being
 * passed the column and row, respectively.  A #VteCharAttributes structure
 * is added to @attributes for each byte added to the returned string detailing
 * the character's position, colors, and other characteristics.  The
 * entire scrollback buffer is scanned, so it is possible to read the entire
 * contents of the buffer using this function.
 *
 * Returns: (transfer full): a newly allocated text string, or %NULL.
 */
char *vte_terminal_get_text_range(NoVteTerminal *terminal,
                                    long start_row,
                                    long start_col,
                                    long end_row,
                                    long end_col,
                                    VteSelectionFunc is_selected,
                                    gpointer user_data,
                                    GArray *attributes)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);
    warn_if_callback(is_selected);
    auto text = IMPL(terminal)->get_text(start_row, start_col,
                                            end_row, end_col,
                                            false /* block */,
                                            true /* wrap */,
                                            true /* include trailing whitespace */,
                                            attributes);
    if (text == nullptr) {
        return nullptr;
    }
    return (char*)g_string_free(text, FALSE);
}

/**
 * vte_terminal_reset:
 * @terminal: a #NoVteTerminal
 * @clear_tabstops: whether to reset tabstops
 * @clear_history: whether to empty the terminal's scrollback buffer
 *
 * Resets as much of the terminal's internal state as possible, discarding any
 * unprocessed input data, resetting character attributes, cursor state,
 * national character set state, status line, terminal modes (insert/delete),
 * selection state, and encoding.
 *
 */
void vte_terminal_reset(NoVteTerminal *terminal,
                        gboolean clear_tabstops,
                        gboolean clear_history)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    IMPL(terminal)->reset(clear_tabstops, clear_history, true);
}

/**
 * vte_terminal_set_size:
 * @terminal: a #NoVteTerminal
 * @columns: the desired number of columns
 * @rows: the desired number of rows
 *
 * Attempts to change the terminal's size in terms of rows and columns.  If
 * the attempt succeeds, the widget will resize itself to the proper size.
 */
void vte_terminal_set_size(NoVteTerminal *terminal,
                            long columns,
                            long rows)
{
    g_return_if_fail(columns >= 1);
    g_return_if_fail(rows >= 1);

    IMPL(terminal)->set_size(columns, rows);
}

/**
 * vte_terminal_get_text_blink_mode:
 * @terminal: a #NoVteTerminal
 *
 * Checks whether or not the terminal will allow blinking text.
 *
 * Returns: the blinking setting
 *
 * Since: 0.52
 */
VteTextBlinkMode vte_terminal_get_text_blink_mode(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), VTE_TEXT_BLINK_ALWAYS);
    return IMPL(terminal)->m_text_blink_mode;
}

/**
 * vte_terminal_set_text_blink_mode:
 * @terminal: a #NoVteTerminal
 * @text_blink_mode: the #VteTextBlinkMode to use
 *
 * Controls whether or not the terminal will allow blinking text.
 *
 * Since: 0.52
 */
void vte_terminal_set_text_blink_mode(NoVteTerminal *terminal,
                                     VteTextBlinkMode text_blink_mode)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_text_blink_mode(text_blink_mode)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_TEXT_BLINK_MODE]);
    }
}

/**
 * vte_terminal_get_allow_bold:
 * @terminal: a #NoVteTerminal
 *
 * Checks whether or not the terminal will attempt to draw bold text,
 * either by using a bold font variant or by repainting text with a different
 * offset.
 *
 * Returns: %TRUE if bolding is enabled, %FALSE if not
 */
gboolean vte_terminal_get_allow_bold(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_allow_bold;
}

/**
 * vte_terminal_set_allow_bold:
 * @terminal: a #NoVteTerminal
 * @allow_bold: %TRUE if the terminal should attempt to draw bold text
 *
 * Controls whether or not the terminal will attempt to draw bold text,
 * either by using a bold font variant or by repainting text with a different
 * offset.
 */
void vte_terminal_set_allow_bold(NoVteTerminal *terminal,
                            gboolean allow_bold)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_allow_bold(allow_bold != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_ALLOW_BOLD]);
    }
}

/**
 * vte_terminal_get_allow_hyperlink:
 * @terminal: a #NoVteTerminal
 *
 * Checks whether or not hyperlinks (OSC 8 escape sequence) are allowed.
 *
 * Returns: %TRUE if hyperlinks are enabled, %FALSE if not
 *
 * Since: 0.50
 */
gboolean vte_terminal_get_allow_hyperlink(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_allow_hyperlink;
}

/**
 * vte_terminal_set_allow_hyperlink:
 * @terminal: a #NoVteTerminal
 * @allow_hyperlink: %TRUE if the terminal should allow hyperlinks
 *
 * Controls whether or not hyperlinks (OSC 8 escape sequence) are allowed.
 *
 * Since: 0.50
 */
void vte_terminal_set_allow_hyperlink(NoVteTerminal *terminal,
                                 gboolean allow_hyperlink)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_allow_hyperlink(allow_hyperlink != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_ALLOW_HYPERLINK]);
    }
}

/**
 * vte_terminal_get_audible_bell:
 * @terminal: a #NoVteTerminal
 *
 * Checks whether or not the terminal will beep when the child outputs the
 * "bl" sequence.
 *
 * Returns: %TRUE if audible bell is enabled, %FALSE if not
 */
gboolean vte_terminal_get_audible_bell(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_audible_bell;
}

/**
 * vte_terminal_set_audible_bell:
 * @terminal: a #NoVteTerminal
 * @is_audible: %TRUE if the terminal should beep
 *
 * Controls whether or not the terminal will beep when the child outputs the
 * "bl" sequence.
 */
void vte_terminal_set_audible_bell(NoVteTerminal *terminal, gboolean is_audible)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_audible_bell(is_audible != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_AUDIBLE_BELL]);
    }
}

#if 0 /* FIXME: removed */
/**
 * vte_terminal_set_backspace_binding:
 * @terminal: a #NoVteTerminal
 * @binding: a #VteEraseBinding for the backspace key
 *
 * Modifies the terminal's backspace key binding, which controls what
 * string or control sequence the terminal sends to its child when the user
 * presses the backspace key.
 */
void
vte_terminal_set_backspace_binding(NoVteTerminal *terminal,
                                   VteEraseBinding binding)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
        g_return_if_fail(binding >= VTE_ERASE_AUTO && binding <= VTE_ERASE_TTY);

        if (IMPL(terminal)->set_backspace_binding(binding))
                g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_BACKSPACE_BINDING]);
}
#endif

/**
 * vte_terminal_get_bold_is_bright:
 * @terminal: a #NoVteTerminal
 *
 * Checks whether the SGR 1 attribute also switches to the bright counterpart
 * of the first 8 palette colors, in addition to making them bold (legacy behavior)
 * or if SGR 1 only enables bold and leaves the color intact.
 *
 * Returns: %TRUE if bold also enables bright, %FALSE if not
 *
 * Since: 0.52
 */
gboolean vte_terminal_get_bold_is_bright(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_bold_is_bright;
}
/**
 * vte_terminal_set_bold_is_bright:
 * @terminal: a #NoVteTerminal
 * @bold_is_bright: %TRUE if bold should also enable bright
 *
 * Sets whether the SGR 1 attribute also switches to the bright counterpart
 * of the first 8 palette colors, in addition to making them bold (legacy behavior)
 * or if SGR 1 only enables bold and leaves the color intact.
 *
 * Since: 0.52
 */
void vte_terminal_set_bold_is_bright(NoVteTerminal *terminal,
                                gboolean bold_is_bright)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_bold_is_bright(bold_is_bright != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_BOLD_IS_BRIGHT]);
    }
}

/**
 * novte_terminal_get_char_height:
 * @terminal: a #NoVteTerminal
 *
 * Returns: the height of a character cell
 *
 * Note that this method should rather be called vte_terminal_get_cell_height,
 * because the return value takes cell-height-scale into account.
 */
glong novte_terminal_get_char_height(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), -1);
    return IMPL(terminal)->get_cell_height();
}

/**
 * novte_terminal_get_char_width:
 * @terminal: a #NoVteTerminal
 *
 * Returns: the width of a character cell
 *
 * Note that this method should rather be called vte_terminal_get_cell_width,
 * because the return value takes cell-width-scale into account.
 */
glong novte_terminal_get_char_width(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), -1);
    return IMPL(terminal)->get_cell_width();
}

/**
 * vte_terminal_get_cjk_ambiguous_width:
 * @terminal: a #NoVteTerminal
 *
 * Returns whether ambiguous-width characters are narrow or wide when using
 * the UTF-8 encoding (vte_terminal_set_encoding()).
 *
 * Returns: 1 if ambiguous-width characters are narrow, or 2 if they are wide
 */
int vte_terminal_get_cjk_ambiguous_width(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), 1);
    return IMPL(terminal)->m_utf8_ambiguous_width;
}

/**
 * vte_terminal_set_cjk_ambiguous_width:
 * @terminal: a #NoVteTerminal
 * @width: either 1 (narrow) or 2 (wide)
 *
 * This setting controls whether ambiguous-width characters are narrow or wide
 * when using the UTF-8 encoding (vte_terminal_set_encoding()). In all other encodings,
 * the width of ambiguous-width characters is fixed.
 */
void vte_terminal_set_cjk_ambiguous_width(NoVteTerminal *terminal, int width)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(width == 1 || width == 2);

    if (IMPL(terminal)->set_cjk_ambiguous_width(width)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_CJK_AMBIGUOUS_WIDTH]);
    }
}

/**
 * vte_terminal_set_color_background:
 * @terminal: a #NoVteTerminal
 * @background: the new background color
 *
 * Sets the background color for text which does not have a specific background
 * color assigned.  Only has effect when no background image is set and when
 * the terminal is not transparent.
 */
void vte_terminal_set_color_background(NoVteTerminal *terminal,
                                  const GdkRGBA *background)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(background != NULL);
    g_return_if_fail(valid_color(background));

    auto impl = IMPL(terminal);
    impl->set_color_background(vte::color::rgb(background));
    impl->set_background_alpha(background->alpha);
}

/**
 * vte_terminal_set_color_bold:
 * @terminal: a #NoVteTerminal
 * @bold: (allow-none): the new bold color or %NULL
 *
 * Sets the color used to draw bold text in the default foreground color.
 * If @bold is %NULL then the default color is used.
 */
void vte_terminal_set_color_bold(NoVteTerminal *terminal,
                                    const GdkRGBA *bold)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(bold == nullptr || valid_color(bold));

    auto impl = IMPL(terminal);
    if (bold) {
        impl->set_color_bold(vte::color::rgb(bold));
    } else {
        impl->reset_color_bold();
    }
}

/**
 * vte_terminal_set_color_cursor:
 * @terminal: a #NoVteTerminal
 * @cursor_background: (allow-none): the new color to use for the text cursor, or %NULL
 *
 * Sets the background color for text which is under the cursor.  If %NULL, text
 * under the cursor will be drawn with foreground and background colors
 * reversed.
 */
void vte_terminal_set_color_cursor(NoVteTerminal *terminal,
                                    const GdkRGBA *cursor_background)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(cursor_background == nullptr || valid_color(cursor_background));

    auto impl = IMPL(terminal);
    if (cursor_background) {
        impl->set_color_cursor_background(vte::color::rgb(cursor_background));
    } else {
        impl->reset_color_cursor_background();
    }
}

/**
 * vte_terminal_set_color_cursor_foreground:
 * @terminal: a #NoVteTerminal
 * @cursor_foreground: (allow-none): the new color to use for the text cursor, or %NULL
 *
 * Sets the foreground color for text which is under the cursor.  If %NULL, text
 * under the cursor will be drawn with foreground and background colors
 * reversed.
 *
 * Since: 0.44
 */
void vte_terminal_set_color_cursor_foreground(NoVteTerminal *terminal,
                                         const GdkRGBA *cursor_foreground)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(cursor_foreground == nullptr || valid_color(cursor_foreground));

    auto impl = IMPL(terminal);
    if (cursor_foreground) {
        impl->set_color_cursor_foreground(vte::color::rgb(cursor_foreground));
    } else {
        impl->reset_color_cursor_foreground();
    }
}

/**
 * vte_terminal_set_color_foreground:
 * @terminal: a #NoVteTerminal
 * @foreground: the new foreground color
 *
 * Sets the foreground color used to draw normal text.
 */
void vte_terminal_set_color_foreground(NoVteTerminal *terminal, const GdkRGBA *foreground)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(foreground != nullptr);
    g_return_if_fail(valid_color(foreground));

    IMPL(terminal)->set_color_foreground(vte::color::rgb(foreground));
}

/**
 * vte_terminal_set_color_highlight:
 * @terminal: a #NoVteTerminal
 * @highlight_background: (allow-none): the new color to use for highlighted text, or %NULL
 *
 * Sets the background color for text which is highlighted.  If %NULL,
 * it is unset.  If neither highlight background nor highlight foreground are set,
 * highlighted text (which is usually highlighted because it is selected) will
 * be drawn with foreground and background colors reversed.
 */
void vte_terminal_set_color_highlight(NoVteTerminal *terminal,
                                        const GdkRGBA *highlight_background)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(highlight_background == nullptr || valid_color(highlight_background));

    auto impl = IMPL(terminal);
    if (highlight_background) {
        impl->set_color_highlight_background(vte::color::rgb(highlight_background));
    } else {
        impl->reset_color_highlight_background();
    }
}

/**
 * vte_terminal_set_color_highlight_foreground:
 * @terminal: a #NoVteTerminal
 * @highlight_foreground: (allow-none): the new color to use for highlighted text, or %NULL
 *
 * Sets the foreground color for text which is highlighted.  If %NULL,
 * it is unset.  If neither highlight background nor highlight foreground are set,
 * highlighted text (which is usually highlighted because it is selected) will
 * be drawn with foreground and background colors reversed.
 */
void vte_terminal_set_color_highlight_foreground(NoVteTerminal *terminal,
                                            const GdkRGBA *highlight_foreground)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(highlight_foreground == nullptr || valid_color(highlight_foreground));

    auto impl = IMPL(terminal);
    if (highlight_foreground) {
        impl->set_color_highlight_foreground(vte::color::rgb(highlight_foreground));
    } else {
        impl->reset_color_highlight_foreground();
    }
}

/**
 * vte_terminal_set_colors:
 * @terminal: a #NoVteTerminal
 * @foreground: (allow-none): the new foreground color, or %NULL
 * @background: (allow-none): the new background color, or %NULL
 * @palette: (array length=palette_size zero-terminated=0) (element-type Gdk.RGBA) (allow-none): the color palette
 * @palette_size: the number of entries in @palette
 *
 * @palette specifies the new values for the 256 palette colors: 8 standard colors,
 * their 8 bright counterparts, 6x6x6 color cube, and 24 grayscale colors.
 * Omitted entries will default to a hardcoded value.
 *
 * @palette_size must be 0, 8, 16, 232 or 256.
 *
 * If @foreground is %NULL and @palette_size is greater than 0, the new foreground
 * color is taken from @palette[7].  If @background is %NULL and @palette_size is
 * greater than 0, the new background color is taken from @palette[0].
 */
void vte_terminal_set_colors(NoVteTerminal *terminal,
                                const GdkRGBA *foreground,
                                const GdkRGBA *background,
                                const GdkRGBA *palette,
                                gsize palette_size)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail((palette_size == 0) ||
                        (palette_size == 8) ||
                        (palette_size == 16) ||
                        (palette_size == 232) ||
                        (palette_size == 256));
    g_return_if_fail(foreground == nullptr || valid_color(foreground));
    g_return_if_fail(background == nullptr || valid_color(background));
    for (gsize i = 0; i < palette_size; ++i) {
        g_return_if_fail(valid_color(&palette[i]));
    }

    vte::color::rgb fg;
    if (foreground) {
        fg = vte::color::rgb(foreground);
    }
    vte::color::rgb bg;
    if (background) {
        bg = vte::color::rgb(background);
    }

    vte::color::rgb* pal = nullptr;
    if (palette_size) {
        pal = g_new0(vte::color::rgb, palette_size);
        for (gsize i = 0; i < palette_size; ++i) {
            pal[i] = vte::color::rgb(palette[i]);
        }
    }

    auto impl = IMPL(terminal);
    impl->set_colors(foreground ? &fg : nullptr,
                        background ? &bg : nullptr,
                        pal, palette_size);
    impl->set_background_alpha(background ? background->alpha : 1.0);
    g_free(pal);
}

/**
 * vte_terminal_set_default_colors:
 * @terminal: a #NoVteTerminal
 *
 * Reset the terminal palette to reasonable compiled-in default color.
 */
void vte_terminal_set_default_colors(NoVteTerminal *terminal)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    IMPL(terminal)->set_colors_default();
}

/**
 * vte_terminal_get_column_count:
 * @terminal: a #NoVteTerminal
 *
 * Returns: the number of columns
 */
glong novte_terminal_get_column_count(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), -1);
    return IMPL(terminal)->m_column_count;
}

/**
 * vte_terminal_get_current_directory_uri:
 * @terminal: a #NoVteTerminal
 *
 * Returns: (transfer none): the URI of the current directory of the
 *   process running in the terminal, or %NULL
 */
const char *vte_terminal_get_current_directory_uri(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);
    return IMPL(terminal)->m_current_directory_uri;
}

/**
 * vte_terminal_get_current_file_uri:
 * @terminal: a #NoVteTerminal
 *
 * Returns: (transfer none): the URI of the current file the
 *   process running in the terminal is operating on, or %NULL if
 *   not set
 */
const char *vte_terminal_get_current_file_uri(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);
    return IMPL(terminal)->m_current_file_uri;
}

/**
 * vte_terminal_get_cursor_blink_mode:
 * @terminal: a #NoVteTerminal
 *
 * Returns the currently set cursor blink mode.
 *
 * Return value: cursor blink mode.
 */
VteCursorBlinkMode vte_terminal_get_cursor_blink_mode(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), VTE_CURSOR_BLINK_SYSTEM);

    return IMPL(terminal)->m_cursor_blink_mode;
}

/**
 * vte_terminal_set_cursor_blink_mode:
 * @terminal: a #NoVteTerminal
 * @mode: the #VteCursorBlinkMode to use
 *
 * Sets whether or not the cursor will blink. Using %VTE_CURSOR_BLINK_SYSTEM
 * will use the #GtkSettings::gtk-cursor-blink setting.
 */
void vte_terminal_set_cursor_blink_mode(NoVteTerminal *terminal,
                                        VteCursorBlinkMode mode)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(mode >= VTE_CURSOR_BLINK_SYSTEM && mode <= VTE_CURSOR_BLINK_OFF);

    if (IMPL(terminal)->set_cursor_blink_mode(mode)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_CURSOR_BLINK_MODE]);
    }
}

/**
 * vte_terminal_get_cursor_shape:
 * @terminal: a #NoVteTerminal
 *
 * Returns the currently set cursor shape.
 *
 * Return value: cursor shape.
 */
VteCursorShape vte_terminal_get_cursor_shape(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), VTE_CURSOR_SHAPE_BLOCK);

    return IMPL(terminal)->m_cursor_shape;
}

/**
 * vte_terminal_set_cursor_shape:
 * @terminal: a #NoVteTerminal
 * @shape: the #VteCursorShape to use
 *
 * Sets the shape of the cursor drawn.
 */
void vte_terminal_set_cursor_shape(NoVteTerminal *terminal, VteCursorShape shape)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(shape >= VTE_CURSOR_SHAPE_BLOCK && shape <= VTE_CURSOR_SHAPE_UNDERLINE);

    if (IMPL(terminal)->set_cursor_shape(shape)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_CURSOR_SHAPE]);
    }
}

#if 0 /* FIXME: removed */
/**
 * vte_terminal_set_delete_binding:
 * @terminal: a #NoVteTerminal
 * @binding: a #VteEraseBinding for the delete key
 *
 * Modifies the terminal's delete key binding, which controls what
 * string or control sequence the terminal sends to its child when the user
 * presses the delete key.
 */
void
vte_terminal_set_delete_binding(NoVteTerminal *terminal,
                                VteEraseBinding binding)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
        g_return_if_fail(binding >= VTE_ERASE_AUTO && binding <= VTE_ERASE_TTY);

        if (IMPL(terminal)->set_delete_binding(binding))
                g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_DELETE_BINDING]);
}
#endif

/**
 * vte_terminal_get_encoding:
 * @terminal: a #NoVteTerminal
 *
 * Determines the name of the encoding in which the terminal expects data to be
 * encoded.
 *
 * Returns: (transfer none): the current encoding for the terminal
 */
const char *vte_terminal_get_encoding(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);
    return IMPL(terminal)->m_encoding;
}

/**
 * vte_terminal_set_encoding:
 * @terminal: a #NoVteTerminal
 * @codeset: (allow-none): a valid #GIConv target, or %NULL to use UTF-8
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Changes the encoding the terminal will expect data from the child to
 * be encoded with.  For certain terminal types, applications executing in the
 * terminal can change the encoding. If @codeset is %NULL, it uses "UTF-8".
 *
 * Returns: %TRUE if the encoding could be changed to the specified one,
 *  or %FALSE with @error set to %G_CONVERT_ERROR_NO_CONVERSION.
 */
gboolean vte_terminal_set_encoding(NoVteTerminal *terminal,
                                    const char *codeset,
                                    GError **error)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    GObject *object = G_OBJECT(terminal);
    g_object_freeze_notify(object);

    bool rv = IMPL(terminal)->set_encoding(codeset);
    if (rv) {
        g_object_notify_by_pspec(object, pspecs[PROP_ENCODING]);
    } else {
        g_set_error(error, G_CONVERT_ERROR, G_CONVERT_ERROR_NO_CONVERSION,
                    "Unable to convert characters from %s to %s.",
                    "UTF-8", codeset);
    }
    g_object_thaw_notify(object);
    return rv;
}

/**
 * vte_terminal_get_font:
 * @terminal: a #NoVteTerminal
 *
 * Queries the terminal for information about the fonts which will be
 * used to draw text in the terminal.  The actual font takes the font scale
 * into account, this is not reflected in the return value, the unscaled
 * font is returned.
 *
 * Returns: (transfer none): a #PangoFontDescription describing the font the
 * terminal uses to render text at the default font scale of 1.0.
 */
const PangoFontDescription *vte_terminal_get_font(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);

    return IMPL(terminal)->m_unscaled_font_desc;
}

/**
 * vte_terminal_set_font:
 * @terminal: a #NoVteTerminal
 * @font_desc: (allow-none): a #PangoFontDescription for the desired font, or %NULL
 *
 * Sets the font used for rendering all text displayed by the terminal,
 * overriding any fonts set using gtk_widget_modify_font().  The terminal
 * will immediately attempt to load the desired font, retrieve its
 * metrics, and attempt to resize itself to keep the same number of rows
 * and columns.  The font scale is applied to the specified font.
 */
void vte_terminal_set_font(NoVteTerminal *terminal,
                            const PangoFontDescription *font_desc)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_font_desc(font_desc)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_FONT_DESC]);
    }
}

/**
 * vte_terminal_get_font_scale:
 * @terminal: a #NoVteTerminal
 *
 * Returns: the terminal's font scale
 */
gdouble vte_terminal_get_font_scale(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), 1.);

    return IMPL(terminal)->m_font_scale;
}

/**
 * vte_terminal_set_font_scale:
 * @terminal: a #NoVteTerminal
 * @scale: the font scale
 *
 * Sets the terminal's font scale to @scale.
 */
void vte_terminal_set_font_scale(NoVteTerminal *terminal,
                            double scale)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    scale = CLAMP(scale, VTE_FONT_SCALE_MIN, VTE_FONT_SCALE_MAX);
    if (IMPL(terminal)->set_font_scale(scale)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_FONT_SCALE]);
    }
}

/**
 * vte_terminal_get_cell_height_scale:
 * @terminal: a #NoVteTerminal
 *
 * Returns: the terminal's cell height scale
 *
 * Since: 0.52
 */
double vte_terminal_get_cell_height_scale(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), 1.);

    return IMPL(terminal)->m_cell_height_scale;
}

/**
 * vte_terminal_set_cell_height_scale:
 * @terminal: a #NoVteTerminal
 * @scale: the cell height scale
 *
 * Sets the terminal's cell height scale to @scale.
 *
 * This can be used to increase the line spacing. (The font's height is not affected.)
 * Valid values go from 1.0 (default) to 2.0 ("double spacing").
 *
 * Since: 0.52
 */
void vte_terminal_set_cell_height_scale(NoVteTerminal *terminal,
                                   double scale)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    scale = CLAMP(scale, VTE_CELL_SCALE_MIN, VTE_CELL_SCALE_MAX);
    if (IMPL(terminal)->set_cell_height_scale(scale)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_CELL_HEIGHT_SCALE]);
    }
}

/**
 * vte_terminal_get_cell_width_scale:
 * @terminal: a #NoVteTerminal
 *
 * Returns: the terminal's cell width scale
 *
 * Since: 0.52
 */
double vte_terminal_get_cell_width_scale(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), 1.);

    return IMPL(terminal)->m_cell_width_scale;
}

/**
 * vte_terminal_set_cell_width_scale:
 * @terminal: a #NoVteTerminal
 * @scale: the cell width scale
 *
 * Sets the terminal's cell width scale to @scale.
 *
 * This can be used to increase the letter spacing. (The font's width is not affected.)
 * Valid values go from 1.0 (default) to 2.0.
 *
 * Since: 0.52
 */
void vte_terminal_set_cell_width_scale(NoVteTerminal *terminal,
                                  double scale)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    scale = CLAMP(scale, VTE_CELL_SCALE_MIN, VTE_CELL_SCALE_MAX);
    if (IMPL(terminal)->set_cell_width_scale(scale)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_CELL_WIDTH_SCALE]);
    }
}

/* Just some arbitrary minimum values */
#define MIN_COLUMNS (16)
#define MIN_ROWS    (2)

/**
 * vte_terminal_get_geometry_hints:
 * @terminal: a #NoVteTerminal
 * @hints: (out caller-allocates): a #GdkGeometry to fill in
 * @min_rows: the minimum number of rows to request
 * @min_columns: the minimum number of columns to request
 *
 * Fills in some @hints from @terminal's geometry. The hints
 * filled are those covered by the %GDK_HINT_RESIZE_INC,
 * %GDK_HINT_MIN_SIZE and %GDK_HINT_BASE_SIZE flags.
 *
 * See gtk_window_set_geometry_hints() for more information.
 *
 * @terminal must be realized (see gtk_widget_get_realized()).
 *
 * Deprecated: 0.52
 */
void vte_terminal_get_geometry_hints(NoVteTerminal *terminal,
                                        GdkGeometry *hints,
                                        int min_rows,
                                        int min_columns)
{
    GtkWidget *widget;
    GtkBorder padding;

    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(hints != NULL);
    widget = &terminal->widget;
    g_return_if_fail(gtk_widget_get_realized(widget));

    auto impl = IMPL(terminal);

    auto context = gtk_widget_get_style_context(widget);
    gtk_style_context_get_padding(context, gtk_style_context_get_state(context),
                                    &padding);

    hints->base_width  = padding.left + padding.right;
    hints->base_height = padding.top  + padding.bottom;
    hints->width_inc   = impl->m_cell_width;
    hints->height_inc  = impl->m_cell_height;
    hints->min_width   = hints->base_width  + hints->width_inc  * min_columns;
    hints->min_height  = hints->base_height + hints->height_inc * min_rows;

    _vte_debug_print(VTE_DEBUG_WIDGET_SIZE,
                         "[Terminal %p] Geometry cell       width %ld height %ld\n"
                         "                       base       width %d height %d\n"
                         "                       increments width %d height %d\n"
                         "                       minimum    width %d height %d\n",
                         terminal,
                         impl->m_cell_width, impl->m_cell_height,
                         hints->base_width, hints->base_height,
                         hints->width_inc, hints->height_inc,
                         hints->min_width, hints->min_height);
}

/**
 * vte_terminal_set_geometry_hints_for_window:
 * @terminal: a #NoVteTerminal
 * @window: a #GtkWindow
 *
 * Sets @terminal as @window's geometry widget. See
 * gtk_window_set_geometry_hints() for more information.
 *
 * @terminal must be realized (see gtk_widget_get_realized()).
 *
 * Deprecated: 0.52
 */
void vte_terminal_set_geometry_hints_for_window(NoVteTerminal *terminal,
                                           GtkWindow *window)
{
    GdkGeometry hints;

    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(gtk_widget_get_realized(&terminal->widget));

    vte_terminal_get_geometry_hints(terminal, &hints, MIN_ROWS, MIN_COLUMNS);
    gtk_window_set_geometry_hints(window,
#if GTK_CHECK_VERSION (3, 19, 5)
                                    NULL,
#else
                                    &terminal->widget,
#endif
                                    &hints,
                                    (GdkWindowHints)(GDK_HINT_RESIZE_INC |
                                                    GDK_HINT_MIN_SIZE |
                                                    GDK_HINT_BASE_SIZE));
}

/**
 * vte_terminal_get_has_selection:
 * @terminal: a #NoVteTerminal
 *
 * Checks if the terminal currently contains selected text.  Note that this
 * is different from determining if the terminal is the owner of any
 * #GtkClipboard items.
 *
 * Returns: %TRUE if part of the text in the terminal is selected.
 */
gboolean vte_terminal_get_has_selection(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_has_selection;
}

/**
 * vte_terminal_get_icon_title:
 * @terminal: a #NoVteTerminal
 *
 * Returns: (transfer none): the icon title
 */
const char *vte_terminal_get_icon_title(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), "");
    return IMPL(terminal)->m_icon_title;
}

/**
 * vte_terminal_get_input_enabled:
 * @terminal: a #NoVteTerminal
 *
 * Returns whether the terminal allow user input.
 */
gboolean vte_terminal_get_input_enabled (NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);

    return IMPL(terminal)->m_input_enabled;
}

/**
 * vte_terminal_set_input_enabled:
 * @terminal: a #NoVteTerminal
 * @enabled: whether to enable user input
 *
 * Enables or disables user input. When user input is disabled,
 * the terminal's child will not receive any key press, or mouse button
 * press or motion events sent to it.
 */
void vte_terminal_set_input_enabled (NoVteTerminal *terminal,
                                        gboolean enabled)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_input_enabled(enabled != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_INPUT_ENABLED]);
    }
}

/**
 * vte_terminal_get_mouse_autohide:
 * @terminal: a #NoVteTerminal
 *
 * Determines the value of the terminal's mouse autohide setting.  When
 * autohiding is enabled, the mouse cursor will be hidden when the user presses
 * a key and shown when the user moves the mouse.  This setting can be changed
 * using vte_terminal_set_mouse_autohide().
 *
 * Returns: %TRUE if autohiding is enabled, %FALSE if not
 */
gboolean vte_terminal_get_mouse_autohide(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_mouse_autohide;
}

/**
 * vte_terminal_set_mouse_autohide:
 * @terminal: a #NoVteTerminal
 * @setting: whether the mouse pointer should autohide
 *
 * Changes the value of the terminal's mouse autohide setting.  When autohiding
 * is enabled, the mouse cursor will be hidden when the user presses a key and
 * shown when the user moves the mouse.  This setting can be read using
 * vte_terminal_get_mouse_autohide().
 */
void vte_terminal_set_mouse_autohide(NoVteTerminal *terminal, gboolean setting)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_mouse_autohide(setting != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_MOUSE_POINTER_AUTOHIDE]);
    }
}

/**
 * vte_terminal_get_rewrap_on_resize:
 * @terminal: a #NoVteTerminal
 *
 * Checks whether or not the terminal will rewrap its contents upon resize.
 *
 * Returns: %TRUE if rewrapping is enabled, %FALSE if not
 */
gboolean vte_terminal_get_rewrap_on_resize(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_rewrap_on_resize;
}

/**
 * vte_terminal_set_rewrap_on_resize:
 * @terminal: a #NoVteTerminal
 * @rewrap: %TRUE if the terminal should rewrap on resize
 *
 * Controls whether or not the terminal will rewrap its contents, including
 * the scrollback history, whenever the terminal's width changes.
 */
void vte_terminal_set_rewrap_on_resize(NoVteTerminal *terminal, gboolean rewrap)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_rewrap_on_resize(rewrap != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_REWRAP_ON_RESIZE]);
    }
}

/**
 * novte_terminal_get_row_count:
 * @terminal: a #NoVteTerminal
 *
 *
 * Returns: the number of rows
 */
glong novte_terminal_get_row_count(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), -1);
    return IMPL(terminal)->m_row_count;
}

/**
 * novte_terminal_set_scrollback_lines:
 * @terminal: a #NoVteTerminal
 * @lines: the length of the history buffer
 *
 * Sets the length of the scrollback buffer used by the terminal.  The size of
 * the scrollback buffer will be set to the larger of this value and the number
 * of visible rows the widget can display, so 0 can safely be used to disable
 * scrollback.
 *
 * A negative value means "infinite scrollback".
 *
 * Note that this setting only affects the normal screen buffer.
 * No scrollback is allowed on the alternate screen buffer.
 */
void novte_terminal_set_scrollback_lines(NoVteTerminal *terminal, glong lines)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));
    g_return_if_fail(lines >= -1);

    GObject *object = G_OBJECT(terminal);
    g_object_freeze_notify(object);

    if (IMPL(terminal)->set_scrollback_lines(lines)) {
        g_object_notify_by_pspec(object, pspecs[PROP_SCROLLBACK_LINES]);
    }

    g_object_thaw_notify(object);
}

/**
 * vte_terminal_get_scrollback_lines:
 * @terminal: a #NoVteTerminal
 *
 * Returns: length of the scrollback buffer used by the terminal.
 * A negative value means "infinite scrollback".
 *
 * Since: 0.52
 */
glong vte_terminal_get_scrollback_lines(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), 0);
    return IMPL(terminal)->m_scrollback_lines;
}

/**
 * vte_terminal_set_scroll_on_keystroke:
 * @terminal: a #NoVteTerminal
 * @scroll: whether the terminal should scroll on keystrokes
 *
 * Controls whether or not the terminal will forcibly scroll to the bottom of
 * the viewable history when the user presses a key.  Modifier keys do not
 * trigger this behavior.
 */
void vte_terminal_set_scroll_on_keystroke(NoVteTerminal *terminal,
                                     gboolean scroll)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_scroll_on_keystroke(scroll != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_SCROLL_ON_OUTPUT]);
    }
}

/**
 * vte_terminal_get_scroll_on_keystroke:
 * @terminal: a #NoVteTerminal
 *
 * Returns: whether or not the terminal will forcibly scroll to the bottom of
 * the viewable history when the user presses a key.  Modifier keys do not
 * trigger this behavior.
 *
 * Since: 0.52
 */
gboolean vte_terminal_get_scroll_on_keystroke(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_scroll_on_keystroke;
}

/**
 * novte_terminal_set_scroll_on_output:
 * @terminal: a #NoVteTerminal
 * @scroll: whether the terminal should scroll on output
 *
 * Controls whether or not the terminal will forcibly scroll to the bottom of
 * the viewable history when the new data is received from the child.
 */
void novte_terminal_set_scroll_on_output(NoVteTerminal *terminal,
                                  gboolean scroll)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_scroll_on_output(scroll != FALSE)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_SCROLL_ON_OUTPUT]);
    }
}

/**
 * vte_terminal_get_scroll_on_output:
 * @terminal: a #NoVteTerminal
 *
 * Returns: whether or not the terminal will forcibly scroll to the bottom of
 * the viewable history when the new data is received from the child.
 *
 * Since: 0.52
 */
gboolean vte_terminal_get_scroll_on_output(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    return IMPL(terminal)->m_scroll_on_output;
}

/**
 * vte_terminal_get_window_title:
 * @terminal: a #NoVteTerminal
 *
 * Returns: (transfer none): the window title
 */
const char *vte_terminal_get_window_title(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), "");
    return IMPL(terminal)->m_window_title;
}

/**
 * vte_terminal_get_word_char_exceptions:
 * @terminal: a #NoVteTerminal
 *
 * Returns the set of characters which will be considered parts of a word
 * when doing word-wise selection, in addition to the default which only
 * considers alphanumeric characters part of a word.
 *
 * If %NULL, a built-in set is used.
 *
 * Returns: (transfer none): a string, or %NULL
 *
 * Since: 0.40
 */
const char *vte_terminal_get_word_char_exceptions(NoVteTerminal *terminal)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), NULL);

    return IMPL(terminal)->m_word_char_exceptions_string;
}

/**
 * vte_terminal_set_word_char_exceptions:
 * @terminal: a #NoVteTerminal
 * @exceptions: a string of ASCII punctuation characters, or %NULL
 *
 * With this function you can provide a set of characters which will
 * be considered parts of a word when doing word-wise selection, in
 * addition to the default which only considers alphanumeric characters
 * part of a word.
 *
 * The characters in @exceptions must be non-alphanumeric, each character
 * must occur only once, and if @exceptions contains the character
 * U+002D HYPHEN-MINUS, it must be at the start of the string.
 *
 * Use %NULL to reset the set of exception characters to the default.
 *
 * Since: 0.40
 */
void vte_terminal_set_word_char_exceptions(NoVteTerminal *terminal,
                                      const char *exceptions)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    if (IMPL(terminal)->set_word_char_exceptions(exceptions)) {
        g_object_notify_by_pspec(G_OBJECT(terminal), pspecs[PROP_WORD_CHAR_EXCEPTIONS]);
    }
}

/**
 * vte_terminal_write_contents_sync:
 * @terminal: a #NoVteTerminal
 * @stream: a #GOutputStream to write to
 * @flags: a set of #VteWriteFlags
 * @cancellable: (allow-none): a #GCancellable object, or %NULL
 * @error: (allow-none): a #GError location to store the error occuring, or %NULL
 *
 * Write contents of the current contents of @terminal (including any
 * scrollback history) to @stream according to @flags.
 *
 * If @cancellable is not %NULL, then the operation can be cancelled by triggering
 * the cancellable object from another thread. If the operation was cancelled,
 * the error %G_IO_ERROR_CANCELLED will be returned in @error.
 *
 * This is a synchronous operation and will make the widget (and input
 * processing) during the write operation, which may take a long time
 * depending on scrollback history and @stream availability for writing.
 *
 * Returns: %TRUE on success, %FALSE if there was an error
 */
gboolean vte_terminal_write_contents_sync (NoVteTerminal *terminal,
                                            GOutputStream *stream,
                                            VteWriteFlags flags,
                                            GCancellable *cancellable,
                                            GError **error)
{
    g_return_val_if_fail(VTE_IS_TERMINAL(terminal), FALSE);
    g_return_val_if_fail(G_IS_OUTPUT_STREAM(stream), FALSE);

    return IMPL(terminal)->write_contents_sync(stream, flags, cancellable, error);
}

/**
 * vte_terminal_set_clear_background:
 * @terminal: a #NoVteTerminal
 * @setting:
 *
 * Sets whether to paint the background with the background colour.
 * The default is %TRUE.
 *
 * This function is rarely useful. One use for it is to add a background
 * image to the terminal.
 *
 * Since: 0.52
 */
void vte_terminal_set_clear_background(NoVteTerminal* terminal,
                                        gboolean setting)
{
    g_return_if_fail(VTE_IS_TERMINAL(terminal));

    IMPL(terminal)->set_clear_background(setting != FALSE);
}
