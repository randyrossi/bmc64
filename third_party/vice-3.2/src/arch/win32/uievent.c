/*
 * uievent.c
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "vice.h"

#include <windows.h>
#include <tchar.h>

#include "intl.h"
#include "lib.h"
#include "res.h"
#include "translate.h"
#include "uievent.h"
#include "uilib.h"
#include "vice-event.h"

void uievent_command(HWND hwnd, WPARAM wparam)
{
    char *fname;

    switch (wparam & 0xffff) {
        case IDM_EVENT_DIRECTORY:
            fname = uilib_select_file(hwnd,
                                      translate_text(IDS_SELECT_START_SNAP_EVENT),
                                      UILIB_FILTER_ALL | UILIB_FILTER_SNAPSHOT,
                                      UILIB_SELECTOR_TYPE_FILE_SAVE,
                                      UILIB_SELECTOR_STYLE_EVENT_START);
            lib_free(fname);
            fname = uilib_select_file(hwnd,
                                      translate_text(IDS_SELECT_END_SNAP_EVENT),
                                      UILIB_FILTER_ALL | UILIB_FILTER_SNAPSHOT,
                                      UILIB_SELECTOR_TYPE_FILE_SAVE,
                                      UILIB_SELECTOR_STYLE_EVENT_END);
            lib_free(fname);
            break;
        case IDM_EVENT_TOGGLE_RECORD:
            {
                int recording_new = (event_record_active() ? 0 : 1);

                if (recording_new) {
                    event_record_start();
                } else {
                    event_record_stop();
                }
            }
            break;
        case IDM_EVENT_TOGGLE_PLAYBACK:
            {
                int playback_new = (event_playback_active() ? 0 : 1);

                if (playback_new) {
                    event_playback_start();
                } else {
                    event_playback_stop();
                }
            }
            break;
        case IDM_EVENT_SETMILESTONE:
            event_record_set_milestone();
            break;
        case IDM_EVENT_RESETMILESTONE:
            event_record_reset_milestone();
            break;
    }
}
