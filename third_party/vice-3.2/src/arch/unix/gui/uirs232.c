/*
 * uirs232.c - Implementation of RS232 UI settings.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
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

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "resources.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

#include "uirs232.h"


UI_MENU_DEFINE_RADIO(RsDevice1Baud)
UI_MENU_DEFINE_RADIO(RsDevice2Baud)

#define SER_C64C128_BAUD_MENU(x)                                                \
ui_menu_entry_t ser##x##_c64c128_baud_submenu[] = {                             \
    { "300", UI_MENU_TYPE_TICK,                                                 \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)300, NULL,    \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { "1200", UI_MENU_TYPE_TICK,                                                \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)1200, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { "2400", UI_MENU_TYPE_TICK,                                                \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)2400, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { "9600", UI_MENU_TYPE_TICK,                                                \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)9600, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { "19200", UI_MENU_TYPE_TICK,                                               \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)19200, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { N_("38400 (Swiftlink/Turbo232 only)"), UI_MENU_TYPE_TICK,                 \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)38400, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { N_("57600 (Turbo232 only)"), UI_MENU_TYPE_TICK,                           \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)57600, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    { N_("115200 (Turbo232 only)"), UI_MENU_TYPE_TICK,                          \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)115200, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                \
    UI_MENU_ENTRY_LIST_END                                                      \
}

#define SER_BAUD_MENU(x)                                                       \
ui_menu_entry_t ser##x##_baud_submenu[] = {                                    \
    { "300", UI_MENU_TYPE_TICK,                                                \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)300, NULL,   \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                               \
    { "1200", UI_MENU_TYPE_TICK,                                               \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)1200, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                               \
    { "2400", UI_MENU_TYPE_TICK,                                               \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)2400, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                               \
    { "9600", UI_MENU_TYPE_TICK,                                               \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)9600, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                               \
    { "19200", UI_MENU_TYPE_TICK,                                              \
      (ui_callback_t)radio_RsDevice##x##Baud, (ui_callback_data_t)19200, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                               \
    UI_MENU_ENTRY_LIST_END                                                     \
}

SER_C64C128_BAUD_MENU(1);
SER_BAUD_MENU(1);
SER_C64C128_BAUD_MENU(2);
SER_BAUD_MENU(2);


UI_CALLBACK(set_rs232_device_file)
{
    uilib_select_dev((char *)UI_MENU_CB_PARAM, _("Select RS232 device file"), UILIB_FILTER_SERIAL);
}


UI_CALLBACK(set_rs232_exec_file)
{
    char *command_text = util_concat(_("Command"), ":", NULL);

    uilib_select_string((char *)UI_MENU_CB_PARAM, _("Command to execute for RS232 (preceed with '|')"), command_text);
    lib_free(command_text);
}


UI_CALLBACK(set_rs232_dump_file)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("File to dump RS232 to"), UILIB_FILTER_ALL);
}
#endif
