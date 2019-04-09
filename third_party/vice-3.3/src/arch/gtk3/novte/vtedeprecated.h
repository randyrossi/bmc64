/*
 * Copyright (C) 2001,2002,2003,2009,2010 Red Hat, Inc.
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

#ifndef __NOVTE_DEPRECATED_H__
#define __NOVTE_DEPRECATED_H__

#include "vteterminal.h"

#define _VTE_DEPRECATED

G_BEGIN_DECLS

_VTE_DEPRECATED
_VTE_PUBLIC
void vte_terminal_copy_clipboard(VteTerminal *terminal) _VTE_GNUC_NONNULL(1);

_VTE_DEPRECATED
_VTE_PUBLIC
void vte_terminal_get_geometry_hints(VteTerminal *terminal,
                                     GdkGeometry *hints,
                                     int min_rows,
                                     int min_columns) _VTE_GNUC_NONNULL(1) _VTE_GNUC_NONNULL(2);

_VTE_DEPRECATED
_VTE_PUBLIC
void vte_terminal_set_geometry_hints_for_window(VteTerminal *terminal,
                                                GtkWindow *window) _VTE_GNUC_NONNULL(1) _VTE_GNUC_NONNULL(2);

G_END_DECLS

#undef _VTE_DEPRECATED

#endif /* !__NOVTE_DEPRECATED__H__ */
