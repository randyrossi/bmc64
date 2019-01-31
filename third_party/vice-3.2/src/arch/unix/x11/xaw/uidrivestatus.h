/*
 * uidrivestatus.h - Xaw only, UI controls for floppy emulation
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
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

#ifndef UIDRIVESTATUS_H_
#define UIDRIVESTATUS_H_

extern void ui_init_drive_status_widget(void);

#define ATT_IMG_SIZE 256
extern char last_attached_images[NUM_DRIVES][ATT_IMG_SIZE];
extern void rebuild_disk_menu_action(Widget w, XEvent *event, String *params, Cardinal *num_params);

#endif /* UIDRIVESTATUS_H_ */
