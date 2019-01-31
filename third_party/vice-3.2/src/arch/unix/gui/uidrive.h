/*
 * uidrive.h
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_UIDRIVE_H
#define VICE_UIDRIVE_H

#include "uimenu.h"

extern struct ui_menu_entry_s ui_drive_options_submenu[];
extern struct ui_menu_entry_s ui_flash_options_submenu[];

extern UI_CALLBACK(uidrive_extend_policy_control);

extern struct ui_menu_entry_s set_drive0_extend_image_policy_submenu[];
extern struct ui_menu_entry_s set_drive1_extend_image_policy_submenu[];
extern struct ui_menu_entry_s set_drive2_extend_image_policy_submenu[];
extern struct ui_menu_entry_s set_drive3_extend_image_policy_submenu[];

extern UI_CALLBACK(radio_Drive8Type);
extern UI_CALLBACK(radio_Drive9Type);
extern UI_CALLBACK(radio_Drive10Type);
extern UI_CALLBACK(radio_Drive11Type);

extern UI_CALLBACK(drive_rpm_settings);
extern UI_CALLBACK(drive_wobble_settings);

#endif
