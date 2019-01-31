/** \file   keysetdialog.c
 * \brief   Widget to set joystick keyset mappings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES KeySet1NorthWest    -vsid
 * $VICERES KeySet1North        -vsid
 * $VICERES KeySet1NorthEast    -vsid
 * $VICERES KeySet1West         -vsid
 * $VICERES KeySet1Fire         -vsid
 * $VICERES KeySet1East         -vsid
 * $VICERES KeySet1SouthWest    -vsid
 * $VICERES KeySet1South        -vsid
 * $VICERES KeySet1SouthEast    -vsid
 * $VICERES KeySet2NorthWest    -vsid
 * $VICERES KeySet2North        -vsid
 * $VICERES KeySet2NorthEast    -vsid
 * $VICERES KeySet2West         -vsid
 * $VICERES KeySet2Fire         -vsid
 * $VICERES KeySet2East         -vsid
 * $VICERES KeySet2SouthWest    -vsid
 * $VICERES KeySet2South        -vsid
 * $VICERES KeySet2SouthEast    -vsid
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

#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "log.h"

#include "keysetdialog.h"

/*
 * Forward declarations
 */

static void set_button_text(GtkWidget *button, int row, int col);



/** \brief  Keyset index
 *
 * Either 1 or 2
 */
static int keyset_index = 0;


/** \brief  GDK key codes for the current keyset
 */
static guint keyset_codes[3][3];


/** \brief  Names of the directions of the keyset keys
 *
 * Do NOT change these, as they are used to construct resource names.
 * Capitalization can be changed if required, since resources are not
 * case-sensitive.
 */
static const char *keyset_labels[3][3] = {
    { "NorthWest", "North", "NorthEast" },
    { "West",      "Fire",       "East" },
    { "SouthWest", "South", "SouthEast" }
};


/** \brief  Matrix of keyset buttons
 *
 * There can be only one (active)
 */
static GtkWidget *keyset_buttons[3][3] = { NULL };


/** \brief  Handler for the "toggled" event of the keyset buttons
 *
 * \param[in,out]   button  gtk toggle button
 * \param[in]       data    extra event data (unused)
 */
static void on_button_toggled(GtkWidget *button, gpointer data)
{
    /* only respond if button is active, otherwise we'll and up in a loop */
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
        /* only one button is allowed to be active, so make all other buttons
         * inactive (untoggled) */
        int row;
        int col;
        for (row = 0; row < 3; row++) {
            for (col = 0; col < 3; col++) {
                if (keyset_buttons[row][col] != button) {
                    gtk_toggle_button_set_active(
                            GTK_TOGGLE_BUTTON(keyset_buttons[row][col]), FALSE);
                }
            }
        }
    }
}


/** \brief  Handler for the "key-press-event" event of the dialog
 *
 * Updates a keyset key. Use Escape to unset a key.
 *
 * \param[in]   widget  dialog
 * \param[in]   event   key event
 * \param[in]   data    extra event data (unused)
 *
 * \return  bool
 */
static gboolean on_key_pressed(GtkWidget *widget, GdkEventKey *event,
        gpointer data)
{
    int row;
    int col;
    guint key = event->keyval;

    /* don't accept Alt keys */
    if (key == GDK_KEY_Alt_L || key == GDK_KEY_Alt_R) {
        return FALSE;
    }

    /* unmap key when Escape is pressed */
    if (key == GDK_KEY_Escape) {
        key = 0;
    }

    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            GtkWidget *button = keyset_buttons[row][col];
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
                keyset_codes[row][col] = key;
                /* update button text and deacivate it */
                set_button_text(button, row, col);
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
                return TRUE;
            }
        }
    }
    return FALSE;
}


/** \brief  Retrieve current key codes from resources
 *
 * \return  bool
 */
static gboolean get_keyset_resources(void)
{
    int row;
    int col;

    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            int value;
            if (resources_get_int_sprintf("KeySet%d%s", &value, keyset_index,
                        keyset_labels[row][col]) < 0) {
                log_error(LOG_ERR,
                        "failed to retrieve value for resource 'KeySet%d%s\n",
                        keyset_index, keyset_labels[row][col]);
                return FALSE;
            }
            keyset_codes[row][col] = (guint)value;
        }
    }
    return TRUE;
}


/** \brief  Update resources with current keycodes
 *
 * \return  bool
 */
static gboolean set_keyset_resources(void)
{
    int row;
    int col;

    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            int value = (int)keyset_codes[row][col];
            if (resources_set_int_sprintf("KeySet%d%s", value, keyset_index,
                        keyset_labels[row][col]) < 0) {
                log_error(LOG_ERR,
                        "failed to set value for resource 'KeySet%d%s\n",
                        keyset_index, keyset_labels[row][col]);
                return FALSE;
            }
        }
    }
    return TRUE;
}


/** \brief  Update label of \a button at \a row, \a col
 *
 * Set label of \a button to "{direction}\n<b>{GDK key code}</b>".
 *
 * \param[in,out]   button  gtk toggle button
 * \param[in]       row     row in the keycodes matrix
 * \param[in]       col     column in the keycodes matrix
 */
static void set_button_text(GtkWidget *button, int row, int col)
{
    GtkWidget *label;
    gchar text[256];
    gchar *name;

    name = gdk_keyval_name(keyset_codes[row][col]);
    label = gtk_bin_get_child(GTK_BIN(button));
    g_snprintf(text, 256, "%s\n\n<b>%s</b>",
            keyset_labels[row][col], name != NULL ? name : "None");
    gtk_label_set_markup(GTK_LABEL(label), text);
}


/** \brief  Create a toggle button with two rows of text
 *
 * Create a toggle button with to rows of text, a direction, and the key used
 * for that direction in bold. For example: "NorthWest\n<b>KP_9".
 *
 * \param[in]   row row in the keys matrix
 * \param[in]   col column in the keys matrix
 *
 * \return  GtkToggleButton
 */
static GtkWidget *create_button(int row, int col)
{
    GtkWidget *button;
    GtkWidget *label;
    button = gtk_toggle_button_new_with_label("foo");
    label = gtk_bin_get_child(GTK_BIN(button));
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    set_button_text(button, row, col);

    g_signal_connect(button, "toggled", G_CALLBACK(on_button_toggled), NULL);
    return button;
}


/** \brief  Create content widget for the dialog
 *
 * \return  GtkGrid
 */
static GtkWidget *create_content_widget(void)
{
    GtkWidget *grid;
#if 0
    GtkWidget *label;
#endif
    int row;
    int col;

    /* setup grid with all buttons the same size */
    grid = vice_gtk3_grid_new_spaced(16, 16);
    g_object_set(G_OBJECT(grid),
            "margin-left", 16, "margin-right", 16,
            "margin-top", 16, "margin-bottom", 16,
            NULL);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);

    /* add buttons for each direction */
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            GtkWidget *button = create_button(row, col);
            keyset_buttons[row][col] = button;
            gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
        }
    }

    /* this doesn't wrap but makes the dialog super wide */
#if 0
    /* add some help text */
    label = gtk_label_new(NULL);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_markup(GTK_LABEL(label),
            "Click a direction button and press a key to use that key for that"
            " direction. Use <b>Escape</b> to remove a key from a direction."
            " To cancel inputting a key, simple press the same direction button"
            " again without pressing a key.");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 3, 1);
#endif
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Show dialog to configure keysets
 *
 * \param[in]   parent  parent window (the settings UI dialog)
 * \param[in]   keyset  keyset number (1 or 2)
 */
void keyset_dialog_show(GtkWindow *parent, int keyset)
{
    GtkWidget *dialog;
    GtkWidget *content;
    gint response;
    gchar title[256];

    if (keyset < 1 || keyset > 2) {
        log_error(LOG_ERR, "Got invalid keyset number: %d\n", keyset);
        return;
    }

    keyset_index = keyset;
    if (!get_keyset_resources()) {
        return;
    }

    /* create title (the joystick/userport joystick widgets in
     * settings_joystick.c use 'keyset A/B', so let's be consistent) */
    g_snprintf(title, 256, "Configure keyset %c", keyset == 1 ? 'A' : 'B');

    /*
     * don't use ui_get_active_window() for the parent, that will break
     * the blocking of this dialog of other dialogs and allows the main settings
     * dialog to move in front of this one (which should not happen)
     */
    dialog = gtk_dialog_new_with_buttons(
            title, parent, GTK_DIALOG_MODAL,
            _("OK"), GTK_RESPONSE_ACCEPT,
            _("Cancel"), GTK_RESPONSE_REJECT,
            NULL);

    /* add the keyset buttons */
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), create_content_widget(), TRUE, TRUE,
            16);

    /* connect key events handler */
    g_signal_connect(dialog, "key-press-event", G_CALLBACK(on_key_pressed), NULL);

    /* wait for the dialog to emit either OK or Cancel */
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        /* update keyset resources */
        set_keyset_resources();
    }
    gtk_widget_destroy(dialog);
}
