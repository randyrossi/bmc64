/*
 * vicewindow.cc - Implementation of the BeVICE's window
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

#include "vice.h"

#include <Bitmap.h>
#include <DirectWindow.h>
#include <FilePanel.h>
#include <Locker.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <View.h>
#include <string.h>

#include "statusbar.h"
#include "ui_file.h"
#include "vicemenu.h"
#include "vicewindow.h"

extern "C" {
#include "constants.h"
#include "log.h"
#include "mousedrv.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "ui.h"
#include "util.h"
#include "video.h"
#include "videoarch.h"
}

/* #define DEBUG_UI */

#ifdef DEBUG_UI
void print_rect(const char *view, BRect r)
{
    log_debug("%s (Width: %f, Height: %f) (Top: %f, Bottom: %f)", view, r.Width(), r.Height(), r.top, r.bottom);
}
#define DBG_RECT(_x_) print_rect _x_
#define DBG_MSG(_x_) log_debug _x_
#else
#define DBG_RECT(_x_)
#define DBG_MSG(_x_)
#endif

/* FIXME: some stuff we need from the ui module */
extern ViceWindow *windowlist[];
extern int window_count;

void ViceWindow::Update_Menu_Toggles(ui_menu_toggle *toggle_list)
{
    int i, value;
    BMenuItem *item;

    if (!toggle_list) {
        return;
    }

    for (i = 0; toggle_list[i].name != NULL; i++) {
        resources_get_int(toggle_list[i].name, &value);
        if (item = menubar->FindItem(toggle_list[i].item_id)) {
            item->SetMarked(value ? true : false);
        }
    }
}

void ViceWindow::Update_Menu_Value_Lists(ui_res_value_list *value_list)
{
    int i, j;
    int value, result;
    BMenuItem *item;

    if (!value_list) {
        return;
    }

    for (i = 0; value_list[i].name != NULL; i++) {
        result = resources_get_int(value_list[i].name, &value);
        if (result == 0) {
            for (j = 0; value_list[i].vals[j].item_id != 0; j++) {
                if (value == value_list[i].vals[j].value) {
                    /* the corresponding menu is supposed to be in RadioMode */
                    if (item = menubar->FindItem(value_list[i].vals[j].item_id)) {
                        item->SetMarked(true);
                    }
                }
            }
        }
    }
}

void ViceWindow::Update_Menu_String_Lists(ui_res_string_list *string_list)
{
    int i, j;
    int result;
    const char *str;
    BMenuItem *item;

    if (!string_list) {
        return;
    }

    for (i = 0; string_list[i].name != NULL; i++) {
        result = resources_get_string(string_list[i].name, &str);
        if (result == 0) {
            for (j = 0; string_list[i].strings[j].item_id != 0; j++) {
                if (!strcasecmp(str, string_list[i].strings[j].string)) {
                    /* the corresponding menu is supposed to be in RadioMode */
                    if (item = menubar->FindItem(string_list[i].strings[j].item_id)) {
                        item->SetMarked(true);
                    }
                }
            }
        }
    }
}

/* the view for the emulators bitmap */
class ViceView : public BView {
    public:
        ViceView(BRect rect);
        virtual void Draw(BRect rect);
        virtual void MouseDown(BPoint point);
        virtual void MouseUp(BPoint point);
};

ViceView::ViceView(BRect rect) : BView(rect, "view", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
}

void ViceView::Draw(BRect rect)
{
    ViceWindow *wnd = (ViceWindow *)Window();

    if (wnd->bitmap && !wnd->use_direct_window) {
        DrawBitmap(wnd->bitmap, rect, rect);
    }
}

/* some hooks for the 1351 mouse emulation */
void ViceView::MouseDown(BPoint point)
{
    BMessage *msg;
    int32 buttons;

    if (!_mouse_enabled) {
        return;
    }

    msg = Window()->CurrentMessage();
    msg->FindInt32("buttons", &buttons);
    if (buttons & B_PRIMARY_MOUSE_BUTTON) {
        mousedrv_button_left(1);
    }
}

void ViceView::MouseUp(BPoint point)
{
    if (!_mouse_enabled) {
        return;
    }

    mousedrv_button_left(0);
}

ViceWindow::ViceWindow(unsigned int width, unsigned int height, char const *title) 
    : BDirectWindow(BRect(0, 0, 300, 100), title, B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS)
{
    BRect r;

    /* create the menubar; key events reserved for the emu */
    menubar = menu_create(machine_class, window_count);
    AddChild(menubar);
    DBG_RECT(("menubar", menubar->Frame()));
    menubar_offset = (int)menubar->Frame().Height() + 1;
    SetKeyMenuBar(NULL);

    /* create the File Panel */
    filepanel = new ViceFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false);

    /* create the Save Panel */
    savepanel = new ViceFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false);

    /* the view for the canvas */
    r = Bounds();
    r.top = menubar_offset;
    view = new ViceView(r);
    AddChild(view);
    DBG_RECT(("view", view->Frame()));

    /* bitmap is NULL; will be created by video_canvas_resize() */
    bitmap = NULL;

    /* statusbar is NULL; will be created in Resize() */
    statusbar = NULL;

    /* the canvas is set by video_canvas_create */
    canvas = NULL;

    /* register the window */
    windowlist[window_count++] = this;

    /* stuff for direct drawing */
    fconnected = false;
    fconnectiondisabled = false;
    locker = new BLocker();
    fclip_list = NULL;
    fcliplist_count = 0;

    /* use the resource to initialize stuff */
    resources_get_int("DirectWindow", &use_direct_window);
    if (!SupportsWindowMode() || CheckForHaiku()) {
        use_direct_window = 0;
    }
    resources_set_int("DirectWindow", use_direct_window);

    /* finally display the window */
    if (width > 0 && height > 0) {
        Resize(width, height);
    }
    MoveTo(window_count * 30, window_count * 30);
    Show();
}

ViceWindow::~ViceWindow()
{
    BView *vsid = FindView("vsid");

    DBG_MSG(("destroying window %s\n", Title()));
    fconnectiondisabled = true;
    Hide();
    Sync();

    if (bitmap) {
        delete bitmap;
    }
    if (vsid) {
        RemoveChild(vsid);
        delete vsid;
    }
    RemoveChild(menubar);
    delete menubar;
    RemoveChild(view);
    delete view;
    RemoveChild(statusbar);
    delete statusbar;
    delete filepanel;
    delete savepanel;
    delete locker;
    lib_free(fclip_list);
    fclip_list = NULL;
}

bool ViceWindow::QuitRequested()
{
    /* send an exit request to ui's event loop 
       but dont't close the window here
     */
    BMessage msg;
    msg.what = MENU_EXIT_REQUESTED;
    ui_add_event(&msg);
    return false;
}

void ViceWindow::MessageReceived(BMessage *message)
{
    /* FIXME: sometimes the menubar holds the focus so we have to delete it */ 
    if (CurrentFocus()) {
        CurrentFocus()->MakeFocus(false);
    }

    ui_add_event(message);
    switch(message->what) {
        default:
            BWindow::MessageReceived(message);
            break;
    }
}

void ViceWindow::Resize(unsigned int width, unsigned int height)
{
    BRect statusbar_frame;

    if (BWindow::Lock()) {
        view->ResizeTo(width - 1, height - 1);
        DBG_RECT(("view after resize", view->Frame()));

        if (statusbar) {
            RemoveChild(statusbar);
            delete statusbar;
            statusbar = NULL;
        }
        statusbar_frame.top = view->Frame().bottom + 1;
        statusbar_frame.bottom = view->Frame().bottom + 67;
        statusbar_frame.left = 0;
        statusbar_frame.right = view->Frame().right;
        statusbar = new ViceStatusbar(statusbar_frame);
        AddChild(statusbar);
        DBG_RECT(("statusbar", statusbar->Frame()));

        ui_statusbar_update();
        DBG_MSG(("statusbar_frame.bottom = %f\n", statusbar_frame.bottom));

        BWindow::ResizeTo(width - 1, statusbar_frame.bottom);
        BWindow::Unlock();
    }
}

void ViceWindow::CreateBitmap(unsigned int width, unsigned int height, unsigned int depth)
{
    color_space use_colorspace;

    if (bitmap) {
        delete bitmap;
        bitmap = NULL;
    }

    switch (depth) {
        case 8:
            use_colorspace = B_CMAP8;
            break;
        case 16:
            use_colorspace = B_RGB16;
            break;
        case 32:
        default:
            use_colorspace = B_RGB32;
    }

    bitmap = new BBitmap(BRect(0, 0, width - 1, height - 1), use_colorspace, false, true);
}

void ViceWindow::DrawBitmap(BBitmap *bitmap, int xs, int ys, int xi, int yi, int w, int h)
{
    if (BWindow::Lock()) {
        view->DrawBitmap(bitmap, BRect(xs, ys, xs + w, ys + h), BRect(xi, yi, xi + w, yi + h));
        BWindow::Unlock();
    }
}

void ViceWindow::DirectConnected(direct_buffer_info *info)
{
    bool isdirty = false;

    if (!fconnected && fconnectiondisabled) {
        return;
    }

    locker->Lock();

    switch (info->buffer_state & B_DIRECT_MODE_MASK) {
        case B_DIRECT_START:
            fconnected = true;
        case B_DIRECT_MODIFY:
            lib_free(fclip_list);
            fclip_list = NULL;
            fcliplist_count = info->clip_list_count;
            fclip_list = (clipping_rect *)lib_malloc(fcliplist_count * sizeof(clipping_rect));
            if (fclip_list) {
                memcpy(fclip_list, info->clip_list, fcliplist_count * sizeof(clipping_rect));
            }
            fbits = (uint8_t *)info->bits;
            fbytes_per_row = info->bytes_per_row;
            fbits_per_pixel = info->bits_per_pixel;
            fbounds = info->window_bounds;
            isdirty = true;
            break;
        case B_DIRECT_STOP:
            fconnected = false;
            break;
    }

    locker->Unlock();

    if (isdirty && use_direct_window && canvas != NULL) {
        video_canvas_refresh_all((struct video_canvas_s *)canvas);
    }
}
