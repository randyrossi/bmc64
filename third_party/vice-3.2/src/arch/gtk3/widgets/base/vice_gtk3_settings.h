/**
 * \brief   Default settings for Gtk3 code
 *
 * This file contains some default values for the Gtk3 UI of VICE.
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

#ifndef VICE_VICE_GTK3_SETTINGS_H
#define VICE_VICE_GTK3_SETTINGS_H

#include <limits.h>

/** \brief  The value to signal to VICE Gtk3 code a default should be used
 */
#define VICE_GTK3_DEFAULT   INT_MIN

/** \brief  Default column spacing for `GtkGrid`'s
 */
#define VICE_GTK3_GRID_COLUMN_SPACING   16


/** \brief  Default row spacing for `GtkGrid`'s
 */
#define VICE_GTK3_GRID_ROW_SPACING      8


#endif
