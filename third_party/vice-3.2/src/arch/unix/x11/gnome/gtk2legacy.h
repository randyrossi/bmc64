/*
 * gtk2legacy.h - GTK only, GTK2 backwards compatibility layer
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

#ifndef GTK2LEGACY_H_
#define GTK2LEGACY_H_

/* FIXME: backwards compatibility macros */
#if 0
/* since ? */
#define gtk_widget_get_parent(w) w->parent
#define gtk_toggle_button_get_active(b) b->active
#define gtk_toggle_button_set_active(b, a) b->active = (a)
#define gtk_adjustment_get_value(a) a->value
#define gtk_adjustment_set_value(a, v) a->value = (v)
#endif

/* FIXME: backwards compatibility functions */
#if 0
/* since 2.22 */
gdk_visual_get_red_pixel_details()
gdk_visual_get_green_pixel_details()
gdk_visual_get_blue_pixel_details()
/* since 2.18 */
gtk_widget_set_can_focus(w)
        GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);

#endif

/* deprecated macros, use the respective functions instead */
#if 0
/* deprecated since 2.22 */
#define GTK_OBJECT_FLAGS(obj) (GTK_OBJECT (obj)->flags)
#define GTK_WIDGET_SET_FLAGS(wid,flag) G_STMT_START{ (GTK_WIDGET_FLAGS (wid) |= (flag)); }G_STMT_END
#define GTK_WIDGET_UNSET_FLAGS(wid,flag)  G_STMT_START{ (GTK_WIDGET_FLAGS (wid) &= ~(flag)); }G_STMT_END
#define GDK_DISPLAY() gdk_x11_get_default_xdisplay()
#endif

#if !GTK_CHECK_VERSION(2, 12, 0)
/* since 2.12 */
extern void gtk_widget_set_tooltip_text(GtkWidget * widget, const char * text);
#endif

#if !GTK_CHECK_VERSION(2, 14, 0)
/* since 2.14 */
#define gtk_dialog_get_content_area(d) (d)->vbox
/* since 2.14 */
#define gtk_widget_get_window(w) (w)->window
#endif

#if !GTK_CHECK_VERSION(2, 18, 0)
/* since 2.18 */
#define gtk_widget_get_visible(w) GTK_WIDGET_VISIBLE(w)
#endif

#if !GTK_CHECK_VERSION(2, 21, 1)
/* since 2.21.1 */
#define gdk_drag_context_list_targets(c) (c)->targets
/* since 2.21.1 */
#define gdk_drag_context_get_selected_action(c) (c)->action
#endif

#if !GTK_CHECK_VERSION(2, 22, 0)
/* since 2.22, deprecated since 2.22 (?) */
#define gdk_image_get_visual(w) (w)->visual
/* since 2.22 */
#define gdk_visual_get_byte_order(v) (v)->byte_order
/* since 2.22 */
#define gdk_visual_get_depth(v) (v)->depth
#endif

#if !GTK_CHECK_VERSION(2, 24, 0)
/* since 2.24 ? */
typedef GtkComboBox GtkComboBoxText;
/* since 2.24 ? */
#define GTK_COMBO_BOX_TEXT(w) GTK_COMBO_BOX(w)
/* since 2.24 */
extern GtkWidget *gtk_combo_box_text_new(void);
/* since 2.24 */
extern void gtk_combo_box_text_append_text(GtkComboBoxText *combo_box, const gchar *text);
#endif

#if !GTK_CHECK_VERSION(3, 0, 0)
/* since 3.0 ? */
extern int gtk_widget_get_allocated_height(GtkWidget *widget);
extern int gtk_widget_get_allocated_width(GtkWidget *widget);
#endif

#if !GTK_CHECK_VERSION(3, 0, 0)
extern GtkWidget *gtk_box_new(GtkOrientation orientation, gint spacing);
#endif

/* Work around an incompatible change in GDK header files
 * http://git.gnome.org/browse/gtk+/commit/gdk/gdkkeysyms.h?id=913cdf3be750a1e74c09b20edf55a57f9a919fcc */

#if defined GDK_KEY_0
#define GDK_KEY(symbol) GDK_KEY_##symbol
#else
#define GDK_KEY(symbol) GDK_##symbol
#endif

#endif /* GTK2LEGACY_H_ */
