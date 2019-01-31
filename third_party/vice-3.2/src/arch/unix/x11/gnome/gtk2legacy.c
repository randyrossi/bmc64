/*
 * gtk2legacy.c - GTK only, GTK2 backwards compatibility layer
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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

/* #define DEBUG_X11UI */

#include "vice.h"

#include "ui.h"
#include "uiarch.h"

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/******************************************************************************/
 
#if !GTK_CHECK_VERSION(2, 12, 0)
/* since 2.12 */
void gtk_widget_set_tooltip_text(GtkWidget * widget, const char * text)
{
    static GtkTooltips * tooltips = NULL;

    if (tooltips == NULL) {
        tooltips = gtk_tooltips_new();
        gtk_tooltips_enable(tooltips);
    }
    gtk_tooltips_set_tip(tooltips, widget, text, NULL);
}
#endif

#if !GTK_CHECK_VERSION(2, 24, 0)
/* since 2.24 */
GtkWidget *gtk_combo_box_text_new(void)
{
    return gtk_combo_box_new_text();
}

/* since 2.24 */
void gtk_combo_box_text_append_text(GtkComboBoxText *combo_box, const gchar *text)
{
    gtk_combo_box_append_text(combo_box, text);
}
#endif

#if !GTK_CHECK_VERSION(3, 0, 0)
/* since 3.0 ? */
int gtk_widget_get_allocated_height(GtkWidget *widget)
{
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    return allocation.height;
}

int gtk_widget_get_allocated_width(GtkWidget *widget)
{
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    return allocation.width;
}
#endif

#if !GTK_CHECK_VERSION(3, 0, 0)
/** \brief  Create box widget
 *
 * \param[in]   orientation orientation of the box:
 *                          GTK_ORIENTATION_HORIZONTAL/GTK_ORIENTATION_VERTICAL
 * \param[in]   spacing     spacing to add between widgets
 *
 * \return  new GtkBox
 *
 * \note    Since gtk_[hv]_box() use a `homogeneous` argument, which is missing
 *          for gtk_box_new(), I use `FALSE` for its value, since all VICE
 *          code uses that as its value.
 */
GtkWidget *gtk_box_new(GtkOrientation orientation, gint spacing)
{
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        return gtk_hbox_new(FALSE, spacing);
    } else {
        return gtk_vbox_new(FALSE, spacing);
    }
}
#endif

