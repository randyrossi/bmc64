/**
 * \brief   Gtk3 basic dialogs (Info, Yes/No, etc)
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

#ifndef VICE_BASEDIALOGS_H
#define VICE_BASEDIALOGS_H

#include "vice.h"
#include <gtk/gtk.h>

#include "openfiledialog.h"
#include "savefiledialog.h"
#include "selectdirectorydialog.h"

/** \brief  Custom response IDs for dialogs
 */
enum {
    VICE_RESPONSE_AUTOSTART = 1     /**< trigger autostart */
};


gboolean vice_gtk3_message_info(const char *title, const char *fmt, ...);
gboolean vice_gtk3_message_confirm(const char *title, const char *fmt, ...);
gboolean vice_gtk3_message_error(const char *title, const char *fmt, ...);

#endif
