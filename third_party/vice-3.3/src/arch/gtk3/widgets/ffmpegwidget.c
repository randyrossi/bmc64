/** \file   ffmpegwidget.c
 * \brief   FFMPEG media recording options dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES FFMPEGFormat                -vsid
 * $VICERES FFMPEGAudioBitrate          -vsid
 * $VICERES FFMPEGVideoBitrate          -vsid
 * $VICERES FFMPEGAudioCodec            -vsid
 * $VICERES FFMPEGVideoCodec            -vsid
 * $VICERES FFMPEGVideoHalveFramerate   -vsid
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
 */

#include "vice.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "debug_gtk3.h"
#include "lib.h"
#include "resources.h"
#include "machine.h"
#include "widgethelpers.h"
#include "basewidgets.h"
#include "ui.h"
#include "gfxoutput.h"

#include "ffmpegwidget.h"


/** \brief  FFMPEG driver info
 *
 * Retrieved via gfxoutput_get_driver(), gets cleaned up when the gfxoutput
 * sub system exits.
 */
static gfxoutputdrv_t *driver_info = NULL;

/* references to combo boxes, used in various event handlers */
static GtkWidget *format_widget = NULL;
static GtkWidget *video_widget = NULL;
static GtkWidget *audio_widget = NULL;


static GtkListStore *create_video_model(int fmt);
static GtkListStore *create_audio_model(int fmt);
static void update_video_combo_box(int id);
static void update_audio_combo_box(int id);


/*****************************************************************************
 *                              Event handlers                               *
 ****************************************************************************/

/** \brief  Handler for the "changed" event of the format combo box
 *
 * \param[in]   widget  format combo box
 * \param[in]   data    extra event data (unused)
 */
static void on_format_changed(GtkWidget *widget, gpointer data)
{
    GtkListStore *video;
    GtkListStore *audio;
    GtkTreeIter iter;
    int fmt_id;
    const char *fmt_name = NULL;
    int vc;
    int ac;

    fmt_id = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    /* convoluted way to get the displayed text of the combo box */
    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)) {
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        gtk_tree_model_get(model, &iter, 0, &fmt_name, -1);
    }

    debug_gtk3("called, index %d: '%s'.", fmt_id, fmt_name);
    if (fmt_name != NULL && *fmt_name != '\0') {
        resources_set_string("FFMPEGFormat", fmt_name);
    }


    video = create_video_model(fmt_id);
    gtk_combo_box_set_active(GTK_COMBO_BOX(video_widget), 0);
    gtk_combo_box_set_model(GTK_COMBO_BOX(video_widget),
            GTK_TREE_MODEL(video));
    if (resources_get_int("FFMPEGVideoCodec", &vc) < 0) {
        vc = 0;
    }
    update_video_combo_box(vc);

    audio = create_audio_model(fmt_id);
    gtk_combo_box_set_active(GTK_COMBO_BOX(audio_widget), 0);
    gtk_combo_box_set_model(GTK_COMBO_BOX(audio_widget),
            GTK_TREE_MODEL(audio));
    if (resources_get_int("FFMPEGAudioCodec", &ac) < 0) {
        ac = 0;
    }
    update_audio_combo_box(ac);
}


/** \brief  Handler for the "changed" event of the video codec combo box
 *
 * \param[in]   combo   video codec combo box
 * \param[in]   data    extra event data (unused)
 */
static void on_video_codec_changed(GtkComboBox *combo, gpointer data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_combo_box_get_active(combo) < 0) {
        return;
    }

    model = gtk_combo_box_get_model(combo);
    if (gtk_combo_box_get_active_iter(combo, &iter)) {
        int codec;

        gtk_tree_model_get(model, &iter, 1, &codec, -1);
        debug_gtk3("setting FFMPEGVideoCodec to %d.", codec);
        resources_set_int("FFMPEGVideoCodec", codec);
    }
}


/** \brief  Handler for the "changed" event of the audio codec combo box
 *
 * \param[in]   combo   audio codec combo box
 * \param[in]   data    extra event data (unused)
 */
static void on_audio_codec_changed(GtkComboBox *combo, gpointer data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_combo_box_get_active(combo) < 0) {
        return;
    }

    model = gtk_combo_box_get_model(combo);
    if (gtk_combo_box_get_active_iter(combo, &iter)) {
        int codec;

        gtk_tree_model_get(model, &iter, 1, &codec, -1);
        debug_gtk3("setting FFMPEGAudioCodec to %d.", codec);
        resources_set_int("FFMPEGAudioCodec", codec);
    }
}


/*****************************************************************************
 *                              Helper functions                             *
 ****************************************************************************/

/** \brief  Get index in format list of \a fmt
 *
 * \param[in]   fmt format name
 *
 * \return  index in list or -1 when not found
 */
static int get_format_index_by_name(const char *fmt)
{
    int i;

    for (i = 0; driver_info->formatlist[i].name != NULL; i++) {
        if (strcmp(driver_info->formatlist[i].name, fmt) == 0) {
            return i;
        }
    }
    return -1;
}


/** \brief  Create a model for the FFMPEG output formats
 *
 * \return  GtkListStore
 */
static GtkListStore *create_format_model(void)
{
    GtkListStore *model;
    GtkTreeIter iter;
    int i;

    model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for (i = 0; driver_info->formatlist[i].name != NULL; i++) {
        const char *name = driver_info->formatlist[i].name;

        /*debug_gtk3("adding FFMPEG format '%s'.", name);*/
        gtk_list_store_append(model, &iter);
        gtk_list_store_set(model, &iter, 0, name, 1, i, -1);
    }

    return model;
}


/** \brief  Create a model for the video codecs of \a fmt
 *
 * \return  GtkListStore
 */
static GtkListStore *create_video_model(int fmt)
{
    GtkListStore *store;
    GtkTreeIter iter;
    gfxoutputdrv_codec_t *codec_list;
    int i;


    codec_list = driver_info->formatlist[fmt].video_codecs;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    if (codec_list == NULL) {
        return store;
    }

    for (i = 0; codec_list[i].name != NULL; i++) {
        const char *name = codec_list[i].name;
        int id = codec_list[i].id;

        debug_gtk3("adding FFMPEG video codec '%s' (%d).", name, id);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, name, 1, id, -1);
    }

    return store;
}


/** \brief  Create a GtkListStore for the audio codecs of \a fmt
 *
 * \return  GtkListStore
 */
static GtkListStore *create_audio_model(int fmt)
{
    GtkListStore *store;
    GtkTreeIter iter;
    gfxoutputdrv_codec_t *codec_list;
    int i;


    codec_list = driver_info->formatlist[fmt].audio_codecs;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    if (codec_list == NULL) {
        return store;
    }

    for (i = 0; codec_list[i].name != NULL; i++) {
        const char *name = codec_list[i].name;
        int id = codec_list[i].id;

        /*debug_gtk3("adding FFMPEG audio codec '%s' (%d).", name, id);*/
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, name, 1, id, -1);
    }

    return store;
}



/** \brief  Create a left-aligned, 16px indented label
 *
 * \param[in]   text    label text
 *
 * \return GtkLabel
 */
static GtkWidget *create_indented_label(const gchar *text)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    return label;
}


/** \brief  Create combo box with supported output formats
 *
 * \return  GtkComboBox
 */
static GtkWidget *create_format_combo_box(void)
{
    GtkWidget *combo;
    GtkListStore *model;
    GtkCellRenderer *renderer;

    model = create_format_model();
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(model));
    /* combo takes ownership, so we can unref the store now */
    g_object_unref(model);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
            "text", 0, NULL);
    return combo;
}


/** \brief  Update the format combo box
 *
 * \param[in]   fmt FFMPEG driver format name
 */
static void update_format_combo_box(const char * fmt)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    int index = 0;

    /* get the model and get an iterator to its first element */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(format_widget));
    if (!gtk_tree_model_get_iter_first(model, &iter)) {
        return;
    }

    do {
        const gchar *s;

        gtk_tree_model_get(model, &iter, 0, &s, -1);
        if (strcmp(s, fmt) == 0) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(format_widget), index);
            return;
        }
        index++;
    } while (gtk_tree_model_iter_next(model, &iter));

    /* not found, set index to 0 (shouldn't happen) */
    gtk_combo_box_set_active(GTK_COMBO_BOX(format_widget), 0);
}


/** \brief  Update the video codec combo box
 *
 * \param[in]   id  video codec ID
 */
static void update_video_combo_box(int id)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    int index = 0;

    /* get the model and get an iterator to its first element */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(video_widget));
    if (!gtk_tree_model_get_iter_first(model, &iter)) {
        return;
    }

    do {
        int codec_id;

        gtk_tree_model_get(model, &iter, 1, &codec_id, -1);
        if (codec_id == id) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(video_widget), index);
        }
        index++;
    } while (gtk_tree_model_iter_next(model, &iter));

    /* not found, set index to 0 (shouldn't happen) */
    gtk_combo_box_set_active(GTK_COMBO_BOX(video_widget), 0);
}


/** \brief  Update the audio codec combo box
 *
 * \param[in]   id  audio codec ID
 */
static void update_audio_combo_box(int id)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    int index = 0;

    /* get the model and get an iterator to its first element */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(audio_widget));
    if (!gtk_tree_model_get_iter_first(model, &iter)) {
        return;
    }

    do {
        int codec_id;

        gtk_tree_model_get(model, &iter, 1, &codec_id, -1);
        if (codec_id == id) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(audio_widget), index);
        }
        index++;
    } while (gtk_tree_model_iter_next(model, &iter));

    /* not found, set index to 0 (shouldn't happen) */
    gtk_combo_box_set_active(GTK_COMBO_BOX(audio_widget), 0);
}


/** \brief  Create combo box with supported video codecs for \a fmt
 *
 * \return  GtkComboBox
 * ne
 */
static GtkWidget *create_video_combo_box(int fmt)
{
    GtkWidget *combo;
    GtkListStore *model;
    GtkCellRenderer *renderer;

    model = create_video_model(fmt);
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(model));
    /* combo takes ownership, so we can unref the store now */
    g_object_unref(model);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
            "text", 0, NULL);

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    return combo;
}


/** \brief  Create combo box with supported audio codecs for \a fmt
 *
 * \return  GtkComboBox
 */
static GtkWidget *create_audio_combo_box(int fmt)
{
    GtkWidget *combo;
    GtkListStore *model;
    GtkCellRenderer *renderer;

    model = create_audio_model(fmt);
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(model));
    /* combo takes ownership, so we can unref the store now */
    g_object_unref(model);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
            "text", 0, NULL);

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    return combo;
}



/*****************************************************************************
 *                              Public functions                             *
 ****************************************************************************/

/** \brief  Create widget to control FFMPEG output options
 *
 * \return  GtkGrid
 */
GtkWidget *ffmpeg_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *fps;
    const char *current_format = NULL;
    int fmt_index;
    int current_vc;
    int current_ac;

    /* retrieve FFMPEG driver info */
    driver_info = gfxoutput_get_driver("FFMPEG");

    /* get current FFMPEG format */
    if (resources_get_string("FFMPEGFormat", &current_format) < 0) {
        current_format = "avi"; /* hope this works out */
    }
    /* get index in table of format */
    fmt_index = get_format_index_by_name(current_format);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    /* format selection */
    label = create_indented_label("format");
    format_widget = create_format_combo_box();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), format_widget, 1, 0, 3, 1);
    update_format_combo_box(current_format);

    /* video codec selection */
    label = create_indented_label("video codec");
    video_widget = create_video_combo_box(fmt_index);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), video_widget, 1, 1, 1, 1);
    if (resources_get_int("FFMPEGVideoCodec", &current_vc) < 0) {
        current_vc = 0;
    }
    update_video_combo_box(current_vc);

    /* audio codec selection */
    label = create_indented_label("audio codec");
    audio_widget = create_audio_combo_box(fmt_index);
    gtk_grid_attach(GTK_GRID(grid), label, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), audio_widget, 3, 1, 1, 1);
    if (resources_get_int("FFMPEGAudioCodec", &current_ac) < 0) {
        current_ac = 0;
    }
    update_audio_combo_box(current_ac);

    /* video codec bitrate */
    label = create_indented_label("video bitrate");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid),
            vice_gtk3_resource_spin_int_new("FFMPEGVideoBitrate",
                VICE_FFMPEG_VIDEO_RATE_MIN, VICE_FFMPEG_VIDEO_RATE_MAX,
                10000),
            1, 2, 1, 1);

    /* audio codec bitrate */
    label = create_indented_label("audio bitrate");
    gtk_grid_attach(GTK_GRID(grid), label, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid),
            vice_gtk3_resource_spin_int_new("FFMPEGAudioBitrate",
                VICE_FFMPEG_AUDIO_RATE_MIN, VICE_FFMPEG_AUDIO_RATE_MAX,
                1000),
            3, 2, 1, 1);

    /* half-FPS widget */
    fps = vice_gtk3_resource_check_button_new("FFMPEGVideoHalveFramerate",
            "Half framerate (25/30 FPS)"),
    gtk_widget_set_halign(fps, GTK_ALIGN_START);
    g_object_set(fps, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), fps, 0, 3, 4, 1);

    update_format_combo_box(current_format);

    /* connect event handlers */
    g_signal_connect(format_widget, "changed",
            G_CALLBACK(on_format_changed), NULL);
    g_signal_connect(video_widget, "changed",
            G_CALLBACK(on_video_codec_changed), NULL);
    g_signal_connect(audio_widget, "changed",
            G_CALLBACK(on_audio_codec_changed), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
