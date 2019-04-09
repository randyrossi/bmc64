
/*
 * uifonts.h - Fonts used by the SDL UI
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

/*
 * TODO:
 *  - the translation tables in uimenu.c may need further tweaks
 */

#ifndef VICE_UIFONTS_H
#define VICE_UIFONTS_H

extern int sdl_ui_crtc_font_init(void);
extern void sdl_ui_crtc_font_shutdown(void);

extern int sdl_ui_cbm2_font_init(void);
extern void sdl_ui_cbm2_font_shutdown(void);

extern int sdl_ui_ted_font_init(void);
extern void sdl_ui_ted_font_shutdown(void);

extern int sdl_ui_vic_font_init(void);
extern void sdl_ui_vic_font_shutdown(void);

extern int sdl_ui_vicii_font_init(void);
extern void sdl_ui_vicii_font_shutdown(void);

/* first charset:
 * - ascii layout, used by the menus and file browser
 * - 01-1f are reserved for custom characters used in the menus, see
 *         the defined values below.
 * - 20-7e are ascii
 * - 80-ff are reserved for extended ascii (FIXME)
 * 
 * second charset:
 * - uppercase petscii layout, used by image browser
 * 
 * third charset:
 * - lowercase petscii layout, used by the monitor
 */

#define UIFONT_MENUHEADERLEFT_STRING "\x01"
#define UIFONT_MENUHEADERRIGHT_STRING "\x02"

#define UIFONT_SUBMENU_STRING "\x03\x04"
#define UIFONT_SUBMENULEFT_CHAR '\x03'
#define UIFONT_SUBMENURIGHT_CHAR '\x04'

#define UIFONT_CHECKMARK_UNCHECKED_STRING "\x05"
#define UIFONT_CHECKMARK_UNCHECKED_CHAR '\x05'
#define UIFONT_CHECKMARK_CHECKED_STRING   "\x06"
#define UIFONT_CHECKMARK_CHECKED_CHAR '\x06'

#define UIFONT_TOPLEFT_STRING "\x07"
#define UIFONT_TOPRIGHT_STRING "\x08"

#define UIFONT_BOTTOMLEFT_STRING "\x09"
#define UIFONT_BOTTOMRIGHT_STRING "\x0a"

#define UIFONT_VERTICAL_STRING "\x0b"
#define UIFONT_HORIZONTAL_STRING "\x0c"

#define UIFONT_RIGHTTEE_STRING "\x0d"
#define UIFONT_LEFTTEE_STRING "\x0e"

#define UIFONT_SLIDERINACTIVE_STRING "\x0f"
#define UIFONT_SLIDERINACTIVE_CHAR '\x0f'
#define UIFONT_SLIDERACTIVE_STRING "\x10"
#define UIFONT_SLIDERACTIVE_CHAR '\x10'
#define UIFONT_SLIDERHALFACTIVE_STRING "\x11"
#define UIFONT_SLIDERHALFACTIVE_CHAR '\x11'

typedef enum {
    MENU_FONT_IMAGES = 0,       /* uppercase petscii layout, used by image browser */
    MENU_FONT_ASCII = 1,        /* ascii layout, used by the menus and file browser */
    MENU_FONT_MONITOR = 2       /* lowercase petscii layout, used by the monitor */
} ui_menu_active_font_t;

extern void sdl_ui_set_active_font(ui_menu_active_font_t font);

/* internal */
extern void sdl_ui_set_menu_font(uint8_t *font, int w, int h);
extern void sdl_ui_set_image_font(uint8_t *font, int w, int h);
extern void sdl_ui_set_monitor_font(uint8_t *font, int w, int h);

extern void sdl_ui_copy_ui_font(uint8_t *dest);

#endif
