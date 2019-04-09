/** \file   uimon-fallback.h
 * \brief   Fallback implementation for the ML-Monitor for when the VTE library
 *          is not available - header
 *
 * \author  groepaz <groepaz@gmx.net>
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

#ifndef VICE_UIMONFALLBACK_H
#define VICE_UIMONFALLBACK_H

#include "vice.h"

#include <gtk/gtk.h>

int consolefb_close_all(void);
int consolefb_init(void);
int consolefb_out(console_t *log, const char *format, ...);
console_t *uimonfb_window_open(void);
void uimonfb_window_close(void);
void uimonfb_window_suspend( void );
console_t *uimonfb_window_resume(void);
int uimonfb_out(const char *buffer);
char *uimonfb_get_in(char **ppchCommandLine, const char *prompt);
void uimonfb_notify_change( void );
void uimonfb_set_interface(struct monitor_interface_s **monitor_interface_init, int count);

#endif
