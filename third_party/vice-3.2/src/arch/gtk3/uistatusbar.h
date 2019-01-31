/**
 * \file    uistatusbar.h
 * \brief   Gtk3 status bar - header
 *
 * This header file only provides functionality that is exposed by the
 * GTK3 statusbar API and that is not declared as part of some other,
 * more general interface. Since the statusbar implementation is very
 * self-contained that means this header file is extremely sparse.
 *
 * \author  Michael C. Martin <mcmartin@gmail.com>
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

#ifndef VICE_UISTATUSBAR_H
#define VICE_UISTATUSBAR_H

#include "vice.h"
#include <gtk/gtk.h>

void ui_statusbar_init(void);
void ui_statusbar_shutdown(void);

GtkWidget *ui_statusbar_create(void);

void ui_display_statustext(const char *text, int fadeout);

#endif
