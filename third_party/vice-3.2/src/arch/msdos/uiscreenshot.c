/*
 * uiscreenshot.c - Screenshot save dialog for the MS-DOS version of VICE.
 *
 * Written by
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

#include "vice.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "gfxoutput.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "screenshot.h"
#include "tui.h"
#include "tuimenu.h"
#include "tuifs.h"
#include "ui.h"
#include "uiscreenshot.h"
#include "util.h"
#include "videoarch.h"

TUI_MENU_DEFINE_RADIO(DoodleOversizeHandling)
TUI_MENU_DEFINE_RADIO(DoodleUndersizeHandling)
TUI_MENU_DEFINE_RADIO(DoodleMultiColorHandling)
TUI_MENU_DEFINE_RADIO(DoodleTEDLumHandling)
TUI_MENU_DEFINE_RADIO(DoodleCRTCTextColor)
TUI_MENU_DEFINE_RADIO(KoalaOversizeHandling)
TUI_MENU_DEFINE_RADIO(KoalaUndersizeHandling)
TUI_MENU_DEFINE_RADIO(KoalaTEDLumHandling)
TUI_MENU_DEFINE_RADIO(KoalaCRTCTextColor)

static TUI_MENU_CALLBACK(bmp_file_name_callback);
static TUI_MENU_CALLBACK(write_bmp_screenshot_callback);

static TUI_MENU_CALLBACK(doodle_file_name_callback);
static TUI_MENU_CALLBACK(write_doodle_screenshot_callback);

static TUI_MENU_CALLBACK(doodle_compressed_file_name_callback);
static TUI_MENU_CALLBACK(write_doodle_compressed_screenshot_callback);

static TUI_MENU_CALLBACK(koala_file_name_callback);
static TUI_MENU_CALLBACK(write_koala_screenshot_callback);

static TUI_MENU_CALLBACK(koala_compressed_file_name_callback);
static TUI_MENU_CALLBACK(write_koala_compressed_screenshot_callback);

static TUI_MENU_CALLBACK(pcx_file_name_callback);
static TUI_MENU_CALLBACK(write_pcx_screenshot_callback);

static TUI_MENU_CALLBACK(ppm_file_name_callback);
static TUI_MENU_CALLBACK(write_ppm_screenshot_callback);

static TUI_MENU_CALLBACK(iff_file_name_callback);
static TUI_MENU_CALLBACK(write_iff_screenshot_callback);

static TUI_MENU_CALLBACK(godot_file_name_callback);
static TUI_MENU_CALLBACK(write_godot_screenshot_callback);

#ifdef HAVE_GIF
static TUI_MENU_CALLBACK(write_gif_screenshot_callback);
static TUI_MENU_CALLBACK(gif_file_name_callback);
#endif

#ifdef HAVE_JPEG
static TUI_MENU_CALLBACK(write_jpeg_screenshot_callback);
static TUI_MENU_CALLBACK(jpeg_file_name_callback);
#endif

#ifdef HAVE_PNG
static TUI_MENU_CALLBACK(write_png_screenshot_callback);
static TUI_MENU_CALLBACK(png_file_name_callback);
#endif

static char *file_name = NULL;

static tui_menu_item_def_t write_bmp_screenshot_menu_def[] = {
    { "_File name:",
      "Specify BMP screenshot file name",
      bmp_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save BMP screenshot with the specified name",
      write_bmp_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_doodle_screenshot_menu_def[] = {
    { "_File name:",
      "Specify DOODLE screenshot file name",
      doodle_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save DOODLE screenshot with the specified name",
      write_doodle_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_doodle_compressed_screenshot_menu_def[] = {
    { "_File name:",
      "Specify compressed DOODLE screenshot file name",
      doodle_compressed_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save compressed DOODLE screenshot with the specified name",
      write_doodle_compressed_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_koala_screenshot_menu_def[] = {
    { "_File name:",
      "Specify KOALA screenshot file name",
      koala_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save KOALA screenshot with the specified name",
      write_koala_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_koala_compressed_screenshot_menu_def[] = {
    { "_File name:",
      "Specify compressed KOALA screenshot file name",
      koala_compressed_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save compressed KOALA screenshot with the specified name",
      write_koala_compressed_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_pcx_screenshot_menu_def[] = {
    { "_File name:",
      "Specify PCX screenshot file name",
      pcx_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save PCX screenshot with the specified name",
      write_pcx_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_ppm_screenshot_menu_def[] = {
    { "_File name:",
      "Specify PPM screenshot file name",
      ppm_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save PPM screenshot with the specified name",
      write_ppm_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_iff_screenshot_menu_def[] = {
    { "_File name:",
      "Specify IFF screenshot file name",
      iff_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save IFF screenshot with the specified name",
      write_iff_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t write_godot_screenshot_menu_def[] = {
    { "_File name:",
      "Specify godot screenshot file name",
      godot_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save godot screenshot with the specified name",
      write_godot_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

#ifdef HAVE_GIF
static tui_menu_item_def_t write_gif_screenshot_menu_def[] = {
    { "_File name:",
      "Specify GIF screenshot file name",
      gif_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save GIF screenshot with the specified name",
      write_gif_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};
#endif

#ifdef HAVE_JPEG
static tui_menu_item_def_t write_jpeg_screenshot_menu_def[] = {
    { "_File name:",
      "Specify JPEG screenshot file name",
      jpeg_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save JPEG screenshot with the specified name",
      write_jpeg_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};
#endif

#ifdef HAVE_PNG
static tui_menu_item_def_t write_png_screenshot_menu_def[] = {
    { "_File name:",
      "Specify PNG screenshot file name",
      png_file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Do it!",
      "Save PNG screenshot with the specified name",
      write_png_screenshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};
#endif

static tui_menu_item_def_t ui_write_screenshot_menu_def[] = {
    { "_Write BMP Screenshot",
      "Write a BMP screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_bmp_screenshot_menu_def, NULL },
    { "_Write DOODLE Screenshot",
      "Write a DOODLE screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_doodle_screenshot_menu_def, NULL },
    { "_Write compressed DOODLE Screenshot",
      "Write a compressed DOODLE screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_doodle_compressed_screenshot_menu_def, NULL },
    { "_Write KOALA Screenshot",
      "Write a KOALA screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_koala_screenshot_menu_def, NULL },
    { "_Write compressed KOALA Screenshot",
      "Write a compressed KOALA screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_koala_compressed_screenshot_menu_def, NULL },
#ifdef HAVE_GIF
    { "W_rite GIF Screenshot",
      "Write a GIF screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_gif_screenshot_menu_def, NULL },
#endif
    { "_Write IFF Screenshot",
      "Write a IFF screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_iff_screenshot_menu_def, NULL },
#ifdef HAVE_JPEG
    { "Wr_ite JPEG Screenshot",
      "Write a JPEG screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_jpeg_screenshot_menu_def, NULL },
#endif
    { "_Write PCX Screenshot",
      "Write a PCX screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_pcx_screenshot_menu_def, NULL },
#ifdef HAVE_PNG
    { "Wri_te PNG Screenshot",
      "Write a PNG screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_png_screenshot_menu_def, NULL },
#endif
    { "Wri_te PPM Screenshot",
      "Write a PPM screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_ppm_screenshot_menu_def, NULL },
    { "Wri_te godot Screenshot",
      "Write a godot screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_godot_screenshot_menu_def, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(doodle_oversize_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("DoodleOversizeHandling", &value);
    switch (value) {
        default:
        case NATIVE_SS_OVERSIZE_SCALE:
            s = "Scale";
            break;
        case NATIVE_SS_OVERSIZE_CROP_LEFT_TOP:
            s = "Crop left top";
            break;
        case NATIVE_SS_OVERSIZE_CROP_CENTER_TOP:
            s = "Crop middle top";
            break;
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP:
            s = "Crop right top";
            break;
        case NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER:
            s = "Crop left center";
            break;
        case NATIVE_SS_OVERSIZE_CROP_CENTER:
            s = "Crop middle center";
            break;
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER:
            s = "Crop right center";
            break;
        case NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM:
            s = "Crop left bottom";
            break;
        case NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM:
            s = "Crop middle bottom";
            break;
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM:
            s = "Crop right bottom";
            break;
    }
    return s;
}

static tui_menu_item_def_t doodle_oversize_submenu[] = {
    { "Scale", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_SCALE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop left top", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_LEFT_TOP, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop middle top", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_CENTER_TOP, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop right top", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop left center", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop middle center", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_CENTER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop right center", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop left bottom", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop middle bottom", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop right bottom", NULL, radio_DoodleOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(doodle_undersize_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("DoodleUndersizeHandling", &value);
    switch (value) {
        default:
        case NATIVE_SS_UNDERSIZE_SCALE:
            s = "Scale";
            break;
        case NATIVE_SS_UNDERSIZE_BORDERIZE:
            s = "Borderize";
            break;
    }
    return s;
}

static tui_menu_item_def_t doodle_undersize_submenu[] = {
    { "Scale", NULL, radio_DoodleUndersizeHandling_callback,
      (void *)NATIVE_SS_UNDERSIZE_SCALE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Borderize", NULL, radio_DoodleUndersizeHandling_callback,
      (void *)NATIVE_SS_UNDERSIZE_BORDERIZE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(doodle_multicolor_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("DoodleMultiColorHandling", &value);
    switch (value) {
        default:
        case NATIVE_SS_MC2HR_BLACK_WHITE:
            s = "Black & white";
            break;
        case NATIVE_SS_MC2HR_2_COLORS:
            s = "2 colors";
            break;
        case NATIVE_SS_MC2HR_4_COLORS:
            s = "4 colors";
            break;
        case NATIVE_SS_MC2HR_GRAY:
            s = "Gray scale";
            break;
        case NATIVE_SS_MC2HR_DITHER:
            s = "Dither";
            break;
    }
    return s;
}

static tui_menu_item_def_t doodle_multicolor_submenu[] = {
    { "Black & white", NULL, radio_DoodleMultiColorHandling_callback,
      (void *)NATIVE_SS_MC2HR_BLACK_WHITE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "2 colors", NULL, radio_DoodleMultiColorHandling_callback,
      (void *)NATIVE_SS_MC2HR_2_COLORS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "4 colors", NULL, radio_DoodleMultiColorHandling_callback,
      (void *)NATIVE_SS_MC2HR_4_COLORS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Gray scale", NULL, radio_DoodleMultiColorHandling_callback,
      (void *)NATIVE_SS_MC2HR_GRAY, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Dither", NULL, radio_DoodleMultiColorHandling_callback,
      (void *)NATIVE_SS_MC2HR_DITHER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(doodle_tedlum_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("DoodleTEDLumHandling", &value);
    switch (value) {
        default:
        case NATIVE_SS_TED_LUM_IGNORE:
            s = "Ignore";
            break;
        case NATIVE_SS_TED_LUM_DITHER:
            s = "Dither";
            break;
    }
    return s;
}

static tui_menu_item_def_t doodle_tedlum_submenu[] = {
    { "Ignore", NULL, radio_DoodleTEDLumHandling_callback,
      (void *)NATIVE_SS_TED_LUM_IGNORE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Dither", NULL, radio_DoodleTEDLumHandling_callback,
      (void *)NATIVE_SS_TED_LUM_DITHER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(doodle_crtctextcolor_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("DoodleCRTCTextColor", &value);
    switch (value) {
        default:
        case NATIVE_SS_CRTC_WHITE:
            s = "White";
            break;
        case NATIVE_SS_CRTC_AMBER:
            s = "Amber";
            break;
        case NATIVE_SS_CRTC_GREEN:
            s = "Green";
            break;
    }
    return s;
}

static tui_menu_item_def_t doodle_crtctextcolor_submenu[] = {
    { "White", NULL, radio_DoodleCRTCTextColor_callback,
      (void *)NATIVE_SS_CRTC_WHITE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Amber", NULL, radio_DoodleCRTCTextColor_callback,
      (void *)NATIVE_SS_CRTC_AMBER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Green", NULL, radio_DoodleCRTCTextColor_callback,
      (void *)NATIVE_SS_CRTC_GREEN, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_doodle_screenshot_menu_def_vic_vicii_vdc[] = {
    { "Oversize handling:", "Oversize handling",
      doodle_oversize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_oversize_submenu,
      "Oversize handling" },
    { "Undersize handling:", "Undersize handling",
      doodle_undersize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_undersize_submenu,
      "Undersize handling" },
    { "Multicolor handling:", "Multicolor handling",
      doodle_multicolor_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_multicolor_submenu,
      "Multicolor handling" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_doodle_screenshot_menu_def_ted[] = {
    { "Oversize handling:", "Oversize handling",
      doodle_oversize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_oversize_submenu,
      "Oversize handling" },
    { "Undersize handling:", "Undersize handling",
      doodle_undersize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_undersize_submenu,
      "Undersize handling" },
    { "Multicolor handling:", "Multicolor handling",
      doodle_multicolor_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_multicolor_submenu,
      "Multicolor handling" },
    { "TED luminosity handling:", "TED luminosity handling",
      doodle_tedlum_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_tedlum_submenu,
      "TED luminosity handling" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_doodle_screenshot_menu_def_crtc[] = {
    { "Oversize handling:", "Oversize handling",
      doodle_oversize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_oversize_submenu,
      "Oversize handling" },
    { "Undersize handling:", "Undersize handling",
      doodle_undersize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_undersize_submenu,
      "Undersize handling" },
    { "CRTC text color:", "CRTC text color",
      doodle_crtctextcolor_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, doodle_crtctextcolor_submenu,
      "CRTC text color" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(koala_oversize_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("KoalaOversizeHandling", &value);
    switch (value) {
        default:
        case NATIVE_SS_OVERSIZE_SCALE:
            s = "Scale";
            break;
        case NATIVE_SS_OVERSIZE_CROP_LEFT_TOP:
            s = "Crop left top";
            break;
        case NATIVE_SS_OVERSIZE_CROP_CENTER_TOP:
            s = "Crop middle top";
            break;
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP:
            s = "Crop right top";
            break;
        case NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER:
            s = "Crop left center";
            break;
        case NATIVE_SS_OVERSIZE_CROP_CENTER:
            s = "Crop middle center";
            break;
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER:
            s = "Crop right center";
            break;
        case NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM:
            s = "Crop left bottom";
            break;
        case NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM:
            s = "Crop middle bottom";
            break;
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM:
            s = "Crop right bottom";
            break;
    }
    return s;
}

static tui_menu_item_def_t koala_oversize_submenu[] = {
    { "Scale", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_SCALE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop left top", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_LEFT_TOP, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop middle top", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_CENTER_TOP, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop right top", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop left center", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop middle center", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_CENTER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop right center", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop left bottom", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop middle bottom", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Crop right bottom", NULL, radio_KoalaOversizeHandling_callback,
      (void *)NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(koala_undersize_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("KoalaUndersizeHandling", &value);
    switch (value) {
        default:
        case NATIVE_SS_UNDERSIZE_SCALE:
            s = "Scale";
            break;
        case NATIVE_SS_UNDERSIZE_BORDERIZE:
            s = "Borderize";
            break;
    }
    return s;
}

static tui_menu_item_def_t koala_undersize_submenu[] = {
    { "Scale", NULL, radio_KoalaUndersizeHandling_callback,
      (void *)NATIVE_SS_UNDERSIZE_SCALE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Borderize", NULL, radio_KoalaUndersizeHandling_callback,
      (void *)NATIVE_SS_UNDERSIZE_BORDERIZE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(koala_tedlum_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("KoalaTEDLumHandling", &value);
    switch (value) {
        default:
        case NATIVE_SS_TED_LUM_IGNORE:
            s = "Ignore";
            break;
        case NATIVE_SS_TED_LUM_DITHER:
            s = "Dither";
            break;
    }
    return s;
}

static tui_menu_item_def_t koala_tedlum_submenu[] = {
    { "Ignore", NULL, radio_KoalaTEDLumHandling_callback,
      (void *)NATIVE_SS_TED_LUM_IGNORE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Dither", NULL, radio_KoalaTEDLumHandling_callback,
      (void *)NATIVE_SS_TED_LUM_DITHER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(koala_crtctextcolor_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("KoalaCRTCTextColor", &value);
    switch (value) {
        default:
        case NATIVE_SS_CRTC_WHITE:
            s = "White";
            break;
        case NATIVE_SS_CRTC_AMBER:
            s = "Amber";
            break;
        case NATIVE_SS_CRTC_GREEN:
            s = "Green";
            break;
    }
    return s;
}

static tui_menu_item_def_t koala_crtctextcolor_submenu[] = {
    { "White", NULL, radio_KoalaCRTCTextColor_callback,
      (void *)NATIVE_SS_CRTC_WHITE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Amber", NULL, radio_KoalaCRTCTextColor_callback,
      (void *)NATIVE_SS_CRTC_AMBER, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Green", NULL, radio_KoalaCRTCTextColor_callback,
      (void *)NATIVE_SS_CRTC_GREEN, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_koala_screenshot_menu_def_vic_vicii_vdc[] = {
    { "Oversize handling:", "Oversize handling",
      koala_oversize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_oversize_submenu,
      "Oversize handling" },
    { "Undersize handling:", "Undersize handling",
      koala_undersize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_undersize_submenu,
      "Undersize handling" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_koala_screenshot_menu_def_ted[] = {
    { "Oversize handling:", "Oversize handling",
      koala_oversize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_oversize_submenu,
      "Oversize handling" },
    { "Undersize handling:", "Undersize handling",
      koala_undersize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_undersize_submenu,
      "Undersize handling" },
    { "TED luminosity handling:", "TED luminosity handling",
      koala_tedlum_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_tedlum_submenu,
      "TED luminosity handling" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_koala_screenshot_menu_def_crtc[] = {
    { "Oversize handling:", "Oversize handling",
      koala_oversize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_oversize_submenu,
      "Oversize handling" },
    { "Undersize handling:", "Undersize handling",
      koala_undersize_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_undersize_submenu,
      "Undersize handling" },
    { "CRTC text color:", "CRTC text color",
      koala_crtctextcolor_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, koala_crtctextcolor_submenu,
      "CRTC text color" },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t ui_screenshot_menu_def_vic_vicii_vdc[] = {
    { "Doodle screenshot settings",
      "Doodle screenshot settings",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_doodle_screenshot_menu_def_vic_vicii_vdc, NULL },
    { "Koala screenshot settings",
      "Koala screenshot settings",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_koala_screenshot_menu_def_vic_vicii_vdc, NULL },
    { "_Write Screenshot",
      "Write a screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_write_screenshot_menu_def, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t ui_screenshot_menu_def_ted[] = {
    { "Doodle screenshot settings",
      "Doodle screenshot settings",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_doodle_screenshot_menu_def_ted, NULL },
    { "Koala screenshot settings",
      "Koala screenshot settings",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_koala_screenshot_menu_def_ted, NULL },
    { "_Write Screenshot",
      "Write a screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_write_screenshot_menu_def, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t ui_screenshot_menu_def_crtc[] = {
    { "Doodle screenshot settings",
      "Doodle screenshot settings",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_doodle_screenshot_menu_def_crtc, NULL },
    { "Koala screenshot settings",
      "Koala screenshot settings",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_koala_screenshot_menu_def_crtc, NULL },
    { "_Write Screenshot",
      "Write a screenshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_write_screenshot_menu_def, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static char *screenshot_selector(const char *title, char *extension)
{
    return tui_file_selector(title, NULL, extension, NULL, NULL, NULL, NULL);
}

static char *screenshot_filename(char *screenshot_save_msg, char *screenshot_save_file_msg, char *screenshot_filter, char *screenshot_extension)
{
    char new_file_name[PATH_MAX];

    if (file_name == NULL) {
        memset(new_file_name, 0, PATH_MAX);
    } else {
        strcpy(new_file_name, file_name);
    }

    while (tui_input_string(screenshot_save_msg, "Enter file name:", new_file_name, PATH_MAX) != -1) {
        util_remove_spaces(new_file_name);
        if (*new_file_name == 0) {
            char *tmp;

            tmp = screenshot_selector(screenshot_save_file_msg, screenshot_filter);
            if (tmp != NULL) {
                strcpy(new_file_name, tmp);
                lib_free(tmp);
            }
        } else {
            char *extension;
            char *last_dot;

            last_dot = strrchr(new_file_name, '.');

            if (last_dot == NULL) {
                extension = screenshot_extension;
            } else {
                char *last_slash, *last_backslash, *last_path_separator;

                last_slash = strrchr(new_file_name, '/');
                last_backslash = strrchr(new_file_name, '\\');

                if (last_slash == NULL) {
                    last_path_separator = last_backslash;
                } else if (last_backslash == NULL) {
                    last_path_separator = last_slash;
                } else if (last_backslash < last_slash) {
                    last_path_separator = last_slash;
                } else {
                    last_path_separator = last_backslash;
                }

                if (last_path_separator == NULL || last_path_separator < last_dot) {
                    extension = "";
                } else {
                    extension = screenshot_extension;
                }
            }
               
            if (file_name == NULL) {
                file_name = util_concat(new_file_name, ".", extension, NULL);
            } else {
                lib_free(file_name);
                file_name = lib_stralloc(new_file_name);
                util_add_extension(&file_name, extension);
            }
            break;
        }
    }
    return file_name;
}

static TUI_MENU_CALLBACK(bmp_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save BMP screenshot", "Save BMP screenshot file", "*.bmp", "bmp");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(doodle_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save DOODLE screenshot", "Save DOODLE screenshot file", "*.dd", "dd");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(doodle_compressed_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save compressed DOODLE screenshot", "Save compressed DOODLE screenshot file", "*.jj", "jj");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(koala_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save KOALA screenshot", "Save KOALA screenshot file", "*.koa", "koa");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(koala_compressed_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save compressed KOALA screenshot", "Save compressed KOALA screenshot file", "*.gg", "gg");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(pcx_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save PCX screenshot", "Save PCX screenshot file", "*.pcx", "pcx");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(ppm_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save PPM screenshot", "Save PPM screenshot file", "*.ppm", "ppm");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(iff_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save IFF screenshot", "Save IFF screenshot file", "*.iff", "iff");
    }
    return file_name;
}

static TUI_MENU_CALLBACK(godot_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save godot screenshot", "Save godot screenshot file", "*.4bt", "4bt");
    }
    return file_name;
}

#ifdef HAVE_GIF
static TUI_MENU_CALLBACK(gif_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save GIF screenshot", "Save GIF screenshot file", "*.gif", "gif");
    }
    return file_name;
}
#endif

#ifdef HAVE_JPEG
static TUI_MENU_CALLBACK(jpeg_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save JPEG screenshot", "Save JPEG screenshot file", "*.jpg", "jpg");
    }
    return file_name;
}
#endif

#ifdef HAVE_PNG
static TUI_MENU_CALLBACK(png_file_name_callback)
{
    if (been_activated) {
        return screenshot_filename("Save PNG screenshot", "Save PNG screenshot file", "*.png", "png");
    }
    return file_name;
}
#endif

static char *screenshot_save_file(char *screenshot_type, char *screenshot_save_failed, char *screenshot_save_success)
{
    if (file_name == NULL || *file_name == 0) {
        tui_error("Specify a file name first.");
        return NULL;
    }

    if (!util_file_exists(file_name) || tui_ask_confirmation("The specified file already exists.  Replace?  (Y/N)")) {
        if (screenshot_save(screenshot_type, file_name, last_canvas) < 0) {
            tui_error(screenshot_save_failed);
        } else {
            tui_message(screenshot_save_success);
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_bmp_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("BMP", "Cannot save BMP screenshot.", "BMP screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_doodle_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("DOODLE", "Cannot save DOODLE screenshot.", "DOODLE screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_doodle_compressed_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("DOODLE_COMPRESSED", "Cannot save compressed DOODLE screenshot.", "Compressed DOODLE screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_koala_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("KOALA", "Cannot save KOALA screenshot.", "KOALA screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_koala_compressed_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("KOALA_COMPRESSED", "Cannot save compressed KOALA screenshot.", "Compressed KOALA screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_pcx_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("PCX", "Cannot save PCX screenshot.", "PCX screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_ppm_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("PPM", "Cannot save PPM screenshot.", "PPM screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_iff_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("IFF", "Cannot save IFF screenshot.", "IFF screenshot save successfully.");
    }
    return NULL;
}

static TUI_MENU_CALLBACK(write_godot_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("4BT", "Cannot save godot screenshot.", "godot screenshot save successfully.");
    }
    return NULL;
}

#ifdef HAVE_GIF
static TUI_MENU_CALLBACK(write_gif_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("GIF", "Cannot save GIF screenshot.", "GIF screenshot save successfully.");
    }
    return NULL;
}
#endif

#ifdef HAVE_JPEG
static TUI_MENU_CALLBACK(write_jpeg_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("JPEG", "Cannot save JPEG screenshot.", "JPEG screenshot save successfully.");
    }
    return NULL;
}
#endif

#ifdef HAVE_PNG
static TUI_MENU_CALLBACK(write_png_screenshot_callback)
{
    if (been_activated) {
        return screenshot_save_file("PNG", "Cannot save PNG screenshot.", "PNG screenshot save successfully.");
    }
    return NULL;
}
#endif
