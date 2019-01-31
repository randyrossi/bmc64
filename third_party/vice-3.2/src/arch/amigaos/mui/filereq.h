/*
 * filereq.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#ifndef VICE_UI_FILEREQ_H
#define VICE_UI_FILEREQ_H

#include "imagecontents.h"

enum {
    IDD_NONE = 0,
    IDD_OPENTAPE_TEMPLATE,
    IDD_OPEN_TEMPLATE
};

extern char *ui_filereq(const char *title, int template, char *initialdir, char *initialfile,
                        image_contents_t *(*read_content)(const char *), int *autostart, char *resource_readonly);

#endif
