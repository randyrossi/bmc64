/** \file   videomodelwidget.h
 * \brief   Video chip model selection widget - header
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  TODO (depends on the machine used and for x128 it will handle two resources)
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

#ifndef VICE_VIDEOMODELWIDGET_H
#define VICE_VIDEOMODELWIDGET_H

#include "vice.h"
#include <gtk/gtk.h>
#include "widgethelpers.h"

void video_model_widget_set_title(const char *title);
void video_model_widget_set_resource(const char *resource);
void video_model_widget_set_models(const vice_gtk3_radiogroup_entry_t *models);

GtkWidget * video_model_widget_create(GtkWidget *machine);
void        video_model_widget_update(GtkWidget *widget);

void        video_model_widget_connect_signals(GtkWidget *widget);

void        video_model_widget_set_callback(GtkWidget *widget,
                                            void (*callback)(int));

#endif
