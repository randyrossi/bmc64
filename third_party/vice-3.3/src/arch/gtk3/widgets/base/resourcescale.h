/** \file   resourcescale.h
 * \brief   Scale to control an integer resource - header
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

#ifndef VICE_RESOURCESCALE_H
#define VICE_RESOURCESCALE_H

#include "vice.h"
#include <gtk/gtk.h>
#include "basewidget_types.h"

/*
 * New API
 */

GtkWidget *vice_gtk3_resource_scale_int_new(
        const char *resource,
        GtkOrientation orientation,
        int low, int high, int step);

GtkWidget *vice_gtk3_resource_scale_int_new_sprintf(
        const char *fmt,
        GtkOrientation orientation,
        int low, int high, int step,
        ...);

gboolean vice_gtk3_resource_scale_int_set(GtkWidget *widget, int value);

void vice_gtk3_resource_scale_int_set_marks(GtkWidget *widget, int step);

gboolean vice_gtk3_resource_scale_int_get(GtkWidget *widget, int value);
gboolean vice_gtk3_resource_scale_int_reset(GtkWidget *widget);
gboolean vice_gtk3_resource_scale_int_factory(GtkWidget *widget);
gboolean vice_gtk3_resource_scale_int_sync(GtkWidget *widget);
gboolean vice_gtk3_resource_scale_int_apply(GtkWidget *widget);

#endif
