/*
 * ui_cbm5x0.cc - CBM5x0 settings
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
#include <Button.h>
#include <CheckBox.h>
#include <RadioButton.h>
#include <string.h>
#include <Window.h>

extern "C" {
#include "cbm2mem.h"
#include "resources.h"
#include "ui.h"
#include "ui_cbm5x0.h"
#include "vsync.h"
}

static const char *cbm5x0_modelline[] = { "60Hz", "50Hz", NULL };
static int cbm5x0_memory[] = { 64, 128, 256, 512, 1024, 0 };
static const char *rambank_res[] = { "Ram08", "Ram1", "Ram2", "Ram4", "Ram6", "RamC", NULL };
static const char *rambank_text[] = {
    "Bank 15 $0800-$0fff RAM",
    "Bank 15 $1000-$1fff RAM",
    "Bank 15 $2000-$3fff RAM",
    "Bank 15 $4000-$5fff RAM",
    "Bank 15 $6000-$7fff RAM",
    "Bank 15 $c000-$cfff RAM",
    NULL
};

class Cbm5x0Window : public BWindow {
    public:
        Cbm5x0Window();
        ~Cbm5x0Window();
        virtual void MessageReceived(BMessage *msg);
};

static Cbm5x0Window *cbm5x0window = NULL;

Cbm5x0Window::Cbm5x0Window()
    : BWindow(BRect(50, 50, 370, 290), "CBM5x0 settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
    BView *background;
    BRect r;
    BBox *box;
    BButton *button;
    BRadioButton *radiobutton;
    BCheckBox *checkbox;
    BMessage *msg;
    int res, i;
    char str[16];

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    /* memory */
    r = BRect(10, 10, 120, 150);
    box = new BBox(r, "Memory");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Memory");
    background->AddChild(box);

    resources_get_int("RamSize", &res);
    for (i = 0; cbm5x0_memory[i]; i++) {
        msg = new BMessage(MESSAGE_CBM5X0_MEMORY);
        msg->AddInt32("memory", i);
        sprintf(str, "%d KByte", cbm5x0_memory[i]);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, 100, 25 + i * 25), str, str, msg);
        radiobutton->SetValue(res == cbm5x0_memory[i]);
        box->AddChild(radiobutton);
    }


    /* model line */
    r = BRect(10, 160, 120, 225);
    box = new BBox(r, "Model line");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Model line");
    background->AddChild(box);

    resources_get_int("ModelLine", &res);
    for (i = 0; cbm5x0_modelline[i]; i++) {
        msg = new BMessage(MESSAGE_CBM5X0_MODELLINE);
        msg->AddInt32("modelline", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, 100, 25 + i * 25), cbm5x0_modelline[i], cbm5x0_modelline[i], msg);
        radiobutton->SetValue(res == i);
        box->AddChild(radiobutton);
    }

    /* ram banks */
    r = BRect(130, 10, 310, 175);
    box = new BBox(r, "RAM Banks");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("RAM Banks");
    background->AddChild(box);

    for (i = 0; rambank_text[i]; i++) {
        msg = new BMessage(MESSAGE_CBM5X0_RAMBANK);
        msg->AddInt32("rambank", i);
        checkbox = new BCheckBox(BRect(10, 15 + i * 25, 170, 25 + i * 25), rambank_text[i], rambank_text[i], msg);
        resources_get_int(rambank_res[i], &res);
        checkbox->SetValue(res);
        box->AddChild(checkbox);
    }

    Show();
}

Cbm5x0Window::~Cbm5x0Window()
{
    cbm5x0window = NULL;
}

void Cbm5x0Window::MessageReceived(BMessage *msg)
{
    int32 res_value;
    int32 dummy;

    switch (msg->what) {
        case MESSAGE_CBM5X0_MODELLINE:
            msg->FindInt32("modelline", &res_value);
            resources_set_int("ModelLine", res_value);
            break;
        case MESSAGE_CBM5X0_MEMORY:
            msg->FindInt32("memory", &res_value);
            resources_set_int("RamSize", cbm5x0_memory[res_value]);
            break;
        case MESSAGE_CBM5X0_RAMBANK:
            msg->FindInt32("rambank", &res_value);
            resources_toggle(rambank_res[res_value], (int *)&dummy);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_cbm5x0() {
    thread_id cbm5x0thread;
    status_t exit_value;

    if (cbm5x0window != NULL) {
        return;
    }

    cbm5x0window = new Cbm5x0Window;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    cbm5x0thread = cbm5x0window->Thread();
    wait_for_thread(cbm5x0thread, &exit_value);
}
