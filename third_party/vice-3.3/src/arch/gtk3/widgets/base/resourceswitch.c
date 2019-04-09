/** \file   resourceswitch.c
 * \brief   GtkSwitch button connected to a resource
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * This widget presents a check button that controls a boolean resource. During
 * construction the current resource value is stored in the widget to allow
 * resetting to default.
 *
 * \code{.c}
 *
 *  GtkWidget *sw;
 *
 *  // create a widget
 *  sw = vice_gtk3_resource_switch_create("SomeResource");
 *  // any state change of the widget will now update the resource
 *
 *  // restore widget & resource to their initial state
 *  vice_gtk3_resource_switch_reset(check);
 *
 * \endcode
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
#include <stdarg.h>

#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "resourcehelpers.h"

#include "resourceswitch.h"


/** \brief  Handler for the "destroy" event of the switch
 *
 * Frees the heap-allocated copy of the resource name.
 *
 * \param[in]   widget      switch
 * \param[in]   user_data   extra event data (unused)
 */
static void on_switch_destroy(GtkWidget *widget, gpointer user_data)
{
    resource_widget_free_resource_name(widget);
}


/** \brief  Handler for the "state-set" event of the switch
 *
 * \param[in]   widget      switch
 * \param[in]   user_data   resource name
 */
static void on_switch_state_set(GtkWidget *widget, gpointer user_data)
{
    const char *resource;
    int state;
    int current;

    resource = resource_widget_get_resource_name(widget);
    state = gtk_switch_get_active(GTK_SWITCH(widget));
    if (resources_get_int(resource, &current) < 0) {
        /* invalid resource, exit */
        log_error(LOG_ERR, "invalid resource name'%s'\n", resource);
        return;
    }

    /* make sure we don't update a resource when the UI happens to be out of
     * sync for some reason */
    if (state != current) {
#if 0
        debug_gtk3("setting %s to %s.", resource, state ? "True": "False");
#endif
        if (resources_set_int(resource, state ? 1 : 0) < 0) {
            log_error(LOG_ERR,
                    "setting %s to %s failed\n",
                    resource, state ? "True": "False");
            /* get current resource value (validity of the name has been
             * checked already */
            resources_get_int(resource, &current);
            gtk_switch_set_active(GTK_SWITCH(widget), current ? TRUE : FALSE);
        }
    }
}


/** \brief  Check button setup helper
 *
 * Called by either vice_gtk3_resource_switch_create() or
 * vice_gtk3_resource_switch_create_printf() to finish setting up the resource
 * switch \a widget.
 *
 * \param[in]   widget  switch
 *
 * \return  new check button
 */
static GtkWidget *resource_switch_new_helper(GtkWidget *widget)
{
    int state;
    const char *resource;

    /* get current resource value */
    resource = resource_widget_get_resource_name(widget);
    if (resources_get_int(resource, &state) < 0) {
        /* invalid resource, set state to off */
        log_error(LOG_ERR, "invalid resource name '%s'\n", resource);
        state = 0;
    }

    /* remember original state for reset() */
    resource_widget_set_int(widget, "ResourceOrig", state);

    gtk_switch_set_active(GTK_SWITCH(widget),
            state ? TRUE : FALSE);

    /* register methods to be used by the resource widget manager */
    resource_widget_register_methods(
            widget,
            vice_gtk3_resource_switch_reset,
            vice_gtk3_resource_switch_factory,
            vice_gtk3_resource_switch_sync,
            vice_gtk3_resource_switch_apply);
    g_signal_connect(widget, "state-set", G_CALLBACK(on_switch_state_set),
            (gpointer)resource);
    g_signal_connect(widget, "destroy", G_CALLBACK(on_switch_destroy),
            NULL);

    gtk_widget_show(widget);
    return widget;
}



/** \brief  Create switch to toggle \a resource
 *
 * Creates a switch to toggle \a resource. Makes a heap-allocated copyof the
 * resource name so that initializing this widget with a constructed/temporary
 * resource name works as well.
 *
 * \param[in]   resource    resource name
 *
 * \note    The resource name is stored in the "ResourceName" property.
 *
 * \return  new switch
 */
GtkWidget *vice_gtk3_resource_switch_new(const char *resource)
{
    GtkWidget *widget;

    widget = gtk_switch_new();

    /* make a copy of the resource name and store the pointer in the propery
     * "ResourceName" */
    resource_widget_set_resource_name(widget, resource);

    return resource_switch_new_helper(widget);
}


/** \brief  Create switch to toggle a resource
 *
 * Creates a switch to toggle a resource. Makes a heap-allocated copy
 * of the resource name so initializing this widget with a constructed/temporary
 * resource name works as well. The resource name can be constructed with a
 * printf() format string.
 *
 * \param[in]   fmt         resource name format string
 *
 * \note    The resource name is stored in the "ResourceName" property.
 *
 * \return  new switch
 */
GtkWidget *vice_gtk3_resource_switch_new_sprintf(const char *fmt, ...)
{
    GtkWidget *widget;
    va_list args;
    char *resource;

    widget = gtk_switch_new();

    va_start(args, fmt);
    resource = lib_mvsprintf(fmt, args);
    g_object_set_data(G_OBJECT(widget), "ResourceName", (gpointer)resource);
    va_end(args);

    return resource_switch_new_helper(widget);
}


/** \brief  Set new \a value for \a widget
 *
 * \param[in,out]   widget  switch
 * \param[in]       value   new value
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_switch_set(GtkWidget *widget, gboolean value)
{
    gtk_switch_set_active(GTK_SWITCH(widget), value);
    return TRUE;
}


/** \brief  Get value for \a widget
 *
 * \param[in,out]   widget  switch
 * \param[out       value   object to store value
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_switch_get(GtkWidget *widget, gboolean *value)
{
    const char *resource = resource_widget_get_resource_name(widget);
    int state;

    if (resources_get_int(resource, &state) < 0) {
        *value = FALSE;
        return FALSE;
    }
    *value = (gboolean)state;
    return TRUE;
}


/** \brief  Reset state to the value during instanciation
 *
 * \param[in,out]   widget  resource switch
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_switch_reset(GtkWidget *widget)
{
    int value = resource_widget_get_int(widget, "ResourceOrig");
    return vice_gtk3_resource_switch_set(widget, value);
}


/** \brief  Reset switch to factory state
 *
 * \param[in,out]   check   check button
 */
gboolean vice_gtk3_resource_switch_factory(GtkWidget *widget)
{
    const char *resource;
    int value;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_default_value(resource, &value) < 0) {
        debug_gtk3("failed to get factory value for resource '%s'.", resource);
        return FALSE;
    }
#if 0
    debug_gtk3("resetting %s to factory value %s.",
            resource, value ? "True" : "False");
#endif
    return vice_gtk3_resource_switch_set(widget, (gboolean)value);
}


/** \brief  Synchronize \a widget with its associated resource value
 *
 * \param[in,out]   widget  resource switch widget
 *
 *
 * \return bool
 */
gboolean vice_gtk3_resource_switch_sync(GtkWidget *widget)
{
    const char *resource;
    gboolean widget_val;
    int resource_val;

    resource = resource_widget_get_resource_name(widget);
    if (!vice_gtk3_resource_switch_get(widget, &widget_val)) {
        log_error(LOG_ERR,
                "failed to retrieve current widget state for resource '%s'",
                resource);
        return FALSE;
    }
    if (resources_get_int(resource, &resource_val) < 0) {
        log_error(LOG_ERR,
                "failed to retrieve current value for resource '%s'",
                resource);
        return FALSE;
    }

    /* update widget if required */
    if (widget_val != (gboolean)resource_val) {
        return vice_gtk3_resource_switch_set(widget, (gboolean)resource_val);
    }
    return TRUE;
}


/** \brief  Set resource to the widget's value
 *
 * \param[in,out]   widget  resource switch button widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_switch_apply(GtkWidget *widget)
{
    NOT_IMPLEMENTED_WARN_ONLY();
    return FALSE;
}
