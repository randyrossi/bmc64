/*
 * uiprinter.h
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

#ifndef VICE_UI_PRINTER_H
#define VICE_UI_PRINTER_H

extern UI_CALLBACK(uiprinter_set_printer_exec_file);
extern UI_CALLBACK(uiprinter_formfeed);

extern ui_menu_entry_t uiprinter_pr4_driver_submenu[];
extern ui_menu_entry_t uiprinter_pr4_output_submenu[];
extern ui_menu_entry_t uiprinter_pr4_device_submenu[];
extern ui_menu_entry_t uiprinter_set_printer4_type_submenu[];

extern ui_menu_entry_t uiprinter_pr5_driver_submenu[];
extern ui_menu_entry_t uiprinter_pr5_output_submenu[];
extern ui_menu_entry_t uiprinter_pr5_device_submenu[];
extern ui_menu_entry_t uiprinter_set_printer5_type_submenu[];

extern ui_menu_entry_t uiprinter_pr6_driver_submenu[];
extern ui_menu_entry_t uiprinter_pr6_output_submenu[];
extern ui_menu_entry_t uiprinter_pr6_device_submenu[];
extern ui_menu_entry_t uiprinter_set_printer6_type_submenu[];

extern ui_menu_entry_t uiprinter_pruser_driver_submenu[];
extern ui_menu_entry_t uiprinter_prUserport_output_submenu[];
extern ui_menu_entry_t uiprinter_pruser_device_submenu[];

extern void uiprinter_pruser_enable(int enable);

#endif
