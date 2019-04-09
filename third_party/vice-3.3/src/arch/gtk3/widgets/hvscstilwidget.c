/** \file   hvscstilwidget.c
 * \brief   High Voltage SID Collection STIL widget
 *
 * Widget providing access to the SID Tune Information List, a document
 * containing extra information of SID tunes (author comments, trivia, which
 * songs were covered (if any)).
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <gtk/gtk.h>

#include "resources.h"
#include "vice_gtk3.h"
#include "debug_gtk3.h"
#include "hvsc.h"
#include "vsidtuneinfowidget.h"

#include "hvscstilwidget.h"


/** \brief  GtkTextArea used to present the STIL entry
 */
static GtkWidget *stil_view;



static int hvsc_stil_widget_create_tags(void)
{

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(stil_view));

    gtk_text_buffer_create_tag(
            buffer,
            "main",
            "left-margin", 16,
            "right-margin", 16,
            NULL);

    gtk_text_buffer_create_tag(
            buffer,
            "main-comment",
            "style", PANGO_STYLE_OBLIQUE,
            "weight", PANGO_WEIGHT_BOLD,
            "left-margin", 4,
            "right-margin", 4,
            NULL);

    gtk_text_buffer_create_tag(
            buffer,
            "tune-header",
            "left-margin", 8,
            "foreground", "blue",
            NULL);

    gtk_text_buffer_create_tag(
            buffer,
            "tune-comment",
            "left-margin", 16,
            "style", PANGO_STYLE_ITALIC,
            NULL);

    gtk_text_buffer_create_tag(
            buffer,
            "timestamp",
            "left-margin", 16,
            "family", "Monospace",
            NULL);

    gtk_text_buffer_create_tag(
            buffer,
            "album",
            "left-margin", 16,
            "foreground", "green",
            NULL);

    gtk_text_buffer_create_tag(
            buffer,
            "title",
            "left-margin", 16,
            "weight", PANGO_WEIGHT_BOLD,
            NULL);

    gtk_text_buffer_create_tag(
            buffer,
            "artist",
            "left-margin", 16,
            "foreground", "darkgreen",
            "weight", PANGO_WEIGHT_MEDIUM,
            NULL);


    return 1;


}



/** \brief  Create TextView widget for the STIL entry text
 *
 * \return  GtkTextView
 */
static GtkWidget *create_view(void)
{
    GtkWidget *textview = gtk_text_view_new();

    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
    gtk_widget_set_can_focus(textview, FALSE);
    gtk_widget_set_vexpand(textview, TRUE);
    return textview;
}



/** \brief  Create HVSC STIL widget
 *
 * \return  GtkGrid
 */
GtkWidget *hvsc_stil_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *scroll;


    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    /* add title label */
    label = gtk_label_new(NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_label_set_markup(GTK_LABEL(label), "<b>STIL entry:</b>");
    g_object_set(G_OBJECT(label), "margin-bottom", 16, NULL);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    /* add view */
    stil_view = create_view();


    hvsc_stil_widget_create_tags();

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_size_request(scroll, 400, 500);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), stil_view);
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 1, 1, 1);

    gtk_widget_set_vexpand(grid, TRUE);
    gtk_widget_show_all(grid);
    return grid;
}




int hvsc_stil_widget_set_psid(const char *psid)
{
    hvsc_stil_t stil;
    size_t t;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(stil_view));
    char line[1024];

    GtkTextIter start;
    GtkTextIter end;


    debug_gtk3("attempting to load STIL entry for '%s'.", psid);
    if (!hvsc_stil_get(&stil, psid)) {
        debug_gtk3("failed: %d: %s.", hvsc_errno, hvsc_strerror(hvsc_errno));
        /* check hvsc error code to see if either loading the STIL failed or
         * there was no STIL entry for the tune requested */
        if (hvsc_errno == HVSC_ERR_NOT_FOUND) {
            gtk_text_buffer_set_text(buffer, "No STIL entry found.", -1);
        } else {
            gtk_text_buffer_set_text(buffer, "Failed to load STIL.", -1);
        }
        return 0;
    }

    gtk_text_buffer_set_text(buffer, "", -1);
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_apply_tag_by_name(buffer, "main", &start, &end);

    if (stil.sid_comment != NULL) {

        gtk_text_buffer_insert_with_tags_by_name(
                buffer,
                &end,
                stil.sid_comment,
                -1,
                "main-comment",
                NULL);
        /* add newlines */
        gtk_text_buffer_insert(buffer, &end, "\n\n", -1);
    }

    /* now add info on each subtune */
    for (t = 0; t < stil.blocks_used; t++) {
        size_t f;
        hvsc_stil_block_t *block;

        block = stil.blocks[t];

        if (t == 0 && block->fields_used == 0) {
            continue;
        }

        if (block->tune == 0) {
            /* FIXME: might be a bug in my HVSC 'lib' */
            continue;
        }

        g_snprintf(line, 1024, "tune #%d:\n", block->tune);

        gtk_text_buffer_insert_with_tags_by_name(
                buffer,
                &end,
                line,
                -1,
                "tune-header",
                NULL);

        /* handle fields for the current subtune */
        for (f = 0; f < block->fields_used; f++) {

#if 0
            g_snprintf(line, 1024, "    %s %s\n",
                    hvsc_get_field_display(block->fields[f]->type),
                    block->fields[f]->text);
            gtk_text_buffer_insert_at_cursor(
                    GTK_TEXT_BUFFER(buffer), line, -1);
#endif
            if (block->fields[f]->type == HVSC_FIELD_COMMENT) {
                /* add tune-specific comment */

                gchar *utf8;

                g_snprintf(line, 1024, "%s\n", block->fields[f]->text);
                utf8 = convert_to_utf8(line);

                gtk_text_buffer_insert_with_tags_by_name(
                        buffer,
                        &end,
                        utf8,
                        -1,
                        "tune-comment",
                        NULL);

                g_free(utf8);
            }

            /* timestamp? */
            if (block->fields[f]->timestamp.from >= 0) {
                /* yup */
                long from = block->fields[f]->timestamp.from;
                long to = block->fields[f]->timestamp.to;

                if (to < 0) {
                    g_snprintf(line, 1024,
                            "%ld:%02ld\n",
                            from / 60, from % 60);
                } else {
                    g_snprintf(line, 1024,
                            "%ld:%02ld-%ld:%02ld\n",
                            from / 60, from % 60, to / 60, to % 60);
                }

                gtk_text_buffer_insert_with_tags_by_name(
                        buffer,
                        &end,
                        line,
                        -1,
                        "timestamp",
                        NULL);
#if 0
                gtk_text_buffer_insert_at_cursor(
                        GTK_TEXT_BUFFER(buffer), line, -1);
#endif
            }

            /* title? */
            if (block->fields[f]->type == HVSC_FIELD_TITLE
                    && block->fields[f]->text != NULL) {

                gchar *utf8;

                g_snprintf(line, 1024, "%s\n", block->fields[f]->text);
                utf8 = convert_to_utf8(line);

                gtk_text_buffer_insert_with_tags_by_name(
                        buffer,
                        &end,
                        utf8,
                        -1,
                        "title",
                        NULL);

                g_free(utf8);
            }


            /* artist? */
            if (block->fields[f]->type == HVSC_FIELD_ARTIST
                    && block->fields[f]->text != NULL) {

                gchar *utf8;

                g_snprintf(line, 1024, "%s\n", block->fields[f]->text);
                utf8 = convert_to_utf8(line);

                gtk_text_buffer_insert_with_tags_by_name(
                        buffer,
                        &end,
                        utf8,
                        -1,
                        "artist",
                        NULL);
                g_free(utf8);
            }

            /* album? */
            if (block->fields[f]->album != NULL) {

                gchar *utf8;

                g_snprintf(line, 1024, "%s\n",
                        block->fields[f]->album);
                utf8 = convert_to_utf8(line);

                gtk_text_buffer_insert_with_tags_by_name(
                        buffer,
                        &end,
                        line,
                        -1,
                        "album",
                        NULL);

                g_free(utf8);
            }
        }

        /* add newlines when not last subtune */
        if (t < stil.blocks_used - 1) {
            gtk_text_buffer_insert(buffer, &end, "\n\n", -1);
        }
    }

    hvsc_stil_close(&stil);
    return 1;
}
