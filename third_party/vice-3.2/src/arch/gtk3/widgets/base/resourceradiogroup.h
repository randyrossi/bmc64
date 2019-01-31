/** \file   resourceradiogroup.h
 * \brief   Group of radio buttons controlling a resource - header
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

#ifndef VICE_RESOURCERADIOGROUP_H
#define VICE_RESOURCERADIOGROUP_H

#include "vice.h"
#include <gtk/gtk.h>
#include "basewidget_types.h"

/*
 * New API
 */

GtkWidget *vice_gtk3_resource_radiogroup_new(
        const char *resource,
        const vice_gtk3_radiogroup_entry_t *entries,
        GtkOrientation orientation);

GtkWidget *vice_gtk3_resource_radiogroup_new_sprintf(
        const char *fmt,
        const vice_gtk3_radiogroup_entry_t *entries,
        GtkOrientation orientation,
        ...);

gboolean vice_gtk3_resource_radiogroup_set(GtkWidget *widget, int id);

gboolean vice_gtk3_resource_radiogroup_get(GtkWidget *widget, int *id);

gboolean vice_gtk3_resource_radiogroup_sync(GtkWidget *widget);

gboolean vice_gtk3_resource_radiogroup_factory(GtkWidget *widget);

gboolean vice_gtk3_resource_radiogroup_reset(GtkWidget *widget);

void vice_gtk3_resource_radiogroup_add_callback(
        GtkWidget *widget,
        void (*callback)(GtkWidget*, int));

#endif
