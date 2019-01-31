/*
 * uirom.h - Implementation of the ROM settings dialog box.
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

#ifndef VICE_UIROM_H
#define VICE_UIROM_H

#include <tchar.h>

#include "ui.h"

#define UIROM_TYPE_MAIN  0
#define UIROM_TYPE_DRIVE 1
#define UIROM_TYPE_OTHER 2
#define UIROM_TYPE_MAX   3

struct uirom_settings_s {
    unsigned int type;
    const TCHAR *realname;
    const char *resname;
    unsigned int idc_filename;
    unsigned int idc_browse;
};
typedef struct uirom_settings_s uirom_settings_t;

extern void uirom_settings_dialog(HWND hwnd, unsigned int idd_dialog_main,
                                  unsigned int idd_dialog_drive,
                                  const uirom_settings_t *uirom_settings,
                                  uilib_localize_dialog_param *uirom_main_trans,
                                  uilib_localize_dialog_param *uirom_drive_trans,
                                  generic_trans_table_t *uirom_generic_trans,
                                  uilib_dialog_group *main_left_group,
                                  uilib_dialog_group *main_middle_group,
                                  uilib_dialog_group *main_right_group,
                                  uilib_dialog_group *drive_left_group,
                                  uilib_dialog_group *drive_middle_group,
                                  uilib_dialog_group *drive_right_group);

#endif
