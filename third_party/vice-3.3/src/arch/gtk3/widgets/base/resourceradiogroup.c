/** \file   resourceradiogroup.c
 * \brief   Group of radio buttons controlling a resource
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * \todo    Make a copy of the \a entries argument to the new() functions, that
 *          way the \a entries don't have to be kept in memory during the
 *          lifetime of the widgets.
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

#include "basewidget_types.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "resourcehelpers.h"

#include "resourceradiogroup.h"


/** \brief  Handler for the "destroy" event of the widget
 *
 * Frees the heap-allocated copy of the resource name
 *
 * \param[in]   widget      radiogroup widget
 * \param[in]   user_data   extra event data (unused)
 */
static void on_radiogroup_destroy(GtkWidget *widget, gpointer user_data)
{
    resource_widget_free_resource_name(widget);
}


/** \brief  Handler for the "toggled" event of a radio button
 *
 * \param[in]   radio       radio button triggering the event
 * \param[in]   user_data   new value for the radio group (`int`)
 */
static void on_radio_toggled(GtkWidget *radio, gpointer user_data)
{
    GtkWidget *parent;
    int old_val;
    int new_val;
    const char *resource;
    void (*callback)(GtkWidget *, int);

    /* parent widget (grid) contains the "ResourceName" property */
    parent = gtk_widget_get_parent(radio);
    resource = resource_widget_get_resource_name(parent);

    /* get new and old values */
    if (resources_get_int(resource, &old_val) < 0) {
        log_error(LOG_ERR, "failed to get value for resource '%s'\n",
                resource);
        return;
    }
    new_val = GPOINTER_TO_INT(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))
            && (old_val != new_val)) {
#if 0
        debug_gtk3("setting %s to %d.", resource, new_val);
#endif
        if (resources_set_int(resource, new_val) < 0) {
            log_error(LOG_ERR, "failed to set resource '%s' to %d\n",
                    resource, new_val);
        } else {
            /* only trigger callback on succesfully setting the resource */

            callback = g_object_get_data(G_OBJECT(parent), "ExtraCallback");
            if (callback != NULL) {
                callback(parent, new_val);
            }
        }
    }
}


/** \brief  Helper function for the new() functions
 *
 * \param[in]   grid        containing grid
 * \param[in]   entries     list of entries for the group
 * \param[in]   orientation layout direction of the radio buttons
 *
 * \return  GtkGrid
 */
static GtkWidget *resource_radiogroup_new_helper(
        GtkWidget *grid,
        const vice_gtk3_radiogroup_entry_t *entries,
        GtkOrientation orientation)
{
    GtkRadioButton *last = NULL;
    GSList *group = NULL;
    int i;
    int current;
    const char *resource;

    resource = resource_widget_get_resource_name(grid);
    if (resources_get_int(resource, &current) < 0) {
        debug_gtk3("failed to get value for resource '%s', defaulting to 0.",
                resource);
        current = 0;
    }
    /* store current resource value in object, for use with reset() */
    resource_widget_set_int(grid, "ResourceOrig", current);

    /* set ExtraCallback to NULL */
    g_object_set_data(G_OBJECT(grid), "ExtraCallback", NULL);

    /* store a reference to the entries in the object */
    g_object_set_data(G_OBJECT(grid), "Entries", (gpointer)entries);
    /* store the orientation in the object */
    g_object_set_data(G_OBJECT(grid), "Orientation",
            GINT_TO_POINTER(orientation));

    /* create radio buttons */
    for (i = 0; entries[i].name != NULL; i++) {
        GtkWidget *radio;

        radio = gtk_radio_button_new_with_label(group, entries[i].name);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio), last);

        if (current == entries[i].id) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
        }
        g_signal_connect(radio, "toggled", G_CALLBACK(on_radio_toggled),
                GINT_TO_POINTER(entries[i].id));

        if (orientation == GTK_ORIENTATION_HORIZONTAL) {
            gtk_grid_attach(GTK_GRID(grid), radio, i, 0, 1, 1);
        } else {
            gtk_grid_attach(GTK_GRID(grid), radio, 0, i, 1, 1);
        }

        last = GTK_RADIO_BUTTON(radio);
    }

    /* register methods to be used by the resource widget manager */
    resource_widget_register_methods(
            grid,
            vice_gtk3_resource_radiogroup_reset,
            vice_gtk3_resource_radiogroup_factory,
            vice_gtk3_resource_radiogroup_sync,
            vice_gtk3_resource_radiogroup_apply);
    g_signal_connect(grid, "destroy", G_CALLBACK(on_radiogroup_destroy), NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create a radiogroup widget
 *
 * A radiogroup widget is a composite widget consisting of a GtkGrid with
 * a number of GtkRadioButton's, laid out according to \a orientation
 *
 * Three GObject properties are set: "ResourceName", a heap-allocated copy
 * of the \a resource argument, "Entries", a reference to the \a entries and
 * "Orientation", the \a orientation argument. These are all required to make
 * the event handlers and resource_radiogroup_update() work properly.
 *
 * The \a entries passed to this function need to be available during the
 * widget's lifetime, since only a pointer to them is stored, no copies are
 * made.
 *
 * \param[in]   resource    resource name
 * \param[in]   entries     list of entries for the group
 * \param[in]   orientation layout direction of the radio buttons
 *
 * \return  GtkGrid
 */
GtkWidget *vice_gtk3_resource_radiogroup_new(
        const char *resource,
        const vice_gtk3_radiogroup_entry_t *entries,
        GtkOrientation orientation)
{
    GtkWidget *grid;

    grid = gtk_grid_new();

    /* store a copy of the resource name in the object */
    resource_widget_set_resource_name(grid, resource);

    return resource_radiogroup_new_helper(grid, entries, orientation);
}


/** \brief  Create a radiogroup widget, using sprintf()-style formatting of
 *          the resource name
 *
 * A radiogroup widget is a composite widget consisting of a GtkGrid with
 * a number of GtkRadioButton's, laid out according to \a orientation
 *
 * Three GObject properties are set: "ResourceName", a heap-allocated copy
 * of the \a resource argument, "Entries", a reference to the \a entries and
 * "Orientation", the \a orientation argument. These are all required to make
 * the event handlers and resource_radiogroup_update() work properly.
 *
 * The \a entries passed to this function need to be available during the
 * widget's lifetime, since only a pointer to them is stored, no copies are
 * made.
 *
 * \param[in]   fmt         resource name format string
 * \param[in]   entries     list of entries for the group
 * \param[in]   orientation layout direction of the radio buttons
 *
 * \return  GtkGrid
 */
GtkWidget *vice_gtk3_resource_radiogroup_new_sprintf(
        const char *fmt,
        const vice_gtk3_radiogroup_entry_t *entries,
        GtkOrientation orientation,
        ...)
{
    GtkWidget *grid;
    char *resource;
    va_list args;

    grid = gtk_grid_new();

    va_start(args, orientation);
    resource = lib_mvsprintf(fmt, args);
    g_object_set_data(G_OBJECT(grid), "ResourceName", (gpointer)resource);
    va_end(args);

    return resource_radiogroup_new_helper(grid, entries, orientation);
}


/** \brief  Set \a widget to \a id
 *
 * \param[in,out]   widget  radiogroup widget
 * \param[in]       id      new value for widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_radiogroup_set(GtkWidget *widget, int id)
{
    int orientation;
    int index;
    vice_gtk3_radiogroup_entry_t *entries;
    GtkWidget *radio;

    orientation = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),
                "Orientation"));
    entries = (vice_gtk3_radiogroup_entry_t *)(g_object_get_data(G_OBJECT(widget),
                "Entries"));

    for (index = 0; entries[index].name != NULL; index++) {
        if (entries[index].id == id) {
            if (orientation == GTK_ORIENTATION_VERTICAL) {
                radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, index);
            } else {
                radio = gtk_grid_get_child_at(GTK_GRID(widget), index, 0);
            }
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
            return TRUE;
        }
    }
    return FALSE;
}


/** \brief  Get resource value of \a widget
 *
 * \param[in]   widget  resource radio group
 * \param[out]  id      object to store resource value
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_radiogroup_get(GtkWidget *widget, int *id)
{
    const char *resource = resource_widget_get_resource_name(widget);

    if (resources_get_int(resource, id) < 0) {
        debug_gtk3("failed to get value for resource '%s'.", resource);
        *id = 0;
        return FALSE;
    }
    return TRUE;
}


/** \brief  Synchronize \a widget with its current resource value
 *
 * \param[in,out]   widget  radiogroup widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_radiogroup_sync(GtkWidget *widget)
{
    const char *resource;
    int value;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_int(resource, &value) < 0) {
        debug_gtk3("failed to get value for resource '%s'.", resource);
        return FALSE;
    }
    return vice_gtk3_resource_radiogroup_set(widget, value);
}


/** \brief  Reset radio group to its factory default
 *
 * \param[in,out]   widget  resource radio group
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_radiogroup_factory(GtkWidget *widget)
{
    const char *resource;
    int value;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_default_value(resource, &value) < 0) {
        debug_gtk3("failed to get factory value for resource '%s'.", resource);
        return FALSE;
    }
#if 0
    debug_gtk3("resetting %s to factory value %d.", resource, value);
#endif
    return vice_gtk3_resource_radiogroup_set(widget, value);
}


/** \brief  Reset radio group to its value on instanciation
 *
 * \param[in,out]   widget  resource radio group
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_radiogroup_reset(GtkWidget *widget)
{
    int orig = resource_widget_get_int(widget, "ResourceOrig");
    return vice_gtk3_resource_radiogroup_set(widget, orig);
}


/** \brief  Set resource to widget's value
 *
 * \param[in,out]   widget  resource radiogroup widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_radiogroup_apply(GtkWidget *widget)
{
    NOT_IMPLEMENTED_WARN_ONLY();
    return FALSE;
}


/** \brief  Add an extra callback to \a widget
 *
 * This callback should allow widgets interacting with other widgets without
 * using any global references.
 *
 * The widget returned is the actual radio group GtkGrid, the integer value is
 * the curent ID of the radiogroup's currently selected radio button.
 *
 * \param[in,out]   widget      radiogroup widget
 * \param[in]       callback    function to call when the radiogroup selection
 *                              changes
 */
void vice_gtk3_resource_radiogroup_add_callback(
        GtkWidget *widget,
        void (*callback)(GtkWidget *, int))
{
    g_object_set_data(G_OBJECT(widget), "ExtraCallback", (gpointer)callback);
}
