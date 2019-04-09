/** \file   resourcewidgetmanager.h
 * \brief   Module to manage resource widgets - header
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

#ifndef VICE_RESOURCEMANAGERWIDGET_H
#define VICE_RESOURCEMANAGERWIDGET_H


#include "vice.h"
#include <gtk/gtk.h>

/** \brief  A reference to a single resource-bound widget
 *
 * In case the widget is one of the resource* widgets in widgets/base, the
 * resource, reset, factory and sync fields don't need to be filled in, and
 * NULL can be passed for those. The resource* widgets themself contain all
 * the information to handle a reset-to-initial-state or a reset-to-factory.
 * In case of a non base resource widget, these fields need to be filled in
 * for the manager to work correctly.
 */
typedef struct resource_widget_entry_s {
    GtkWidget *widget;  /**< widget bound to a resource */
    char *resource;     /**< resource name (optional) */
    gboolean (*reset)(GtkWidget *);     /**< reset method (optional) */
    gboolean (*factory)(GtkWidget *);   /**< factory method (optiona) */
    gboolean (*sync)(GtkWidget *);      /**< sync method (optional) */
} resource_widget_entry_t;


/** \brief  Resource widgets manager object
 *
 * Keeps track of all the resource-bound widgets and allows resetting widgets
 * and their resources to their state during construction, setting widgets
 * and their resources to their factory setting, or synchronizing widgets with
 * their resources, should the resources have changed and for some reason the
 * widget wasn't updated.
 */
typedef struct resource_widget_manager_s {
    resource_widget_entry_t **widget_list;
    size_t widget_num;
    size_t widget_max;
} resource_widget_manager_t;



void vice_resource_widget_manager_init(resource_widget_manager_t *manager);
void vice_resource_widget_manager_exit(resource_widget_manager_t *manager);

void vice_resource_widget_manager_add_widget(
        resource_widget_manager_t *manager,
        GtkWidget *widget,
        const char *resource,
        gboolean (*reset)(GtkWidget *),
        gboolean (*factory)(GtkWidget *),
        gboolean (*sync)(GtkWidget *));


void vice_resource_widget_manager_dump(resource_widget_manager_t *manager);

gboolean vice_resource_widget_manager_reset(resource_widget_manager_t *manager);
gboolean vice_resource_widget_manager_factory(resource_widget_manager_t *manager);
gboolean vice_resource_widget_manager_apply(resource_widget_manager_t *manager);

#endif


