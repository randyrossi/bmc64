/*
 * statusbar.cc - Implementation of the BeVICE statusbar.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include <Bitmap.h>
#include <Font.h>
#include <stdio.h>
#include <String.h>

#include "statusbar.h"

extern "C" {
#include "datasette.h"
#include "drive.h"
#include "log.h"
#include "ui.h"
}

/* #define DEBUG_UI */

#ifdef DEBUG_UI
void print_rect(const char *view, BRect r)
{
    log_debug("%s (Width: %.1f, Height: %.1f) (Left: %.1f, Right: %.1f)", view, r.Width(), r.Height(), r.left, r.right);
}
#define DBG_RECT(_x_) print_rect _x_
#define DBG_MSG(_x_) log_debug _x_
#else
#define DBG_RECT(_x_)
#define DBG_MSG(_x_)
#endif

const rgb_color statusbar_background = { 200, 200, 200, 0 };
const rgb_color statusbar_green_led = { 10, 200, 10, 0 };
const rgb_color statusbar_red_led = { 200, 10, 10, 0 };
const rgb_color statusbar_black_led = { 5, 5, 5, 0 };
const rgb_color statusbar_motor_on = { 250, 250, 0, 0 };
const rgb_color statusbar_motor_off = { 120, 120, 120, 0 };
const rgb_color statusbar_joystick_dir = { 10, 250, 10, 0};
const rgb_color statusbar_joystick_fire = { 250, 10, 10, 0};
const rgb_color statusbar_joystick_off = { 100, 100, 100, 0 };

ViceStatusbar::ViceStatusbar(BRect r) 
    : BView(r,"statusbar", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
    BFont font;
    float strwidth;

    /* set the maximum font size */
    GetFont(&font);
    if (font.Size() > 11.0) {
        font.SetSize(11.0);
        SetFont(&font, B_FONT_SIZE);
    }

    /* setup some coordinates and rectangles */
    strwidth = StringWidth("Speed: 100% at 100fps (warp)");
    str_x = strwidth + 15;
    strwidth = StringWidth("10:");
    str2_x = str_x + strwidth + 2;
    strwidth = StringWidth("88.8");

    speed_rect = BRect(1, 1, str_x - 10, 20);
    DBG_RECT(("speed_rect", speed_rect));
    status_rect = BRect(str_x - 5, 1, str2_x + strwidth + 25, 13);
    DBG_RECT(("status_rect", status_rect));
    led_rect = BRect(status_rect.right - 20, 3, status_rect.right - 10, 9);
    DBG_RECT(("led_rect", led_rect));
    image_rect = BRect(status_rect.right + 5, 1, Bounds().Width() - 5, 13);
    DBG_RECT(("image_rect", image_rect));
    DBG_MSG(("\n"));

    BView::SetViewColor(statusbar_background);
    r.OffsetTo(0, 0);
    statusbitmap = new BBitmap(r, B_CMAP8, true, true);
    drawview = new BView(r, "drawview", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
    statusbitmap->AddChild(drawview);
    statusbitmap->Lock();
    drawview->SetFont(&font, B_FONT_SIZE);
    drawview->SetLowColor(statusbar_background);
    drawview->FillRect(r, B_SOLID_LOW);
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(r);
}

ViceStatusbar::~ViceStatusbar()
{
    statusbitmap->RemoveChild(drawview);
    delete drawview;
    delete statusbitmap;
} 

void ViceStatusbar::DisplaySpeed(float percent, float framerate, int warp_flag)
{
    char str[40];

    if (percent > 9999) {
        percent = 9999;
    }

    /* take care of encoding of pause mode */
    if (warp_flag == -1) {
        sprintf(str, "(Paused)");
    } else if (warp_flag == -2) {
        sprintf(str, "(Resumed)");
    } else {
        sprintf(str, "Speed: %.0f%% at %.0ffps %s", percent, framerate, (warp_flag? "(warp)":"      "));
    }
    statusbitmap->Lock();
    drawview->SetLowColor(statusbar_background);
    drawview->FillRect(speed_rect, B_SOLID_LOW);
    drawview->DrawString(str, BPoint(5, 10));
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(speed_rect);
}

void ViceStatusbar::DisplayDriveStatus(int drive_num, int drive_led_color, double drive_track)
{
    char str[20], str2[20];
    bool erase_bar = false;
    BRect frame;
    float drive_offset;

    if (drive_num < 0) {
        erase_bar = true;
        drive_num = -drive_num - 1;
    }

    drive_offset = drive_num * 13;
    frame = status_rect.OffsetByCopy(0, drive_offset);
    sprintf(str, "%2d:", drive_num + 8);
    sprintf(str2, "%.1f", drive_track);
    statusbitmap->Lock();
    drawview->SetLowColor(statusbar_background);
    drawview->FillRect(frame, B_SOLID_LOW);
    if (!erase_bar) {
        drawview->DrawString(str, BPoint(str_x, 10 + drive_offset));
        drawview->DrawString(str2, BPoint(str2_x, 10 + drive_offset));

        /* this is a bit of a hack. */
        if (drive_led_color > 0) {
            drawview->SetLowColor(statusbar_green_led);
        } else if (drive_led_color == 0) {
            drawview->SetLowColor(statusbar_red_led);
        } else {
            drawview->SetLowColor(statusbar_black_led);
        }

        drawview->FillRect(led_rect.OffsetByCopy(0, drive_offset), B_SOLID_LOW);
    }
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(frame);
}

void ViceStatusbar::OffsetPoints(const BPoint *in_array, int count, BPoint *out_array)
{
    int i;

    for (i = 0; i < count; i++) {
        out_array[i].x = in_array[i].x + led_rect.left;
        out_array[i].y = in_array[i].y + 52;
    }
}

void ViceStatusbar::DisplayTapeStatus(int enabled, int counter, int motor, int control)
{
    char str[20];
    BRect frame, motor_rect;
    BPoint points[3];
    const BPoint play_button[] = { BPoint(3, 3), BPoint(6, 6), BPoint(3, 9) };
    const BPoint ff_button1[] = { BPoint(2, 3), BPoint(5, 6), BPoint(2, 9) };
    const BPoint ff_button2[] = { BPoint(5, 3), BPoint(8, 6), BPoint(5, 9) };
    const BPoint rewind_button1[] = { BPoint(8, 3), BPoint(6, 6), BPoint(8, 9) };
    const BPoint rewind_button2[] = { BPoint(5, 3), BPoint(2, 6), BPoint(5, 9) };

    frame = status_rect.OffsetByCopy(0, 52);
    sprintf(str, " T: %03d", counter % 1000);
    statusbitmap->Lock();
    drawview->SetLowColor(statusbar_background);
    drawview->FillRect(frame, B_SOLID_LOW);
    if (enabled) {
        drawview->DrawString(str, BPoint(str_x, 62));
        if (motor) {
            drawview->SetLowColor(statusbar_motor_on);
        } else {
            drawview->SetLowColor(statusbar_motor_off);
        }
        motor_rect = BRect(led_rect.left, 53, led_rect.right, 63);
        drawview->FillRect(motor_rect, B_SOLID_LOW);
        switch (control) {
            case DATASETTE_CONTROL_STOP:
                drawview->FillRect(motor_rect.InsetByCopy(2, 2), B_SOLID_HIGH);
                break;
            case DATASETTE_CONTROL_RECORD:
                drawview->SetLowColor(statusbar_red_led);
                motor_rect = BRect(led_rect.right + 3, 56, led_rect.right + 7, 60);
                drawview->FillEllipse(motor_rect, B_SOLID_LOW);
            case DATASETTE_CONTROL_START:
                OffsetPoints(play_button, 3, points);
                drawview->FillPolygon(points, 3);
                break;
            case DATASETTE_CONTROL_REWIND:
                OffsetPoints(rewind_button1, 3, points);
                drawview->StrokePolygon(points, 3, false);
                OffsetPoints(rewind_button2, 3, points);
                drawview->StrokePolygon(points, 3, false);
                break;
            case DATASETTE_CONTROL_FORWARD:
                OffsetPoints(ff_button1, 3, points);
                drawview->StrokePolygon(points, 3, false);
                OffsetPoints(ff_button2, 3, points);
                drawview->StrokePolygon(points, 3, false);
                break;
        }

    }
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(frame);
}

void ViceStatusbar::DisplayImage(int drive_num, const char *image)
{
    BRect frame;
    BString img_bstr;

    if (drive_num < 0) {
        drive_num = 4; /* tape image */
    }

    frame = image_rect.OffsetByCopy(0, drive_num * 13);
    statusbitmap->Lock();
    drawview->SetLowColor(statusbar_background);
    drawview->FillRect(frame, B_SOLID_LOW);
    if (image) {
        img_bstr.SetTo(image);
        TruncateString(&img_bstr, B_TRUNCATE_MIDDLE, frame.Width());
        drawview->DrawString(img_bstr.String(), BPoint(frame.left, 10 + drive_num * 13));
    }
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(frame);
}

static struct _xy { int x; int y; } direction_offset[] = 
{
    { 6, 1 },   /* up    */
    { 6, 11 },  /* down  */
    { 1, 6 },   /* left  */
    { 11, 6 },  /* right */
    { 6, 6 }    /* fire  */
};

static BRect joystickframe = BRect(10, 20, 130, 39);

void ViceStatusbar::EnableJoyport(int on)
{
    statusbitmap->Lock();
    drawview->SetLowColor(statusbar_background);
    drawview->FillRect(joystickframe, B_SOLID_LOW);
    if (on) {
        drawview->DrawString("Port A", BPoint(10, 30));
        drawview->DrawString("Port B", BPoint(80, 30));
    }
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(joystickframe);
}

void ViceStatusbar::DisplayJoyport(int port_num, int status)
{
    int dir_index;
    BRect frame, led_template, led;

    frame = BRect(45, 20, 79, 39);
    frame.OffsetBy((port_num - 1) * 70, 0);
    led_template = frame;
    led_template.right = led_template.left + 2;
    led_template.bottom = led_template.top + 2;

    statusbitmap->Lock();
    for (dir_index = 0; dir_index < 5; dir_index++) {
        if (status & (1 << dir_index)) {
            drawview->SetLowColor((dir_index == 4) ? statusbar_joystick_fire:statusbar_joystick_dir);
        } else {
            drawview->SetLowColor(statusbar_joystick_off);
        }
        led = led_template;
        led.OffsetBy(direction_offset[dir_index].x, direction_offset[dir_index].y);
        drawview->FillRect(led, B_SOLID_LOW);
    }
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(frame);
}

void ViceStatusbar::DisplayMessage(const char *text)
{
    BRect frame = BRect(5, 53, 150, 65);

    statusbitmap->Lock();
    drawview->SetLowColor(statusbar_background);
    drawview->FillRect(frame, B_SOLID_LOW);
    drawview->DrawString(text, BPoint(5, 62));
    drawview->Sync();
    statusbitmap->Unlock();
    Draw(frame);
}

void ViceStatusbar::Draw(BRect rect)
{
    DrawBitmap(statusbitmap, rect, rect);
}
