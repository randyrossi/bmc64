/** \file   resourcebrowser.h
 * \brief   Text entry with label and browse button connected to a resource - header
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

#ifndef VICE_RESOURCEBROWSER_H
#define VICE_RESOURCEBROWSER_H

#include "vice.h"
#include <gtk/gtk.h>

GtkWidget *vice_gtk3_resource_browser_new(
        const char *resource,
        const char **patterns,
        const char *pattern_name,
        const char *browser_title,
        const char *label,
        void (*callback)(GtkWidget *, gpointer));

gboolean vice_gtk3_resource_browser_set(GtkWidget *widget, const char *new);
gboolean vice_gtk3_resource_browser_get(GtkWidget *widget, const char **dest);
gboolean vice_gtk3_resource_browser_reset(GtkWidget *widget);
gboolean vice_gtk3_resource_browser_sync(GtkWidget *widget);
gboolean vice_gtk3_resource_browser_factory(GtkWidget *widget);

#endif
