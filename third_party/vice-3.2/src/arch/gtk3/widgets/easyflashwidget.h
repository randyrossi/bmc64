/** \file   easyflashwidget.h
 * \brief   Widget to control Easy Flash resources - header
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

#ifndef VICE_EASYFLASHWIDGET_H
#define VICE_EASYFLASHWIDGET_H

#include "vice.h"
#include <gtk/gtk.h>

GtkWidget * easyflash_widget_create(GtkWidget *parent);
void easyflash_widget_set_save_func(int (*func)(int, const char *));
void easyflash_widget_set_flush_func(int (*func)(int));

#endif
