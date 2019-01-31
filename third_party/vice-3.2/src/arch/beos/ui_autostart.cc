/*
 * ui_autostart.cc - Autostart settings
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
#include <stdlib.h>
#include <string.h>
#include <TextControl.h>
#include <Window.h>

extern "C" {
#include "resources.h"
#include "ui.h"
#include "ui_autostart.h"
#include "vsync.h"
}

class AutostartWindow : public BWindow {
    public:
        AutostartWindow();
        ~AutostartWindow();
        virtual void MessageReceived(BMessage *msg);
    private:
        BTextControl *delaycontrol;
};


static AutostartWindow *autostart_window = NULL;

AutostartWindow::AutostartWindow()
    : BWindow(BRect(50, 50, 250, 120), "Autostart delay",
            B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
            B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
    int delay;
    char str[256];
    BView *background;
    BRect rect;

    rect = Bounds();
    background = new BView(rect, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    resources_get_int("AutostartDelay", &delay);
    sprintf(str, "%d", delay);

    rect.bottom -= rect.Height() * 2 / 3;
    rect.InsetBy(5, 5);

    delaycontrol = new BTextControl(rect, "delay", "Delay (seconds):", str, NULL);
    delaycontrol->SetDivider(120);
    background->AddChild(delaycontrol);

    rect.OffsetBy(0, rect.Height() + 15);

    background->AddChild(new BButton(rect, "apply", "Apply",
                new BMessage(MESSAGE_AUTOSTART_DELAY_APPLY)));

    Show();
}

AutostartWindow::~AutostartWindow()
{
    autostart_window = NULL;
}

void AutostartWindow::MessageReceived(BMessage *msg)
{
    int delay;

    switch (msg->what) {
        case MESSAGE_AUTOSTART_DELAY_APPLY:
            delay = (int)strtol(delaycontrol->Text(), NULL, 10);
            if (delay < 0) {
                delay = 0;
            } else if (delay > 1000) {
                delay = 1000;
            }
            resources_set_int("AutostartDelay", delay);
            BWindow::Quit();
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}


void ui_autostart()
{
    thread_id autostart_thread;
    status_t exit_value;

    if (autostart_window != NULL) {
        return;
    }

    autostart_window = new AutostartWindow;

    vsync_suspend_speed_eval();

    autostart_thread = autostart_window->Thread();
    wait_for_thread(autostart_thread, &exit_value);
}

