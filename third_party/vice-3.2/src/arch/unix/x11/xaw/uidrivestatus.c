/*
 * uidrivestatus.c - Xaw(3d) drive status code
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
 *  Andreas Boose <viceteam@t-online.de>
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

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/Tip.h>
#else
#include <X11/Xaw/Tip.h>
#endif

#include "drive.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "uiapi.h"
#include "uicontents.h"
#include "uidrivestatus.h"
#include "uifliplist.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "x11ui.h"

/* #define DEBUG_X11UI */

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

char last_attached_images[NUM_DRIVES][ATT_IMG_SIZE];

/* Enabled drives.  */
ui_drive_enable_t enabled_drives = UI_DRIVE_ENABLE_NONE;
static int tracks_shown;

/* Color of the drive active LED.  */
static int *drive_active_led;

static Widget left_drive_menu[NUM_DRIVES];
static Widget right_drive_menu[NUM_DRIVES];
static XtTranslations drive_menu_translations[NUM_DRIVES];

static void hide_drive_track(unsigned int drive_number);
static void ui_display_drive_current_images(void);

void ui_destroy_drive_menu(int drive)
{
    if (drive >= 0 && drive < NUM_DRIVES) {
        if (left_drive_menu[drive]) {
            ui_menu_delete(left_drive_menu[drive]);
            left_drive_menu[drive] = 0;
        }
        if (right_drive_menu[drive]) {
            ui_menu_delete(right_drive_menu[drive]);
            right_drive_menu[drive] = 0;
        }
    }
}

void ui_set_drive_menu(int drive, ui_menu_entry_t *flipmenu)
{
    char *leftmenuname;
    int i;
    Widget w;
    int num_app_shells = get_num_shells();

    if (drive < 0 || drive >= NUM_DRIVES) {
        return;
    }

    leftmenuname = lib_msprintf("leftDrive%iMenu", drive + 8);
    if (flipmenu != NULL) {
        w = ui_menu_create(leftmenuname, flipmenu, NULL);
        left_drive_menu[drive] = w;
    }

    if (!drive_menu_translations[drive]) {
        char *translation_table;
        char number[16];
        char *rightmenuname;

        sprintf(number, "%d", drive);
        rightmenuname = lib_msprintf("rightDrive%iMenu", drive + 8);

        translation_table = util_concat(
                "<Btn1Down>: "
                        "XawPositionSimpleMenu(", leftmenuname, ") "
                        "XtMenuPopup(", leftmenuname, ")\n",
                "Meta Shift <KeyDown>z: "
                        "FakeButton(1) "
                        "XawPositionSimpleMenu(", leftmenuname, ") "
                        "XtMenuPopup(", leftmenuname, ")\n",
                "<Btn3Down>: "
                        "RebuildDiskMenu(", number, ",", rightmenuname, ") "
                        "XawPositionSimpleMenu(", rightmenuname, ") "
                        "XtMenuPopup(", rightmenuname, ")\n",
                "Meta Shift <KeyDown>x: "
                        "RebuildDiskMenu(", number, ",", rightmenuname, ") "
                        "FakeButton(3) "
                        "XawPositionSimpleMenu(", rightmenuname, ") "
                        "XtMenuPopup(", rightmenuname, ")\n",
                NULL);
        drive_menu_translations[drive] =
                                XtParseTranslationTable(translation_table);
        lib_free(translation_table);
        lib_free(rightmenuname);

    }

    for (i = 0; i < num_app_shells; i++) {
        XtOverrideTranslations(app_shells[i].
                                        drive_widgets[drive].track_label,
                               drive_menu_translations[drive]);
    }

    lib_free(leftmenuname);
}

void ui_init_drive_status_widget()
{
    int i;

    DBG(("ui_init_drive_status_widget: enable=%x", enabled_drives));
    tracks_shown = 1;
    /* This is necessary because the status might have been set before we
    actually open the canvas window. e.g. by commandline */
    ui_enable_drive_status(enabled_drives, drive_active_led);
    ui_display_drive_current_images();

    /* make sure that all drive status widgets are initialized.
    This is needed for proper dual disk/dual led drives (8050, 8250). */
    for (i = 0; i < NUM_DRIVES; i++) {
        ui_display_drive_led(i, 1000, 1000);
    }
}

void ui_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color)
{
    int i, j, true_emu;
    int num_app_shells = get_num_shells();

    enabled_drives = enable;
    drive_active_led = drive_led_color;

    resources_get_int("DriveTrueEmulation", &true_emu);

    DBG(("ui_enable_drive_status: enable=%x true_emu=%d", enable, true_emu));

    if (!true_emu) {
        int prev_tracks_shown = tracks_shown;
        /*
         * Note that I'm changing the criterion used to decide whether
         * to show the UI elements for the drive. I think it is silly that
         * a drive which has no disk inserted is not shown, since the menu
         * has a handy item to insert (attach) such a disk.
         * 'enable' tends to be 0 when !true_emu.
         * Additionally, I had to change drive-resources.c to get this
         * function to be called when a drive type was changed from
         * DRIVE_TYPE_NONE to something else. (It was already called when
         * it was made DRIVE_TYPE_NONE).
         */
        for (i = 0; i < NUM_DRIVES; i++) {
            DBG(("ui_enable_drive_status: drive %d type %d", i, drive_get_disk_drive_type(i)));
            /* if (strcmp(&(last_attached_images[i][0]), "") != 0) { //} */
            if (drive_get_disk_drive_type(i) != DRIVE_TYPE_NONE) {
                enable |= 1<<i;
            }
            if (prev_tracks_shown) {
                hide_drive_track(i);
            }
        }
        enabled_drives = enable;
    }

    for (i = 0; i < num_app_shells; i++) {
        /* now show widgets ... */
        for (j = 0; j < NUM_DRIVES; j++) {
            int enabled = (enable & (1 << j));

            DBG(("app_shell %d drive %d enabled %d", i, j, enabled));

            if (enabled) {
                XtMapWidget(app_shells[i].drive_widgets[j].status);
            } else {
                XtUnmapWidget(app_shells[i].drive_widgets[j].status);
            }
            /* Show led widgets in true drive emulation mode */
            if (enabled && (true_emu > 0)) {
                app_shells[i].drive_nleds[j] = drive_num_leds(j);

                if (app_shells[i].drive_nleds[j] == 1) {
                    XtMapWidget(app_shells[i].drive_widgets[j].driveled);
                    XtUnmapWidget(app_shells[i].drive_widgets[j].driveled1);
                    XtUnmapWidget(app_shells[i].drive_widgets[j].driveled2);
                } else {
                    XtUnmapWidget(app_shells[i].drive_widgets[j].driveled);
                    XtMapWidget(app_shells[i].drive_widgets[j].driveled1);
                    XtMapWidget(app_shells[i].drive_widgets[j].driveled2);
                }
            } else {
                XtUnmapWidget(app_shells[i].drive_widgets[j].driveled);
                XtUnmapWidget(app_shells[i].drive_widgets[j].driveled1);
                XtUnmapWidget(app_shells[i].drive_widgets[j].driveled2);
            }
        }
    }
}

void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base, unsigned int half_track_number)
{
    int i;
    /* FIXME: Fixed length.  */
    char str[64];
    double track_number = (double)half_track_number / 2.0;
    int d = drive_base ? (drive_base + drive_number) : (drive_number & 1);
    int num_app_shells = get_num_shells();

    sprintf(str, _("%d: Track %.1f"), d, (double)track_number);
    for (i = 0; i < num_app_shells; i++) {
        Widget w;

        w = app_shells[i].drive_widgets[drive_number].track_label;

        if (w) {
            XtVaSetValues(w, XtNlabel, str, NULL);
        }
    }

    tracks_shown = 1;
}

static void hide_drive_track(unsigned int drive_number)
{
    int i;
    /* FIXME: Fixed length.  */
    char str[32];
    int d = 8 + drive_number;
    int num_app_shells = get_num_shells();

    sprintf(str, _("Drive %d"), d);
    for (i = 0; i < num_app_shells; i++) {
        Widget w;

        w = app_shells[i].drive_widgets[drive_number].track_label;

        if (w) {
            XtVaSetValues(w, XtNlabel, str, NULL);
        }
    }

    tracks_shown = 0;
}

void ui_display_drive_led(int drive_number, unsigned int led_pwm1, unsigned int led_pwm2)
{
    Pixel pixel;
    int status = 0;
    int i;
    int num_app_shells = get_num_shells();

    if (led_pwm1 > 100) {
        status |= 1;
    }
    if (led_pwm2 > 100) {
        status |= 2;
    }

    for (i = 0; i < num_app_shells; i++) {
        Widget w;
        Pixel on_pixel;

        on_pixel = drive_active_led[drive_number] ? drive_led_on_green_pixel
                                                  : drive_led_on_red_pixel;
        pixel = status ? on_pixel : drive_led_off_pixel;
        w = app_shells[i].drive_widgets[drive_number].driveled;
        XtVaSetValues(w, XtNbackground, pixel, NULL);

        on_pixel = (drive_active_led[drive_number] & 1) ? drive_led_on_green_pixel
                                                  : drive_led_on_red_pixel;
        pixel = (status & 1) ? on_pixel : drive_led_off_pixel;
        w = app_shells[i].drive_widgets[drive_number].driveled1;
        XtVaSetValues(w, XtNbackground, pixel, NULL);

        on_pixel = (drive_active_led[drive_number] & 2) ? drive_led_on_green_pixel
                                                  : drive_led_on_red_pixel;
        pixel = (status & 2) ? on_pixel : drive_led_off_pixel;
        w = app_shells[i].drive_widgets[drive_number].driveled2;
        XtVaSetValues(w, XtNbackground, pixel, NULL);
    }
}

void ui_display_drive_current_image(unsigned int drive_number, const char *image)
{
    char *name;
    int i;
    int num_app_shells;

    DBG(("ui_display_drive_current_image %d %s", drive_number, image));
    if (console_mode) {
        return;
    }

    if (drive_number >= NUM_DRIVES) {
        return;
    }

    strncpy(&(last_attached_images[drive_number][0]), image, ATT_IMG_SIZE);

    if (last_attached_images[drive_number][0]) {
        util_fname_split(&(last_attached_images[drive_number][0]), NULL, &name);
    } else {
        name = util_concat("<", _("empty"), ">", NULL);
    }

    num_app_shells = get_num_shells();
    for (i = 0; i < num_app_shells; i++) {
        Widget w;

        w = app_shells[i].drive_widgets[drive_number].track_label;
        XtVaSetValues(w, XtNtip, name, NULL);
    }

    lib_free(name);

    /* Also update drive menu; will call ui_set_drive_menu() */
    uifliplist_update_menus(drive_number + 8, drive_number + 8);
}

static void ui_display_drive_current_images (void)
{
    int i, j;
    char *name;
    int num_app_shells = get_num_shells();

    for (i = 0; i < num_app_shells; i++) {
        for (j = 0; j < NUM_DRIVES; j++) {
            Widget w;

            w = app_shells[i].drive_widgets[j].track_label;

            util_fname_split(&(last_attached_images[j][0]), NULL, &name);
            XtVaSetValues(w,
                            XtNtip, (name[0] ? name : _("<empty>")),
                            NULL);
            lib_free(name);

            /* Also update drive menu; will call ui_set_drive_menu() */
            if (i == 0) {
                uifliplist_update_menus(8 + j, 8 + j);
            }
        }
    }
}

void rebuild_disk_menu_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    DBG(("rebuild_disk_menu_action"));
    if (*num_params >= 2) {
        int drive;
        char *menuname;
        char *image;

        drive = atoi(params[0]);
        menuname = params[1];

        image = last_attached_images[drive];
        if (image && image[0]) {
            if (right_drive_menu[drive]) {
                ui_menu_delete(right_drive_menu[drive]);
            }
            right_drive_menu[drive] =
                rebuild_contents_menu(menuname, drive + 8, image);
        }
    }
}

