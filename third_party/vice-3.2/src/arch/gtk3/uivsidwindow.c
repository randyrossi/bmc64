/** \file   uivsidwindow.c
 * \brief   Native GTK3 main vsid window code
 *
 * \author  Marcus Sutton <loggedoubt@gmail.com>
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

/* XXX: It should be possible to compile, link and run all emulators while this
 *      entire file (amongst others) is contained inside an #if 0 wrapper.
 */
#if 1

#include "vice.h"

#include <gtk/gtk.h>

#include "videoarch.h"

#include "vice_gtk3.h"
#include "ui.h"
#include "uivsidmenu.h"
#include "vsidmainwidget.h"

#include "uivsidwindow.h"


/** \brief  Main widget of VSID
 *
 * This should contain play/stop/rewind etc controls, and data on the currently
 * loaded SID. A proper playlist and Songlength.[txt|md5] support wouldn't
 * hurt either.
 */
static GtkWidget *main_widget = NULL;


/** \brief  Create  VSID window
 *
 * \param[in]   canvas  something
 */
static void vsid_window_create(video_canvas_t *canvas)
{
    GtkWidget *menu_bar;

    canvas->renderer_backend = NULL;
    canvas->drawing_area = NULL;

    main_widget = vsid_main_widget_create();
    gtk_widget_set_size_request(main_widget, 400, 300);
    gtk_widget_set_hexpand(main_widget, TRUE);
    gtk_widget_set_vexpand(main_widget, TRUE);
    gtk_widget_show(main_widget);

    menu_bar = ui_vsid_menu_bar_create();
    gtk_container_add(GTK_CONTAINER(canvas->grid), menu_bar);
    gtk_container_add(GTK_CONTAINER(canvas->grid), main_widget);
}


/** \brief  Initialize VSID window
 */
void ui_vsid_window_init(void)
{
    ui_set_create_window_func(vsid_window_create);
}

#endif
