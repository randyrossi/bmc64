/*
 * uispeed.c - Implementation of the custom speed dialog box.
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

#include "vice.h"

#include <string.h>
#include <windows.h>
#include <tchar.h>

#include "lib.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uilib.h"
#include "uispeed.h"
#include "winmain.h"


int ui_speed_current(void)
{
    int res_value;

    resources_get_int("Speed", &res_value);

    return res_value;
}

void ui_speed_settings_dialog(HWND hwnd)
{
    uilib_dialogbox_param_t param;
    int speed;

    resources_get_int("Speed", &speed);

    param.hwnd = hwnd;
    param.idd_dialog = IDD_CUSTOM_SPEED_DIALOG;
    param.idc_dialog = IDC_CUSTOM_SPEED;
    _itot(speed, param.string, 10);
    param.idc_dialog_trans = IDC_ENTER_CUSTOM_SPEED;
    param.idc_dialog_trans_text = translate_text(IDS_ENTER_CUSTOM_SPEED);
    param.idd_dialog_caption = translate_text(IDS_CUSTOM_SPEED_CAPTION);

    uilib_dialogbox(&param);

    if (param.updated > 0) {
        speed = _ttoi(param.string);
        if (speed > 0 && speed < 1000000) {
            resources_set_int("Speed", speed);
        }
    }
}
