/*
 * ui_printer.cc - Printer settings
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
#include <Button.h>
#include <CheckBox.h>
#include <OptionPopUp.h>
#include <TabView.h>
#include <TextControl.h>
#include <Window.h>

#include <string.h>

extern "C" {
#include "printer.h"
#include "resources.h"
#include "ui_printer.h"
#include "vsync.h"
}

static int machine_printer_count;

static int machine_has_iec_printer;

static struct _printer_emulation {
    const char *text;
    int id;
} printer_emulation[] = {
    { "None", PRINTER_DEVICE_NONE },
    { "Filesystem", PRINTER_DEVICE_FS },
    { NULL, 0 }
};

struct _driver {
    const char *text;
    const char *value_str;
};

static struct _driver printer_driver[] = {
    { "ASCII", "ascii" },
    { "MPS803", "mps803" },
    { "NL10", "nl10" },
    { "Raw", "raw" },
    { NULL, NULL }
};

static struct _driver plotter_driver[] = {
    { "1520", "1520" },
    { "Raw", "raw" },
    { NULL, NULL }
};

static struct _driver userport_driver[] = {
    { "ASCII", "ascii" },
    { "NL10", "nl10" },
    { "Raw", "raw" },
    { NULL, NULL }
};

static struct _driver *driver_list[] = {
    printer_driver,
    printer_driver,
    plotter_driver,
    userport_driver
};

static struct _printer_output {
    const char *text;
    const char *value_str;
} printer_output[] = {
    { "Text", "text" },
    { "Graphics", "graphics" },
    { NULL, NULL }
};

class PrinterView : public BView {
    public:
        PrinterView(BRect r, int device_num);
        void EnableControls();
    private:
        BCheckBox *ieccheckbox;
        BOptionPopUp *emulation_popup;
};

void PrinterView::EnableControls()
{
    BBox *box;
    int32 children, i;
    int32 enable = 1;

    if (ieccheckbox) {
        enable = ieccheckbox->Value();
        if (!enable) {
            emulation_popup->SelectOptionFor((int32)PRINTER_DEVICE_NONE);
        }
    }

    emulation_popup->SetEnabled(enable);

    if (enable) {
        enable = (emulation_popup->Value() == PRINTER_DEVICE_FS);
    }

    ((BControl *)FindView("formfeed"))->SetEnabled(enable);

    box = ((BBox *)FindView("box"));
    children = box->CountChildren();
    for (i = 0; i < children; i++) {
        ((BControl *)box->ChildAt(i))->SetEnabled(enable);
    }

}

PrinterView::PrinterView(BRect r, int device_num) : BView(r, "printer_view", B_FOLLOW_NONE, B_WILL_DRAW)
{
    BBox *box;
    BButton *button;
    BOptionPopUp *option_popup;
    BMessage *msg;

    char printer_str[20];
    char resname[32];
    const char *current_string;
    int current_value;
    int i;

    BView::SetViewColor(220, 220, 220, 0);

    if (!machine_has_iec_printer || device_num == 7) {
        ieccheckbox = NULL;
    } else {
        sprintf(resname, "IECDevice%d", device_num);
        resources_get_int(resname, &current_value);
        msg = new BMessage(MESSAGE_PRINTER_IEC_DEVICE);
        msg->AddInt32("device_num", device_num);
        msg->AddString("resname", resname);
        ieccheckbox = new BCheckBox(BRect(15, 15, 200, 30), "iecdevice", "Use IEC Device", msg);
        ieccheckbox->SetValue(current_value);
        AddChild(ieccheckbox);
    }
    
    if (device_num != 7) {
        sprintf(printer_str, "Printer%d", device_num);
    } else {
        sprintf(printer_str, "PrinterUserport");
    }

    /* the printer emulation control */
    resources_get_int(printer_str, &current_value);
    msg = new BMessage(MESSAGE_PRINTER_EMULATION);
    msg->AddInt32("device_num", device_num);
    msg->AddString("resname", printer_str);
    emulation_popup = new BOptionPopUp(BRect(15, 40, 230, 65), "emulation", "Printer Emulation", msg);
    for (i = 0; printer_emulation[i].text != NULL; i++) {
        emulation_popup->AddOption(printer_emulation[i].text, printer_emulation[i].id);
    }
    emulation_popup->SelectOptionFor(current_value);
    AddChild(emulation_popup);

    /* the formfeed button */
    msg = new BMessage(MESSAGE_PRINTER_SEND_FF);
    msg->AddInt32("device_num", device_num);
    button = new BButton(BRect(245, 40, 400, 60), "formfeed", "Send Formfeed", msg);
    AddChild(button);

    r.InsetBy(10, 40);
    r.OffsetBy(0, 30);
    box = new BBox(r, "box");
    AddChild(box);

    /* the printer driver control */
    sprintf(resname, "%sDriver", printer_str);
    resources_get_string(resname, &current_string);
    current_value = 0;
    msg = new BMessage(MESSAGE_PRINTER_DRIVER);
    msg->AddInt32("device_num", device_num);
    msg->AddString("resname", resname);
    option_popup = new BOptionPopUp(BRect(20, 15, 220, 40), "driver", "Driver", msg);
    for (i = 0; driver_list[device_num - 4][i].text != NULL; i++) {
        option_popup->AddOption(driver_list[device_num - 4][i].text, i);
        if (!strcmp(driver_list[device_num - 4][i].value_str, current_string)) {
            current_value = i;
        }
    }
    option_popup->SelectOptionFor(current_value);
    box->AddChild(option_popup);

    /* the output type control */
    sprintf(resname, "%sOutput", printer_str);
    resources_get_string(resname, &current_string);
    current_value = 0;
    msg = new BMessage(MESSAGE_PRINTER_OUTPUT_TYPE);
    msg->AddString("resname", resname);
    option_popup = new BOptionPopUp(BRect(20, 45, 220, 70), "output", "Output Type", msg);
    for (i = 0; printer_output[i].text != NULL; i++) {
        option_popup->AddOption(printer_output[i].text, i);
        if (!strcmp(printer_output[i].value_str, current_string)) {
            current_value = i;
        }
    }
    option_popup->SelectOptionFor(current_value);
    box->AddChild(option_popup);

    /* the output device file control */
    sprintf(resname, "%sTextDevice", printer_str);
    resources_get_int(resname, &current_value);
    msg = new BMessage(MESSAGE_PRINTER_OUTPUT_DEVICE);
    msg->AddString("resname", resname);
    option_popup = new BOptionPopUp(BRect(20, 75, 220, 100), "textdev", "Output Device File", msg);
    for (i = 0; i < 3; i++) {
        /* re-use printer_str */
        sprintf(printer_str, "#%d", i + 1);
        option_popup->AddOption(printer_str, i + 1);
    }
    option_popup->SelectOptionFor(current_value);
    box->AddChild(option_popup);

    EnableControls();
}

class PrinterWindow : public BWindow {
    public:
        PrinterWindow();
        ~PrinterWindow();
        virtual void MessageReceived(BMessage *msg);
    private:
        PrinterView *pv[4]; /* pointers to the Printers (4, 5, 6, & Userport) */
};

static PrinterWindow *printerwindow = NULL;

PrinterWindow::PrinterWindow() 
    : BWindow(BRect(50, 50, 550, 270),"Printer settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
    BRect frame, r;
    BView *textdevview;
    BTabView *tabview;
    BTab *tab;
    BBox *box;
    BTextControl *textdev_control;
    BMessage *msg;
    int i;
    const char *printerfile;
    char str[24];

    frame = Bounds();
    tabview = new BTabView(frame, "tab_view");
    tabview->SetViewColor(220, 220, 220, 0);

    frame = tabview->Bounds();
    frame.InsetBy(5, 5);
    frame.OffsetTo(3, 3);
    frame.bottom -= tabview->TabHeight();

    /* the printer devices 4, 5, 6, & userport */
    r = frame;
    for (i = 0; i < machine_printer_count; i++) {
        tab = new BTab();
        tabview->AddTab(pv[i] = new PrinterView(frame, i + 4), tab);
        if (i != 3) {
            sprintf(str, "Printer %d", i + 4);
            tab->SetLabel(str);
        } else {
            tab->SetLabel("Userport Printer");
        }
    }

    tab = new BTab();
    tabview->AddTab(textdevview = new BView(r, "textdev_view", B_FOLLOW_NONE, B_WILL_DRAW), tab);
    tab->SetLabel("Output Devices");
    textdevview->SetViewColor(220,220,220,0);

    r.InsetBy(10, 10);
    box = new BBox(r);
    box->SetLabel("Output Files and Devices");
    textdevview->AddChild(box);

    for (i = 0; i < 3; i++) {
        sprintf(str, "PrinterTextDevice%d", i + 1);
        resources_get_string(str, &printerfile);
        msg = new BMessage(MESSAGE_PRINTER_DEVICE_FILE);
        msg->AddString("resname", str);

        sprintf(str, "Output File #%d", i + 1);
        textdev_control = new BTextControl(BRect(10, 30 + i * 25, 240, 40 + i * 25), str, str, printerfile, msg);
        textdev_control->SetDivider(80);
        box->AddChild(textdev_control);
    }

    AddChild(tabview);
    tabview->SetTabWidth(B_WIDTH_FROM_LABEL);
    Show();
}

PrinterWindow::~PrinterWindow() 
{
    printerwindow = NULL;
}

void PrinterWindow::MessageReceived(BMessage *msg)
{
    BTextControl *textdev_control;
    const char *resname;
    int32 res_value;
    int32 device_num;

    switch (msg->what) {
        case MESSAGE_PRINTER_EMULATION:
            msg->FindInt32("device_num", &device_num);
            msg->FindString("resname", &resname);
            msg->FindInt32("be:value", &res_value);
            resources_set_int(resname, res_value);
            pv[device_num - 4]->EnableControls();
            break;
        case MESSAGE_PRINTER_SEND_FF:
            msg->FindInt32("device_num", &device_num);
            printer_formfeed(device_num - 4);
            break;
        case MESSAGE_PRINTER_DRIVER:
            msg->FindInt32("device_num", &device_num);
            msg->FindString("resname", &resname);
            msg->FindInt32("be:value", &res_value);
            resources_set_string(resname, driver_list[device_num - 4][res_value].value_str);
            break;
        case MESSAGE_PRINTER_OUTPUT_TYPE:
            msg->FindString("resname", &resname);
            msg->FindInt32("be:value", &res_value);
            resources_set_string(resname, printer_output[res_value].value_str);
            break;
        case MESSAGE_PRINTER_OUTPUT_DEVICE:
            msg->FindString("resname", &resname);
            msg->FindInt32("be:value", &res_value);
            resources_set_int(resname, res_value);
            break;
        case MESSAGE_PRINTER_IEC_DEVICE:
            msg->FindInt32("device_num", &device_num);
            msg->FindString("resname", &resname);
            resources_toggle(resname, NULL);
            pv[device_num - 4]->EnableControls();
            break;
        case MESSAGE_PRINTER_DEVICE_FILE:
            msg->FindString("resname", &resname);
            msg->FindPointer("source", (void **)&textdev_control);
            resources_set_string(resname, textdev_control->Text());
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_printer(int caps)
{
    thread_id printerthread;
    status_t exit_value;

    if (printerwindow != NULL) {
        return;
    }

    machine_printer_count = (caps & HAS_USERPORT_PRINTER) ? 4 : 3;

    machine_has_iec_printer = (caps & HAS_IEC_BUS) ? 1 : 0;

    printerwindow = new PrinterWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    printerthread = printerwindow->Thread();
    wait_for_thread(printerthread, &exit_value);
}
