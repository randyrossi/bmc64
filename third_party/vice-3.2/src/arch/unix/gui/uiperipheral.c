/*
 * uiperipheral.c
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

#include <stdio.h>

#include "attach.h"
#include "lib.h"
#include "resources.h"
#include "uilib.h"
#include "uimenu.h"
#include "uiprinter.h"
#include "util.h"
#include "vsync.h"

#include "uiperipheral.h"


UI_MENU_DEFINE_RADIO(FileSystemDevice8)
UI_MENU_DEFINE_RADIO(FileSystemDevice9)
UI_MENU_DEFINE_RADIO(FileSystemDevice10)
UI_MENU_DEFINE_RADIO(FileSystemDevice11)

UI_CALLBACK(uiperipheral_set_fsdevice_directory)
{
    int unit = vice_ptr_to_int(UI_MENU_CB_PARAM);
    char *resname;
    char *title;
    char *path = util_concat(_("Path"), ":", NULL);

    resname = lib_msprintf("FSDevice%dDir", unit);
    title = lib_msprintf("Attach file system directory to device #%d", unit);

    uilib_select_dir(resname, title, path);

    lib_free(resname);
    lib_free(title);
    lib_free(path);
}


UI_CALLBACK(uiperipheral_set_rawdevice_name)
{
    uilib_select_dev((char *)UI_MENU_CB_PARAM, _("Select block device for raw access"), UILIB_FILTER_ALL);
}

#define UIPERIPHERAL_SET_DEVICE_TYPE_MENU_COMMON(x)                                           \
    { N_("None"), UI_MENU_TYPE_TICK,                                                          \
      (ui_callback_t)radio_FileSystemDevice##x, (ui_callback_data_t)ATTACH_DEVICE_NONE, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                              \
    { N_("File system access"), UI_MENU_TYPE_TICK,                                            \
      (ui_callback_t)radio_FileSystemDevice##x, (ui_callback_data_t)ATTACH_DEVICE_FS, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }

#define UIPERIPHERAL_SET_DEVICE_TYPE_MENU_OPENCBM(x)                                          \
    { N_("Real device access"), UI_MENU_TYPE_TICK,                                            \
      (ui_callback_t)radio_FileSystemDevice##x, (ui_callback_data_t)ATTACH_DEVICE_REAL, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }

#define UIPERIPHERAL_SET_DEVICE_TYPE_MENU_RAWDRIVE(x)                                        \
    { N_("Raw device access"), UI_MENU_TYPE_TICK,                                            \
      (ui_callback_t)radio_FileSystemDevice##x, (ui_callback_data_t)ATTACH_DEVICE_RAW, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }

ui_menu_entry_t uiperipheral_set_device8_type_submenu[] = {
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_COMMON(8),
#ifdef HAVE_OPENCBM
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_OPENCBM(8),
#endif
#ifdef HAVE_RAWDRIVE
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_RAWDRIVE(8),
#endif
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiperipheral_set_device9_type_submenu[] = {
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_COMMON(9),
#ifdef HAVE_OPENCBM
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_OPENCBM(9),
#endif
#ifdef HAVE_RAWDRIVE
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_RAWDRIVE(9),
#endif
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiperipheral_set_device10_type_submenu[] = {
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_COMMON(10),
#ifdef HAVE_OPENCBM
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_OPENCBM(10),
#endif
#ifdef HAVE_RAWDRIVE
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_RAWDRIVE(10),
#endif
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiperipheral_set_device11_type_submenu[] = {
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_COMMON(11),
#ifdef HAVE_OPENCBM
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_OPENCBM(11),
#endif
#ifdef HAVE_RAWDRIVE
    UIPERIPHERAL_SET_DEVICE_TYPE_MENU_RAWDRIVE(11),
#endif
    UI_MENU_ENTRY_LIST_END
};
