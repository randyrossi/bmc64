/** \file   c64dtvui.c
 * \brief   Native GTK3 C64DTV UI
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

#include "c64dtvmodel.h"
#include "crtcontrolwidget.h"
#include "machine.h"
#include "machinemodelwidget.h"
#include "not_implemented.h"
#include "ui.h"
#include "uimachinewindow.h"
#include "settings_sampler.h"
#include "vicii.h"
#include "videomodelwidget.h"

#include "c64ui.h"


/** \brief  List of DTV models
 *
 * Used in the machine-model widget
 */
static const char *c64dtv_model_list[] = {
    "V2 PAL",
    "V2 NTSC",
    "V3 PAL",
    "V3 NTSC",
    "Hummer (NTSC)",
    NULL
};


/** \brief  VIC-II models
 *
 * Used in the VIC-II models widget
 */
static const vice_gtk3_radiogroup_entry_t c64dtv_vicii_models[] = {
     { "PAL-G", MACHINE_SYNC_PAL },
     { "NTSC-M", MACHINE_SYNC_NTSC },
     { NULL, -1 }
};


/** \brief  Identify the canvas used to create a window
 *
 * \return  window index on success, -1 on failure
 */
static int identify_canvas(video_canvas_t *canvas)
{
    if (canvas != vicii_get_canvas()) {
        return -1;
    }

    return PRIMARY_WINDOW;
}

/** \brief  Create CRT controls widget for \a target window
 *
 * \return  GtkGrid
 */
static GtkWidget *create_crt_widget(int target_window)
{
    return crt_control_widget_create(NULL, "VICII");
}

/** \brief  Pre-initialize the UI before the canvas window gets created
 *
 * \return  0 on success, -1 on failure
 */
int c64dtvui_init_early(void)
{
    ui_machine_window_init();
    ui_set_identify_canvas_func(identify_canvas);
    ui_set_create_controls_widget_func(create_crt_widget);

    INCOMPLETE_IMPLEMENTATION();
    return 0;
}


/** \brief  Initialize the UI
 *
 * \return  0 on success, -1 on failure
 */
int c64dtvui_init(void)
{
    machine_model_widget_getter(dtvmodel_get);
    machine_model_widget_setter(dtvmodel_set);
    machine_model_widget_set_models(c64dtv_model_list);

    video_model_widget_set_title("VIC-II model");
    video_model_widget_set_resource("MachineVideoStandard");
    video_model_widget_set_models(c64dtv_vicii_models);

    settings_sampler_set_devices_getter(sampler_get_devices);

    INCOMPLETE_IMPLEMENTATION();
    return 0;
}


/** \brief  Shut down the UI
 */
void c64dtvui_shutdown(void)
{
    INCOMPLETE_IMPLEMENTATION();
}
