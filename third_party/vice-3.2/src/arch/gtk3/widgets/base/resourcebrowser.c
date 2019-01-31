/** \file   resourcebrowser.c
 * \brief   Text entry with label and browse button connected to a resource
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
#include <string.h>

#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "openfiledialog.h"
#include "widgethelpers.h"
#include "resourcehelpers.h"
#include "resourceentry.h"

#include "resourcebrowser.h"


/** \brief  Object keeping track of the state of the widget
 */
typedef struct resource_browser_state_s {
    char *res_name; /**< resource name */
    char *res_orig; /**< resource value at widget creation */
    char **patterns;    /**< file matching patterns */
    char *pattern_name; /**< name to display for the file patterns */
    char *browser_title;    /**< title to display for the file browser */
    void (*callback)(GtkWidget *, gpointer);    /**< optional callback */
    GtkWidget *entry;   /**< GtkEntry reference */
    GtkWidget *button;  /**< GtkButton reference */
} resource_browser_state_t;


/*
 * Forward declarations of functions
 */
static void free_patterns(char **patterns);


/** \brief  Clean up memory used by the main widget
 *
 * \param[in]   widget  resource browser widget
 * \param[in]   data    unused
 */
static void on_resource_browser_destroy(GtkWidget *widget, gpointer data)
{
    resource_browser_state_t *state;

    state = (resource_browser_state_t *)(g_object_get_data(G_OBJECT(widget),
                "ViceState"));
    lib_free(state->res_name);
    if (state->res_orig != NULL) {
        lib_free(state->res_orig);
    }
    if (state->patterns != NULL) {
        free_patterns(state->patterns);
    }
    if (state->pattern_name != NULL) {
        lib_free(state->pattern_name);
    }
    if (state->browser_title != NULL) {
        lib_free(state->browser_title);
    }
    lib_free(state);
}


/** \brief  Handler for the "clicked" event of the browse button
 *
 * \param[in]   widget  browse button
 * \param[in]   data    unused
 *
 */
static void on_resource_browser_browse_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *parent;
    char *filename;
    resource_browser_state_t *state;

    parent = gtk_widget_get_parent(widget);
    state = g_object_get_data(G_OBJECT(parent), "ViceState");

    filename = vice_gtk3_open_file_dialog(state->browser_title,
            state->pattern_name, (const char **)(state->patterns), NULL);
    if (filename != NULL) {
        debug_gtk3("got image name '%s'\n", filename);
        if (!vice_gtk3_resource_entry_full_set(state->entry, filename)){
            log_error(LOG_ERR,
                    "failed to set resource %s to '%s', reverting\n",
                    state->res_name, filename);
            /* restore resource to original state */
            resources_set_string(state->res_name, state->res_orig);
            gtk_entry_set_text(GTK_ENTRY(state->entry), state->res_orig);
        }
        g_free(filename);
    }
}


/** \brief  Create a heap allocated copy of \a patterns
 *
 * \return  deep copy of \a patterns
 */
static char **copy_patterns(const char **patterns)
{
    size_t num = 0;
    char **arr;

    if (patterns == NULL || *patterns == NULL) {
        return NULL;
    }

    /* count number of patterns */
    while (patterns[num++] != NULL) {
        /* NOP */
    }

    arr = lib_malloc((num + 1) * sizeof *arr);
    num = 0;
    while (patterns[num] != NULL) {
        arr[num] = lib_stralloc(patterns[num]);
        num++;
    }
    arr[num] = NULL;
    return arr;
}


/** \brief  Clean up copies of the file name patterns
 *
 * \param[in]   patterns    list of file name matching patterns
 */
static void free_patterns(char **patterns)
{
    size_t i = 0;

    while (patterns[i] != NULL) {
        lib_free(patterns[i]);
        i++;
    }
    lib_free(patterns);
}


/** \brief  Create file selection widget with browse button
 *
 * \param[in]   resource        resource name
 * \param[in]   patterns        file match patterns (optional)
 * \param[in]   pattern_name    name to use for \a patterns in the file dialog
 *                              (optional)
 * \param[in]   browser_title   title to display in the file dialog (optional,
 *                              defaults to "Select file")
 * \param[in]   label           label (optional)
 * \param[in]   callback        user callback (optional, not implemented yet)
 *
 * \note    both \a patterns and \a pattern_name need to be defined (ie not
 *          `NULL` for the \a patterns to work.
 *
 * \return  GtkGrid
 */
GtkWidget *vice_gtk3_resource_browser_new(
        const char *resource,
        const char **patterns,
        const char *pattern_name,
        const char *browser_title,
        const char *label,
        void (*callback)(GtkWidget *, gpointer))
{
    GtkWidget *grid;
    GtkWidget *lbl = NULL;
    resource_browser_state_t *state;
    const char *orig;
    int column = 0;

    grid = vice_gtk3_grid_new_spaced(16, 0);

    /* allocate and init state object */
    state = lib_malloc(sizeof *state);

    /* copy resource name */
    state->res_name = lib_stralloc(resource);

    /* get current value of resource */
    if (resources_get_string(resource, &orig) < 0) {
        debug_gtk3("failed to retrieve current value for resource '%s'\n",
                resource);
        orig = "";
    } else if (orig == NULL) {
        orig = "";
    }
    state->res_orig = lib_stralloc(orig);

    /* store optional callback */
    state->callback = callback;

    /* copy file matching patterns */
    state->patterns = copy_patterns(patterns);

    /* copy pattern name */
    if (pattern_name != NULL && *pattern_name != '\0') {
        state->pattern_name = lib_stralloc(pattern_name);
    } else {
        state->pattern_name = NULL;
    }

    /* copy browser title */
    if (browser_title != NULL && *browser_title != '\0') {
        state->browser_title = lib_stralloc("Select file");
    } else {
        state->browser_title = lib_stralloc(browser_title);
    }

    /*
     * add widgets to the grid
     */

    /* label, if given */
    if (label != NULL && *label != '\0') {
        lbl = gtk_label_new(label);
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, 1, 1);
        column++;
    }

    /* text entry */
    state->entry = vice_gtk3_resource_entry_full_new(resource);
    gtk_widget_set_hexpand(state->entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), state->entry, column, 0, 1, 1);
    column++;

    /* browse button */
    state->button = gtk_button_new_with_label("Browse ...");
    gtk_grid_attach(GTK_GRID(grid), state->button, column, 0, 1,1);

    /* store the state object in the widget */
    g_object_set_data(G_OBJECT(grid), "ViceState", (gpointer)state);

    /* connect signal handlers */
    g_signal_connect(state->button, "clicked",
            G_CALLBACK(on_resource_browser_browse_clicked), NULL);
    g_signal_connect(grid, "destroy", G_CALLBACK(on_resource_browser_destroy),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set \a widget value to \a new
 *
 * \param[in,out]   widget  resource browser widget
 * \param[in]       new     new value for \a widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_browser_set(GtkWidget *widget, const char *new)
{
    resource_browser_state_t *state;
    state = g_object_get_data(G_OBJECT(widget), "ViceState");

    if (resources_set_string(state->res_name, new) < 0) {
        /* restore to default */
        resources_set_string(state->res_name, state->res_orig);
        gtk_entry_set_text(GTK_ENTRY(state->entry), state->res_orig);
        return FALSE;
    } else {
        gtk_entry_set_text(GTK_ENTRY(state->entry), new != NULL ? new : "");
        return TRUE;
    }
}


/** \brief  Get the current value of \a widget
 *
 * Get the current resource value of \a widget and store it in \a dest. If
 * getting the resource value fails for some reason, `FALSE` is returned and
 * \a dest is set to `NULL`.
 *
 * \param[in]   widget  resource browser widget
 * \param[out]  dest    destination of value
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_browser_get(GtkWidget *widget, const char **dest)
{
    resource_browser_state_t *state;
    state = g_object_get_data(G_OBJECT(widget), "ViceState");

    if (resources_get_string(state->res_name, dest) < 0) {
        *dest = NULL;
        return FALSE;
    }
    return TRUE;
}


/** \brief  Restore resource in \a widget to its original value
 *
 * \param[in,out]   widget  resource browser widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_browser_reset(GtkWidget *widget)
{
    resource_browser_state_t *state;

    state = g_object_get_data(G_OBJECT(widget), "ViceState");

    /* restore resource value */
    if (resources_set_string(state->res_name, state->res_orig) < 0) {
        return FALSE;
    }
    /* update text entry */
    gtk_entry_set_text(GTK_ENTRY(state->entry), state->res_orig);
    return TRUE;
}


/** \brief  Synchronize widget with current resource value
 *
 * Only needed to call if the resource's value is changed from code other than
 * this widget's code.
 *
 * \param[in,out]   widget  resource browser widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_browser_sync(GtkWidget *widget)
{
    resource_browser_state_t *state;
    const char *value;

    /* get current resource value */
    state = g_object_get_data(G_OBJECT(widget), "ViceState");
    if (resources_get_string(state->res_name, &value) < 0) {
        return FALSE;
    }

    gtk_entry_set_text(GTK_ENTRY(state->entry), value);
    return TRUE;
}


/** \brief  Reset widget to the resource's factory value
 *
 * \param[in,out]   widget  resource browser widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_browser_factory(GtkWidget *widget)
{
    resource_browser_state_t *state;
    const char *value;

    /* get resource factory value */
    state = g_object_get_data(G_OBJECT(widget), "ViceState");
    if (resources_get_default_value(state->res_name, &value) < 0) {
        return FALSE;
    }
    return vice_gtk3_resource_browser_set(widget, value);
}
