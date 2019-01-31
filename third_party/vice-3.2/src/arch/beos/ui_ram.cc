/*
 * ui_ram.cc - RAM settings
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
#include <ListItem.h>
#include <ListView.h>
#include <RadioButton.h>
#include <ScrollView.h>
#include <string.h>
#include <Window.h>
#include <CheckBox.h>

extern "C" {
#include "resources.h"
#include "ui.h"
#include "ui_ram.h"
#include "vsync.h"
}

static int ui_ram_startvalue[] = { 0, 255, -1 };
static int ui_ram_invertvalue[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, -1 };

BListView *valueinvertlistview;
BListView *patterninvertlistview;

class RamWindow : public BWindow {
    public:
        RamWindow();
        ~RamWindow();
        virtual void MessageReceived(BMessage *msg);
};

static RamWindow *ramwindow = NULL;

RamWindow::RamWindow() 
    : BWindow(BRect(50, 50, 170, 310), "RAM Init", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BView *background;
    BRect r;
    BBox *box;
    BMessage *msg;
    BRadioButton *radiobutton;
    BListItem *item;
    char str[128];
    int i;
    int start_value;
    int orig_valueinvert;
    int orig_patterninvert;

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    /* Start value */
    resources_get_int("RAMInitStartValue", &start_value);
    r = Bounds();
    r.InsetBy(10, 5);
    r.bottom = r.top + 40;
    r.right = r.left + 100;
    box = new BBox(r, "Start Value");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Start Value");
    background->AddChild(box);

    for (i = 0; ui_ram_startvalue[i] >= 0; i++) {
        msg = new BMessage(MESSAGE_RAM_STARTVALUE);
        msg->AddInt32("value", i);
        sprintf(str, "%02x", ui_ram_startvalue[i]);
        radiobutton = new BRadioButton(BRect(10 + 50 * i, 15, 40 + 50 * i, 35), str, str, msg);
        radiobutton->SetValue(start_value == ui_ram_startvalue[i]);
        box->AddChild(radiobutton);
    }

    /* invert lists*/
    resources_get_int("RAMInitValueInvert", &orig_valueinvert);
    resources_get_int("RAMInitPatternInvert",  &orig_patterninvert);

    r = Bounds();
    r.InsetBy(10, 10);
    r.top += 50;
    r.right = r.left + 100;
    box = new BBox(r, "Invert");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Invert at");
    background->AddChild(box);

    valueinvertlistview = new BListView(BRect(5, 20, 40, 180), "Value Invert");
    valueinvertlistview->SetSelectionMessage(new BMessage(MESSAGE_RAM_VALUEINVERT));
    box->AddChild(valueinvertlistview);

    patterninvertlistview = new BListView(BRect(55, 20, 95, 180), "Pattern Invert");
    patterninvertlistview->SetSelectionMessage(new BMessage(MESSAGE_RAM_PATTERNINVERT));
    box->AddChild(patterninvertlistview);

    for (i = 0; ui_ram_invertvalue[i] >= 0; i++) {
        sprintf(str, "%d", ui_ram_invertvalue[i]);
        valueinvertlistview->AddItem(item = new BStringItem(str));
        if (ui_ram_invertvalue[i] == orig_valueinvert) {
            valueinvertlistview->Select(valueinvertlistview->IndexOf(item));
        }
        patterninvertlistview->AddItem(item = new BStringItem(str));
        if (ui_ram_invertvalue[i] == orig_patterninvert) {
            patterninvertlistview->Select(patterninvertlistview->IndexOf(item));
        }
    }

    Show();
}

RamWindow::~RamWindow() 
{
    ramwindow = NULL;	
}

void RamWindow::MessageReceived(BMessage *msg)
{
    int32 res_value;
    int i;

    switch (msg->what) {
        case MESSAGE_RAM_STARTVALUE:
            msg->FindInt32("value", &res_value);
            resources_set_int("RAMInitStartValue", ui_ram_startvalue[res_value]);
            break;
        case MESSAGE_RAM_VALUEINVERT:
            i = valueinvertlistview->CurrentSelection();
            if (i >= 0) {
                resources_set_int("RAMInitValueInvert", ui_ram_invertvalue[i]);
            }
            break;
        case MESSAGE_RAM_PATTERNINVERT:
            i = patterninvertlistview->CurrentSelection();
            if (i >= 0) {
                resources_set_int("RAMInitPatternInvert", ui_ram_invertvalue[i]);
            }
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_ram()
{
    thread_id ramthread;
    status_t exit_value;

    if (ramwindow != NULL) {
        return;
    }

    ramwindow = new RamWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    ramthread=ramwindow->Thread();
    wait_for_thread(ramthread, &exit_value);
}
