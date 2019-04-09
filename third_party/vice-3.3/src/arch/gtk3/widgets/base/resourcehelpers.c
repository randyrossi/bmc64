/**
 * \brief   Helper functions for resource widgets
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
 */

#include "vice.h"
#include <gtk/gtk.h>
#include "lib.h"
#include "resources.h"

#include "resourcehelpers.h"


/** \brief  Set property \a key of \a widget to integer \a value
 *
 * Sets \a key to \a value using g_object_set_data(). Since this is a simple
 * scalar, it shouldn't be freed.
 *
 * \param[in,out]   widget  widget
 * \param[in]       key     property name
 * \param[in]       value   property value
 */
void resource_widget_set_int(GtkWidget *widget, const char *key, int value)
{
    g_object_set_data(G_OBJECT(widget), key, GINT_TO_POINTER(value));
}


/** \brief  Get integer value of  property \a key of \a widget
 *
 * \param[in,out]   widget  widget
 * \param[in]       key     property name
 *
 * \return  integer value of property \a key
 */
int resource_widget_get_int(GtkWidget *widget, const char *key)
{
    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), key));
}


/** \brief  Set property \a key of \a widget to string \a value
 *
 * Sets \a key to \a value using g_object_set_data() using lib_stralloc(), so
 * this function can be called with temporary data from the caller.
 * Needs to be freed with resource_widget_free_string().
 *
 * \param[in,out]   widget  widget
 * \param[in]       key     property name
 * \param[in]       value   property value
 */
void resource_widget_set_string(GtkWidget *widget,
                                const char *key,
                                const char *value)
{
    g_object_set_data(G_OBJECT(widget), key,
            (gpointer)lib_stralloc(value != NULL ? value : ""));
}


/** \brief  Get string value for property \a key of \a widget
 *
 * \param[in]   widget  widget
 * \param[in]   key     property name
 *
 * \return  value for \a key
 */
const char *resource_widget_get_string(GtkWidget *widget, const char *key)
{
    return (const char *)g_object_get_data(G_OBJECT(widget), key);
}


/** \brief  Free memory used by property \a key in \a widget
 *
 * Frees the string allocated on the heap by lib_stralloc().
 *
 * \param[in,out]   widget  widget
 * \param[in]       key     property name
 */
void resource_widget_free_string(GtkWidget *widget, const char *key)
{
    /* free(NULL) is valid, so this is safe */
    lib_free(g_object_get_data(G_OBJECT(widget), key));
}


/** \brief  Set the "ResourceName" property of \a widget to \a resoucre
 *
 * Stores a copy of \a resource in \a widget using lib_stralloc()
 *
 * \param[in]   widget      widget
 * \param[in]   resource    resource name
 */
void resource_widget_set_resource_name(GtkWidget *widget, const char *resource)
{
    resource_widget_set_string(widget, "ResourceName", resource);
}


/** \brief  Get name of resource currently registerd to \a widget
 *
 * \param[in]   widget  widget
 *
 * \return  value of the "ResourceName" property of \a widget
 */
const char *resource_widget_get_resource_name(GtkWidget *widget)
{
    return resource_widget_get_string(widget, "ResourceName");
}


/** \brief  Free memory used by the "ResourceName" property of \a widget
 *
 * \param[in,out]   widget  widget
 */
void resource_widget_free_resource_name(GtkWidget *widget)
{
    resource_widget_free_string(widget, "ResourceName");
}


/** \brief  Register the reset(), factory() and sync() methods for \a widget
 *
 * \param[in,out]   widget  vice resource widget
 * \param[in]       reset   reset() method pointer
 * \param[in]       factory factory() method pointer
 * \param[in]       sync    sync() method pointer
 */
void resource_widget_register_methods(
        GtkWidget *widget,
        gboolean (*reset)(GtkWidget *),
        gboolean (*factory)(GtkWidget *),
        gboolean (*sync)(GtkWidget *),
        gboolean (*apply)(GtkWidget *))
{
    g_object_set_data(G_OBJECT(widget), "MethodReset", (gpointer)reset);
    g_object_set_data(G_OBJECT(widget), "MethodFactory", (gpointer)factory);
    g_object_set_data(G_OBJECT(widget), "MethodSync", (gpointer)sync);
    g_object_set_data(G_OBJECT(widget), "MethodApply", (gpointer)apply);
}


/** \brief  Get the \a method function pointer of \a widget
 *
 * Tries to retrieve the \a method from the data in \a widget as set by
 * resource_widget_register_methods() and stores the function pointer in
 * \a func.
 *
 * On error this function returns FALSE and \a func  is set to `NULL`.
 *
 * \param[in]   widget  resource widget to retrieve the \a method from
 * \param[out]  func    object to store the function pointer
 * \param[in]   method  name of the method (ie "MethodSync", "MethodReset", etc)
 *
 * \return  bool
 */
static gboolean resource_widget_get_method_helper(
        GtkWidget *widget,
        gboolean *(*func)(GtkWidget *),
        const char *method)
{
    return FALSE;
}


/** \brief  Get the reset method of \a widget
 *
 * \param[in]   widget  resource widget
 * \param[out]  reset   object to store the reset() function pointer
 *
 * \return bool
 */
gboolean resource_widget_get_method_reset(
        GtkWidget *widget,
        gboolean *(*reset)(GtkWidget *))
{
    return resource_widget_get_method_helper(widget, reset, "MethodReset");
}


/** \brief  Get the factory method of \a widget
 *
 * \param[in]   widget  resource widget
 * \param[out]  factory object to store the factory() function pointer
 *
 * \return bool
 */
gboolean resource_widget_get_method_factory(
        GtkWidget *widget,
        gboolean *(*factory)(GtkWidget *))
{
    return resource_widget_get_method_helper(widget, factory, "MethodFactory");
}


/** \brief  Get the sync method of \a widget
 *
 * \param[in]   widget  resource widget
 * \param[out]  sync    object to store the sync() function pointer
 *
 * \return bool
 */
gboolean resource_widget_get_method_sync(
        GtkWidget *widget,
        gboolean *(*sync)(GtkWidget *))
{
    return resource_widget_get_method_helper(widget, sync, "MethodReset");
}


/** \brief  Set the AutoUpdate property of \a widget to \a state
 *
 * The AutoUpdate property decides whether a resource-bound widget updates its
 * value when it changes. This is TRUE by default.
 *
 * \param[in,out]   widget  resource-bound widget
 * \param[in]       state   do auto-update
 */
void resource_widget_set_auto_update(GtkWidget *widget, gboolean state)
{
    resource_widget_set_int(widget, "AutoUpdate", (int)state);
}


/** \brief  Get the AutoUpdate property of \a widget to \a state
 *
 * The AutoUpdate property decides whether a resource-bound widget updates its
 * value when it changes. This is TRUE by default.
 *
 * \param[in,out]   widget  resource-bound widget
 *
 * \return  auto-update state
 */
gboolean resource_widget_get_auto_update(GtkWidget *widget)
{
    return (gboolean)resource_widget_get_int(widget, "AutoUpdate");
}
