/*
 * ui_cia.cc - CIA settings
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
#include "ui_cia.h"
#include "vsync.h"
}

static const char *cia_model_name[] = {
    "6526 (old)",
    "8521 (new)",
    NULL
};

class CiaView : public BView {
    public:
        CiaView(BRect r, int cia_num);
};

class CiaWindow : public BWindow {
    public:
        CiaWindow(int num_cias);
        ~CiaWindow();
        virtual void MessageReceived(BMessage *msg);
};

static CiaWindow *ciawindow = NULL;

CiaView::CiaView(BRect r, int cia_num) : BView(r, "cia_view", B_FOLLOW_NONE, B_WILL_DRAW)
{
    BBox *box;
    BRadioButton *radiobutton;
    BMessage *msg;
    char str[16];
    int i;
    int res_val;

    SetViewColor(220, 220, 220, 0);

    r = Bounds();
    // r.InsetBy(10, 10);
    sprintf(str, "CIA %d Model", cia_num);
    box = new BBox(r, str);
    box->SetLabel(str);
    AddChild(box);

    sprintf(str, "CIA%dModel", cia_num);
    resources_get_int(str, &res_val);
    r = box->Bounds();
    r.InsetBy(10, 20);
    r.bottom = 35;
    for (i = 0; cia_model_name[i]; i++) {
        msg = new BMessage(MESSAGE_CIA_MODEL);
        msg->AddString("resname", str);
        msg->AddInt32("model", i);
        radiobutton = new BRadioButton(r.OffsetByCopy(0, i * 20), cia_model_name[i], cia_model_name[i], msg);
        radiobutton->SetValue(res_val == i);
        box->AddChild(radiobutton);
    }
}

CiaWindow::CiaWindow(int num_cias) 
    : BWindow(BRect(50, 50, 180, 60 + num_cias * 80), "CIA settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BView *background;
    BRect r;
    int i;

    r = Bounds();
    background = new BView(r, NULL,  B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    for (i = 0; i < num_cias; i++) {
        r = Bounds();
        r.InsetBy(10, 10);
        r.bottom = 80;
        r.OffsetBy(0, i * 80);
        background->AddChild(new CiaView(r, i + 1));
    }

    Show();
}

CiaWindow::~CiaWindow() 
{
    ciawindow = NULL;
}

void CiaWindow::MessageReceived(BMessage *msg)
{
    const char *resname;
    int32 res_value;

    switch (msg->what) {
        case MESSAGE_CIA_MODEL:
            msg->FindString("resname", &resname);
            msg->FindInt32("model", &res_value);
            resources_set_int(resname, res_value);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_cia(int num_cias)
{
    thread_id ciathread;
    status_t exit_value;

    if (ciawindow != NULL) {
        return;
    }

    ciawindow = new CiaWindow(num_cias);

    vsync_suspend_speed_eval();

    /* wait until window closed */
    ciathread = ciawindow->Thread();
    wait_for_thread(ciathread, &exit_value);
}
