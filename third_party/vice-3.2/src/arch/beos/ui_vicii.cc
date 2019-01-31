/*
 * ui_vicii.cc - VIC-II settings
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
 
#include <Box.h>
#include <CheckBox.h>
#include <RadioButton.h>
#include <string.h>
#include <Window.h>

extern "C" { 
#include "machine.h"
#include "resources.h"
#include "ui.h"
#include "ui_vicii.h"
#include "vicii.h"
#include "vsync.h"
}

static struct border_mode_s {
    const char *text;
    int value;
} border_mode[] = {
    { "Normal", VICII_NORMAL_BORDERS },
    { "Full", VICII_FULL_BORDERS },
    { "Debug", VICII_DEBUG_BORDERS },
    { "None", VICII_NO_BORDERS },
    { NULL, 0 }
};

class ViciiWindow : public BWindow {
    public:
        ViciiWindow();
        ~ViciiWindow();
        virtual void MessageReceived(BMessage *msg);
};

static ViciiWindow *viciiwindow = NULL;

ViciiWindow::ViciiWindow() 
    : BWindow(BRect(50, 50, 350, 175), "VIC-II settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BView *background;
    BRect r;
    BBox *box;
    BCheckBox *checkbox;
    BRadioButton *radiobutton;
    BMessage *msg;
    int i;
    int res_val;

    r = Bounds();
    background = new BView(r, NULL,  B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    /* sprite collisions */
    box = new BBox(BRect(10, 10, 150, 75), "Sprite Collision");
    box->SetLabel("Sprite Collision");
    background->AddChild(box);

    checkbox = new BCheckBox(BRect(10, 20, 130, 35), NULL, "Sprite-Sprite", new BMessage(MESSAGE_VICII_SSCOLL));
    resources_get_int("VICIICheckSsColl", &res_val);
    checkbox->SetValue(res_val);
    box->AddChild(checkbox);

    checkbox = new BCheckBox(BRect(10, 40, 130, 55), NULL, "Sprite-Background", new BMessage(MESSAGE_VICII_SBCOLL));
    resources_get_int("VICIICheckSbColl", &res_val);
    checkbox->SetValue(res_val);
    box->AddChild(checkbox);

    /* new colors */
    if (machine_class == VICE_MACHINE_C64DTV) {
        checkbox = new BCheckBox(BRect(20, 80, 140, 95), NULL, "New Luminances", new BMessage(MESSAGE_VICII_NEWLUMINANCE));
        resources_get_int("VICIINewLuminances", &res_val);
        checkbox->SetValue(res_val);
        background->AddChild(checkbox);
    }

    /* VSP bug */
    if (machine_class == VICE_MACHINE_C64SC) {
        checkbox = new BCheckBox(BRect(20, 100, 140, 95), NULL, "VSP bug", new BMessage(MESSAGE_VICII_VSPBUG));
        resources_get_int("VICIIVSPBug", &res_val);
        checkbox->SetValue(res_val);
        background->AddChild(checkbox);
    }

    /* border mode */
    r = Bounds();
    r.left = r.Width() / 2;
    r.InsetBy(10, 10);
    box = new BBox(r, "Border Mode");
    box->SetLabel("Border Mode");
    background->AddChild(box);

    resources_get_int("VICIIBorderMode", &res_val);
    r = box->Bounds();
    r.InsetBy(10, 20);
    r.bottom = 35;
    for (i = 0; border_mode[i].text; i++) {
        msg = new BMessage(MESSAGE_VICII_BORDERS);
        msg->AddInt32("border", border_mode[i].value);
        radiobutton = new BRadioButton(r.OffsetByCopy(0, i * 20), border_mode[i].text, border_mode[i].text, msg);
        radiobutton->SetValue(res_val == border_mode[i].value);
        box->AddChild(radiobutton);
    }

    Show();
}

ViciiWindow::~ViciiWindow() 
{
    viciiwindow = NULL;
}

void ViciiWindow::MessageReceived(BMessage *msg)
{
    int32 res_value;

    switch (msg->what) {
        case MESSAGE_VICII_SSCOLL:
            resources_toggle("VICIICheckSsColl", (int *)&res_value);
            break;
        case MESSAGE_VICII_SBCOLL:
            resources_toggle("VICIICheckSbColl", (int *)&res_value);
            break;
        case MESSAGE_VICII_NEWLUMINANCE:
            resources_toggle("VICIINewLuminances", (int *)&res_value);
            break;
        case MESSAGE_VICII_VSPBUG:
            resources_toggle("VICIIVSPBug", (int *)&res_value);
            break;
        case MESSAGE_VICII_BORDERS:
            msg->FindInt32("border", &res_value);
            resources_set_int("VICIIBorderMode", res_value);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_vicii()
{
    thread_id viciithread;
    status_t exit_value;

    if (viciiwindow != NULL) {
        return;
    }

    viciiwindow = new ViciiWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    viciithread = viciiwindow->Thread();
    wait_for_thread(viciithread, &exit_value);
}
