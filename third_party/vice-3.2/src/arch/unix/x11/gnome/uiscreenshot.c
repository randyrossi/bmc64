/*
 * uiscreenshot.c - screenshot dialog for the Gnome widget set.
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
 *
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

#include <stdio.h>
#include <string.h>

#include "gfxoutput.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "screenshot.h"
#include "ui.h"
#include "uiapi.h"
#include "uiarch.h"
#include "uifileentry.h"
#include "uilib.h"
#include "uiscreenshot.h"
#include "util.h"

/* #define DEBUG_UI */

#ifdef DEBUG_UI
#define DBG(x)  printf x
#else
#define DBG(x)
#endif

#ifdef HAVE_FFMPEG
#include "gfxoutputdrv/ffmpegdrv.h"
#endif

/* #undef HAVE_FFMPEG */

extern GtkWidget *video_ctrl_checkbox;
static GtkWidget *screenshot_dialog;
static GtkWidget *drv_menu;

static GtkWidget *native_opts, *native_oversize, *native_undersize, *native_mcolor;
static GtkWidget *native_tedluma, *native_crtccolor;

#ifdef HAVE_FFMPEG
static GtkWidget *ffmpg_opts, *ffmpg_audio, *ffmpg_video;
static GtkWidget *ffmpeg_omenu, *acmenu, *vcmenu, *fpsmenu, *ffmpg_audio, *ffmpg_video;
static char *selected_driver = NULL;
static int selected_ac, selected_vc;
static void ffmpeg_details (GtkWidget *w, gpointer data);
#endif

typedef struct {
    const char *driver;
    const char *ext;
    GtkWidget *w;
} img_type_buttons;

static img_type_buttons *buttons = NULL;
static int combo_box_current_active = 0;

#ifdef HAVE_FFMPEG
enum { DRV_NAME, DRV_INDEX, DRV_ACMENU, DRV_VCMENU, DRV_N };
#endif

/******************************************************************************/

static int is_koala(void)
{
    int d = gtk_combo_box_get_active(GTK_COMBO_BOX(drv_menu));
    if ((strcmp(buttons[d].driver, "KOALA") == 0) ||
        (strcmp(buttons[d].driver, "KOALA_COMPRESSED") == 0)) {
        return 1;
    }
    return 0;
}

static int is_doodle(void)
{
    int d = gtk_combo_box_get_active(GTK_COMBO_BOX(drv_menu));
    if ((strcmp(buttons[d].driver, "DOODLE") == 0) ||
        (strcmp(buttons[d].driver, "DOODLE_COMPRESSED") == 0)) {
        return 1;
    }
    return 0;
}
static int is_native(void)
{
    return is_koala() || is_doodle();
}

/* "output driver" combo box */
static void ffmpeg_output_driver_changed(GtkWidget *w, gpointer data)
{
    int i;

    g_return_if_fail(GTK_IS_COMBO_BOX(w));

    i = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    log_verbose("ffmpeg_output_driver_changed (driver:%s)\n", buttons[i].driver);
#ifdef HAVE_FFMPEG
    if (strcmp(buttons[i].driver, "FFMPEG") == 0) {
        GtkListStore *store;
        GtkTreeIter iter;
        int index, found;
        char *drv_name = NULL;
        const char *current_driver;

        gtk_widget_hide(native_opts);
        gtk_widget_show(ffmpg_opts);
        resources_get_string("FFMPEGFormat", &current_driver);
        store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ffmpeg_omenu)));
        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
            do {
                gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &drv_name, 1, &index, -1);
                found = !strcmp(drv_name, current_driver);
                g_free(drv_name);
            } while (!found && gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter));
            if (found) {
                /* make copy through lib_stralloc() to avoid freeing const
                 * data */
                selected_driver = lib_stralloc(current_driver);
                gtk_combo_box_set_active(GTK_COMBO_BOX(ffmpeg_omenu), index);
            }
            gtk_widget_set_sensitive(ffmpg_opts, TRUE);
            return;
        }
        gtk_widget_set_sensitive(ffmpg_opts, FALSE);
    } else {
        if (ffmpg_opts) { 
            gtk_widget_hide(ffmpg_opts);
        }
#endif
        if (is_native()) {
            int n0 = 0, n1 = 0, n2 = 0, n3 = 0, n4 = 0;
            if(is_doodle()) {
                resources_get_int("DoodleOversizeHandling", &n0);
                resources_get_int("DoodleUndersizeHandling", &n1);
                resources_get_int("DoodleMultiColorHandling", &n2);
                if (machine_class == VICE_MACHINE_PLUS4) {
                    resources_get_int("DoodleTEDLumHandling", &n3);
                }
                if ((machine_class == VICE_MACHINE_PET) || (machine_class == VICE_MACHINE_CBM6x0)) {
                    resources_get_int("DoodleCRTCTextColor", &n4);
                }
                gtk_widget_set_sensitive(native_mcolor, TRUE);
            } else {
                resources_get_int("KoalaOversizeHandling", &n0);
                resources_get_int("KoalaUndersizeHandling", &n1);
                /* resources_get_int("KoalaMultiColorHandling", &n2); */
                if (machine_class == VICE_MACHINE_PLUS4) {
                    resources_get_int("KoalaTEDLumHandling", &n3);
                }
                if ((machine_class == VICE_MACHINE_PET) || (machine_class == VICE_MACHINE_CBM6x0)) {
                    resources_get_int("KoalaCRTCTextColor", &n4);
                }
                gtk_widget_set_sensitive(native_mcolor, FALSE);
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(native_oversize), n0);
            gtk_combo_box_set_active(GTK_COMBO_BOX(native_undersize), n1);
            gtk_combo_box_set_active(GTK_COMBO_BOX(native_mcolor), n2);
            if (machine_class == VICE_MACHINE_PLUS4) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(native_tedluma), n3);
            }
            if ((machine_class == VICE_MACHINE_PET) ||(machine_class == VICE_MACHINE_CBM6x0)) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(native_crtccolor), n4);
            }
            gtk_widget_set_sensitive(native_tedluma, (machine_class == VICE_MACHINE_PLUS4));
            gtk_widget_set_sensitive(native_crtccolor, ((machine_class == VICE_MACHINE_PET) || (machine_class == VICE_MACHINE_CBM6x0)));
            gtk_widget_show(native_opts);
            gtk_widget_set_sensitive(native_opts, TRUE);
            return;
        }
        gtk_widget_hide(native_opts);
        gtk_widget_set_sensitive(native_opts, FALSE);
#ifdef HAVE_FFMPEG
    }
#endif
}

/******************************************************************************
    actual FFMPEG options
 ******************************************************************************/

#ifdef HAVE_FFMPEG
static void ffmpeg_audio_codec_changed(GtkWidget *widget, gpointer data)
{
    GtkTreeModel *audioCodecStore;
    GtkTreeIter driverIterator, audioCodecIterator;
    gint selected;
    int index = -1;
    int id;

    /* get the selected driver. */
    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(data), &driverIterator) == FALSE) {
        log_error(LOG_DEFAULT, "internal error. iter in driver combo box not found.");
        return;
    }

    /* get the index of the selected audio codec. */
    selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if (selected == -1) {
        log_error(LOG_DEFAULT, "internal error: no item selected in combo box.");
        return;
    }

    /* get the data model from the driver combo box. */
    gtk_tree_model_get(GTK_TREE_MODEL(gtk_combo_box_get_model(GTK_COMBO_BOX(data))), &driverIterator, DRV_ACMENU, &audioCodecStore, -1);

    /* iterate over the audio codecs. */
    if (gtk_tree_model_get_iter_first(audioCodecStore, &audioCodecIterator)) {
        do {
            if (++index == selected) {
                /* get the ID for the selected index. */
                gtk_tree_model_get(GTK_TREE_MODEL(audioCodecStore), &audioCodecIterator, 1, &id, -1);
                selected_ac = id;
                break;
            }
        } while (gtk_tree_model_iter_next(audioCodecStore, &audioCodecIterator));
    }

}

static void ffmpeg_video_codec_changed(GtkWidget *widget, gpointer data)
{
    GtkTreeModel *videoCodecStore;
    GtkTreeIter driverIterator, videoCodecIterator;
    gint selected;
    int index = -1;
    int id;

    /* get the selected driver. */
    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(data), &driverIterator) == FALSE) {
        log_error(LOG_DEFAULT, "internal error. iter in driver combo box not found.");
        return;
    }

    /* get the index of the selected video codec. */
    selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if (selected == -1) {
        log_error(LOG_DEFAULT, "internal error: no item selected in combo box.");
        return;
    }

    /* get the data model from the driver combo box. */
    gtk_tree_model_get(GTK_TREE_MODEL(gtk_combo_box_get_model(GTK_COMBO_BOX(data))), &driverIterator, DRV_VCMENU, &videoCodecStore, -1);

    /* iterate over the video codecs. */
    if (gtk_tree_model_get_iter_first(videoCodecStore, &videoCodecIterator)) {
        do {
            if (++index == selected) {
                /* get the ID for the selected index. */
                gtk_tree_model_get(GTK_TREE_MODEL(videoCodecStore), &videoCodecIterator, 1, &id, -1);
                selected_vc = id;
                break;
            }
        } while (gtk_tree_model_iter_next(videoCodecStore, &videoCodecIterator));
    }

}

/* ffmpeg format combo box */
static void ffmpeg_details(GtkWidget *w, gpointer data)
{
    int current_ac_id, current_vc_id;
    const char *current_driver;
    GtkTreeIter iter;
    GtkListStore *ac_store, *vc_store;

    gchararray sd_tmp = NULL;

    g_return_if_fail(GTK_IS_COMBO_BOX(w));

    resources_get_int("FFMPEGAudioCodec", &current_ac_id);
    resources_get_int("FFMPEGVideoCodec", &current_vc_id);
    resources_get_string("FFMPEGFormat", &current_driver);

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(w), &iter) == FALSE) {
        log_error(LOG_DEFAULT, "internal error. iter in combo box not found.");
        return;
    }

    if (selected_driver != NULL) {
        lib_free(selected_driver);
    }

    gtk_tree_model_get(GTK_TREE_MODEL(gtk_combo_box_get_model(GTK_COMBO_BOX(w))), &iter, DRV_NAME, &sd_tmp, DRV_ACMENU, &ac_store, DRV_VCMENU, &vc_store, -1);
    /* Make a copy so we can later use lib_free(). Things allocated by
     * Gtk/GObject should be freed with g_free() not lib_free(), this caused
     * uiscreenshot_shutdown() to leak memory */
    selected_driver = lib_stralloc(sd_tmp);
    g_free(sd_tmp);

    if (ac_store) {
        GtkListStore *store;
        GtkTreeIter iter;
        int index, id, found;

        store = ac_store;
        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
            do {
                gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &id, 2, &index, -1);
                found = (id == current_ac_id);
            } while (!found && gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter));
            gtk_combo_box_set_model(GTK_COMBO_BOX(acmenu), GTK_TREE_MODEL(ac_store));
            gtk_widget_set_sensitive(GTK_WIDGET(acmenu), TRUE);
            if (found) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(acmenu), index);
            } else {
                gtk_combo_box_set_active(GTK_COMBO_BOX(acmenu), 0);
            }
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(acmenu), FALSE);
        }
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(acmenu), FALSE);
    }

    if (vc_store) {
        GtkListStore *store;
        GtkTreeIter iter;
        int index, id, found;

        store = vc_store;
        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
            do {
                gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &id, 2, &index, -1);
                found = (id == current_vc_id);
            } while (!found && gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter));
            gtk_combo_box_set_model(GTK_COMBO_BOX(vcmenu), GTK_TREE_MODEL(vc_store));
            gtk_widget_set_sensitive(GTK_WIDGET(vcmenu), TRUE);
            if (found) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(vcmenu), index);
            } else {
                gtk_combo_box_set_active(GTK_COMBO_BOX(vcmenu), 0);
            }
            gtk_widget_set_sensitive(GTK_WIDGET(fpsmenu), TRUE);
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(vcmenu), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(fpsmenu), FALSE);
        }
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(vcmenu), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(fpsmenu), FALSE);
    }
}

/* "Half Framerate" checkbox */
static void update_halffps_checkbox(GtkWidget *w, gpointer data)
{
    if (!w || !GTK_IS_TOGGLE_BUTTON(w)) {
        return;
    }
    resources_set_int("FFMPEGVideoHalveFramerate", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
    DBG(("update_halffps_checkbox (%d)\n", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))));
}

#endif

/******************************************************************************/
static void update_oversize(GtkWidget *w, gpointer data)
{
    int i = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    DBG(("update_oversize (%d)\n", i));

    if (is_doodle()) {
        resources_set_int("DoodleOversizeHandling", i);
    } else if (is_koala()) {
        resources_set_int("KoalaOversizeHandling", i);
    }
}

static void update_undersize(GtkWidget *w, gpointer data)
{
    int i = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    DBG(("update_undersize (%d)\n", i));

    if (is_doodle()) {
        resources_set_int("DoodleUndersizeHandling", i);
    } else if (is_koala()) {
        resources_set_int("KoalaUndersizeHandling", i);
    }
}

static void update_multicolor(GtkWidget *w, gpointer data)
{
    int i = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    DBG(("update_multicolor (%d)\n", i));

    if (is_doodle()) {
        resources_set_int("DoodleMultiColorHandling", i);
    }
}

static void update_tedluma(GtkWidget *w, gpointer data)
{
    int i = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    DBG(("update_tedluma (%d)\n", i));

    if (is_doodle()) {
        resources_set_int("DoodleTEDLumHandling", i);
    } else if (is_koala()) {
        resources_set_int("KoalaTEDLumHandling", i);
    }
}

static void update_crtccolor(GtkWidget *w, gpointer data)
{
    int i = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
    DBG(("update_crtccolor (%d)\n", i));

    if (is_doodle()) {
        resources_set_int("DoodleCRTCTextColor", i);
    } else if (is_koala()) {
        resources_set_int("KoalaCRTCTextColor", i);
    }
}

/******************************************************************************/

static GtkWidget *build_native_options(void)
{
    GtkWidget *vbox, *omenu, *hbox, *tmp, *table, *align;

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    table = gtk_table_new(3, 4, FALSE);

    tmp = gtk_label_new(_("Oversize handling"));

    native_oversize = omenu = gtk_combo_box_text_new();

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("scale down"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop left top"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop center top"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop right top"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop left center"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop center"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop right center"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop left bottom"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop center bottom"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("crop right bottom"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), 0);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), tmp);

    gtk_table_attach(GTK_TABLE(table), align, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);
    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), omenu);
    gtk_table_attach(GTK_TABLE(table), align, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(omenu);

    g_signal_connect(G_OBJECT(omenu), "changed", G_CALLBACK(update_oversize), NULL);

    tmp = gtk_label_new(_("Undersize handling"));
    native_undersize = omenu = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("scale up"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("add borders"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), 0);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), tmp);
    gtk_table_attach(GTK_TABLE(table), align, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);
    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), omenu);
    gtk_table_attach(GTK_TABLE(table), align, 3, 4, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(omenu);

    g_signal_connect(G_OBJECT(omenu), "changed", G_CALLBACK(update_undersize), NULL);

    tmp = gtk_label_new(_("Multicolor handling"));
    native_mcolor = omenu = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("b&w"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("2 colors"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("4 colors"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("gray scale"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("best cell colors"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), 0);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), tmp);
    gtk_table_attach(GTK_TABLE(table), align, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), omenu);
    gtk_table_attach(GTK_TABLE(table), align, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(omenu);

    g_signal_connect(G_OBJECT(omenu), "changed", G_CALLBACK(update_multicolor), NULL);

    tmp = gtk_label_new(_("TED luma handling"));
    native_tedluma = omenu = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("ignore"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("dither"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), 0);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), tmp);
    gtk_table_attach(GTK_TABLE(table), align, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), omenu);
    gtk_table_attach(GTK_TABLE(table), align, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(omenu);

    g_signal_connect(G_OBJECT(omenu), "changed", G_CALLBACK(update_tedluma), NULL);

    native_crtccolor = omenu = gtk_combo_box_text_new();
    tmp = gtk_label_new(_("CRTC textcolor"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("white"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("amber"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), _("green"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), 0);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), tmp);
    gtk_table_attach(GTK_TABLE(table), align, 2, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);

    align = gtk_alignment_new(0.0, 0.5, 0.0, 0.0);
    gtk_container_add(GTK_CONTAINER(align), omenu);
    gtk_table_attach(GTK_TABLE(table), align, 3, 4, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(omenu);

    gtk_box_pack_start(GTK_BOX(hbox), table, FALSE, FALSE, 0);
    gtk_widget_show(table);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    g_signal_connect(G_OBJECT(omenu), "changed", G_CALLBACK(update_crtccolor), NULL);

    return vbox;
}

static GtkWidget *build_screenshot_dialog(void)
{
    GtkWidget *d, *frame, *vbox, *omenu;
    int i, num_buttons;
    gfxoutputdrv_t *driver;
    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;
    char *title;

#ifdef HAVE_FFMPEG
    GtkWidget *l, *tmp, *table;
    GtkAdjustment *adj;
    int audio_bitrate, video_bitrate;
    gfxoutputdrv_format_t *f;
    GtkListStore *drv_store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkListStore *ac_store = NULL, *vc_store = NULL;
#endif

    num_buttons = gfxoutput_num_drivers();
    if (num_buttons < 1) {
        log_message(LOG_DEFAULT, "No gfxoutputdrivers available");
        return 0;
    }

    title = util_concat(_("Save media file"), "...", NULL);
    d = vice_file_entry(title, NULL, NULL, &filter, 1, UI_FC_SAVE);
    lib_free(title);
    if (!d) {
        return 0;
    }
    gtk_dialog_set_default_response(GTK_DIALOG(d), GTK_RESPONSE_ACCEPT);

    frame = gtk_frame_new(_("Media Format"));
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    if (! buttons) {
        buttons = lib_malloc(sizeof(img_type_buttons) * num_buttons);
    }

    /* "output driver" combo box */
    drv_menu = omenu = gtk_combo_box_text_new();

    driver = gfxoutput_drivers_iter_init();
    for (i = 0; i < num_buttons; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(omenu), driver->displayname);
        buttons[i].driver = driver->name;
        buttons[i].ext = driver->default_extension;
        driver = gfxoutput_drivers_iter_next();
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(omenu), 0);
    gtk_box_pack_start(GTK_BOX(vbox), omenu, FALSE, FALSE, 0);
    gtk_widget_show(omenu);

    native_opts = build_native_options();

    gtk_box_pack_start(GTK_BOX(vbox), native_opts, FALSE, FALSE, 0);
    gtk_widget_show_all(native_opts);
    gtk_widget_set_sensitive(native_opts, FALSE);

    g_signal_connect(G_OBJECT(omenu), "changed", G_CALLBACK(ffmpeg_output_driver_changed), (gpointer)omenu);

#ifdef HAVE_FFMPEG
    if (!ffmpegdrv_formatlist || !ffmpegdrv_formatlist[0].name) {
        goto no_ffmpeg;
    }

    /* ffmpeg options */

    drv_store = gtk_list_store_new(DRV_N, G_TYPE_STRING, G_TYPE_INT, G_TYPE_OBJECT, G_TYPE_OBJECT);
    for (i = 0; ffmpegdrv_formatlist[i].name != NULL; i++) {
        int j;
        GtkTreeIter ac_iter, vc_iter;

        f = &ffmpegdrv_formatlist[i];

        gtk_list_store_append(drv_store, &iter);
        gtk_list_store_set(drv_store, &iter, DRV_NAME, f->name, DRV_INDEX, i, -1);

        log_message(LOG_DEFAULT, "FFMPEG Driver: %s", f->name);

        ac_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
        for (j = 0; f->audio_codecs && f->audio_codecs[j].name; j++) {
            gtk_list_store_append(ac_store, &ac_iter);
            gtk_list_store_set(ac_store, &ac_iter, 0, f->audio_codecs[j].name, 1, f->audio_codecs[j].id, 2, j, -1);
            log_message(LOG_DEFAULT, "\tAC: %s(%d)", f->audio_codecs[j].name, f->audio_codecs[j].id);
        }

        vc_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
        for (j = 0; f->video_codecs && f->video_codecs[j].name; j++) {
            gtk_list_store_append(vc_store, &vc_iter);
            gtk_list_store_set(vc_store, &vc_iter, 0, f->video_codecs[j].name, 1, f->video_codecs[j].id, 2, j, -1);
            log_message(LOG_DEFAULT, "\tVC: %s(%d)", f->video_codecs[j].name, f->video_codecs[j].id);
        }

        gtk_list_store_set(drv_store, &iter, DRV_ACMENU, ac_store, DRV_VCMENU, vc_store, -1);
    }
    /* ffmpeg format combo box */
    ffmpeg_omenu = omenu = gtk_combo_box_new_with_model(GTK_TREE_MODEL(drv_store));    
    g_signal_connect(G_OBJECT(omenu), "changed", G_CALLBACK(ffmpeg_details), (gpointer) omenu);
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(omenu), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(omenu), renderer, "text", DRV_NAME, NULL);

    ffmpg_opts = table = gtk_table_new(5, 4, TRUE);

    tmp = gtk_label_new(_("Format"));
    gtk_table_attach(GTK_TABLE(table), tmp, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);
    gtk_table_attach(GTK_TABLE(table), omenu, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(omenu);

    acmenu = gtk_combo_box_new_with_model(GTK_TREE_MODEL(ac_store));    
    g_signal_connect(G_OBJECT(acmenu), "changed", G_CALLBACK(ffmpeg_audio_codec_changed), (gpointer) omenu);
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(acmenu), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(acmenu), renderer, "text", 0, NULL);
    tmp = gtk_label_new(_("Audio codec"));
    gtk_table_attach(GTK_TABLE(table), tmp, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);
    gtk_table_attach(GTK_TABLE(table), acmenu, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(acmenu);

    vcmenu = gtk_combo_box_new_with_model(GTK_TREE_MODEL(vc_store));    
    g_signal_connect(G_OBJECT(vcmenu), "changed", G_CALLBACK(ffmpeg_video_codec_changed), (gpointer) omenu);
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(vcmenu), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(vcmenu), renderer, "text", 0, NULL);
    tmp = gtk_label_new(_("Video codec"));
    gtk_table_attach(GTK_TABLE(table), tmp, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(tmp);
    gtk_table_attach(GTK_TABLE(table), vcmenu, 3, 4, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show(vcmenu);

    resources_get_int("FFMPEGAudioBitrate", &audio_bitrate);
    adj = GTK_ADJUSTMENT(gtk_adjustment_new((gfloat)audio_bitrate, (gfloat)VICE_FFMPEG_AUDIO_RATE_MIN, (gfloat)VICE_FFMPEG_AUDIO_RATE_MAX, (gfloat)1000, (gfloat)10000, (gfloat)00000));
    ffmpg_audio = gtk_spin_button_new(GTK_ADJUSTMENT(adj), (gfloat)1000, 0);
    gtk_widget_set_size_request(ffmpg_audio, 100, -1);
    l = gtk_label_new(_("Audio bitrate"));
    gtk_table_attach(GTK_TABLE(table), l, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), ffmpg_audio, 1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

    resources_get_int("FFMPEGVideoBitrate", &video_bitrate);
    adj = GTK_ADJUSTMENT(gtk_adjustment_new((gfloat)video_bitrate, (gfloat)VICE_FFMPEG_VIDEO_RATE_MIN, (gfloat)VICE_FFMPEG_VIDEO_RATE_MAX, (gfloat)10000, (gfloat)100000, (gfloat)000000));
    ffmpg_video = gtk_spin_button_new(GTK_ADJUSTMENT(adj), (gfloat)10000, 0);
    gtk_widget_set_size_request(ffmpg_video, 100, -1);
    l = gtk_label_new(_("Video bitrate"));
    gtk_table_attach(GTK_TABLE(table), l, 2, 3, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(table), ffmpg_video, 3, 4, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

    gtk_box_pack_start(GTK_BOX(vbox), ffmpg_opts, FALSE, FALSE, 0);
    gtk_widget_show_all(ffmpg_opts);
    gtk_widget_set_sensitive(ffmpg_opts, FALSE);

    {
        GtkWidget *fcb;
        /* "Half Framerate" checkbox */
        fpsmenu = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(fpsmenu), GTK_SHADOW_IN);
        fcb = gtk_check_button_new_with_label(_("Half Framerate (25/30 fps)"));
        gtk_widget_set_can_focus(fcb, 0);
        g_signal_connect(G_OBJECT(fcb), "toggled", G_CALLBACK(update_halffps_checkbox), 0);
        gtk_container_add(GTK_CONTAINER(fpsmenu), fcb);
        gtk_widget_show(fcb);
        gtk_table_attach(GTK_TABLE(table), fpsmenu, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
        gtk_widget_show(fpsmenu);
    }

  no_ffmpeg:
#endif    

    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_widget_show(vbox);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), frame, TRUE, TRUE, 0);

    gtk_widget_show(frame);
    gtk_widget_show(d);

    ffmpeg_output_driver_changed(drv_menu, NULL);

    return d;
}

int ui_screenshot_dialog(char *name, struct video_canvas_s *wid)
{
    int res, i, driveridx = -1;
    char *fn, *tmp, *tmpext;
    const char *driver, *ext;

    /* build the dialog, or raise it if it exists */
    if (screenshot_dialog) {
        gdk_window_show(gtk_widget_get_window(screenshot_dialog));
        gdk_window_raise(gtk_widget_get_window(screenshot_dialog));
        gtk_widget_show(screenshot_dialog);
        gtk_combo_box_set_active(GTK_COMBO_BOX(drv_menu), combo_box_current_active);
    } else {
        screenshot_dialog = build_screenshot_dialog();
        if (screenshot_dialog) {
            g_signal_connect(G_OBJECT(screenshot_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &screenshot_dialog);
        } else {
            return -1;
        }
    }

    /* pop up and run the dialog */
    ui_popup(screenshot_dialog, _("Save media file"), FALSE);
    res = gtk_dialog_run(GTK_DIALOG(screenshot_dialog));
    ui_popdown(screenshot_dialog);

    if (res != GTK_RESPONSE_ACCEPT) {
        return -1;
    }

    fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(screenshot_dialog));
    if (!fn) {
        ui_error(_("Invalid filename"));
        return -1;
    }

    tmpext = util_get_extension(fn);
    if (tmpext) {
        for (i = 0; i < gfxoutput_num_drivers(); i++) {
            if (buttons[i].ext && !strcmp(buttons[i].ext, tmpext)) {
                driveridx = i;
                break;
            }
        }
    }

    i = gtk_combo_box_get_active(GTK_COMBO_BOX(drv_menu));
    if (i < 0) {
        return -1;
    }
    if ((driveridx != -1) && (driveridx != i)) {
        i = driveridx;
    }
    combo_box_current_active = i;

    driver = buttons[i].driver;
    ext = buttons[i].ext;

#ifdef HAVE_FFMPEG
    if (strcmp(driver, "FFMPEG") == 0) {
        int audio_bitrate, video_bitrate;

        audio_bitrate = (int)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ffmpg_audio));
        resources_set_int("FFMPEGAudioBitrate", audio_bitrate);
        video_bitrate = (int)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ffmpg_video));
        resources_set_int("FFMPEGVideoBitrate", video_bitrate);

        resources_set_string("FFMPEGFormat", selected_driver);
        resources_set_int("FFMPEGAudioCodec", selected_ac);
        resources_set_int("FFMPEGVideoCodec", selected_vc);
        log_message(LOG_DEFAULT, "FFMPEG: Driver: %s, ac: %d, vc: %d\n", selected_driver, selected_ac, selected_vc);
    }
#endif
    tmp = lib_stralloc(fn);
    free(fn); /* do not use lib_free on pointers not allocated by lib_malloc */
    if (!util_get_extension(tmp)) {
        util_add_extension(&tmp, ext);
    }
    if (screenshot_save(driver, tmp, wid) < 0) {
        ui_error(_("Couldn't write screenshot to `%s' with driver `%s'."), tmp, driver);
        lib_free(tmp);
        return -1;
    } else {
        if (screenshot_is_recording()) {
            gtk_widget_show(video_ctrl_checkbox);
        }
        ui_message(_("Successfully wrote `%s'"), tmp);
        lib_free(tmp);
    }

    return 0;
}

void uiscreenshot_shutdown(void)
{
    lib_free(buttons);
    buttons = NULL;
#ifdef HAVE_FFMPEG
    if (selected_driver != NULL) {
        lib_free(selected_driver);
        selected_driver = NULL;
    }
#endif
    if (screenshot_dialog && GTK_IS_WIDGET(screenshot_dialog)) {
        gtk_widget_destroy(screenshot_dialog);
    }
    screenshot_dialog = NULL;
}
