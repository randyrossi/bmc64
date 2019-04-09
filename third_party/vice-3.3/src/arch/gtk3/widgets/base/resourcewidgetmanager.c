/** \file   resourcewidgetmanager.c
 * \brief   Module to manage resource widgets
 *
 * This module allows one to register resource-bound widgets to the manager,
 * so the manager can then be invoked to either reset all widgets to their
 * state when they were registered, or reset the widgets to the
 * resource's default state.
 *
 * Example:
 *
 * \code{.c}
 *
 *  // Resource manager instance
 *  static resource_manager_t manager;
 *
 *  // Initialize manager
 *  vice_resource_widget_manager_init(&manager);
 *
 *  // Create a resource widget ...
 *  GtkWidget *check = vice_resource_check_button_create("SomeResource");
 *  // and add it to the manager (we pass NULL for the custom method pointers
 *  // since the resource check button has its own default reset(), sync() and
 *  // factory() methods.
 *  // To override a default method, pass a function pointer instead of NULL.
 *  vice_resource_widget_manager_add_widget(&manager, check, NULL, NULL, NULL);
 *
 *  // To reset the widgets registered with the manager we use:
 *  // (usually this function will be called from a "Reset" GtkButton)
 *  if (vice_resource_widget_manager_reset(&manager)) {
 *      g_print("OK\n");
 *  } else {
 *      g_print("oops\n");
 *  }
 *
 *  // And finally, to clean up resources used by the manager we use:
 *  // (usually this function will be called from a dialog's/containing widget's
 *  // "destroy" event handler)
 *  vice_resource_widget_manager_exit(&manager);
 * \endcode
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

#include "basewidgets.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "resourcehelpers.h"

#include "resourcewidgetmanager.h"


/** \brief  Initial number of entries for widgets allocated
 *
 * Seems like a decent number, but should one register another widget, the
 * list is simply doubled in size.
 */
#define INITIAL_ENTRIES 64



/** \brief  Initialize \a entry
 *
 * \param[in,out]   entry   resource widget entry
 */
static void resource_widget_entry_init(resource_widget_entry_t *entry)
{
    entry->widget = NULL;
    entry->resource = NULL;
    entry->reset = NULL;
    entry->factory = NULL;
    entry->sync = NULL;
}


/** \brief  Free members of \a entry
 *
 * \param[in]   entry   resource widget entry
 */
static void resource_widget_entry_cleanup(resource_widget_entry_t *entry)
{
    if (entry != NULL) {
        if (entry->resource != NULL) {
            lib_free(entry->resource);
        }
    }
}


/** \brief  Initialize the resource widget manager
 *
 * Initializes \a manager to an empty state. Allocates a list of widget entry
 * pointers which must be freed by vice_resource_widget_manager_exit().
 *
 * \param[in,out]   manager resource widget manager
 */
void vice_resource_widget_manager_init(resource_widget_manager_t *manager)
{
    size_t i;
    return;

    manager->widget_list = lib_malloc(sizeof *(manager->widget_list)
            * INITIAL_ENTRIES);
    manager->widget_num = 0;
    manager->widget_max = INITIAL_ENTRIES;
    for (i = 0; i < INITIAL_ENTRIES; i++) {
        manager->widget_list[i] = NULL;
    }
}


/** \brief  Clean up resources used by \a manager's members
 *
 * Doesn't free \a manager itself
 *
 * \param[in,out]   manager resource widget manager instance
 */
void vice_resource_widget_manager_exit(resource_widget_manager_t *manager)
{
    size_t i;
    return;

    if (manager->widget_list == NULL) {
        debug_gtk3("Got NULL as widget list, shouldn't happen!");
        return;
    }

    for (i = 0; i < manager->widget_num; i++) {
        resource_widget_entry_cleanup(manager->widget_list[i]);
        lib_free(manager->widget_list[i]);
    }
    lib_free(manager->widget_list);
}


/** \brief  Add a widget to the resource widget manager
 *
 */
void vice_resource_widget_manager_add_widget(
        resource_widget_manager_t *manager,
        GtkWidget *widget,
        const char *resource,
        gboolean (*reset)(GtkWidget *),
        gboolean (*factory)(GtkWidget *),
        gboolean (*sync)(GtkWidget *))
{
    resource_widget_entry_t *entry;

    return ;

    /* do we need to resize the list? */
    if (manager->widget_max == manager->widget_num) {
        /* resize widget array */
        manager->widget_list = lib_realloc(manager->widget_list,
            sizeof *(manager->widget_list) * manager->widget_max * 2);
        manager->widget_max *= 2;
    }

    /* create new entry */
    entry = lib_malloc(sizeof *entry);
    resource_widget_entry_init(entry);
    entry->widget = widget;
    if (resource != NULL) {
        entry->resource = lib_stralloc(resource);
    } else {
        entry->resource = NULL;
    }
    entry->reset = reset;
    entry->factory = factory;
    entry->sync = sync;

    /* store entry */
    manager->widget_list[manager->widget_num++] = entry;
}


/** \brief  Debug hook: dump information on \a manager on stdout
 *
 * \param[in]   manager resource widget manager
 */
void vice_resource_widget_manager_dump(resource_widget_manager_t *manager)
{
    size_t i;
    return;

    debug_gtk3("Resource Widget Manager: registered resources:");
    for (i = 0; i < manager->widget_num; i++) {
        resource_widget_entry_t *entry = manager->widget_list[i];

        if (entry->resource == NULL) {
            debug_gtk3("    standard widget: '%s'",
                    resource_widget_get_resource_name(entry->widget));
        } else {
            debug_gtk3("    custom widget  : '%s'",
                    entry->resource);
        }
    }
}


/** \brief  Reset all widgets registered with \a manager to their initial state
 *
 * Iterates the widgets registered with \a manager and executes their reset()
 * method in order. It first tries to find the default reset() method of the
 * resource widgets in gtk3/base/widgets, if that fails it tries to invoke the
 * custom reset() method passed when calling
 * vice_resource_widget_manager_add_widget(), and when that is NULL, it will
 * return FALSE to indicate failure.
 *
 * \param[in]   manager resource widget manager
 *
 * \return  bool
 */
gboolean vice_resource_widget_manager_reset(resource_widget_manager_t *manager)
{
    size_t i;

    printf("Resource Widget Manager: registered resources:\n");
    for (i = 0; i < manager->widget_num; i++) {
        resource_widget_entry_t *entry = manager->widget_list[i];

#ifdef HAVE_DEBUG_GTK3UI
        const char *resource;
        if (entry->resource == NULL) {
            resource = resource_widget_get_resource_name(entry->widget);
        } else {
            resource = entry->resource;
        }
#endif

        debug_gtk3("resetting resource '%s'", resource);
        /* custom reset func? */
        if (entry->reset != NULL) {
            debug_gtk3("calling custom reset function");
            entry->reset(entry->widget);
        } else {
            gboolean (*reset)(GtkWidget *) = NULL;
            debug_gtk3("calling default reset function");
            reset = g_object_get_data(G_OBJECT(entry->widget), "MethodReset");
            if (reset != NULL) {
                reset(entry->widget);
            } else {
                debug_gtk3("failed to find the reset method of the widget");
                return FALSE;
            }
        }
    }
    return TRUE;
}


/** \brief  Reset all widgets registered with \a manager to their factory state
 *
 * Iterates the widgets registered with \a manager and executes their factory()
 * method in order. It first tries to find the default factory() method of the
 * resource widgets in gtk3/base/widgets, if that fails it tries to invoke the
 * custom factory() method passed when calling
 * vice_resource_widget_manager_add_widget(), and when that is NULL, it will
 * return FALSE to indicate failure.
 *
 * \param[in]   manager resource widget manager
 *
 * \return  bool
 */
gboolean vice_resource_widget_manager_factory(resource_widget_manager_t *manager)
{
    size_t i;

    printf("Resource Widget Manager: registered resources:\n");
    for (i = 0; i < manager->widget_num; i++) {
        resource_widget_entry_t *entry = manager->widget_list[i];

#ifdef HAVE_DEBUG_GTK3UI
        const char *resource;
        if (entry->resource == NULL) {
            resource = resource_widget_get_resource_name(entry->widget);
        } else {
            resource = entry->resource;
        }
#endif

        debug_gtk3("resetting resource '%s' to factory value", resource);
        /* custom reset func? */
        if (entry->factory != NULL) {
            debug_gtk3("calling custom factory function");
            entry->reset(entry->widget);
        } else {
            gboolean (*factory)(GtkWidget *) = NULL;
            debug_gtk3("calling default factory function");
            factory = g_object_get_data(G_OBJECT(entry->widget),
                    "MethodFactory");
            if (factory != NULL) {
                factory(entry->widget);
            } else {
                debug_gtk3("failed to find the factory method of the widget");
                return FALSE;
            }
        }
    }
    return TRUE;
}


/** \brief  Apply all registered widget's values to their resources
 *
 * \param[in]   manager resource widget manager
 *
 * \return  bool
 */
gboolean vice_resource_widget_manager_apply(resource_widget_manager_t *manager)
{
    size_t i;

    printf("iterating widgets:\n");
    for (i = 0; i < manager->widget_num; i++) {
        resource_widget_entry_t *entry = manager->widget_list[i];
        GtkWidget *widget = entry->widget;
        if (resource_widget_get_auto_update(widget)) {
            debug_gtk3("updating resource '%s'.", entry->resource);
        }
    }

    return FALSE;
}

