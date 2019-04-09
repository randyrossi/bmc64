/** \file   uidata.h
 * \brief   GTK3 binary resources handling - header
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

#ifndef VICE_UIDATA_H
#define VICE_UIDATA_H

#include <gtk/gtk.h>

/** \brief  Root namespace of the GResouce files
 *
 * The GResource handling requires setting a namespace for the files, which
 * is similar to the argument to gtk_application_new(), basically inverting
 * a URL's components.
 *
 * In this case, I have used vice.pokefinder.org as our website.
 */
#define UIDATA_ROOT_PATH    "/org/pokefinder/vice"


int         uidata_init(void);
void        uidata_shutdown(void);

GdkPixbuf * uidata_get_pixbuf(const char *name);

#endif
