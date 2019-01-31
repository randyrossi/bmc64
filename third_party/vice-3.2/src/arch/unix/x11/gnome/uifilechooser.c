/*
 * uifilechooser.c - GTK only, file chooser/requester
 *
 * Written by
 *  Ettore Perazzoli
 *  Oliver Schaertel
 *  pottendo <pottendo@gmx.net>
 *  groepaz <groepaz@gmx.net>
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

/* #define DEBUG_X11UI */

#include "vice.h"

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "fullscreenarch.h"
#include "ioutil.h"
#include "lib.h"
#include "log.h"
#include "ui.h"
#include "uiarch.h"
#include "uicontents.h"
#include "uidrivestatus.h"
#include "uifileentry.h"
#include "uilib.h"
#include "uimenu.h"
#include "uitapestatus.h"
#include "util.h"
#include "video.h"
#include "resources.h"
#include "screenshot.h"
#include "videoarch.h"
#include "vsync.h"

#include "uifilechooser.h"


#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/******************************************************************************/

extern int have_cbm_font;
extern char *fixedfontname;

static GtkWidget *image_preview_list, *auto_start_button, *last_file_selection;
static GtkWidget *scrollw = NULL;

static char *filesel_dir = NULL;

/******************************************************************************/

static void ui_fill_preview(GtkFileChooser *fs, gpointer data);
static gboolean ui_select_contents_cb(GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer userdata);

/******************************************************************************/

static void sh_checkbox_cb(GtkWidget *w, gpointer data)
{
    g_return_if_fail(GTK_IS_CHECK_BUTTON(w));
    g_return_if_fail(GTK_IS_FILE_CHOOSER(data));
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
        gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(data), TRUE);
    } else {
        gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(data), FALSE);
    }
}

/** \brief  Event handler for the "toggled" event of the "Show preview" cb
 *
 * \param[in,out]   widget  event source
 * \param[in]       data    event data
 *
 * \todo    Perhaps resize the file chooser widget
 */
static void sp_checkbox_cb(GtkWidget *widget, gpointer data)
{
    if (scrollw != NULL) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
            gtk_widget_show(scrollw);
        } else {
            gtk_widget_hide(scrollw);
        }
    }
}



/* These functions build all the widgets. */
GtkWidget *build_file_selector(const char *title, GtkWidget **attach_write_protect, int allow_autostart, int show_preview,
                                      uilib_file_filter_enum_t* patterns, int num_patterns, const char *default_dir, ui_filechooser_t action, read_contents_func_type read_contents_func)
{
    GtkWidget *fileselect, *wp_checkbox, *sh_checkbox, *extra;
    GtkWidget *sp_checkbox = NULL;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    fileselect = vice_file_entry(title, get_active_toplevel(), default_dir, patterns, num_patterns, action);

    /* Contents preview */
    if (show_preview) {
        char *cbm_font = have_cbm_font ? fixedfontname : "monospace 10";

        store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
        image_preview_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL (store));
        renderer = gtk_cell_renderer_text_new ();
        g_object_set (renderer, "font", cbm_font, "ypad", 0, NULL);
        column = gtk_tree_view_column_new_with_attributes (_("Contents"), renderer, "text", 0, NULL);

        gtk_tree_view_append_column(GTK_TREE_VIEW(image_preview_list), column);

        /* ignored on Gtk3:
        gtk_widget_set_size_request(image_preview_list, 350, 180);
        */
        gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(image_preview_list), FALSE);

        gtk_container_set_border_width(GTK_CONTAINER(image_preview_list), 1);

        gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(image_preview_list)), GTK_SELECTION_SINGLE);

        scrollw = gtk_scrolled_window_new(NULL, NULL);

        /* this works though, both on Gtk2 and Gtk3: 410 pixels makes the
         * horizontal scrollbar disappear */
        gtk_widget_set_size_request(scrollw, 410, 180);

        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_set_border_width(GTK_CONTAINER(scrollw), 1);
        gtk_container_add(GTK_CONTAINER(scrollw), image_preview_list);
        gtk_container_set_focus_vadjustment(GTK_CONTAINER (image_preview_list), gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW (scrollw)));
        gtk_container_set_focus_hadjustment(GTK_CONTAINER (image_preview_list), gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW (scrollw)));
        gtk_widget_show(image_preview_list);
        gtk_widget_show(scrollw);
        gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(fileselect), scrollw);
        g_signal_connect(fileselect, "update-preview", G_CALLBACK(ui_fill_preview), read_contents_func);
        if (allow_autostart) {
            gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(image_preview_list)), ui_select_contents_cb, NULL, NULL);
        }
    }

    extra = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    /* Write protect checkbox */
    if (attach_write_protect) {
        /* write-protect checkbox */
        wp_checkbox = gtk_check_button_new_with_label(_("Attach read only"));
        gtk_box_pack_start(GTK_BOX(extra), wp_checkbox, FALSE, FALSE, 5);
        gtk_widget_show(wp_checkbox);
        *attach_write_protect = wp_checkbox;
    }

    /* show hidden files */
    sh_checkbox = gtk_check_button_new_with_label(_("Show hidden files"));
    g_signal_connect(G_OBJECT(sh_checkbox), "toggled", G_CALLBACK(sh_checkbox_cb), (gpointer)fileselect);
    gtk_box_pack_start(GTK_BOX(extra), sh_checkbox, FALSE, FALSE, 5);
    gtk_widget_show(sh_checkbox);

    /* add a toggle button for the image contents preview */
    if (show_preview) {
        sp_checkbox = gtk_check_button_new_with_label(_("Show image contents"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sp_checkbox), TRUE);
        g_signal_connect(G_OBJECT(sp_checkbox), "toggled",
                G_CALLBACK(sp_checkbox_cb), (gpointer)fileselect);
        gtk_box_pack_start(GTK_BOX(extra), sp_checkbox, FALSE, FALSE, 5);
        gtk_widget_show(sp_checkbox);
    }

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(fileselect), extra);
    gtk_widget_show(extra);

    if (allow_autostart) {
        auto_start_button = gtk_dialog_add_button(GTK_DIALOG(fileselect), _("Autostart"), GTK_RESPONSE_NO);
    }

    last_file_selection = fileselect;
    return fileselect;
}

static void ui_fill_preview(GtkFileChooser *fs, gpointer data)
{
    char *tmp1, *tmp2;
    image_contents_t *contents = NULL;
    image_contents_file_list_t *element;
    gchar *fname;
    GtkListStore *store;
    GtkTreeIter iter;
    int row;
    read_contents_func_type current_image_contents_func = (read_contents_func_type)data;

    g_return_if_fail(fs != NULL);
    g_return_if_fail(GTK_IS_FILE_CHOOSER(fs));

    fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fs));
    if (fname && current_image_contents_func) {
        struct stat st;

        if (stat(fname, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                contents = current_image_contents_func(fname);
            }
        }
        g_free(fname);
    }

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(image_preview_list)));
    gtk_list_store_clear(store);

    row = 0;

    if (!contents) {
        gtk_list_store_append(store, &iter);
        tmp1 = lib_stralloc(_("NO IMAGE CONTENTS AVAILABLE"));
        gtk_list_store_set(store, &iter, 0, tmp1, 1, row, -1);
        lib_free(tmp1);
    } else {
        tmp1 = image_contents_to_string(contents, !have_cbm_font);
        tmp2 = (char *)convert_utf8((unsigned char *)tmp1);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, tmp2, 1, row++, -1);
        lib_free(tmp1);
        lib_free(tmp2);
        for (element = contents->file_list; element != NULL; element = element->next, row++) {
            tmp1 = (char *)image_contents_file_to_string(element, !have_cbm_font);
            tmp2 = (char *)convert_utf8((unsigned char *)tmp1);
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, tmp2, 1, row, -1);
            lib_free(tmp1);
            lib_free(tmp2);
        }
        if (contents->blocks_free >= 0) {
            tmp2 = lib_msprintf("%d BLOCKS FREE.", contents->blocks_free);
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, tmp2, 1, row, -1);
            lib_free(tmp2);
        }
        image_contents_destroy(contents);
    }
}

static gboolean ui_select_contents_cb(GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer userdata)
{
    GtkTreeIter iter;
    int row = 0;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 1, &row, -1);
    }
    ui_set_selected_file(row);
    if (row) {
        gtk_button_clicked(GTK_BUTTON(auto_start_button));
    }
    return TRUE;
}

/* File browser. */
char *ui_select_file(const char *title, read_contents_func_type read_contents_func, unsigned int allow_autostart,
                     const char *default_dir, uilib_file_filter_enum_t* patterns, int num_patterns,
                     ui_button_t *button_return, unsigned int show_preview, int *attach_wp, ui_filechooser_t action)
{
    static GtkWidget* file_selector = NULL;
    char *ret;
    char *current_dir = NULL;
    char *filename = NULL;
    GtkWidget *wp;
    gint res;

    /* reset old selection */
    ui_set_selected_file(0);

    /* we preserve the current directory over the invocations */
    current_dir = ioutil_current_dir(); /* might be changed elsewhere */
    if (filesel_dir != NULL) {
        ioutil_chdir(filesel_dir);
    }

    if (attach_wp) {
        file_selector = build_file_selector(title, &wp, allow_autostart, show_preview, patterns, num_patterns, default_dir, action, read_contents_func);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wp), *attach_wp);
    } else {
        file_selector = build_file_selector(title, NULL, allow_autostart, show_preview, patterns, num_patterns, default_dir, action, read_contents_func);
    }

    g_signal_connect(G_OBJECT(file_selector), "destroy", G_CALLBACK(gtk_widget_destroyed), &file_selector);

    ui_popup(file_selector, title, FALSE);
    res = gtk_dialog_run(GTK_DIALOG(file_selector));
    ui_popdown(file_selector);

    switch (res) {
        case GTK_RESPONSE_ACCEPT:
        case GTK_RESPONSE_OK:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selector));
            *button_return = UI_BUTTON_OK;
            break;
        case GTK_RESPONSE_NO:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selector));
            *button_return = UI_BUTTON_AUTOSTART;
            break;
        default:
            *button_return = UI_BUTTON_CANCEL;
            auto_start_button = NULL;
            gtk_widget_destroy(file_selector);
            if (current_dir != NULL) {
                ioutil_chdir(current_dir);
                lib_free(current_dir);
            }
            return NULL;
            break;
    }

    if (attach_wp) {
        *attach_wp = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wp));
    }

    auto_start_button = NULL;
    gtk_widget_destroy(file_selector);

    /* `ret' gets always malloc'ed. caller has to free  */
    if (filename) {
        ret = lib_stralloc(filename);
        g_free(filename);
    } else {
        ret = lib_stralloc("");
    }

    lib_free(filesel_dir);
    filesel_dir = ioutil_current_dir();

    if (current_dir != NULL) {
        ioutil_chdir(current_dir);
        lib_free(current_dir);
    }
    return ret;
}

/* called from ui_shutdown */
void shutdown_file_selector(void)
{
    if (filesel_dir != NULL) {
        lib_free(filesel_dir);
    }
}
