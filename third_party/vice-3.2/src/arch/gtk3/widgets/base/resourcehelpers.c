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
