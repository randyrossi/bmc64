/*
 * ui.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifndef VICE_UI_H
#define VICE_UI_H

#include "types.h"
#include "uiapi.h"
#include "uilib.h"

typedef struct ui_resources_s {
    int fullscreenenabled;
    int statusbarenabled;
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    int videooverlayenabled;
#endif
    int save_resources_on_exit;
    int confirm_on_exit;
    char *initialdir[UILIB_SELECTOR_STYLES_NUM];
} ui_resources_t;

extern ui_resources_t ui_resources;
extern int ui_requester(char *title, char *msg, char *buttons, int defval);
extern void ui_message(const char *format, ...);
extern void ui_event_handle(void);
extern void ui_pause_emulation(int flag);

extern void (*ui_event_handling)(void);

extern int ui_emulation_is_paused(void);

#endif
