/*
 * ui_netplay.cc - Netplay settings
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
 
#include <Button.h>
#include <stdlib.h>
#include <string.h>
#include <TextControl.h>
#include <Window.h>

extern "C" {
#include "network.h"
#include "resources.h"
#include "ui.h"
#include "ui_netplay.h"
#include "vsync.h"
}

class NetplayWindow : public BWindow {
    public:
        NetplayWindow();
        ~NetplayWindow();
        virtual void MessageReceived(BMessage *msg);
    private:
        BTextControl *servertextcontrol;
        BTextControl *porttextcontrol;
};

static NetplayWindow *netplaywindow = NULL;

NetplayWindow::NetplayWindow() 
    : BWindow(BRect(50, 50, 200, 150), "Netplay settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    int port;
    char str[256];
    const char *server;
    BView *background;
    BRect r;

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    resources_get_string("NetworkServerName", &server);
    resources_get_int("NetworkServerPort", &port);
    sprintf(str, "%d", port);

    r.bottom -= r.Height() * 2 / 3;
    r.InsetBy(5, 5);

    servertextcontrol = new BTextControl(r, "server", "Server", server, NULL);
    servertextcontrol->SetDivider(50);
    background->AddChild(servertextcontrol);

    r.OffsetBy(0, r.Height() + 5);

    porttextcontrol = new BTextControl(r, "port", "Port", str, NULL);
    porttextcontrol->SetDivider(50);
    background->AddChild(porttextcontrol);

    r.OffsetBy(0, r.Height() + 5);

    background->AddChild(new BButton(r, "apply", "Apply settings", new BMessage(MESSAGE_NETPLAY_APPLY)));

    Show();
}

NetplayWindow::~NetplayWindow() 
{
    netplaywindow = NULL;	
}

void NetplayWindow::MessageReceived(BMessage *msg)
{
    switch (msg->what) {
        case MESSAGE_NETPLAY_APPLY:
            resources_set_string("NetworkServerName", servertextcontrol->Text());
            resources_set_int("NetworkServerPort", atoi(porttextcontrol->Text()));
            BWindow::Quit();
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_netplay()
{
    thread_id netplaythread;
    status_t exit_value;

    if (netplaywindow != NULL) {
        return;
    }

    netplaywindow = new NetplayWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    netplaythread=netplaywindow->Thread();
    wait_for_thread(netplaythread, &exit_value);
}
