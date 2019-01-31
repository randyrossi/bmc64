/*
 * uifileentry.h - file entry widget.
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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

#ifndef VICE__uifileentry_h__
#define VICE__uifileentry_h__

enum uilib_file_filter_enum_s;
enum ui_filechooser_s;

extern GtkWidget *vice_file_entry(const char *title, GtkWidget* parent_window, const char *default_dir, enum uilib_file_filter_enum_s* patterns, int num_patterns, enum ui_filechooser_s action);

#endif /* __uifileentry_h__ */
