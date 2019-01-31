/**
 * \file    uimachinewindow.h
 * \brief   Native GTK3 main emulator window code
 *
 * \author  Marcus Sutton <loggedoubt@gmail.com>
 *
 * A "machine window" is any window whose primary purpose is to
 * display an emulated screen. It usually also includes a menu bar and
 * a status bar.
 */

/* This file is part of VICE, the Versatile Commodore Emulator.
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

#ifndef VICE_UIMACHINEWINDOW_H
#define VICE_UIMACHINEWINDOW_H

#include "vice.h"
#include <gtk/gtk.h>

/** \brief Set up any resources needed to create new machine
 *         windows. */
void ui_machine_window_init(void);

#endif /* VICE_UIMACHINEWINDOW_H */
