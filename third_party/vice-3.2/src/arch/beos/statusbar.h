/*
 * statusbar.h - Implementation of the BeVICE statusbar.
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

#ifndef VICE_STATUSBAR_H
#define VICE_STATUSBAR_H

#include <View.h>

class BBitmap;

class ViceStatusbar : public BView {
    public:
        ViceStatusbar(BRect r);
        ~ViceStatusbar();
        void Draw(BRect rect);
        void DisplaySpeed(float percent, float framerate, int warp_flag);
        void DisplayDriveStatus(int drive_num,int drive_led_color,double drive_track);
        void DisplayTapeStatus(int enabled, int counter, int motor, int control);
        void DisplayImage(int drive_num, const char *image);
        void EnableJoyport(int onvoid);
        void DisplayJoyport(int port_num, int status);
        void DisplayMessage(const char *text);
    private:
        void OffsetPoints(const BPoint *in_array, int count, BPoint *out_array);
        BBitmap *statusbitmap;
        BView *drawview;
        BRect speed_rect;
        BRect status_rect;
        BRect led_rect;
        BRect image_rect;
        float str_x, str2_x;
};
#endif
