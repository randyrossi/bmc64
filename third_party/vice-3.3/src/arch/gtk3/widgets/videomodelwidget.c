/** \file   videomodelwidget.c
 * \brief   Video chip model selection widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * FIXME:   This widget get's its resource name from another widget, so its
 *          a little more involved to figure out exacly what resources are
 *          set for which video chip.
 * Controls the following resource(s):
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

#include <gtk/gtk.h>

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "machine.h"
#include "log.h"

#include "videomodelwidget.h"


static const char *widget_title = NULL;
static const char *resource_name = NULL;
static const vice_gtk3_radiogroup_entry_t *model_list = NULL;
static GtkWidget *machine_widget = NULL;


/** \brief  Get index in model list for model-ID \a model
 *
 * \return  index in list or -1 when not found
 */
static int get_model_index(int model)
{
    return vice_gtk3_radiogroup_get_list_index(model_list, model);
}


/** \brief  Handler for the "toggled" event of the model radio buttons
 *
 * \param[in]   widget      radio button
 * \param[in]   user_data   model ID (`int`
 */
static void on_model_toggled(GtkWidget *widget, gpointer user_data)
{
    int model_id = GPOINTER_TO_INT(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        debug_gtk3("setting '%s' to %d.", resource_name, model_id);
        if (resources_set_int(resource_name, model_id) < 0) {
            log_error(LOG_ERR, "failed to set %s to %d\n",
                    resource_name, model_id);
        } else {
            GtkWidget *parent;
            void (*callback)(int);

            parent = gtk_widget_get_parent(widget);
            if (parent != NULL) {
                callback = g_object_get_data(G_OBJECT(parent), "ExtraCallback");
                if (callback != NULL) {
                    debug_gtk3("triggering extra callback with %d.", model_id);
                    callback(model_id);
                }
            }
        }
    }
}


/** \brief  Set title of the widget
 *
 * \param[in]   title   title for the widget
 */
void video_model_widget_set_title(const char *title)
{
    widget_title = title;
}


/** \brief  Set resource name of the widget
 *
 * \param[in]   resource    name of the resource
 */
void video_model_widget_set_resource(const char *resource)
{
    resource_name = resource;
}


/** \brief  Set list of valid models for the widget
 *
 * \param[in]   models  list of video models
 */
void video_model_widget_set_models(const vice_gtk3_radiogroup_entry_t *models)
{
    model_list = models;
}


/** \brief  Create widget to control video model
 *
 * The \a machine argument is used to
 *
 * \param[in,out]   machine machine model widget
 *
 * \return  GtkGrid
 */
GtkWidget *video_model_widget_create(GtkWidget *machine)
{
    GtkWidget *grid;
    GtkWidget *radio;
    GtkRadioButton *last = NULL;
    GSList *group = NULL;
    int i;

    machine_widget = machine;
    grid = uihelpers_create_grid_with_label(widget_title, 1);

    if (model_list != NULL) {
        for (i = 0; model_list[i].name != NULL; i++) {
            radio = gtk_radio_button_new_with_label(group, model_list[i].name);
            gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio), last);
            g_object_set(radio, "margin-left", 16, NULL);
            gtk_grid_attach(GTK_GRID(grid), radio, 0, i + 1, 1, 1);
            last = GTK_RADIO_BUTTON(radio);
        }

        /* now set the proper value */
        video_model_widget_update(grid);
    }

    g_object_set_data(G_OBJECT(grid), "ExtraCallback", NULL);


    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update the video model \a widget
 *
 * \param[in,out]   widget  video model widget
 */
void video_model_widget_update(GtkWidget *widget)
{
    GtkWidget *radio;
    int index;
    int model_id;
    int i = 0;

    resources_get_int(resource_name, &model_id);
    index = get_model_index(model_id);
    debug_gtk3("got resource %d, index %d.", model_id, index);

    while ((radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, i + 1)) != NULL) {
        if (GTK_IS_RADIO_BUTTON(radio) && (index == i)) {
            /* don't update radio button when it's already active, inhibiting
             * the event handler */
            if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
            }
            break;
        }
        i++;
    }
}


/** \brief  Setup signal handlers of the video model \a widget
 *
 * \param[in,out]   widget  video model widget
 */
void video_model_widget_connect_signals(GtkWidget *widget)
{
    GtkWidget *radio;
    int i = 0;

    while ((radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, i + 1)) != NULL) {
        if (GTK_IS_RADIO_BUTTON(radio)) {
            g_signal_connect(radio, "toggled", G_CALLBACK(on_model_toggled),
                    GINT_TO_POINTER(model_list[i].id));
        } else {
            break;
        }
        i++;
    }
}


/** \brief  Set extra callback to trigger when the model changes
 *
 * \param[in]   widget      the video model widget
 * \param[in]   callback    function to call on model change
 */
void video_model_widget_set_callback(GtkWidget *widget, void (*callback)(int))
{
    g_object_set_data(G_OBJECT(widget), "ExtraCallback", (gpointer)callback);
}

