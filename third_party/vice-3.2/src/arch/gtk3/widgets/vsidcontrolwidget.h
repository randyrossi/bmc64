/** \file   vsidcontrolwidget.h
 * \brief   GTK3 control widget for VSID - header
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

#ifndef VICE_VSIDCONTROLWIDGET_H
#define VICE_VSIDCONTROLWIDGET_H

#include "vice.h"
#include <gtk/gtk.h>

GtkWidget *vsid_control_widget_create(void);

void vsid_control_widget_set_tune_count(int nr);
void vsid_control_widget_set_tune_current(int nr);
void vsid_control_widget_set_tune_default(int nr);
#endif
