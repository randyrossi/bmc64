/** \file   mousedrv.h
 * \brief   Mouse handling for native GTK3 UI - header
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef VICE_MOUSEDRV_H
#define VICE_MOUSEDRV_H

#include "types.h"

#include "mouse.h"

/** \brief Register callbacks for mouse button presses.
 *  \param funcs The callbacks to register.
 *  \return Zero on success, nonzero on failure. */
int mousedrv_resources_init(mouse_func_t *funcs);

/** \brief Register and parse mouse-related command-line options.
 *  \return Zero on success, nonzero on failure. */
int mousedrv_cmdline_options_init(void);

/** \brief Initialize the mouse-handling subsystem. */
void mousedrv_init(void);

/** \brief Called by the emulation core to announce the mouse has been
 *         enabled or disabled. */
void mousedrv_mouse_changed(void);

/** \brief Returns the current mouse X value.
 *
 *  This is a running total of mouse movements and does not
 *  necessarily correspond to any particular screen position.
 *
 *  \return The current X value, in the range 0-65536.
 */
int mousedrv_get_x(void);

/** \brief Returns the current mouse Y value.
 *
 *  This is a running total of mouse movements and does not
 *  necessarily correspond to any particular screen position.
 *
 *  \return The current Y value, in the range 0-65536.
 */
int mousedrv_get_y(void);

/** \brief Returns the last time the mouse position changed.
 *
 *  \note A button press or release is not a change.
 *
 *  \return The current X value, in the range 0-65536.
 */
unsigned long mousedrv_get_timestamp(void);

/** \brief Called by the UI event handler to announce the user has
 *         pressed or released a button.
 *  \param bnumber Which button was pressed or released.
 *  \param state   Nonzero if button was pressed, zero if it was released.
 */
void mouse_button(int bnumber, int state);

/** \brief Called by the UI event handler to announce that the mouse
 *         has moved.
 *  \param dx Amount the mouse moved in the X direction
 *  \param dy Amount the mouse moved in the Y direction
 *  \todo  Determine what the actual units here are.
 */    
void mouse_move(float dx, float dy);

#endif
