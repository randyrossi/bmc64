/*
 * video.c - MS-DOS graphics handling, based on the Allegro library by Shawn
 * Hargreaves (http://www.talula.demon.co.uk/allegro).
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include <conio.h>
#include <stdio.h>
#include <string.h>

#include "cmdline.h"
#include "kbd.h"
#include "lib.h"
#include "log.h"
#include "palette.h"
#include "resources.h"
#include "translate.h"
#include "tui.h"
#include "tui_backend.h"
#include "types.h"
#include "ui.h"
#include "video.h"
#include "videoarch.h"

/* #define DEBUG_VIDEO */

vga_mode_t vga_modes[] = {
    { 320, 200, 8, " 320x200  8Bit" },
    { 360, 240, 8, " 360x240  8Bit" },
    { 360, 270, 8, " 360x270  8Bit" },
    { 376, 282, 8, " 376x282  8Bit" },
    { 400, 300, 8, " 400x300  8Bit" },
    { 640, 480, 8, " 640x480  8Bit" },
    { 800, 600, 8, " 800x600  8Bit" },
    { 1024,768, 8, "1024x768  8Bit" },
    { 320, 200, 16, " 320x200 16Bit" },
    { 400, 300, 16, " 400x300 16Bit" },
    { 640, 480, 16, " 640x480 16Bit" },
    { 800, 600, 16, " 800x600 16Bit" },
    { 1024,768, 16, "1024x768 16Bit" },
    { 640, 480, 32, " 640x480 32Bit" },
    { 800, 600, 32, " 800x600 32Bit" },
    { 1024,768, 32, "1024x768 32Bit" }
};

video_canvas_t *last_canvas;

#define MAX_CANVAS_NUM 2
static video_canvas_t *canvaslist[MAX_CANVAS_NUM];

#ifdef DEBUG_VIDEO
#define DEBUG(x) log_debug x
#else
#define DEBUG(x)
#endif

/* Flag: are we in graphics mode?  */
static int in_gfx_mode;

/* Logging goes here.  */
static log_t video_log = LOG_ERR;

/* forward declaration */
static void canvas_change_palette(video_canvas_t *c);

/* ------------------------------------------------------------------------- */

/* Video-related resources.  */

/* VGA Video mode to use.  */
static int vga_mode;

int video_in_gfx_mode() {
    return in_gfx_mode;
}

static int set_vga_mode(int val, void *param)
{
    switch (val) {
        case VGA_320x200x8:
        case VGA_360x240x8:
        case VGA_360x270x8:
        case VGA_376x282x8:
        case VGA_400x300x8:
        case VGA_640x480x8:
        case VGA_800x600x8:
        case VGA_1024x768x8:
        case VGA_320x200x16:
        case VGA_400x300x16:
        case VGA_640x480x16:
        case VGA_800x600x16:
        case VGA_1024x768x16:
        case VGA_640x480x32:
        case VGA_800x600x32:
        case VGA_1024x768x32:
            break;
        default:
            return -1;
    }

    vga_mode = val;

    return 0;
}

#ifndef USE_MIDAS_SOUND

/* Flag: do we try to use triple buffering if possible?  */
static int try_triple_buffering;

static int set_try_triple_buffering(int val, void *param)
{
    /* FIXME: this has only effect when we switch to gfx mode.  This is OK
       for now, but is not the correct behavior.  */
    try_triple_buffering = val ? 1 : 0;
    return 0;
}

#endif

static const resource_int_t resources_int[] = {
    { "VGAMode", VGA_320x200x8, RES_EVENT_NO, NULL,
      &vga_mode, set_vga_mode, NULL },
#ifndef USE_MIDAS_SOUND
    { "TripleBuffering", 0, RES_EVENT_NO, NULL,
      &try_triple_buffering, set_try_triple_buffering, NULL },
#endif
    RESOURCE_INT_LIST_END
};

int video_arch_resources_init(void)
{
    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
}

/* ------------------------------------------------------------------------- */

/* Video-specific command-line options.  */

static const cmdline_option_t cmdline_options[] = {
    { "-vgamode", SET_RESOURCE, 1,
      NULL, NULL, "VGAMode", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<mode>", "Set VGA mode to <mode>. (0:320x200x8, 1:360x240x8, 2:360x270x8, 3:376x282x8, 4:400x300x8, 5:640x480x8, 6:800x600x8, 7:1024x768x8, 8:320x200x16, 9:400x300x16, 10:640x480x16, 11:800x600x16, 12:1024x768x16, 13:640x480x32, 14:800x600x32, 15:1024x768x32)" },
#ifndef USE_MIDAS_SOUND
    { "-triplebuf", SET_RESOURCE, 0,
      NULL, NULL, "TripleBuffering", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Try to use triple buffering when possible" },
    { "+triplebuf", SET_RESOURCE, 0,
      NULL, NULL, "TripleBuffering", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable usage of triple buffering" },
#endif
    CMDLINE_LIST_END
};

int video_arch_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

int video_init(void)
{
    int i;

    video_log = log_open("Video");

    in_gfx_mode = 0;

    last_canvas = NULL;

    for (i = 0; i < MAX_CANVAS_NUM; i++) {
        canvaslist[i] = NULL;
    }

    return 0;
}

void video_shutdown(void)
{
}

static void canvas_free_bitmaps(video_canvas_t *c)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (c->pages[i]) {
            destroy_bitmap(c->pages[i]);
            c->pages[i] = NULL;
        }
    }

    if (c->render_bitmap) {
        destroy_bitmap(c->render_bitmap);
        c->render_bitmap = NULL;
    }
}

static void canvas_update_colors(video_canvas_t *c)
{
    int i;

    if (c == NULL) {
        return;
    }

    for (i = 0; i < NUM_AVAILABLE_COLORS; i++) {
        set_color(i, &c->colors[i]);
    }
}

static int canvas_set_vga_mode(struct video_canvas_s *c)
{
    statusbar_reset_bitmaps_to_update();
    set_color_depth(c->depth);

#ifndef USE_MIDAS_SOUND
    /* If the user wants triple buffering, try Mode X first of all, as that
       is (currently) the only reliable way to achieve the result.  Virtual
       height is twice visible height to allow smooth page flipping.  */
    if (try_triple_buffering && (set_gfx_mode(GFX_MODEX, c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height, 0, c->draw_buffer->canvas_physical_height * 2) >= 0)) {
        DEBUG(("GFX_MODEX successful with width=%d height=%d vheight=%d", c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height, c->draw_buffer->canvas_physical_height * 2));
        c->use_triple_buffering = 1;
    } else
#endif
    /* If we don't want triple buffering, try to get a VESA linear mode
       first, which might not be the default. */
    if (set_gfx_mode(GFX_VESA2L, c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height, 0, 0) >= 0) {
        DEBUG(("GFX_VESA2L successful with width=%d height=%d", c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height));
        c->use_triple_buffering = 0;
        statusbar_append_bitmap_to_update(screen);
    } else if (set_gfx_mode(GFX_AUTODETECT, c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height, 0, 0) >= 0) {
        DEBUG(("GFX_AUTODETECT successful with width=%d height=%d", c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height));
        c->use_triple_buffering = 0;
        statusbar_append_bitmap_to_update(screen);
    } else {
        log_error(video_log, "Cannot enable %dx%d (%dBit) graphics.", c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height, c->depth);
        return -1;
    }

    log_message(video_log, "Using mode %dx%d (%dBit) (%s)%s.", c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height, c->depth,
                is_linear_bitmap(screen) ? "linear" : "planar", c->use_triple_buffering ? "; triple buffering possible" : "");
    in_gfx_mode = 1;

#ifndef USE_MIDAS_SOUND
    /* If using triple buffering, setup the timer used by Allegro to emulate
       vertical retrace interrupts.  Wish I had $D012/$D011 on VGA.  */
    timer_simulate_retrace(c->use_triple_buffering);
#endif

    canvas_free_bitmaps(c);

    c->render_bitmap = create_bitmap(c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height);
    if (c->use_triple_buffering) {
        c->pages[0] = create_sub_bitmap(screen, 0, 0, c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height);
        c->pages[1] = create_sub_bitmap(screen, 0, c->draw_buffer->canvas_physical_height, c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height);
        c->back_page = 1;

        statusbar_append_bitmap_to_update(c->pages[0]);
        statusbar_append_bitmap_to_update(c->pages[1]);
    }

    statusbar_set_width(c->draw_buffer->canvas_physical_width);

    canvas_update_colors(c);

    return 0;
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    canvas->video_draw_buffer_callback = NULL;
}

/* Note: `mapped' is ignored.  */
video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height, int mapped)
{
    int result = 0;
    int next_canvas = 0;

    DEBUG(("Creating canvas width=%d height=%d", *width, *height));
    if (canvas->palette && canvas->palette->num_entries > NUM_AVAILABLE_COLORS) {
        log_error(video_log, "Too many colors requested.");
        return NULL;
    }

    canvas->pages[0] = canvas->pages[1] = NULL;
    canvas->render_bitmap = NULL;

    DEBUG(("Setting VGA mode"));
    do {
        video_canvas_resize(canvas, 1);
        *width = canvas->draw_buffer->canvas_physical_width;
        *height = canvas->draw_buffer->canvas_physical_height;
        result += canvas_set_vga_mode(canvas);
        if (result == -1) {
            log_error(video_log, "Falling back to default VGA mode.");
            resources_set_int("VGAMode", (int)VGA_640x480x32);
        }
        if (result == -2) {
            log_error(video_log, "Even default VGA mode doesn't work. Exiting...");
            exit(-1);
        }
    } while (result < 0);

    video_canvas_set_palette(canvas, canvas->palette);

    canvas->back_page = 1;

    while (canvaslist[next_canvas] != NULL && next_canvas < MAX_CANVAS_NUM - 1) {
        next_canvas++;
    }
    canvaslist[next_canvas] = canvas;

    return canvas;
}

void video_canvas_destroy(video_canvas_t *canvas)
{
    int i;

    if (canvas == NULL) {
        return;
    }

    canvas_free_bitmaps(canvas);

    for (i = 0; i < MAX_CANVAS_NUM; i++) {
        if (canvaslist[i] == canvas) {
            canvaslist[i] = NULL;
        }
    }
}

static void canvas_change_palette(video_canvas_t *c)
{
    unsigned int i;
    int col;
    int next_avail = 0;

    if (c->palette) {
        for (i = 0; i < c->palette->num_entries; i++) {
            if (c->depth == 8) {
                /* For 8-bit-mode we need to use the global palette */
                c->colors[i].r = c->palette->entries[i].red >> 2;
                c->colors[i].g = c->palette->entries[i].green >> 2;
                c->colors[i].b = c->palette->entries[i].blue >> 2;

                col = i;
                next_avail++;

                DEBUG(("canvas_change_palette: palette entry %d: %d-%d-%d", i,
                            c->colors[i].r,
                            c->colors[i].g,
                            c->colors[i].b));
            } else {
                col = makecol_depth(c->depth,
                        c->palette->entries[i].red,
                        c->palette->entries[i].green,
                        c->palette->entries[i].blue);
            }

            DEBUG(("canvas_change_palette: videoconfig col %d: %d", i, col));

            video_render_setphysicalcolor(c->videoconfig, i, col, c->depth);
        }
    }

    if (c->depth > 8) {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i,
                                   makecol_depth(c->depth, i, 0, 0),
                                   makecol_depth(c->depth, 0, i, 0),
                                   makecol_depth(c->depth, 0, 0, i));
        }
        video_render_initraw(c->videoconfig);
    }

    canvas_update_colors(c);

    statusbar_register_colors(next_avail, c->colors);
}


int video_canvas_set_palette(struct video_canvas_s *canvas, palette_t *palette)
{
    if (palette == NULL) {
        return 0; /* no palette, nothing to do */
    }

    DEBUG(("Allocating %d colors", palette->num_entries));

    canvas->palette = palette;

    canvas_change_palette(canvas);

    return 0;
}

void video_canvas_map(video_canvas_t *canvas)
{
    /* Not implemented. */
}

void video_canvas_unmap(video_canvas_t *canvas)
{
    /* Not implemented. */
}

/* Warning: this does not do what you would expect from it.  It just sets the
   canvas size according to the `VGAMode' resource. */
void video_canvas_resize(video_canvas_t *canvas, char resize_canvas)
{
    /*
    FIXME: the possible height for the statusbar isn't calculated,
    it's only checked whether VGA-mode has >200 lines
    */
    statusbar_set_height(vga_modes[vga_mode].height > 200 ?
        STATUSBAR_HEIGHT : 0);

    DEBUG(("Resizing, vga_mode=%d", vga_mode));
    canvas->draw_buffer->canvas_physical_width = vga_modes[vga_mode].width;
    canvas->draw_buffer->canvas_physical_height = vga_modes[vga_mode].height;
    canvas->depth = vga_modes[vga_mode].depth;
    canvas->bytes_per_line = canvas->draw_buffer->canvas_physical_width * canvas->depth / 8;
}

void video_ack_vga_mode(void)
{
    if (last_canvas != NULL) {
        last_canvas->draw_buffer->canvas_width = last_canvas->draw_buffer->canvas_physical_width;
        last_canvas->draw_buffer->canvas_height = last_canvas->draw_buffer->canvas_physical_height;
        video_viewport_resize(last_canvas, 0);

        DEBUG(("Acknowledged vgaMode %d", vga_mode));
    }
}

/* ------------------------------------------------------------------------- */

void enable_text(void)
{
    DEBUG(("Enabling text mode"));
    kbd_uninstall();
    set_gfx_mode(GFX_TEXT, 80, 25, 0, 0);
    _set_screen_lines(25);
    DEBUG(("Successful"));
    in_gfx_mode = 0;
}

void disable_text(void)
{
    int i;
    video_canvas_t *canvas;


    DEBUG(("Enabling gfx mode"));
    kbd_install();

    video_ack_vga_mode();

    for (i = 0; i<MAX_CANVAS_NUM; i++) {
        canvas = canvaslist[i];
        if (canvas != NULL) {
            if (canvas_set_vga_mode(canvas) < 0) {
                resources_set_int("VGAMode", (int)VGA_320x200x8);
                video_canvas_resize(canvas, 1);
                canvas_set_vga_mode(canvas);
                ui_error("Cannot enable the selected VGA mode, falling back to default.");
            }
            canvas_change_palette(canvas);

            canvas->draw_buffer->canvas_width = canvas->draw_buffer->canvas_physical_width;
            canvas->draw_buffer->canvas_height = canvas->draw_buffer->canvas_physical_height;
            video_viewport_resize(canvas, 0);
        }
    }
    
    canvas_update_colors(last_canvas);

    in_gfx_mode = 1;

    statusbar_update();

    DEBUG(("Successful"));
}


inline void video_canvas_refresh(video_canvas_t *c,
                                 unsigned int xs, unsigned int ys,
                                 unsigned int xi, unsigned int yi,
                                 unsigned int w, unsigned int h)
{
    int y_diff;

    /* Just to be sure...  */
    if (screen == NULL)
        return;

    xs *= c->videoconfig->scalex;
    xi /= c->videoconfig->scalex;
    w *= c->videoconfig->scalex;

    ys *= c->videoconfig->scaley;
    yi /= c->videoconfig->scaley;
    h *= c->videoconfig->scaley;

    /* this is a hack for F7 change between VICII and VDC */
    if (last_canvas != c) {
        last_canvas = c;
        canvas_update_colors(c);
        clear(screen);
    }

    w = MIN(w, vga_modes[vga_mode].width - xi);
    h = MIN(h, vga_modes[vga_mode].height - yi);

    /* don't overwrite statusbar */
    if (statusbar_enabled() && (yi < STATUSBAR_HEIGHT)) {
        y_diff = STATUSBAR_HEIGHT - yi;
        ys += y_diff;
        yi += y_diff;
        h -= y_diff;
    }
    if (statusbar_enabled() && (c->use_triple_buffering)
        && (yi >= c->draw_buffer->canvas_physical_height) && (yi < c->draw_buffer->canvas_physical_height + STATUSBAR_HEIGHT)) {
        y_diff = STATUSBAR_HEIGHT + c->draw_buffer->canvas_physical_height - yi;
        ys += y_diff;
        yi += y_diff;
        h -= y_diff;
    }

    video_canvas_render(c,
                        (uint8_t *)(c->render_bitmap->line[0]),
                        w, h,
                        xs, ys,
                        xi, yi,
                        c->bytes_per_line,
                        c->depth);

    DEBUG(("video_canvas_render: VR:%p w=%d h=%d xs=%d ys=%d xi=%d yi=%d linef=%d linev=%d",
                      (uint8_t *)(c->render_bitmap->line[0]),
                      w, h,
                      xs, ys,
                      xi, yi,
                      c->bytes_per_line,
                      c->depth));

    if (c->use_triple_buffering) {
#if 0
        /* (This should be theoretically correct, but in practice it makes us
           loose time, and sometimes click.  So it's better to just discard
           the frame if this happens, as we do in the #else case.  */
        while (poll_modex_scroll())
            /* Make sure we have finished flipping the previous frame.  */ ;
#else
        if (poll_scroll())
            return;
#endif
        blit(c->render_bitmap, c->pages[c->back_page], xi, yi, xi, yi, w, h);
        request_scroll(0, c->back_page * c->draw_buffer->canvas_physical_height);
        c->back_page = 1 - c->back_page;
    } else {
        blit(c->render_bitmap, screen, xi, yi, xi, yi, w, h);
   }
}



void canvas_set_border_color(video_canvas_t *canvas, uint8_t color)
{
    inportb(0x3da);
    outportb(0x3c0, 0x31);
    outportb(0x3c0, color);
}

void fullscreen_capability(cap_fullscreen_t *cap_fullscreen)
{
    cap_fullscreen->device_num = 0;
}

char video_canvas_can_resize(struct video_canvas_s *canvas)
{
  return 0;
}
