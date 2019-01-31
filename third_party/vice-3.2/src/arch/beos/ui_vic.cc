/*
 * ui_vic.cc - VIC-I settings
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
#include "ui_vic.h"
#include "vic.h"
#include "vsync.h"
}

static struct border_mode_s {
    const char *text;
    int value;
} border_mode[] = {
    { "Normal", VIC_NORMAL_BORDERS },
    { "Full", VIC_FULL_BORDERS },
    { "Debug", VIC_DEBUG_BORDERS },
    { "None", VIC_NO_BORDERS },
    { NULL, 0 }
};

class VicWindow : public BWindow {
    public:
        VicWindow();
        ~VicWindow();
        virtual void MessageReceived(BMessage *msg);
};

static VicWindow *vicwindow = NULL;

VicWindow::VicWindow() 
    : BWindow(BRect(50, 50, 190, 175), "VIC settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
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

    resources_get_int("VICBorderMode", &res_val);
    r = box->Bounds();
    r.InsetBy(10, 20);
    r.bottom = 35;
    for (i = 0; border_mode[i].text; i++) {
        msg = new BMessage(MESSAGE_VIC_BORDERS);
        msg->AddInt32("border", border_mode[i].value);
        radiobutton = new BRadioButton(r.OffsetByCopy(0, i * 20), border_mode[i].text, border_mode[i].text, msg);
        radiobutton->SetValue(res_val == border_mode[i].value);
        box->AddChild(radiobutton);
    }

    Show();
}

VicWindow::~VicWindow() 
{
    vicwindow = NULL;
}

void VicWindow::MessageReceived(BMessage *msg)
{
    int32 res_value;

    switch (msg->what) {
        case MESSAGE_VIC_BORDERS:
            msg->FindInt32("border", &res_value);
            resources_set_int("VICBorderMode", res_value);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_vic()
{
    thread_id victhread;
    status_t exit_value;

    if (vicwindow != NULL) {
        return;
    }

    vicwindow = new VicWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    victhread = vicwindow->Thread();
    wait_for_thread(victhread, &exit_value);
}
