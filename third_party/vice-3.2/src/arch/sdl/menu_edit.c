/*
 * menu_edit.c - Implementation of the EDIT for the SDL UI.
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

#include "vice.h"

#include "machine.h"
#include "menu_common.h"
#include "menu_edit.h"
#include "charset.h"
#include "clipboard.h"
#include "lib.h"
#include "kbdbuf.h"

#ifdef USE_SDLUI2
/** \brief  Callback for the edit->paste menu item
 *
 * Copies the host clipboard into the emulated machine's screen
 *
 * \param[in]   widget  widget (unused)
 * \param[in]   data    extra data (unused)
 */
UI_MENU_CALLBACK(edit_menu_paste_callback)
{
    char *text_in_petscii;
    char *text = SDL_GetClipboardText();

    if (activated) {
        if (text == NULL) {
            return NULL;
        }
        text_in_petscii = lib_stralloc(text);

        charset_petconvstring((unsigned char*)text_in_petscii, 0);
        kbdbuf_feed(text_in_petscii);
        lib_free(text_in_petscii);
        SDL_free(text);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

/** \brief  Callback for the edit->copy menu item
 *
 * Copies the screen of the emulated machine into the host clipboard
 *
 * \param[in]   widget  widget (unused)
 * \param[in]   data    extra data (unused)
 */
UI_MENU_CALLBACK(edit_menu_copy_callback)
{
    if (activated) {
        char * text = clipboard_read_screen_output("\n");
        if (text != NULL) {
            SDL_SetClipboardText(text);
            return sdl_menu_text_exit_ui;
        }
    }
    return NULL;
}

const ui_menu_entry_t edit_menu[3] = {
    { "Copy to Clipboard", MENU_ENTRY_DIALOG, edit_menu_copy_callback, NULL},
    { "Paste from Clipboard", MENU_ENTRY_DIALOG, edit_menu_paste_callback, NULL},
    SDL_MENU_LIST_END
};
#endif
