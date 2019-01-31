/*
 * uinetwork.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifdef HAVE_NETWORK
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intl.h"
#include "network.h"
#include "uinetwork.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "vsync.h"
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

static int ui_port_range[] = {
    0,
    0xffff
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_INTEGER, "NetworkServerPort", NULL, ui_port_range, NULL },
    { NULL, MUI_TYPE_TEXT, "NetworkServerBindAddress", NULL, NULL, NULL },
    { NULL, MUI_TYPE_TEXT, "NetworkServerName", NULL, NULL, NULL },
    UI_END /* mandatory */
};

#define BTN_START_SERVER (256 + 0)
#define BTN_CONNECT_TO   (256 + 1)
#define BTN_DISCONNECT   (256 + 2)

static APTR build_gui(void)
{
    APTR app, ui, mode, start_server, connect_to, disconnect, cancel, bind_server;

    app = mui_get_app();

    ui = GroupObject,
           Child, mode = StringObject,
             MUIA_Frame, MUIV_Frame_String,
             MUIA_FrameTitle, translate_text(IDS_CURRENT_MODE),
           End,
           Child, GroupObject,
             MUIA_Frame, MUIV_Frame_Group,
             MUIA_Group_Horiz, TRUE,
             Child, ui_to_from[0].object = StringObject,
               MUIA_Frame, MUIV_Frame_String,
               MUIA_FrameTitle, translate_text(IDS_TCP_PORT),
               MUIA_String_Accept, "0123456789",
               MUIA_String_MaxLen, 5+1,
             End,
             Child, start_server = TextObject,
               ButtonFrame,
               MUIA_Background, MUII_ButtonBack,
               MUIA_Text_Contents, translate_text(IDS_START_SERVER),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode, MUIV_InputMode_RelVerify,
             End,
           End,
           Child, GroupObject,
             MUIA_Frame, MUIV_Frame_Group,
             MUIA_Group_Horiz, TRUE,
             Child, bind_server = TextObject,
               ButtonFrame,
               MUIA_Background, MUII_ButtonBack,
               MUIA_Text_Contents, translate_text(IDS_SERVER_BIND),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode, MUIV_InputMode_RelVerify,
             End,
             Child, ui_to_from[1].object = StringObject,
               MUIA_Frame, MUIV_Frame_String,
               MUIA_String_Accept, ".0123456789",
               MUIA_String_MaxLen, 15+1,
             End,
           End,
           Child, GroupObject,
             MUIA_Frame, MUIV_Frame_Group,
             MUIA_Group_Horiz, TRUE,
             Child, connect_to = TextObject,
               ButtonFrame,
               MUIA_Background, MUII_ButtonBack,
               MUIA_Text_Contents, translate_text(IDS_CONNECT_TO),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode, MUIV_InputMode_RelVerify,
             End,
             Child, ui_to_from[2].object = StringObject,
               MUIA_Frame, MUIV_Frame_String,
               MUIA_String_Accept, ".0123456789",
               MUIA_String_MaxLen, 15+1,
             End,
           End,
           Child, GroupObject,
             MUIA_Frame, MUIV_Frame_Group,
             MUIA_Group_Horiz, TRUE,
             Child, disconnect = TextObject,
               ButtonFrame,
               MUIA_Background, MUII_ButtonBack,
               MUIA_Text_Contents, translate_text(IDS_DISCONNECT),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode, MUIV_InputMode_RelVerify,
             End,
             Child, cancel = TextObject,
               ButtonFrame,
               MUIA_Background, MUII_ButtonBack,
               MUIA_Text_Contents, translate_text(IDS_CANCEL),
               MUIA_Text_PreParse, "\033c",
               MUIA_InputMode, MUIV_InputMode_RelVerify,
             End,
           End,
         End;

    if (ui != NULL) {
        DoMethod(start_server, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_START_SERVER);

        DoMethod(connect_to, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_CONNECT_TO);

        DoMethod(disconnect, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_DISCONNECT);

        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        switch(network_get_mode()) {
            case NETWORK_IDLE:
                set(mode, MUIA_String_Contents, translate_text(IDS_IDLE));
                break;
            case NETWORK_SERVER:
                set(mode, MUIA_String_Contents, translate_text(IDS_SERVER_LISTENING));
                break;
            case NETWORK_SERVER_CONNECTED:
                set(mode, MUIA_String_Contents, translate_text(IDS_CONNECTED_SERVER));
                break;
            case NETWORK_CLIENT:
                set(mode, MUIA_String_Contents, translate_text(IDS_CONNECTED_CLIENT));
                break;
        }
    }

    return ui;
}

void ui_network_dialog(void)
{
    APTR window = mui_make_simple_window(build_gui(), translate_text(IDS_NETPLAY_SETTINGS));

    /* FIXME: Bad workaround */
    resources_set_value("EventSnapshotDir", (resource_value_t)"");

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from);
        set(window, MUIA_Window_Open, TRUE);
        switch (mui_run()) {
            case BTN_START_SERVER:
                ui_get_from(ui_to_from);
                if (network_start_server() < 0) {
                    ui_error(translate_text(IDMES_ERROR_STARTING_SERVER));
                }
                break;
            case BTN_CONNECT_TO:
                ui_get_from(ui_to_from);
                if (network_connect_client() < 0) {
                    ui_error(translate_text(IDMES_ERROR_CONNECTING_CLIENT));
                }
                break;
            case BTN_DISCONNECT:
                network_disconnect();
                break;
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
#endif
