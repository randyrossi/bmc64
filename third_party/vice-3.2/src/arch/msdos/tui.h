/*
 * tui.h - A (very) simple text-based user interface.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_TUI_H
#define VICE_TUI_H

#define CENTER_X(w) ((tui_num_cols() - (w)) / 2)
#define CENTER_Y(h) ((tui_num_lines() - (h)) / 2)

#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

extern void tui_error(const char *format, ...);
extern void tui_message(const char *format, ...);
extern int tui_ask_confirmation(const char *format, ...);
extern int tui_input_string(const char *title, const char *prompt, char *buf, int buflen);
extern void tui_display_text(int x, int y, int width, int height, const char *text);

#endif /* VICE_TUI_H */
