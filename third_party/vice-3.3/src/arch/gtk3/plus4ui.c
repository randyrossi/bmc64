/** \file   plus4ui.c
 * \brief   Native GTK3 PLUS4 UI
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
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

#include <stdio.h>

#include "debug_gtk3.h"
#include "cartridge.h"
#include "carthelpers.h"
#include "crtcontrolwidget.h"
#include "machine.h"
#include "machinemodelwidget.h"
#include "plus4model.h"
#include "sampler.h"
#include "ted.h"
#include "ui.h"
#include "uicart.h"
#include "uimachinewindow.h"
#include "settings_sampler.h"
#include "videomodelwidget.h"
#include "widgethelpers.h"

#include "plus4ui.h"


/** \brief  List of Plus/4 models
 *
 * Used in the machine-model widget
 */
static const char *plus4_model_list[] = {
    "C16 PAL",
    "C16 NTSC",
    "Plus4 PAL",
    "Plus4 NTSC",
    "V364 NTSC",
    "232 NTSC",
    NULL
};


/** \brief  List of TED models
 *
 * Used in the TED model widget
 */
static const vice_gtk3_radiogroup_entry_t plus4_ted_models[] = {
    { "PAL", MACHINE_SYNC_PAL },
    { "NTSC", MACHINE_SYNC_NTSC },
    { NULL, -1 }
};


/** \brief  Identify the canvas used to create a window
 *
 * \param[in]   canvas  video canvas
 *
 * \return  window index on success, -1 on failure
 */
static int identify_canvas(video_canvas_t *canvas)
{
    if (canvas != ted_get_canvas()) {
        return -1;
    }

    return PRIMARY_WINDOW;
}

/** \brief  Create CRT controls widget for \a target window
 *
 * \param[in]   target_window   target window index
 *
 * \return  GtkGrid
 */
static GtkWidget *create_crt_widget(int target_window)
{
    return crt_control_widget_create(NULL, "TED", TRUE);
}

/** \brief  Pre-initialize the UI before the canvas window gets created
 *
 * \return  0 on success, -1 on failure
 */
int plus4ui_init_early(void)
{
    ui_machine_window_init();
    ui_set_identify_canvas_func(identify_canvas);
    ui_set_create_controls_widget_func(create_crt_widget);
    return 0;
}


/** \brief  Initialize the UI
 *
 * \return  0 on success, -1 on failure
 */
int plus4ui_init(void)
{
    machine_model_widget_getter(plus4model_get);
    machine_model_widget_setter(plus4model_set);
    machine_model_widget_set_models(plus4_model_list);

    video_model_widget_set_title("TED model");
    video_model_widget_set_resource("MachineVideoStandard");
    video_model_widget_set_models(plus4_ted_models);

    settings_sampler_set_devices_getter(sampler_get_devices);

    /* uicart_set_detect_func(cartridge_detect); only cbm2/plus4 */
/*    uicart_set_list_func(cartridge_get_info_list); */
    uicart_set_attach_func(cartridge_attach_image);
/*    uicart_set_freeze_func(cartridge_trigger_freeze); */
    uicart_set_detach_func(cartridge_detach_image);
/*    uicart_set_default_func(cartridge_set_default); */
    return 0;
}


/** \brief  Shut down the UI
 */
void plus4ui_shutdown(void)
{
    /* NOP */
}
