/**
 * \brief   Helper functions for resource widgets - header
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

#ifndef VICE_RESOURCEHELPERS_H
#define VICE_RESOURCEHELPERS_H

#include "vice.h"
#include <gtk/gtk.h>

void resource_widget_set_int(GtkWidget *widget, const char *key, int value);
int resource_widget_get_int(GtkWidget *widget, const char *key);
void resource_widget_set_string(GtkWidget *widget, const char *key,
                                const char *value);
const char *resource_widget_get_string(GtkWidget *widget, const char *key);
void resource_widget_free_string(GtkWidget *widget, const char *key);

void resource_widget_set_resource_name(GtkWidget *widget, const char *resource);
const char *resource_widget_get_resource_name(GtkWidget *widget);
void resource_widget_free_resource_name(GtkWidget *widget);

#endif
