/** \file   vsidui.c
 * \brief   Native GTK3 VSID UI
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

#include "vice_gtk3.h"
#include "machine.h"
#include "ui.h"
#include "uisidattach.h"
#include "uivsidwindow.h"
#include "vicii.h"
#include "resources.h"

#include "videomodelwidget.h"
#include "vsidcontrolwidget.h"
#include "vsidtuneinfowidget.h"
#include "vsidmainwidget.h"

#include "hvsc.h"

#include "vsidui.h"


static const vice_gtk3_radiogroup_entry_t vsid_vicii_models[] = {
    { "PAL",            MACHINE_SYNC_PAL },
    { "NTSC",           MACHINE_SYNC_NTSC },
    { "NTSC (old)",     MACHINE_SYNC_NTSCOLD },
    { "PAL-N/Drean",    MACHINE_SYNC_PALN },
    { NULL,             -1 }
};


void vsid_ui_close(void)
{
    hvsc_exit();
    uisidattach_shutdown();
}


/** \brief  Display tune author in the UI
 *
 * \param[in]   author  author name
 */
void vsid_ui_display_author(const char *author)
{
    vsid_tune_info_widget_set_author(author);
}


/** \brief  Display tune copyright info in the UI
 *
 * \param[in]   copright    copyright info
 */
void vsid_ui_display_copyright(const char *copyright)
{
    vsid_tune_info_widget_set_copyright(copyright);
}


/** \brief  Set IRQ type for the UI
 *
 * \param[in]   irq IRQ type
 */
void vsid_ui_display_irqtype(const char *irq)
{
    vsid_tune_info_widget_set_irq(irq);
}


/** \brief  Display tune name in the UI
 *
 * \param[in]   name    tune name
 */
void vsid_ui_display_name(const char *name)
{
    vsid_tune_info_widget_set_name(name);
}


/** \brief  Set number of tunes for the UI
 *
 * \param[in]   count   number of tunes
 */
void vsid_ui_display_nr_of_tunes(int count)
{
    vsid_main_widget_set_tune_count(count);
}


/** \brief  Set SID model for the UI
 *
 * \param[in]   model   SID model
 */
void vsid_ui_display_sid_model(int model)
{
    vsid_tune_info_widget_set_model(model);
}


/** \brief  Set sync factor for the UI
 *
 * \param[in]   sync    sync factor
 */
void vsid_ui_display_sync(int sync)
{
    vsid_tune_info_widget_set_sync(sync);
}


/** \brief  Set run time of tune in the UI
 *
 * \param[in]   sec seconds of play time
 */
void vsid_ui_display_time(unsigned int sec)
{
    vsid_tune_info_widget_set_time(sec);
}


/** \brief  Set current tune number in the UI
 *
 * \param[in]   nr  tune number
 */
void vsid_ui_display_tune_nr(int nr)
{
    vsid_main_widget_set_tune_current(nr);
}


/** \brief  Set driver info text for the UI
 *
 * \param[in]   driver_info_text    text with driver info (duh)
 */
void vsid_ui_setdrv(char *driver_info_text)
{
    vsid_tune_info_widget_set_driver(driver_info_text);
}


/** \brief  Set default tune number in the UI
 *
 * \param[in]   nr  tune number
 */
void vsid_ui_set_default_tune(int nr)
{
    vsid_main_widget_set_tune_default(nr);
}


/** \brief  Set driver address
 *
 * \param[in]   addr    driver address
 */
void vsid_ui_set_driver_addr(uint16_t addr)
{
    vsid_tune_info_widget_set_driver_addr(addr);
}


/** \brief  Set load address
 *
 * \param[in]   addr    load address
 */
void vsid_ui_set_load_addr(uint16_t addr)
{
    vsid_tune_info_widget_set_load_addr(addr);
}


/** \brief  Set init routine address
 *
 * \param[in]   addr    init routine address
 */
void vsid_ui_set_init_addr(uint16_t addr)
{
    vsid_tune_info_widget_set_init_addr(addr);
}


/** \brief  Set play routine address
 *
 * \param[in]   addr    play routine address
 */
void vsid_ui_set_play_addr(uint16_t addr)
{
    vsid_tune_info_widget_set_play_addr(addr);
}


/** \brief  Set size of SID on actual machine
 *
 * \param[in]   size    size of SID
 */
void vsid_ui_set_data_size(uint16_t size)
{
    vsid_tune_info_widget_set_data_size(size);
}



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


/** \brief  Initialize the VSID UI
 *
 * \return  0 on success, -1 on failure
 */
int vsid_ui_init(void)
{
    video_canvas_t *canvas = vicii_get_canvas();

    video_model_widget_set_title("VIC-II model");
    video_model_widget_set_resource("MachineVideoStandard");
    video_model_widget_set_models(vsid_vicii_models);


    ui_vsid_window_init();
    ui_set_identify_canvas_func(identify_canvas);

    ui_create_main_window(canvas);
    ui_display_main_window(canvas->window_index);

    /* for debugging */
    debug_gtk3("libhvsc version: %s.", hvsc_lib_version_str());
    return 0;
}
