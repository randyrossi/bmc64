/** \file   resourcespinbutton.h
 * \brief   Spin buttons to control resources - header
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

#ifndef VICE_RESOURCESPINBUTTON_H
#define VICE_RESOURCESPINBUTTON_H

#include "vice.h"
#include <gtk/gtk.h>

/*
 * New API
 */

GtkWidget *vice_gtk3_resource_spin_int_new(
        const char *resource,
        int lower, int upper, int step);

GtkWidget *vice_gtk3_resource_spin_int_new_sprintf(
        const char *fmt,
        int lower, int upper, int step,
        ...);

gboolean vice_gtk3_resource_spin_int_set(GtkWidget *widget, int value);
gboolean vice_gtk3_resource_spin_int_get(GtkWidget *widget, int *value);
gboolean vice_gtk3_resource_spin_int_reset(GtkWidget *widget);
gboolean vice_gtk3_resource_spin_int_sync(GtkWidget *widget);
gboolean vice_gtk3_resource_spin_int_factory(GtkWidget *widget);
gboolean vice_gtk3_resource_spin_int_apply(GtkWidget *widget);

void     vice_gtk3_resource_spin_int_set_fake_digits(GtkWidget *spin,
                                                     int digits);

#endif
