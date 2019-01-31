/*
 * vicewindow.h - Implementation of the BeVICE's window
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

#ifndef VICE_VICEWINDOW_H
#define VICE_VICEWINDOW_H

#include <Bitmap.h>
#include <MenuBar.h>
#include <View.h>
#include <DirectWindow.h>
#include <Locker.h>

#include "statusbar.h"
#include "ui_file.h"

extern "C" {
#include "ui.h"
}

class ViceWindow : public BDirectWindow {
    public:
        ViceWindow(unsigned int width, unsigned int height, char const *title);
        ~ViceWindow();
        void Resize(unsigned int width, unsigned int height);
        void CreateBitmap(unsigned int width, unsigned int height, unsigned int depth);
        void DrawBitmap(BBitmap *framebitmap, int xs, int ys, int xi, int yi, int w, int h);
        void Update_Menu_Toggles(ui_menu_toggle *toggle_list);
        void Update_Menu_Value_Lists(ui_res_value_list *value_list);
        void Update_Menu_String_Lists(ui_res_string_list *string_list);
        virtual bool QuitRequested();
        virtual void MessageReceived(BMessage *message);
        virtual void DirectConnected(direct_buffer_info *info);
        BMenuBar *menubar;
        BBitmap *bitmap;
        BView *view;
        ViceStatusbar *statusbar;
        ViceFilePanel *filepanel;
        ViceFilePanel *savepanel;
        uint8_t *fbits;
        int32 fbytes_per_row;
        uint32 fbits_per_pixel;
        uint32 fcliplist_count;
        clipping_rect *fclip_list;
        clipping_rect fbounds;
        BLocker *locker;
        bool fconnected;
        bool fconnectiondisabled;
        int menubar_offset;
        void *canvas;
        int use_direct_window;
};

#endif
