/*
 * Copyright (C) 2003,2008 Red Hat, Inc.
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

#include "vice.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* M_PI isn't defined in math.h when using strict standards compliance */
#ifndef M_PI
# define M_PI 3.14159265358979323846264338327950288
#endif

#define howmany(x,y) (((x)+((y)-1))/(y))

#include <glib.h>
#include <gtk/gtk.h>

#include "vtedraw.hh"
#include "vtedefines.hh"
#include "debug.h"

#include <pango/pangocairo.h>

/* Have a space between letters to make sure ligatures aren't used when caching the glyphs: bug 793391. */
#define VTE_DRAW_SINGLE_WIDE_CHARACTERS \
                    "  ! \" # $ % & ' ( ) * + , - . / " \
                    "0 1 2 3 4 5 6 7 8 9 " \
                    ": ; < = > ? @ " \
                    "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z " \
                    "[ \\ ] ^ _ ` " \
                    "a b c d e f g h i j k l m n o p q r s t u v w x y z " \
                    "{ | } ~ " \
                    ""

static inline bool _vte_double_equal(double a, double b)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
        return a == b;
#pragma GCC diagnostic pop
}

/* Overview:
 *
 *
 * This file implements vte rendering using pangocairo.  Note that this does
 * NOT implement any kind of complex text rendering.  That's not currently a
 * goal.
 *
 * The aim is to be super-fast and avoid unneeded work as much as possible.
 * Here is an overview of how that is accomplished:
 *
 *   - We attach a font_info to the draw.  A font_info has all the information
 *     to quickly draw text.
 *
 *   - A font_info keeps uses unistr_font_info structs that represent all
 *     information needed to quickly draw a single vteunistr.  The font_info
 *     creates those unistr_font_info structs on demand and caches them
 *     indefinitely.  It uses a direct array for the ASCII range and a hash
 *     table for the rest.
 *
 *
 * Fast rendering of unistrs:
 *
 * A unistr_font_info (uinfo) calls Pango to set text for the unistr upon
 * initialization and then caches information needed to draw the results
 * later.  It uses three different internal representations and respectively
 * three drawing paths:
 *
 *   - COVERAGE_USE_CAIRO_GLYPH:
 *     Keeping a single glyph index and a cairo scaled-font.  This is the
 *     fastest way to draw text as it bypasses Pango completely and allows
 *     for stuffing multiple glyphs into a single cairo_show_glyphs() request
 *     (if scaled-fonts match).  This method is used if the glyphs used for
 *     the vteunistr as determined by Pango consists of a single regular glyph
 *     positioned at 0,0 using a regular font.  This method is used for more
 *     than 99% of the cases.  Only exceptional cases fall through to the
 *     other two methods.
 *
 *   - COVERAGE_USE_PANGO_GLYPH_STRING:
 *     Keeping a pango glyphstring and a pango font.  This is slightly slower
 *     than the previous case as drawing each glyph goes through pango
 *     separately and causes a separate cairo_show_glyphs() call.  This method
 *     is used when the previous method cannot be used but the glyphs for the
 *     character all use a single font.  This is the method used for hexboxes
 *     and "empty" characters like U+200C ZERO WIDTH NON-JOINER for example.
 *
 *   - COVERAGE_USE_PANGO_LAYOUT_LINE:
 *     Keeping a pango layout line.  This method is used only in the very
 *     weird and exceptional case that a single vteunistr uses more than one
 *     font to be drawn.  This happens for example if some diacretics is not
 *     available in the font chosen for the base character.
 *
 *
 * Caching of font infos:
 *
 * To avoid recreating font info structs for the same font again and again we
 * do the following:
 *
 *   - Use a global cache to share font info structs across different widgets.
 *     We use pango language, cairo font options, resolution, and font description
 *     as the key for our hash table.
 *
 *   - When a font info struct is no longer used by any widget, we delay
 *     destroying it for a while (FONT_CACHE_TIMEOUT seconds).  This is
 *     supposed to serve two purposes:
 *
 *       * Destroying a terminal widget and creating it again right after will
 *         reuse the font info struct from the previous widget.
 *
 *       * Zooming in and out a terminal reuses the font info structs.
 *
 *
 * Pre-caching ASCII letters:
 *
 * When initializing a font info struct we measure a string consisting of all
 * ASCII letters and some other ASCII characters.  Since we have a shaped pango
 * layout at hand, we walk over it and cache unistr font info for the ASCII
 * letters if we can do that easily using COVERAGE_USE_CAIRO_GLYPH.  This
 * means that we precache all ASCII letters without any extra pango shaping
 * involved.
 */



#define FONT_CACHE_TIMEOUT (30) /* seconds */


/* All shared data structures are implicitly protected by GDK mutex, because
 * that's how vte.c works and we only get called from there. */


/* cairo_show_glyphs accepts runs up to 102 glyphs before it allocates a
 * temporary array.
 *
 * Setting this to a large value can cause dramatic slow-downs for some
 * xservers (notably fglrx), see bug #410534.
 *
 * Moreover, setting it larger than %VTE_DRAW_MAX_LENGTH is nonsensical,
 * as the higher layers will not submit runs longer than that value.
 */
#define MAX_RUN_LENGTH 100


enum unistr_coverage {
    /* in increasing order of speed */
    COVERAGE_UNKNOWN = 0,       /* we don't know about the character yet */
    COVERAGE_USE_PANGO_LAYOUT_LINE, /* use a PangoLayoutLine for the character */
    COVERAGE_USE_PANGO_GLYPH_STRING,    /* use a PangoGlyphString for the character */
    COVERAGE_USE_CAIRO_GLYPH    /* use a cairo_glyph_t for the character */
};

union unistr_font_info {
    /* COVERAGE_USE_PANGO_LAYOUT_LINE */
    struct {
        PangoLayoutLine *line;
    } using_pango_layout_line;
    /* COVERAGE_USE_PANGO_GLYPH_STRING */
    struct {
        PangoFont *font;
        PangoGlyphString *glyph_string;
    } using_pango_glyph_string;
    /* COVERAGE_USE_CAIRO_GLYPH */
    struct {
        cairo_scaled_font_t *scaled_font;
        unsigned int glyph_index;
    } using_cairo_glyph;
};

struct unistr_info {
    guchar coverage;
    guchar has_unknown_chars;
    guint16 width;
    union unistr_font_info ufi;
};

static struct unistr_info *unistr_info_create (void)
{
    return g_slice_new0 (struct unistr_info);
}

static void unistr_info_finish (struct unistr_info *uinfo)
{
    union unistr_font_info *ufi = &uinfo->ufi;

    switch (uinfo->coverage) {
        default:
        case COVERAGE_UNKNOWN:
            break;
        case COVERAGE_USE_PANGO_LAYOUT_LINE:
            /* we hold a manual reference on layout */
            g_object_unref (ufi->using_pango_layout_line.line->layout);
            ufi->using_pango_layout_line.line->layout = NULL;
            pango_layout_line_unref (ufi->using_pango_layout_line.line);
            ufi->using_pango_layout_line.line = NULL;
            break;
        case COVERAGE_USE_PANGO_GLYPH_STRING:
            if (ufi->using_pango_glyph_string.font)
                g_object_unref (ufi->using_pango_glyph_string.font);
            ufi->using_pango_glyph_string.font = NULL;
            pango_glyph_string_free (ufi->using_pango_glyph_string.glyph_string);
            ufi->using_pango_glyph_string.glyph_string = NULL;
            break;
        case COVERAGE_USE_CAIRO_GLYPH:
            cairo_scaled_font_destroy (ufi->using_cairo_glyph.scaled_font);
            ufi->using_cairo_glyph.scaled_font = NULL;
            break;
    }
}

static void unistr_info_destroy (struct unistr_info *uinfo)
{
    unistr_info_finish (uinfo);
    g_slice_free (struct unistr_info, uinfo);
}

struct font_info {
    /* lifecycle */
    int ref_count;
    guint destroy_timeout; /* only used when ref_count == 0 */

    /* reusable layout set with font and everything set */
    PangoLayout *layout;

    /* cache of character info */
    struct unistr_info ascii_unistr_info[128];
    GHashTable *other_unistr_info;

    /* cell metrics as taken from the font, not yet scaled by cell_{width,height}_scale */
    gint width, height, ascent;

    /* reusable string for UTF-8 conversion */
    GString *string;

#ifdef VTE_DEBUG
    /* profiling info */
    int coverage_count[4];
#endif
};


static struct unistr_info *font_info_find_unistr_info (struct font_info *info, vteunistr c)
{
    struct unistr_info *uinfo;

    if (G_LIKELY (c < G_N_ELEMENTS (info->ascii_unistr_info))) {
        return &info->ascii_unistr_info[c];
    }

    if (G_UNLIKELY (info->other_unistr_info == NULL)) {
        info->other_unistr_info = g_hash_table_new_full (NULL, NULL, NULL, (GDestroyNotify) unistr_info_destroy);
    }

    uinfo = (struct unistr_info *)g_hash_table_lookup (info->other_unistr_info, GINT_TO_POINTER (c));
    if (G_LIKELY (uinfo)) {
        return uinfo;
    }

    uinfo = unistr_info_create ();
    g_hash_table_insert (info->other_unistr_info, GINT_TO_POINTER (c), uinfo);
    return uinfo;
}


static void font_info_cache_ascii (struct font_info *info)
{
    PangoLayoutLine *line;
    PangoGlyphItemIter iter;
    PangoGlyphItem *glyph_item;
    PangoGlyphString *glyph_string;
    PangoFont *pango_font;
    cairo_scaled_font_t *scaled_font;
    const char *text;
    gboolean more;
    PangoLanguage *language;
    gboolean latin_uses_default_language;

    /* We have info->layout holding most ASCII characters.  We want to
     * cache as much info as we can about the ASCII letters so we don't
     * have to look them up again later */

    /* Don't cache if unknown glyphs found in layout */
    if (pango_layout_get_unknown_glyphs_count (info->layout) != 0) {
        return;
    }

    language = pango_context_get_language (pango_layout_get_context (info->layout));
    if (language == NULL) {
        language = pango_language_get_default ();
    }
    latin_uses_default_language = pango_language_includes_script (language, PANGO_SCRIPT_LATIN);

    text = pango_layout_get_text (info->layout);

    line = pango_layout_get_line_readonly (info->layout, 0);

    /* Don't cache if more than one font used for the line */
    if (G_UNLIKELY (!line || !line->runs || line->runs->next)) {
        return;
    }

    glyph_item = (PangoGlyphItem *)line->runs->data;
    glyph_string = glyph_item->glyphs;
    pango_font = glyph_item->item->analysis.font;
    if (!pango_font) {
        return;
    }
    scaled_font = pango_cairo_font_get_scaled_font ((PangoCairoFont *) pango_font);
    if (!scaled_font) {
        return;
    }

    for (more = pango_glyph_item_iter_init_start (&iter, glyph_item, text);
         more;
         more = pango_glyph_item_iter_next_cluster (&iter))
    {
        struct unistr_info *uinfo;
        union unistr_font_info *ufi;
        PangoGlyphGeometry *geometry;
        PangoGlyph glyph;
        vteunistr c;

        /* Only cache simple clusters */
        if (iter.start_char +1 != iter.end_char  ||
            iter.start_index+1 != iter.end_index ||
            iter.start_glyph+1 != iter.end_glyph) {
            continue;
        }

        c = text[iter.start_index];
        glyph = glyph_string->glyphs[iter.start_glyph].glyph;
        geometry = &glyph_string->glyphs[iter.start_glyph].geometry;

        /* If not using the default locale language, only cache non-common
         * characters as common characters get their font from their neighbors
         * and we don't want to force Latin on them. */
        if (!latin_uses_default_language &&
            pango_script_for_unichar (c) <= PANGO_SCRIPT_INHERITED) {
            continue;
        }

        /* Only cache simple glyphs */
        if (!(glyph <= 0xFFFF) || (geometry->x_offset | geometry->y_offset) != 0) {
            continue;
        }

        uinfo = font_info_find_unistr_info (info, c);
        if (G_UNLIKELY (uinfo->coverage != COVERAGE_UNKNOWN)) {
            continue;
        }

        ufi = &uinfo->ufi;

        uinfo->width = PANGO_PIXELS_CEIL (geometry->width);
        uinfo->has_unknown_chars = FALSE;

        uinfo->coverage = COVERAGE_USE_CAIRO_GLYPH;

        ufi->using_cairo_glyph.scaled_font = cairo_scaled_font_reference (scaled_font);
        ufi->using_cairo_glyph.glyph_index = glyph;

#ifdef VTE_DEBUG
        info->coverage_count[0]++;
        info->coverage_count[uinfo->coverage]++;
#endif
    }

#ifdef VTE_DEBUG
    _vte_debug_print (VTE_DEBUG_PANGOCAIRO,
                        "vtepangocairo: %p cached %d ASCII letters\n",
                        info, info->coverage_count[0]);
#endif
}

static void font_info_measure_font (struct font_info *info)
{
    PangoRectangle logical;

    /* Estimate for ASCII characters. */
    pango_layout_set_text (info->layout, VTE_DRAW_SINGLE_WIDE_CHARACTERS, -1);
    pango_layout_get_extents (info->layout, NULL, &logical);
    /* We don't do CEIL for width since we are averaging;
     * rounding is more accurate */
    info->width  = PANGO_PIXELS (howmany (logical.width, strlen(VTE_DRAW_SINGLE_WIDE_CHARACTERS)));
        /* Guard against pathological font since width=0 causes a FPE later on */
    info->width = MAX (info->width, 1);

    info->height = PANGO_PIXELS_CEIL (logical.height);
    info->ascent = PANGO_PIXELS_CEIL (pango_layout_get_baseline (info->layout));

    /* Now that we shaped the entire ASCII character string, cache glyph
     * info for them */
    font_info_cache_ascii (info);

    if (info->height == 0) {
        info->height = PANGO_PIXELS_CEIL (logical.height);
    }
    if (info->ascent == 0) {
        info->ascent = PANGO_PIXELS_CEIL (pango_layout_get_baseline (info->layout));
    }

    _vte_debug_print (VTE_DEBUG_MISC,
                        "vtepangocairo: %p font metrics = %dx%d (%d)\n",
                        info, info->width, info->height, info->ascent);
}

static struct font_info *font_info_allocate (PangoContext *context)
{
    struct font_info *info;
    PangoTabArray *tabs;

    info = g_slice_new0 (struct font_info);

    _vte_debug_print (VTE_DEBUG_PANGOCAIRO,
                        "vtepangocairo: %p allocating font_info\n",
                        info);

    info->layout = pango_layout_new (context);
    tabs = pango_tab_array_new_with_positions (1, FALSE, PANGO_TAB_LEFT, 1);
    pango_layout_set_tabs (info->layout, tabs);
    pango_tab_array_free (tabs);

    info->string = g_string_sized_new (VTE_UTF8_BPC+1);

    font_info_measure_font (info);

    return info;
}

static void font_info_free (struct font_info *info)
{
    vteunistr i;

#ifdef VTE_DEBUG
    _vte_debug_print (VTE_DEBUG_PANGOCAIRO,
                        "vtepangocairo: %p freeing font_info.  coverages %d = %d + %d + %d\n",
                        info,
                        info->coverage_count[0],
                        info->coverage_count[1],
                        info->coverage_count[2],
                        info->coverage_count[3]);
#endif

    g_string_free (info->string, TRUE);
    g_object_unref (info->layout);

    for (i = 0; i < G_N_ELEMENTS (info->ascii_unistr_info); i++) {
        unistr_info_finish (&info->ascii_unistr_info[i]);
    }

    if (info->other_unistr_info) {
        g_hash_table_destroy (info->other_unistr_info);
    }

    g_slice_free (struct font_info, info);
}


static GHashTable *font_info_for_context;

static struct font_info *
font_info_register (struct font_info *info)
{
    g_hash_table_insert (font_info_for_context, pango_layout_get_context (info->layout), info);

    return info;
}

static void font_info_unregister (struct font_info *info)
{
    g_hash_table_remove (font_info_for_context, pango_layout_get_context (info->layout));
}


static struct font_info *font_info_reference (struct font_info *info)
{
    if (!info) {
        return info;
    }

    g_return_val_if_fail (info->ref_count >= 0, info);

    if (info->destroy_timeout) {
        g_source_remove (info->destroy_timeout);
        info->destroy_timeout = 0;
    }

    info->ref_count++;

    return info;
}

static gboolean font_info_destroy_delayed (struct font_info *info)
{
    info->destroy_timeout = 0;

    font_info_unregister (info);
    font_info_free (info);

    return FALSE;
}

static void font_info_destroy (struct font_info *info)
{
    if (!info) {
        return;
    }

    g_return_if_fail (info->ref_count > 0);

    info->ref_count--;
    if (info->ref_count) {
        return;
    }

    /* Delay destruction by a few seconds, in case we need it again */
    info->destroy_timeout = gdk_threads_add_timeout_seconds (FONT_CACHE_TIMEOUT,
                                 (GSourceFunc) font_info_destroy_delayed,
                                 info);
}

static GQuark fontconfig_timestamp_quark (void)
{
    static GQuark quark;

    if (G_UNLIKELY (!quark)) {
        quark = g_quark_from_static_string ("vte-fontconfig-timestamp");
    }

    return quark;
}

static void vte_pango_context_set_fontconfig_timestamp (PangoContext *context,
                                    guint fontconfig_timestamp)
{
    g_object_set_qdata ((GObject *) context,
                fontconfig_timestamp_quark (),
                GUINT_TO_POINTER (fontconfig_timestamp));
}

static guint vte_pango_context_get_fontconfig_timestamp (PangoContext *context)
{
    return GPOINTER_TO_UINT (g_object_get_qdata ((GObject *) context,
                             fontconfig_timestamp_quark ()));
}

static guint context_hash (PangoContext *context)
{
    return pango_units_from_double (pango_cairo_context_get_resolution (context))
         ^ pango_font_description_hash (pango_context_get_font_description (context))
         ^ cairo_font_options_hash (pango_cairo_context_get_font_options (context))
         ^ GPOINTER_TO_UINT (pango_context_get_language (context))
         ^ vte_pango_context_get_fontconfig_timestamp (context);
}

static gboolean context_equal (PangoContext *a, PangoContext *b)
{
    return _vte_double_equal(pango_cairo_context_get_resolution(a), pango_cairo_context_get_resolution (b))
        && pango_font_description_equal (pango_context_get_font_description (a), pango_context_get_font_description (b))
        && cairo_font_options_equal (pango_cairo_context_get_font_options (a), pango_cairo_context_get_font_options (b))
        && pango_context_get_language (a) == pango_context_get_language (b)
        && vte_pango_context_get_fontconfig_timestamp (a) == vte_pango_context_get_fontconfig_timestamp (b);
}

static struct font_info *font_info_find_for_context (PangoContext *context)
{
    struct font_info *info;

    if (G_UNLIKELY (font_info_for_context == NULL)) {
        font_info_for_context = g_hash_table_new ((GHashFunc) context_hash, (GEqualFunc) context_equal);
    }

    info = (struct font_info *)g_hash_table_lookup (font_info_for_context, context);
    if (G_LIKELY (info)) {
        _vte_debug_print (VTE_DEBUG_PANGOCAIRO,
                  "vtepangocairo: %p found font_info in cache\n",
                  info);
        info = font_info_reference (info);
    } else {
        info = font_info_allocate (context);
        info->ref_count = 1;
        font_info_register (info);
    }

    g_object_unref (context);

    return info;
}

/* assumes ownership/reference of context */
static struct font_info *font_info_create_for_context (PangoContext *context,
    const PangoFontDescription *desc, PangoLanguage *language, guint fontconfig_timestamp)
{
    if (!PANGO_IS_CAIRO_FONT_MAP (pango_context_get_font_map (context))) {
        /* Ouch, Gtk+ switched over to some drawing system?
         * Lets just create one from the default font map.
         */
        g_object_unref (context);
        context = pango_font_map_create_context (pango_cairo_font_map_get_default ());
    }

    vte_pango_context_set_fontconfig_timestamp (context, fontconfig_timestamp);

    pango_context_set_base_dir (context, PANGO_DIRECTION_LTR);

    if (desc) {
        pango_context_set_font_description (context, desc);
    }

    pango_context_set_language (context, language);

    /* Make sure our contexts have a font_options set.  We use
     * this invariant in our context hash and equal functions.
     */
    if (!pango_cairo_context_get_font_options (context)) {
        cairo_font_options_t *font_options;

        font_options = cairo_font_options_create ();
        pango_cairo_context_set_font_options (context, font_options);
        cairo_font_options_destroy (font_options);
    }

    return font_info_find_for_context (context);
}

static struct font_info *font_info_create_for_screen (
                                        GdkScreen                  *screen,
                                        const PangoFontDescription *desc,
                                        PangoLanguage              *language)
{
    GtkSettings *settings = gtk_settings_get_for_screen (screen);
    int fontconfig_timestamp;
    g_object_get (settings, "gtk-fontconfig-timestamp", &fontconfig_timestamp, nullptr);
    return font_info_create_for_context (gdk_pango_context_get_for_screen (screen),
                                            desc, language, fontconfig_timestamp);
}

static struct font_info *font_info_create_for_widget (GtkWidget *widget,
                                                        const PangoFontDescription *desc)
{
    GdkScreen *screen = gtk_widget_get_screen (widget);
    PangoLanguage *language = pango_context_get_language (gtk_widget_get_pango_context (widget));

    return font_info_create_for_screen (screen, desc, language);
}

static struct unistr_info *font_info_get_unistr_info (struct font_info *info, vteunistr c)
{
    struct unistr_info *uinfo;
    union unistr_font_info *ufi;
    PangoRectangle logical;
    PangoLayoutLine *line;

    uinfo = font_info_find_unistr_info (info, c);
    if (G_LIKELY (uinfo->coverage != COVERAGE_UNKNOWN)) {
        return uinfo;
    }

    ufi = &uinfo->ufi;

    g_string_set_size (info->string, 0);
    _vte_unistr_append_to_string (c, info->string);
    pango_layout_set_text (info->layout, info->string->str, info->string->len);
    pango_layout_get_extents (info->layout, NULL, &logical);

    uinfo->width = PANGO_PIXELS_CEIL (logical.width);

    line = pango_layout_get_line_readonly (info->layout, 0);

    uinfo->has_unknown_chars = pango_layout_get_unknown_glyphs_count (info->layout) != 0;
    /* we use PangoLayoutRun rendering unless there is exactly one run in the line. */
    if (G_UNLIKELY (!line || !line->runs || line->runs->next)) {
        uinfo->coverage = COVERAGE_USE_PANGO_LAYOUT_LINE;

        ufi->using_pango_layout_line.line = pango_layout_line_ref (line);
        /* we hold a manual reference on layout.  pango currently
         * doesn't work if line->layout is NULL.  ugh! */
        pango_layout_set_text (info->layout, "", -1); /* make layout disassociate from the line */
        ufi->using_pango_layout_line.line->layout = (PangoLayout *)g_object_ref (info->layout);

    } else {
        PangoGlyphItem *glyph_item = (PangoGlyphItem *)line->runs->data;
        PangoFont *pango_font = glyph_item->item->analysis.font;
        PangoGlyphString *glyph_string = glyph_item->glyphs;

        /* we use fast cairo path if glyph string has only one real
         * glyph and at origin */
        if (!uinfo->has_unknown_chars &&
            glyph_string->num_glyphs == 1 && glyph_string->glyphs[0].glyph <= 0xFFFF &&
            (glyph_string->glyphs[0].geometry.x_offset |
             glyph_string->glyphs[0].geometry.y_offset) == 0) {
            cairo_scaled_font_t *scaled_font = pango_cairo_font_get_scaled_font ((PangoCairoFont *) pango_font);

            if (scaled_font) {
                uinfo->coverage = COVERAGE_USE_CAIRO_GLYPH;

                ufi->using_cairo_glyph.scaled_font = cairo_scaled_font_reference (scaled_font);
                ufi->using_cairo_glyph.glyph_index = glyph_string->glyphs[0].glyph;
            }
        }

        /* use pango fast path otherwise */
        if (G_UNLIKELY (uinfo->coverage == COVERAGE_UNKNOWN)) {
            uinfo->coverage = COVERAGE_USE_PANGO_GLYPH_STRING;

            ufi->using_pango_glyph_string.font = pango_font ? (PangoFont *)g_object_ref (pango_font) : NULL;
            ufi->using_pango_glyph_string.glyph_string = pango_glyph_string_copy (glyph_string);
        }
    }

    /* release internal layout resources */
    pango_layout_set_text (info->layout, "", -1);

#ifdef VTE_DEBUG
    info->coverage_count[0]++;
    info->coverage_count[uinfo->coverage]++;
#endif

    return uinfo;
}

guint _vte_draw_get_style(gboolean bold, gboolean italic) {
    guint style = 0;
    if (bold) {
        style |= VTE_DRAW_BOLD;
    }
    if (italic) {
        style |= VTE_DRAW_ITALIC;
    }
    return style;
}

struct _vte_draw {
    struct font_info *fonts[4];
    /* cell metrics, already adjusted by cell_{width,height}_scale */
    int cell_width, cell_height;
    GtkBorder char_spacing;

    cairo_t *cr;

    /* Cache the undercurl's rendered look. */
    cairo_surface_t *undercurl_surface;
};

struct _vte_draw *_vte_draw_new (void)
{
    struct _vte_draw *draw;

    /* Create the structure. */
    draw = g_slice_new0 (struct _vte_draw);

    _vte_debug_print (VTE_DEBUG_DRAW, "draw_new\n");

    return draw;
}

void _vte_draw_free (struct _vte_draw *draw)
{
    gint style;
    _vte_debug_print (VTE_DEBUG_DRAW, "draw_free\n");

    /* Free all fonts (make sure to destroy every font only once)*/
    for (style = 3; style >= 0; style--) {
        if (draw->fonts[style] != NULL &&
            (style == 0 || draw->fonts[style] != draw->fonts[style-1])) {
            font_info_destroy (draw->fonts[style]);
            draw->fonts[style] = NULL;
        }
    }

    if (draw->undercurl_surface != NULL) {
        cairo_surface_destroy (draw->undercurl_surface);
        draw->undercurl_surface = NULL;
    }

    g_slice_free (struct _vte_draw, draw);
}

void _vte_draw_set_cairo (struct _vte_draw *draw, cairo_t *cr)
{
    _vte_debug_print (VTE_DEBUG_DRAW, "%s cairo context\n", cr ? "Settings" : "Unsetting");

    if (cr) {
        g_assert (draw->cr == NULL);
        draw->cr = cr;
    } else {
        g_assert (draw->cr != NULL);
        draw->cr = NULL;
    }
}

static void _vte_draw_set_source_color_alpha (struct _vte_draw *draw,
                                                vte::color::rgb const* color,
                                                double alpha)
{
    g_assert(draw->cr);
    cairo_set_source_rgba (draw->cr,
                            color->red / 65535.,
                            color->green / 65535.,
                            color->blue / 65535.,
                            alpha);
}

void _vte_draw_clear (struct _vte_draw *draw, gint x, gint y, gint width, gint height,
                        vte::color::rgb const* color, double alpha)
{
    _vte_debug_print (VTE_DEBUG_DRAW, "draw_clear (%d, %d, %d, %d)\n",
                        x,y,width, height);

    g_assert(draw->cr);
    cairo_rectangle (draw->cr, x, y, width, height);
    cairo_set_operator (draw->cr, CAIRO_OPERATOR_SOURCE);
    _vte_draw_set_source_color_alpha(draw, color, alpha);
    cairo_fill (draw->cr);
}

void _vte_draw_set_text_font (struct _vte_draw *draw,
                                GtkWidget *widget,
                                const PangoFontDescription *fontdesc,
                                double cell_width_scale,
                                double cell_height_scale)
{
    PangoFontDescription *bolddesc   = NULL;
    PangoFontDescription *italicdesc = NULL;
    PangoFontDescription *bolditalicdesc = NULL;
    gint style, normal, bold, ratio;

    _vte_debug_print (VTE_DEBUG_DRAW, "draw_set_text_font\n");

    /* Free all fonts (make sure to destroy every font only once)*/
    for (style = 3; style >= 0; style--) {
        if (draw->fonts[style] != NULL &&
            (style == 0 || draw->fonts[style] != draw->fonts[style-1])) {
            font_info_destroy (draw->fonts[style]);
            draw->fonts[style] = NULL;
        }
    }

    /* calculate bold font desc */
    bolddesc = pango_font_description_copy (fontdesc);
    pango_font_description_set_weight (bolddesc, PANGO_WEIGHT_BOLD);

    /* calculate italic font desc */
    italicdesc = pango_font_description_copy (fontdesc);
    pango_font_description_set_style (italicdesc, PANGO_STYLE_ITALIC);

    /* calculate bold italic font desc */
    bolditalicdesc = pango_font_description_copy (bolddesc);
    pango_font_description_set_style (bolditalicdesc, PANGO_STYLE_ITALIC);

    draw->fonts[VTE_DRAW_NORMAL]  = font_info_create_for_widget (widget, fontdesc);
    draw->fonts[VTE_DRAW_BOLD]    = font_info_create_for_widget (widget, bolddesc);
    draw->fonts[VTE_DRAW_ITALIC]  = font_info_create_for_widget (widget, italicdesc);
    draw->fonts[VTE_DRAW_ITALIC | VTE_DRAW_BOLD] = font_info_create_for_widget (widget, bolditalicdesc);
    pango_font_description_free (bolddesc);
    pango_font_description_free (italicdesc);
    pango_font_description_free (bolditalicdesc);

    /* Decide if we should keep this bold font face, per bug 54926:
     *  - reject bold font if it is not within 10% of normal font width
     */
    normal = VTE_DRAW_NORMAL;
    bold   = normal | VTE_DRAW_BOLD;
    ratio = draw->fonts[bold]->width * 100 / draw->fonts[normal]->width;
    if (abs(ratio - 100) > 10) {
        _vte_debug_print (VTE_DEBUG_DRAW, "Rejecting bold font (%i%%).\n", ratio);
        font_info_destroy (draw->fonts[bold]);
        draw->fonts[bold] = draw->fonts[normal];
    }
    normal = VTE_DRAW_ITALIC;
    bold   = normal | VTE_DRAW_BOLD;
    ratio = draw->fonts[bold]->width * 100 / draw->fonts[normal]->width;
    if (abs(ratio - 100) > 10) {
        _vte_debug_print (VTE_DEBUG_DRAW, "Rejecting italic bold font (%i%%).\n", ratio);
        font_info_destroy (draw->fonts[bold]);
        draw->fonts[bold] = draw->fonts[normal];
    }

    /* Apply letter spacing and line spacing. */
    draw->cell_width = draw->fonts[VTE_DRAW_NORMAL]->width * cell_width_scale;
    draw->char_spacing.left = (draw->cell_width - draw->fonts[VTE_DRAW_NORMAL]->width) / 2;
    draw->char_spacing.right = (draw->cell_width - draw->fonts[VTE_DRAW_NORMAL]->width + 1) / 2;
    draw->cell_height = draw->fonts[VTE_DRAW_NORMAL]->height * cell_height_scale;
    draw->char_spacing.top = (draw->cell_height - draw->fonts[VTE_DRAW_NORMAL]->height + 1) / 2;
    draw->char_spacing.bottom = (draw->cell_height - draw->fonts[VTE_DRAW_NORMAL]->height) / 2;

    /* Drop the undercurl's cached look. Will recache on demand. */
    if (draw->undercurl_surface != NULL) {
        cairo_surface_destroy (draw->undercurl_surface);
        draw->undercurl_surface = NULL;
    }
}

void _vte_draw_get_text_metrics(struct _vte_draw *draw,
                                int *cell_width, int *cell_height,
                                int *char_ascent, int *char_descent,
                                GtkBorder *char_spacing)
{
    g_return_if_fail (draw->fonts[VTE_DRAW_NORMAL] != NULL);

    if (cell_width) {
        *cell_width = draw->cell_width;
    }
    if (cell_height) {
        *cell_height = draw->cell_height;
    }
    if (char_ascent) {
        *char_ascent = draw->fonts[VTE_DRAW_NORMAL]->ascent;
    }
    if (char_descent) {
        *char_descent = draw->fonts[VTE_DRAW_NORMAL]->height - draw->fonts[VTE_DRAW_NORMAL]->ascent;
    }
    if (char_spacing) {
        *char_spacing = draw->char_spacing;
    }
}


/* Stores the left and right edges of the given glyph, relative to the cell's left edge. */
void _vte_draw_get_char_edges (struct _vte_draw *draw, vteunistr c, int columns, guint style,
                                int *left, int *right)
{
    int l, w, normal_width, fits_width;

    if (G_UNLIKELY (draw->fonts[VTE_DRAW_NORMAL] == NULL)) {
        if (left) {
            *left = 0;
        }
        if (right) {
            *right = 0;
        }
        return;
    }

    w = font_info_get_unistr_info (draw->fonts[style], c)->width;
    normal_width = draw->fonts[VTE_DRAW_NORMAL]->width * columns;
    fits_width = draw->cell_width * columns;

    if (G_LIKELY (w <= normal_width)) {
        /* The regular case: The glyph is not wider than one (CJK: two) regular character(s).
            * Align to the left, after applying half (CJK: one) letter spacing. */
        l = draw->char_spacing.left + (columns == 2 ? draw->char_spacing.right : 0);
    } else if (G_UNLIKELY (w <= fits_width)) {
        /* Slightly wider glyph, but still fits in the cell (spacing included). This case can
            * only happen with nonzero letter spacing. Center the glyph in the cell(s). */
        l = (fits_width - w) / 2;
    } else {
        /* Even wider glyph: doesn't fit in the cell. Align at left and overflow on the right. */
        l = 0;
    }

    if (left) {
        *left = l;
    }
    if (right) {
        *right = l + w;
    }
}

gboolean _vte_draw_has_bold (struct _vte_draw *draw, guint style)
{
    return (draw->fonts[style ^ VTE_DRAW_BOLD] != draw->fonts[style]);
}

/* Check if a unicode character is actually a graphic character we draw
 * ourselves to handle cases where fonts don't have glyphs for them. */
static gboolean _vte_draw_unichar_is_local_graphic(vteunistr c)
{
    /* Box Drawing & Block Elements */
    return (c >= 0x2500) && (c <= 0x259f);
}

#include "box_drawing.h"

/* Draw the graphic representation of a line-drawing or special graphics
 * character. */
static void _vte_draw_terminal_draw_graphic(struct _vte_draw *draw, vteunistr c, vte::color::rgb const* fg,
                                            gint x, gint y,
                                            gint font_width, gint columns, gint font_height)
{
    gint width, height, xcenter, xright, ycenter, ybottom;
    int upper_half, lower_half, left_half, right_half;
    int light_line_width, heavy_line_width;
    double adjust;
    cairo_t *cr = draw->cr;

    cairo_save (cr);

    width = draw->cell_width * columns;
    height = draw->cell_height;
    upper_half = height / 2;
    lower_half = height - upper_half;
    left_half = width / 2;
    right_half = width - left_half;

    /* Note that the upper/left halves above are the same as 4 eights */
    /* FIXME: this could be smarter for very small n (< 8 resp. < 4) */
#define EIGHTS(n, k) \
        ({ int k_eights = (n) * (k) / 8; \
           k_eights = MAX(k_eights, 1); \
           k_eights; \
        })

    /* Exclude the spacing for line width computation. */
    light_line_width = font_width / 5;
    light_line_width = MAX (light_line_width, 1);

    if (c >= 0x2550 && c <= 0x256c) {
        heavy_line_width = 3 * light_line_width;
    } else {
        heavy_line_width = light_line_width + 2;
    }

    xcenter = x + left_half;
    ycenter = y + upper_half;
    xright = x + width;
    ybottom = y + height;

    switch (c) {

        /* Box Drawing */
        case 0x2500: /* box drawings light horizontal */
        case 0x2501: /* box drawings heavy horizontal */
        case 0x2502: /* box drawings light vertical */
        case 0x2503: /* box drawings heavy vertical */
        case 0x250c: /* box drawings light down and right */
        case 0x250d: /* box drawings down light and right heavy */
        case 0x250e: /* box drawings down heavy and right light */
        case 0x250f: /* box drawings heavy down and right */
        case 0x2510: /* box drawings light down and left */
        case 0x2511: /* box drawings down light and left heavy */
        case 0x2512: /* box drawings down heavy and left light */
        case 0x2513: /* box drawings heavy down and left */
        case 0x2514: /* box drawings light up and right */
        case 0x2515: /* box drawings up light and right heavy */
        case 0x2516: /* box drawings up heavy and right light */
        case 0x2517: /* box drawings heavy up and right */
        case 0x2518: /* box drawings light up and left */
        case 0x2519: /* box drawings up light and left heavy */
        case 0x251a: /* box drawings up heavy and left light */
        case 0x251b: /* box drawings heavy up and left */
        case 0x251c: /* box drawings light vertical and right */
        case 0x251d: /* box drawings vertical light and right heavy */
        case 0x251e: /* box drawings up heavy and right down light */
        case 0x251f: /* box drawings down heavy and right up light */
        case 0x2520: /* box drawings vertical heavy and right light */
        case 0x2521: /* box drawings down light and right up heavy */
        case 0x2522: /* box drawings up light and right down heavy */
        case 0x2523: /* box drawings heavy vertical and right */
        case 0x2524: /* box drawings light vertical and left */
        case 0x2525: /* box drawings vertical light and left heavy */
        case 0x2526: /* box drawings up heavy and left down light */
        case 0x2527: /* box drawings down heavy and left up light */
        case 0x2528: /* box drawings vertical heavy and left light */
        case 0x2529: /* box drawings down light and left up heavy */
        case 0x252a: /* box drawings up light and left down heavy */
        case 0x252b: /* box drawings heavy vertical and left */
        case 0x252c: /* box drawings light down and horizontal */
        case 0x252d: /* box drawings left heavy and right down light */
        case 0x252e: /* box drawings right heavy and left down light */
        case 0x252f: /* box drawings down light and horizontal heavy */
        case 0x2530: /* box drawings down heavy and horizontal light */
        case 0x2531: /* box drawings right light and left down heavy */
        case 0x2532: /* box drawings left light and right down heavy */
        case 0x2533: /* box drawings heavy down and horizontal */
        case 0x2534: /* box drawings light up and horizontal */
        case 0x2535: /* box drawings left heavy and right up light */
        case 0x2536: /* box drawings right heavy and left up light */
        case 0x2537: /* box drawings up light and horizontal heavy */
        case 0x2538: /* box drawings up heavy and horizontal light */
        case 0x2539: /* box drawings right light and left up heavy */
        case 0x253a: /* box drawings left light and right up heavy */
        case 0x253b: /* box drawings heavy up and horizontal */
        case 0x253c: /* box drawings light vertical and horizontal */
        case 0x253d: /* box drawings left heavy and right vertical light */
        case 0x253e: /* box drawings right heavy and left vertical light */
        case 0x253f: /* box drawings vertical light and horizontal heavy */
        case 0x2540: /* box drawings up heavy and down horizontal light */
        case 0x2541: /* box drawings down heavy and up horizontal light */
        case 0x2542: /* box drawings vertical heavy and horizontal light */
        case 0x2543: /* box drawings left up heavy and right down light */
        case 0x2544: /* box drawings right up heavy and left down light */
        case 0x2545: /* box drawings left down heavy and right up light */
        case 0x2546: /* box drawings right down heavy and left up light */
        case 0x2547: /* box drawings down light and up horizontal heavy */
        case 0x2548: /* box drawings up light and down horizontal heavy */
        case 0x2549: /* box drawings right light and left vertical heavy */
        case 0x254a: /* box drawings left light and right vertical heavy */
        case 0x254b: /* box drawings heavy vertical and horizontal */
        case 0x2550: /* box drawings double horizontal */
        case 0x2551: /* box drawings double vertical */
        case 0x2552: /* box drawings down single and right double */
        case 0x2553: /* box drawings down double and right single */
        case 0x2554: /* box drawings double down and right */
        case 0x2555: /* box drawings down single and left double */
        case 0x2556: /* box drawings down double and left single */
        case 0x2557: /* box drawings double down and left */
        case 0x2558: /* box drawings up single and right double */
        case 0x2559: /* box drawings up double and right single */
        case 0x255a: /* box drawings double up and right */
        case 0x255b: /* box drawings up single and left double */
        case 0x255c: /* box drawings up double and left single */
        case 0x255d: /* box drawings double up and left */
        case 0x255e: /* box drawings vertical single and right double */
        case 0x255f: /* box drawings vertical double and right single */
        case 0x2560: /* box drawings double vertical and right */
        case 0x2561: /* box drawings vertical single and left double */
        case 0x2562: /* box drawings vertical double and left single */
        case 0x2563: /* box drawings double vertical and left */
        case 0x2564: /* box drawings down single and horizontal double */
        case 0x2565: /* box drawings down double and horizontal single */
        case 0x2566: /* box drawings double down and horizontal */
        case 0x2567: /* box drawings up single and horizontal double */
        case 0x2568: /* box drawings up double and horizontal single */
        case 0x2569: /* box drawings double up and horizontal */
        case 0x256a: /* box drawings vertical single and horizontal double */
        case 0x256b: /* box drawings vertical double and horizontal single */
        case 0x256c: /* box drawings double vertical and horizontal */
        case 0x2574: /* box drawings light left */
        case 0x2575: /* box drawings light up */
        case 0x2576: /* box drawings light right */
        case 0x2577: /* box drawings light down */
        case 0x2578: /* box drawings heavy left */
        case 0x2579: /* box drawings heavy up */
        case 0x257a: /* box drawings heavy right */
        case 0x257b: /* box drawings heavy down */
        case 0x257c: /* box drawings light left and heavy right */
        case 0x257d: /* box drawings light up and heavy down */
        case 0x257e: /* box drawings heavy left and light right */
        case 0x257f: /* box drawings heavy up and light down */
        {
            guint32 bitmap = _vte_draw_box_drawing_bitmaps[c - 0x2500];
            int xboundaries[6] = { 0,
                                    left_half - heavy_line_width / 2,
                                    left_half - light_line_width / 2,
                                    left_half - light_line_width / 2 + light_line_width,
                                    left_half - heavy_line_width / 2 + heavy_line_width,
                                    width};
            int yboundaries[6] = { 0,
                                    upper_half - heavy_line_width / 2,
                                    upper_half - light_line_width / 2,
                                    upper_half - light_line_width / 2 + light_line_width,
                                    upper_half - heavy_line_width / 2 + heavy_line_width,
                                    height};
            int xi, yi;
            cairo_set_line_width(cr, 0);
            for (yi = 4; yi >= 0; yi--) {
                for (xi = 4; xi >= 0; xi--) {
                    if (bitmap & 1) {
                        cairo_rectangle(cr,
                                        x + xboundaries[xi],
                                        y + yboundaries[yi],
                                        xboundaries[xi + 1] - xboundaries[xi],
                                        yboundaries[yi + 1] - yboundaries[yi]);
                        cairo_fill(cr);
                    }
                    bitmap >>= 1;
                }
            }
            break;
        }

        case 0x2504: /* box drawings light triple dash horizontal */
        case 0x2505: /* box drawings heavy triple dash horizontal */
        case 0x2506: /* box drawings light triple dash vertical */
        case 0x2507: /* box drawings heavy triple dash vertical */
        case 0x2508: /* box drawings light quadruple dash horizontal */
        case 0x2509: /* box drawings heavy quadruple dash horizontal */
        case 0x250a: /* box drawings light quadruple dash vertical */
        case 0x250b: /* box drawings heavy quadruple dash vertical */
        case 0x254c: /* box drawings light double dash horizontal */
        case 0x254d: /* box drawings heavy double dash horizontal */
        case 0x254e: /* box drawings light double dash vertical */
        case 0x254f: /* box drawings heavy double dash vertical */
        {
            const guint v = c - 0x2500;
            int size, line_width;

            size = (v & 2) ? height : width;

            switch (v >> 2) {
                case 1: /* triple dash */
                {
                    double segment = size / 8.;
                    double dashes[2] = { segment * 2., segment };
                    cairo_set_dash(cr, dashes, G_N_ELEMENTS(dashes), 0.);
                    break;
                }
                case 2: /* quadruple dash */
                {
                    double segment = size / 11.;
                    double dashes[2] = { segment * 2., segment };
                    cairo_set_dash(cr, dashes, G_N_ELEMENTS(dashes), 0.);
                    break;
                }
                case 19: /* double dash */
                {
                    double segment = size / 5.;
                    double dashes[2] = { segment * 2., segment };
                    cairo_set_dash(cr, dashes, G_N_ELEMENTS(dashes), 0.);
                    break;
                }
            }

            line_width = (v & 1) ? heavy_line_width : light_line_width;
            adjust = (line_width & 1) ? .5 : 0.;

            cairo_set_line_width(cr, line_width);
            cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
            if (v & 2) {
                cairo_move_to(cr, xcenter + adjust, y);
                cairo_line_to(cr, xcenter + adjust, y + height);
            } else {
                cairo_move_to(cr, x, ycenter + adjust);
                cairo_line_to(cr, x + width, ycenter + adjust);
            }
            cairo_stroke(cr);
            break;
        }

        case 0x256d: /* box drawings light arc down and right */
        case 0x256e: /* box drawings light arc down and left */
        case 0x256f: /* box drawings light arc up and left */
        case 0x2570: /* box drawings light arc up and right */
        {
            const guint v = c - 0x256d;
            int line_width;
            int radius;

            cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);

            line_width = light_line_width;
            adjust = (line_width & 1) ? .5 : 0.;
            cairo_set_line_width(cr, line_width);

            radius = (font_width + 2) / 3;
            radius = MAX(radius, heavy_line_width);

            if (v & 2) {
                cairo_move_to(cr, xcenter + adjust, y);
                cairo_line_to(cr, xcenter + adjust, ycenter - radius + 2 * adjust);
            } else {
                cairo_move_to(cr, xcenter + adjust, ybottom);
                cairo_line_to(cr, xcenter + adjust, ycenter + radius);
            }
            cairo_stroke(cr);

            cairo_arc(cr,
                        (v == 1 || v == 2) ? xcenter - radius + 2 * adjust
                                            : xcenter + radius,
                        (v & 2) ? ycenter - radius + 2 * adjust
                                : ycenter + radius,
                        radius - adjust,
                        (v + 2) * M_PI / 2.0, (v + 3) * M_PI / 2.0);
            cairo_stroke(cr);

            if (v == 1 || v == 2) {
                cairo_move_to(cr, xcenter - radius + 2 * adjust, ycenter + adjust);
                cairo_line_to(cr, x, ycenter + adjust);
            } else {
                cairo_move_to(cr, xcenter + radius, ycenter + adjust);
                cairo_line_to(cr, xright, ycenter + adjust);
            }

            cairo_stroke(cr);
            break;
        }

        case 0x2571: /* box drawings light diagonal upper right to lower left */
        case 0x2572: /* box drawings light diagonal upper left to lower right */
        case 0x2573: /* box drawings light diagonal cross */
        {
            cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
            cairo_set_line_width(cr, light_line_width);
            adjust = light_line_width / 2.;
            if (c != 0x2571) {
                cairo_move_to(cr, x + adjust, y + adjust);
                cairo_line_to(cr, xright - adjust, ybottom - adjust);
                cairo_stroke(cr);
            }
            if (c != 0x2572) {
                cairo_move_to(cr, xright - adjust, y + adjust);
                cairo_line_to(cr, x + adjust, ybottom - adjust);
                cairo_stroke(cr);
            }
            break;
        }

        /* Block Elements */
        case 0x2580: /* upper half block */
            cairo_rectangle(cr, x, y, width, upper_half);
            cairo_fill (cr);
            break;

        case 0x2581: /* lower one eighth block */
        case 0x2582: /* lower one quarter block */
        case 0x2583: /* lower three eighths block */
        case 0x2584: /* lower half block */
        case 0x2585: /* lower five eighths block */
        case 0x2586: /* lower three quarters block */
        case 0x2587: /* lower seven eighths block */
        {
            const guint v = c - 0x2580;
            /* Use the number of eights from the top, so that
                * U+2584 aligns with U+2596..U+259f.
                */
            const int h = EIGHTS (height, 8 - v);

            cairo_rectangle(cr, x, y + h, width, height - h);
            cairo_fill (cr);
            break;
        }

        case 0x2588: /* full block */
        case 0x2589: /* left seven eighths block */
        case 0x258a: /* left three quarters block */
        case 0x258b: /* left five eighths block */
        case 0x258c: /* left half block */
        case 0x258d: /* left three eighths block */
        case 0x258e: /* left one quarter block */
        case 0x258f: /* left one eighth block */
        {
            const guint v = c - 0x2588;
            /* Use the number of eights from the top, so that
                * U+258c aligns with U+2596..U+259f.
                */
            const int w = EIGHTS (width, 8 - v);

            cairo_rectangle(cr, x, y, w, height);
            cairo_fill (cr);
            break;
        }

        case 0x2590: /* right half block */
            cairo_rectangle(cr, x + left_half, y, right_half, height);
            cairo_fill (cr);
            break;

        case 0x2591: /* light shade */
        case 0x2592: /* medium shade */
        case 0x2593: /* dark shade */
            cairo_set_source_rgba (cr,
                                    fg->red / 65535.,
                                    fg->green / 65535.,
                                    fg->blue / 65535.,
                                    (c - 0x2590) / 4.);
            cairo_rectangle(cr, x, y, width, height);
            cairo_fill (cr);
            break;

        case 0x2594: /* upper one eighth block */
        {
            const int h = EIGHTS (height, 1); /* Align with U+2587 */
            cairo_rectangle(cr, x, y, width, h);
            cairo_fill (cr);
            break;
        }

        case 0x2595: /* right one eighth block */
        {
            const int w = EIGHTS (width, 7);  /* Align with U+2589 */
            cairo_rectangle(cr, x + w, y, width - w, height);
            cairo_fill (cr);
            break;
        }

        case 0x2596: /* quadrant lower left */
            cairo_rectangle(cr, x, y + upper_half, left_half, lower_half);
            cairo_fill (cr);
            break;

        case 0x2597: /* quadrant lower right */
            cairo_rectangle(cr, x + left_half, y + upper_half, right_half, lower_half);
            cairo_fill (cr);
            break;

        case 0x2598: /* quadrant upper left */
            cairo_rectangle(cr, x, y, left_half, upper_half);
            cairo_fill (cr);
            break;

        case 0x2599: /* quadrant upper left and lower left and lower right */
            cairo_rectangle(cr, x, y, left_half, upper_half);
            cairo_rectangle(cr, x, y + upper_half, left_half, lower_half);
            cairo_rectangle(cr, x + left_half, y + upper_half, right_half, lower_half);
            cairo_fill (cr);
            break;

        case 0x259a: /* quadrant upper left and lower right */
            cairo_rectangle(cr, x, y, left_half, upper_half);
            cairo_rectangle(cr, x + left_half, y + upper_half, right_half, lower_half);
            cairo_fill (cr);
            break;

        case 0x259b: /* quadrant upper left and upper right and lower left */
            cairo_rectangle(cr, x, y, left_half, upper_half);
            cairo_rectangle(cr, x + left_half, y, right_half, upper_half);
            cairo_rectangle(cr, x, y + upper_half, left_half, lower_half);
            cairo_fill (cr);
            break;

        case 0x259c: /* quadrant upper left and upper right and lower right */
            cairo_rectangle(cr, x, y, left_half, upper_half);
            cairo_rectangle(cr, x + left_half, y, right_half, upper_half);
            cairo_rectangle(cr, x + left_half, y + upper_half, right_half, lower_half);
            cairo_fill (cr);
            break;

        case 0x259d: /* quadrant upper right */
            cairo_rectangle(cr, x + left_half, y, right_half, upper_half);
            cairo_fill (cr);
            break;

        case 0x259e: /* quadrant upper right and lower left */
            cairo_rectangle(cr, x + left_half, y, right_half, upper_half);
            cairo_rectangle(cr, x, y + upper_half, left_half, lower_half);
            cairo_fill (cr);
            break;

        case 0x259f: /* quadrant upper right and lower left and lower right */
            cairo_rectangle(cr, x + left_half, y, right_half, upper_half);
            cairo_rectangle(cr, x, y + upper_half, left_half, lower_half);
            cairo_rectangle(cr, x + left_half, y + upper_half, right_half, lower_half);
            cairo_fill (cr);
            break;

        default:
            g_assert_not_reached();
    }

#undef EIGHTS

    cairo_restore(cr);
}

static void _vte_draw_text_internal (struct _vte_draw *draw,
             struct _vte_draw_text_request *requests, gsize n_requests,
             vte::color::rgb const* color, double alpha, guint style)
{
    gsize i;
    cairo_scaled_font_t *last_scaled_font = NULL;
    int n_cr_glyphs = 0;
    cairo_glyph_t cr_glyphs[MAX_RUN_LENGTH];
    struct font_info *font = draw->fonts[style];

    g_return_if_fail (font != NULL);

    g_assert(draw->cr);
    _vte_draw_set_source_color_alpha (draw, color, alpha);
    cairo_set_operator (draw->cr, CAIRO_OPERATOR_OVER);

    for (i = 0; i < n_requests; i++) {
        vteunistr c = requests[i].c;
        struct unistr_info *uinfo = font_info_get_unistr_info (font, c);
        union unistr_font_info *ufi = &uinfo->ufi;
        int x, y;

        _vte_draw_get_char_edges(draw, c, requests[i].columns, style, &x, NULL);
        x += requests[i].x;
        y = requests[i].y + draw->char_spacing.top + font->ascent;

        if (_vte_draw_unichar_is_local_graphic(c)) {
            _vte_draw_terminal_draw_graphic(draw, c, color,
                                            requests[i].x, requests[i].y,
                                            font->width, requests[i].columns, font->height);
            continue;
        }

        switch (uinfo->coverage) {
            default:
            case COVERAGE_UNKNOWN:
                g_assert_not_reached ();
                break;
            case COVERAGE_USE_PANGO_LAYOUT_LINE:
                cairo_move_to (draw->cr, x, y);
                pango_cairo_show_layout_line (draw->cr,
                                ufi->using_pango_layout_line.line);
                break;
            case COVERAGE_USE_PANGO_GLYPH_STRING:
                cairo_move_to (draw->cr, x, y);
                pango_cairo_show_glyph_string (draw->cr,
                                ufi->using_pango_glyph_string.font,
                                ufi->using_pango_glyph_string.glyph_string);
                break;
            case COVERAGE_USE_CAIRO_GLYPH:
                if (last_scaled_font != ufi->using_cairo_glyph.scaled_font || n_cr_glyphs == MAX_RUN_LENGTH) {
                    if (n_cr_glyphs) {
                        cairo_set_scaled_font (draw->cr, last_scaled_font);
                        cairo_show_glyphs (draw->cr, cr_glyphs, n_cr_glyphs);
                        n_cr_glyphs = 0;
                    }
                    last_scaled_font = ufi->using_cairo_glyph.scaled_font;
                }
                cr_glyphs[n_cr_glyphs].index = ufi->using_cairo_glyph.glyph_index;
                cr_glyphs[n_cr_glyphs].x = x;
                cr_glyphs[n_cr_glyphs].y = y;
                n_cr_glyphs++;
                break;
        }
    }
    if (n_cr_glyphs) {
        cairo_set_scaled_font (draw->cr, last_scaled_font);
        cairo_show_glyphs (draw->cr, cr_glyphs, n_cr_glyphs);
        n_cr_glyphs = 0;
    }
}

void _vte_draw_text (struct _vte_draw *draw,
                        struct _vte_draw_text_request *requests, gsize n_requests,
                        vte::color::rgb const* color, double alpha, guint style)
{
    g_assert(draw->cr);

    if (_vte_debug_on (VTE_DEBUG_DRAW)) {
        GString *string = g_string_new ("");
        gchar *str;
        gsize n;
        for (n = 0; n < n_requests; n++) {
            g_string_append_unichar (string, requests[n].c);
        }
        str = g_string_free (string, FALSE);
        g_printerr ("draw_text (\"%s\", len=%" G_GSIZE_FORMAT ", color=(%d,%d,%d,%.3f), %s - %s)\n",
                str, n_requests, color->red, color->green, color->blue, alpha,
                (style & VTE_DRAW_BOLD)   ? "bold"   : "normal",
                (style & VTE_DRAW_ITALIC) ? "italic" : "regular");
        g_free (str);
    }

    _vte_draw_text_internal (draw, requests, n_requests, color, alpha, style);

    /* handle fonts that lack a bold face by double-striking */
    if ((style & VTE_DRAW_BOLD) && !_vte_draw_has_bold (draw, style)) {
        gsize i;

        /* Take a step to the right. */
        for (i = 0; i < n_requests; i++) {
            requests[i].x++;
        }
        _vte_draw_text_internal (draw, requests, n_requests, color, alpha, style);
        /* Now take a step back. */
        for (i = 0; i < n_requests; i++) {
            requests[i].x--;
        }
    }
}

/* The following two functions are unused since commit 154abade902850afb44115cccf8fcac51fc082f0,
 * but let's keep them for now since they may become used again.
 */
gboolean _vte_draw_has_char (struct _vte_draw *draw, vteunistr c, guint style)
{
    struct unistr_info *uinfo;

    _vte_debug_print (VTE_DEBUG_DRAW, "draw_has_char ('0x%04X', %s - %s)\n", c,
                (style & VTE_DRAW_BOLD)   ? "bold"   : "normal",
                (style & VTE_DRAW_ITALIC) ? "italic" : "regular");

    g_return_val_if_fail (draw->fonts[VTE_DRAW_NORMAL] != NULL, FALSE);

    uinfo = font_info_get_unistr_info (draw->fonts[style], c);
    return !uinfo->has_unknown_chars;
}

gboolean _vte_draw_char (struct _vte_draw *draw,
                            struct _vte_draw_text_request *request,
                            vte::color::rgb const* color, double alpha, guint style)
{
    gboolean has_char;

    _vte_debug_print (VTE_DEBUG_DRAW,
                        "draw_char ('%c', color=(%d,%d,%d,%.3f), %s, %s)\n",
                        request->c,
                        color->red, color->green, color->blue,
                        alpha,
                        (style & VTE_DRAW_BOLD)   ? "bold"   : "normal",
                        (style & VTE_DRAW_ITALIC) ? "italic" : "regular");

    has_char =_vte_draw_has_char (draw, request->c, style);
    if (has_char) {
        _vte_draw_text (draw, request, 1, color, alpha, style);
    }

    return has_char;
}

void _vte_draw_draw_rectangle (struct _vte_draw *draw,
                                gint x, gint y, gint width, gint height,
                                vte::color::rgb const* color, double alpha)
{
    g_assert(draw->cr);

    _vte_debug_print (VTE_DEBUG_DRAW,
                        "draw_rectangle (%d, %d, %d, %d, color=(%d,%d,%d,%.3f))\n",
                        x,y,width,height,
                        color->red, color->green, color->blue,
                        alpha);

    cairo_set_operator (draw->cr, CAIRO_OPERATOR_OVER);
    cairo_rectangle (draw->cr, x+VTE_LINE_WIDTH/2., y+VTE_LINE_WIDTH/2., width-VTE_LINE_WIDTH, height-VTE_LINE_WIDTH);
    _vte_draw_set_source_color_alpha (draw, color, alpha);
    cairo_set_line_width (draw->cr, VTE_LINE_WIDTH);
    cairo_stroke (draw->cr);
}

void _vte_draw_fill_rectangle (struct _vte_draw *draw,
                                gint x, gint y, gint width, gint height,
                                vte::color::rgb const* color, double alpha)
{
    g_assert(draw->cr);

    _vte_debug_print (VTE_DEBUG_DRAW,
                        "draw_fill_rectangle (%d, %d, %d, %d, color=(%d,%d,%d,%.3f))\n",
                        x,y,width,height,
                        color->red, color->green, color->blue,
                        alpha);

    cairo_set_operator (draw->cr, CAIRO_OPERATOR_OVER);
    cairo_rectangle (draw->cr, x, y, width, height);
    _vte_draw_set_source_color_alpha (draw, color, alpha);
    cairo_fill (draw->cr);
}


void _vte_draw_draw_line(struct _vte_draw *draw,
                            gint x, gint y, gint xp, gint yp,
                            int line_width,
                            vte::color::rgb const *color, double alpha)
{
    _vte_draw_fill_rectangle(draw,
                                 x, y,
                                 MAX(line_width, xp - x + 1), MAX(line_width, yp - y + 1),
                                 color, alpha);
}

static inline double _vte_draw_get_undercurl_rad(gint width)
{
    return width / 2. / sqrt(2);
}

static inline double _vte_draw_get_undercurl_arc_height(gint width)
{
    return _vte_draw_get_undercurl_rad(width) * (1. - sqrt(2) / 2.);
}

double _vte_draw_get_undercurl_height(gint width, double line_width)
{
    return 2. * _vte_draw_get_undercurl_arc_height(width) + line_width;
}

void _vte_draw_draw_undercurl(struct _vte_draw *draw,
                                gint x, double y,
                                double line_width,
                                gint count,
                                vte::color::rgb const *color, double alpha)
{
    /* The end of the curly line slightly overflows to the next cell, so the canvas
        * caching the rendered look has to be wider not to chop this off. */
    gint x_padding = line_width + 1;  /* ceil, kind of */

    gint surface_top = y;  /* floor */

    g_assert(draw->cr);

    _vte_debug_print (VTE_DEBUG_DRAW,
                        "draw_undercurl (x=%d, y=%f, count=%d, color=(%d,%d,%d,%.3f))\n",
                        x, y, count,
                        color->red, color->green, color->blue,
                        alpha);

    if (G_UNLIKELY (draw->undercurl_surface == NULL)) {
       /* Cache the undercurl's look. The design assumes that until the cached look is
        * invalidated (the font is changed), this method is always called with the "y"
        * parameter having the same fractional part, and the same "line_width" parameter.
        * For caching, only the fractional part of "y" is used. */
        cairo_t *undercurl_cr;

        double rad = _vte_draw_get_undercurl_rad(draw->cell_width);
        double y_bottom = y + _vte_draw_get_undercurl_height(draw->cell_width, line_width);
        double y_center = (y + y_bottom) / 2.;
        gint surface_bottom = y_bottom + 1;  /* ceil, kind of */

        _vte_debug_print (VTE_DEBUG_DRAW,
                            "caching undercurl shape\n");

        /* Add a line_width of margin horizontally on both sides, for nice antialias overflowing. */
        draw->undercurl_surface = cairo_surface_create_similar (cairo_get_target (draw->cr),
                                                                CAIRO_CONTENT_ALPHA,
                                                                draw->cell_width + 2 * x_padding,
                                                                surface_bottom - surface_top);
        undercurl_cr = cairo_create (draw->undercurl_surface);
        cairo_set_operator (undercurl_cr, CAIRO_OPERATOR_OVER);
        /* First quarter circle, similar to the left half of the tilde symbol. */
        cairo_arc (undercurl_cr, x_padding + draw->cell_width / 4., y_center - surface_top + draw->cell_width / 4., rad, M_PI * 5 / 4, M_PI * 7 / 4);
        /* Second quarter circle, similar to the right half of the tilde symbol. */
        cairo_arc_negative (undercurl_cr, x_padding + draw->cell_width * 3 / 4., y_center - surface_top - draw->cell_width / 4., rad, M_PI * 3 / 4, M_PI / 4);
        cairo_set_line_width (undercurl_cr, line_width);
        cairo_stroke (undercurl_cr);
        cairo_destroy (undercurl_cr);
    }

   /* Paint the cached look of the undercurl using the desired look.
    * The cached look takes the fractional part of "y" into account,
    * here we only offset by its integer part. */
    cairo_save (draw->cr);
    cairo_set_operator (draw->cr, CAIRO_OPERATOR_OVER);
    _vte_draw_set_source_color_alpha (draw, color, alpha);
    for (int i = 0; i < count; i++) {
        cairo_mask_surface (draw->cr, draw->undercurl_surface, x - x_padding + i * draw->cell_width, surface_top);
    }
    cairo_restore (draw->cr);
}
