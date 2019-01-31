/** \file   resourcescale.c
 * \brief   GktScale widget to control resources
 *
 * \note    Only integer resources are supported, which should be fine since
 *          VICE resources are either int or string (or strings abused to store
 *          arbitraty data)
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
#include <stdarg.h>

#include "basewidget_types.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "resourcehelpers.h"

#include "resourcescale.h"


/** \brief  Handler for the "destroy" event of the \a scale widget
 *
 * Frees memory used by the copy of the resource name
 *
 * \param[in]   widget      integer scale widget
 * \param[in]   user_data   extra event data (unused)
 */
static void on_scale_int_destroy(GtkWidget *widget, gpointer user_data)
{
    resource_widget_free_resource_name(widget);
}


/** \brief  Handler for the "value-changed" event of the \a scale widget
 *
 * Updates resource value
 *
 * \param[in]   widget      integer scale widget
 * \param[in]   user_data   extra event data (unused)
 */
static void on_scale_int_changed(GtkWidget *widget, gpointer user_data)
{
    const char *resource;
    int old_val;
    int new_val;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_int(resource, &old_val) < 0) {
        log_error(LOG_ERR, "failed to get value for resource '%s'\n",
                resource);
        return;
    }
    new_val = (int)gtk_range_get_value(GTK_RANGE(widget));
    /* only update resource when required */
    if (old_val != new_val) {
#if 0
        debug_gtk3("setting %s to %d\n", resource, new_val);
#endif
        resources_set_int(resource, new_val);
    }
}


/** \brief  Create a scale for an integer resource - helper
 *
 * \param[in]   widget      interger scale widget
 * \param[in]   orientation scale orientation (\see GtkOrientation)
 * \param[in]   low         lowest value for scale
 * \param[in]   high        highest value for scale
 * \param[in]   step        value to incr/decr value with cursor keys
 *
 * \return  GtkScale
 */
static GtkWidget *resource_scale_int_new_helper(GtkWidget *widget)
{
    int value;
    const char *resource;

    resource = resource_widget_get_resource_name(widget);

    gtk_scale_set_digits(GTK_SCALE(widget), 0);

    /* set current value */
    if (resources_get_int(resource, &value) < 0) {
        log_error(LOG_ERR, "failed to get value for resource '%s'\n",
                resource);
        value = 0;
    }

    /* remember original value for reset() */
    resource_widget_set_int(widget, "ResourceOrig", value);

    gtk_range_set_value(GTK_RANGE(widget), (gdouble)value);

    g_signal_connect(widget, "value-changed", G_CALLBACK(on_scale_int_changed),
            NULL);
    g_signal_connect(widget, "destroy", G_CALLBACK(on_scale_int_destroy), NULL);

    gtk_widget_show(widget);
    return widget;
}

/** \brief  Create a scale for an integer resource
 *
 * \param[in]   resource    resource name
 * \param[in]   orientation scale orientation (\see GtkOrientation)
 * \param[in]   low         lowest value for scale
 * \param[in]   high        highest value for scale
 * \param[in]   step        value to incr/decr value with cursor keys
 *
 * \return  GtkScale
 */
GtkWidget *vice_gtk3_resource_scale_int_new(
        const char *resource,
        GtkOrientation orientation,
        int low, int high, int step)
{
    GtkWidget *scale;

    scale = gtk_scale_new_with_range(orientation,
            (gdouble)low, (gdouble)high, (gdouble)step);
    /* store copy of resource name */
    resource_widget_set_resource_name(scale, resource);

    return resource_scale_int_new_helper(scale);
}


/** \brief  Create a scale for an integer resource
 *
 * \param[in]   fmt         resource name format string
 * \param[in]   orientation scale orientation (\see GtkOrientation)
 * \param[in]   low         lowest value for scale
 * \param[in]   high        highest value for scale
 * \param[in]   step        value to incr/decr value with cursor keys
 *
 * \return  GtkScale
 */
GtkWidget *vice_gtk3_resource_scale_int_new_sprintf(
        const char *fmt,
        GtkOrientation orientation,
        int low, int high, int step,
        ...)
{
    GtkWidget *scale;
    char *resource;
    va_list args;

    scale = gtk_scale_new_with_range(orientation,
            (gdouble)low, (gdouble)high, (gdouble)step);

    va_start(args, step);
    resource = lib_mvsprintf(fmt, args);
    g_object_set_data(G_OBJECT(scale), "ResourceName", (gpointer)resource);
    va_end(args);

    return resource_scale_int_new_helper(scale);
}


/** \brief  Add marks to integer \a scale widget at each \a step increment
 *
 * \param[in,out]   scale   integer scale widget
 * \param[in]       step    distance between marks
 */
void vice_gtk3_resource_scale_int_set_marks(GtkWidget *widget, int step)
{
    GtkAdjustment *adj;
    int lower;
    int upper;
    int i;

    adj = gtk_range_get_adjustment(GTK_RANGE(widget));
    lower = (int)gtk_adjustment_get_lower(adj);
    upper = (int)gtk_adjustment_get_upper(adj);

    for (i = lower; i <= upper; i += step) {
        gtk_scale_add_mark(GTK_SCALE(widget), (gdouble)i, GTK_POS_BOTTOM, NULL);
    }
}


/** \brief  Set the \a scale widget to \a value
 *
 * \param[in,out]   widget  integer scale widget
 * \param[in]       value   new value for \a scale
 */
gboolean vice_gtk3_resource_scale_int_set(GtkWidget *widget, int value)
{
    gtk_range_set_value(GTK_RANGE(widget), (gdouble)value);
    return TRUE;
}


/** \brief  Restore the \a scale to the resource's value on instanciation
 *
 * Restores the widget to the resource value on instanciantion.
 *
 * \param[in,out]   widget  integer scale widget
 */
gboolean vice_gtk3_resource_scale_int_reset(GtkWidget *widget)
{
    int orig = resource_widget_get_int(widget, "ResourceOrig");

    return vice_gtk3_resource_scale_int_set(widget, orig);
}


/** \brief  Reset \a widget to the resource factory value
 *
 * \param[in,out]   widget  integer scale widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_scale_int_factory(GtkWidget *widget)
{
    const char *resource;
    int factory;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_default_value(resource, &factory) < 0) {
        debug_gtk3("failed to get factory value for resource '%s'\n", resource);
        return FALSE;
    }
    return vice_gtk3_resource_scale_int_set(widget, factory);
}


/** \brief  Synchronize \a widget with its resource
 *
 * \param[in,out]   integer scale widget
 *
 * \return  bool
 */
gboolean vice_gtk3_resource_scale_int_sync(GtkWidget *widget)
{
    const char *resource;
    int current;

    resource = resource_widget_get_resource_name(widget);
    if (resources_get_int(resource, &current) < 0) {
        debug_gtk3("failed to get value for resource '%s'\n", resource);
        return FALSE;
    }
    return vice_gtk3_resource_scale_int_set(widget, current);
}
