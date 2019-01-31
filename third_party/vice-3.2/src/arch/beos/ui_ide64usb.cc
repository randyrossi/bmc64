/*
 * ui_ide64usb.cc - IDE64 USB server settings
 *
 * Written by
 *   Bas Wassink <b.wassink@ziggo.nl>
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
#include <CheckBox.h>
#include <stdlib.h>
#include <string.h>
#include <TextControl.h>
#include <Window.h>

extern "C" {
#include "resources.h"
#include "ui.h"
#include "ui_ide64usb.h"
#include "vsync.h"
}

class IDE64USBWindow : public BWindow {
    public:
        IDE64USBWindow();
        ~IDE64USBWindow();
        virtual void MessageReceived(BMessage *msg);
    private:
        BCheckBox *enabled;
        BTextControl *address;
};


static IDE64USBWindow *ide64usb_window = NULL;

IDE64USBWindow::IDE64USBWindow()
    : BWindow(BRect(50, 50, 320, 150), "IDE64 USB server",
            B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
            B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
    const char *addr_res;
    int enabled_res;
    
    BView *background;
    BRect rect;
    BMessage *msg;

    rect = Bounds();
    background = new BView(rect, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    resources_get_int("IDE64USBServer", &enabled_res);
    resources_get_string("IDE64USBServerAddress", &addr_res);

    rect.InsetBy(5, 5);

    enabled = new BCheckBox(BRect(10, 10, 250, 30), "enabled",
            "USB server enabled", new BMessage());
    enabled->SetValue(enabled_res);
    background->AddChild(enabled);

    address = new BTextControl(BRect(10, 40, 250, 30), "address", "Address:", addr_res, NULL);
    address->SetDivider(60);
    background->AddChild(address);

    background->AddChild(new BButton(BRect(100, 70, 250, 30), "apply",
                "Apply settings", new BMessage(MESSAGE_IDE64_USB_APPLY)));

    Show();
}


IDE64USBWindow::~IDE64USBWindow()
{
    ide64usb_window = NULL;
}

void IDE64USBWindow::MessageReceived(BMessage *msg)
{
    int server_enabled = 0;

    switch (msg->what) {
        case MESSAGE_IDE64_USB_APPLY:
            server_enabled = enabled->Value() ? 1 : 0;
            resources_set_int("IDE64USBServer", server_enabled);
            resources_set_string("IDE64USBServerAddress", address->Text());
            BWindow::Quit();
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}


// execute the dialog
void ui_ide64usb()
{
    thread_id ide64usb_thread;
    status_t exit_value;

    if (ide64usb_window != NULL) {
        return;
    }

    ide64usb_window = new IDE64USBWindow;

    vsync_suspend_speed_eval();

    ide64usb_thread = ide64usb_window->Thread();
    wait_for_thread(ide64usb_thread, &exit_value);
}

