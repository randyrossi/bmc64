/** \file   isepicwidget.h
 * \brief   Widget to control ISEPIC resources - header
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

#ifndef VICE_ISEPICWIDGET_H
#define VICE_ISEPICWIDGET_H

#include "vice.h"
#include <gtk/gtk.h>

GtkWidget * isepic_widget_create(GtkWidget *parent);
void        isepic_widget_set_save_handler(int (*func)(int, const char *));
void        isepic_widget_set_flush_handler(int (*func)(int));

#endif
