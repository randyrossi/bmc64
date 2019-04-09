/*
 * Copyright (C) 2003 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* The interfaces in this file are subject to change at any time. */

#ifndef vte_vtedraw_h_included
#define vte_vtedraw_h_included

#include <glib.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include "vteunistr.h"
#include "vtetypes.hh"

G_BEGIN_DECLS

#define VTE_DRAW_OPAQUE (1.0)
#define VTE_DRAW_MAX_LENGTH 1024

#define VTE_DRAW_NORMAL 0
#define VTE_DRAW_BOLD   1
#define VTE_DRAW_ITALIC 2

struct _vte_draw;

/* A request to draw a particular character spanning a given number of columns
   at the given location.  Unlike most APIs, (x,y) specifies the top-left
   corner of the cell into which the character will be drawn instead of the
   left end of the baseline. */
struct _vte_draw_text_request {
    vteunistr c;
    gshort x, y, columns;
};

guint _vte_draw_get_style(gboolean bold, gboolean italic);

/* Create and destroy a draw structure. */
struct _vte_draw *_vte_draw_new(void);
void _vte_draw_free(struct _vte_draw *draw);

void _vte_draw_set_cairo(struct _vte_draw *draw,
                         cairo_t *cr);

void _vte_draw_clear(struct _vte_draw *draw,
                        gint x, gint y, gint width, gint height,
                        vte::color::rgb const* color, double alpha);

void _vte_draw_set_text_font(struct _vte_draw *draw,
                             GtkWidget *widget,
                             const PangoFontDescription *fontdesc,
                             double cell_width_scale, double cell_height_scale);
void _vte_draw_get_text_metrics(struct _vte_draw *draw,
                                int *cell_width, int *cell_height,
                                int *char_ascent, int *char_descent,
                                GtkBorder *char_spacing);
void _vte_draw_get_char_edges (struct _vte_draw *draw, vteunistr c, int columns, guint style,
                               int *left, int *right);
gboolean _vte_draw_has_bold (struct _vte_draw *draw, guint style);

void _vte_draw_text(struct _vte_draw *draw,
                    struct _vte_draw_text_request *requests, gsize n_requests,
                    vte::color::rgb const* color, double alpha, guint style);
gboolean _vte_draw_char(struct _vte_draw *draw,
                        struct _vte_draw_text_request *request,
                        vte::color::rgb const* color, double alpha, guint style);
gboolean _vte_draw_has_char(struct _vte_draw *draw, vteunistr c, guint style);

void _vte_draw_fill_rectangle(struct _vte_draw *draw,
                                gint x, gint y, gint width, gint height,
                                vte::color::rgb const* color, double alpha);
void _vte_draw_draw_rectangle(struct _vte_draw *draw,
                                gint x, gint y, gint width, gint height,
                                vte::color::rgb const* color, double alpha);

void _vte_draw_draw_line(struct _vte_draw *draw,
                         gint x, gint y, gint xp, gint yp,
                         int line_width,
                         vte::color::rgb const *color, double alpha);

double _vte_draw_get_undercurl_height(gint width, double line_width);

void _vte_draw_draw_undercurl(struct _vte_draw *draw,
                                gint x, double y,
                                double line_width,
                                gint count,
                                vte::color::rgb const *color, double alpha);

G_END_DECLS

#endif
