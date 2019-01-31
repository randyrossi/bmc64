/**
 * \brief   Machine model selection widget- header
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *
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

#ifndef VICE_MACHINEMODELWIDGET_H
#define VICE_MACHINEMODELWIDGET_H

#include "vice.h"
#include <gtk/gtk.h>

void        machine_model_widget_getter(int (*f)(void));
void        machine_model_widget_setter(void (*f)(int model));
void        machine_model_widget_set_models(const char **list);

GtkWidget * machine_model_widget_create(void);
void        machine_model_widget_update(GtkWidget *widget);
void        machine_model_widget_connect_signals(GtkWidget *widget);


void        machine_model_widget_set_callback(void (*callback)(int));

#endif
