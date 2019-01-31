/*
 * ui_ted.cc - TED settings
 *
 * Written by
 *  Marcus Sutton <loggedoubt@gmail.com>
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
 
#include <Box.h>
#include <RadioButton.h>
#include <string.h>
#include <Window.h>

extern "C" { 
#include "resources.h"
#include "ui.h"
#include "ui_ted.h"
#include "ted.h"
#include "vsync.h"
}

static struct border_mode_s {
    const char *text;
    int value;
} border_mode[] = {
    { "Normal", TED_NORMAL_BORDERS },
    { "Full", TED_FULL_BORDERS },
    { "Debug", TED_DEBUG_BORDERS },
    { "None", TED_NO_BORDERS },
    { NULL, 0 }
};

class TedWindow : public BWindow {
    public:
        TedWindow();
        ~TedWindow();
        virtual void MessageReceived(BMessage *msg);
};

static TedWindow *tedwindow = NULL;

TedWindow::TedWindow() 
    : BWindow(BRect(50, 50, 190, 175), "TED settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BView *background;
    BRect r;
    BBox *box;
    BRadioButton *radiobutton;
    BMessage *msg;
    int i;
    int res_val;

    r = Bounds();
    background = new BView(r, NULL,  B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    /* border mode */
    r = Bounds();
    r.InsetBy(10, 10);
    box = new BBox(r, "Border Mode");
    box->SetLabel("Border Mode");
    background->AddChild(box);

    resources_get_int("TEDBorderMode", &res_val);
    r = box->Bounds();
    r.InsetBy(10, 20);
    r.bottom = 35;
    for (i = 0; border_mode[i].text; i++) {
        msg = new BMessage(MESSAGE_TED_BORDERS);
        msg->AddInt32("border", border_mode[i].value);
        radiobutton = new BRadioButton(r.OffsetByCopy(0, i * 20), border_mode[i].text, border_mode[i].text, msg);
        radiobutton->SetValue(res_val == border_mode[i].value);
        box->AddChild(radiobutton);
    }

    Show();
}

TedWindow::~TedWindow() 
{
    tedwindow = NULL;
}

void TedWindow::MessageReceived(BMessage *msg)
{
    int32 res_value;

    switch (msg->what) {
        case MESSAGE_TED_BORDERS:
            msg->FindInt32("border", &res_value);
            resources_set_int("TEDBorderMode", res_value);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_ted()
{
    thread_id tedthread;
    status_t exit_value;

    if (tedwindow != NULL) {
        return;
    }

    tedwindow = new TedWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    tedthread = tedwindow->Thread();
    wait_for_thread(tedthread, &exit_value);
}
