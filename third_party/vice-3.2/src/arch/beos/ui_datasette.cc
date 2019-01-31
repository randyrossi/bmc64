/*
 * ui_datasette.cc - Datasette settings
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
 
#include <Box.h>
#include <CheckBox.h>
#include <RadioButton.h>
#include <string.h>
#include <stdlib.h>
#include <TextControl.h>
#include <Window.h>

extern "C" { 
#include "resources.h"
#include "ui.h"
#include "ui_datasette.h"
#include "vsync.h"
}

static int ui_datasette_zero_gap_delay_count = 7;
static int ui_datasette_zero_gap_delay[]={
    1000,
    2000,
    5000,
    10000,
    20000,
    50000,
    100000
};


class DatasetteWindow : public BWindow {
    public:
        DatasetteWindow();
        ~DatasetteWindow();
        virtual void MessageReceived(BMessage *msg);
    private:
        BTextControl *wobbletextcontrol;
};

static DatasetteWindow *datasettewindow = NULL;

DatasetteWindow::DatasetteWindow() 
    : BWindow(BRect(50, 50, 350, 270), "Datasette settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BView *background;
    BRect r;
    BBox *box;
    BMessage *msg;
    BRadioButton *radiobutton;
    BCheckBox *checkbox;
    char str[128];
    int i;
    int res_value;
    int wobble;

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);


    /* Reset with CPU */
    resources_get_int("DatasetteResetWithCPU", &res_value);
    checkbox = new BCheckBox(BRect(145, 10, 295, 20), "RESET", "Reset Datasette with CPU", new BMessage(MESSAGE_DATASETTE_RESET));
    checkbox->SetValue(res_value);
    background->AddChild(checkbox);

    /* Tape Wobble */
    resources_get_int("DatasetteTapeWobble", &wobble);
    sprintf(str, "%d", wobble);
    wobbletextcontrol = new BTextControl(BRect(145, 30, 295, 20), "Tape wobble", "Tape wobble", str, new BMessage(MESSAGE_DATASETTE_WOBBLE));
    wobbletextcontrol->SetDivider(60);
    background->AddChild(wobbletextcontrol);

    /* Speed Tuning */
    r = Bounds();
    r.right = r.left + r.Width() / 2;
    r.InsetBy(10, 5);
    box = new BBox(r, "Motor Tuning");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Motor Tuning");

    resources_get_int("DatasetteSpeedTuning", &res_value);
	
    for (i = 0; i < 8; i++) {
        msg = new BMessage(MESSAGE_DATASETTE_SPEED);
        msg->AddInt32("speed", i);
        sprintf(str, "%d cycles delay", i);
        radiobutton = new BRadioButton(BRect(10, 20 + 20 * i, r.Width() - 10, 35 + 20 * i), str, str, msg);
        radiobutton->SetValue(res_value == i);
        box->AddChild(radiobutton);
    }
    background->AddChild(box);

    /* Zero Delay */
    r = Bounds();
    r.left = r.right - r.Width() / 2;
    r.InsetBy(10, 5);
    r.top += 45;
    box = new BBox(r, "Delay for Zero");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Delay for Zero");

    resources_get_int("DatasetteZeroGapDelay", &res_value);

    for (i = 0; i < ui_datasette_zero_gap_delay_count; i++) {
        msg = new BMessage(MESSAGE_DATASETTE_ZEROGAP);
        msg->AddInt32("zerodelay", ui_datasette_zero_gap_delay[i]);
        sprintf(str, "%d cycles", ui_datasette_zero_gap_delay[i]);
        radiobutton = new BRadioButton(BRect(10, 20 + 20 * i, r. Width() - 10, 35 + 20 * i), str, str, msg);
        radiobutton->SetValue(res_value == ui_datasette_zero_gap_delay[i]);
        box->AddChild(radiobutton);
    }
    background->AddChild(box);

    Show();
}

DatasetteWindow::~DatasetteWindow() 
{
    datasettewindow = NULL;	
}

void DatasetteWindow::MessageReceived(BMessage *msg)
{
    int32 res_value;
    int temp;

    switch (msg->what) {
        case MESSAGE_DATASETTE_RESET:
            resources_toggle("DatasetteResetWithCPU", (int *)&res_value);
            break;
        case MESSAGE_DATASETTE_SPEED:
            msg->FindInt32("speed", &res_value);
            resources_set_int("DatasetteSpeedTuning", res_value);
            break;
        case MESSAGE_DATASETTE_ZEROGAP:
            msg->FindInt32("zerodelay", &res_value);
            resources_set_int("DatasetteZeroGapDelay", res_value);
            break;
        case MESSAGE_DATASETTE_WOBBLE:
            temp = atoi(wobbletextcontrol->Text());
            resources_set_int("DatasetteTapeWobble", temp);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_datasette() {
    thread_id datasettethread;
    status_t exit_value;
	
    if (datasettewindow != NULL) {
        return;
    }

    datasettewindow = new DatasetteWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    datasettethread = datasettewindow->Thread();
    wait_for_thread(datasettethread, &exit_value);
}
