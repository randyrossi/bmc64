/*
 * uimonmsg.h - Private Window messages for the monitor UI implementation.
 *
 * Written by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
 *  Tibor Biczo <crown@t-online.de>
 *
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

#ifndef VICE_UIMONMSG_H
#define VICE_UIMONMSG_H

#define WM_CONSOLE_INSERTLINE (WM_USER)
#define WM_CONSOLE_CLOSED     (WM_USER + 1)
#define WM_CONSOLE_RESIZED    (WM_USER + 2)
#define WM_CONSOLE_ACTIVATED  (WM_USER + 3)

#endif
