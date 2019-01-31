/*
 * video.m - MacVICE video interface
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *  Michael Klein <michael.klein@puffin.lb.shuttle.de>
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

#include "videoarch.h"
#include "palette.h"
#include "resources.h"
#include "cmdline.h"
#include "translate.h"
#include "log.h"

#import "vicemachinenotifier.h"
#import "vicemachine.h"
#import "vicewindow.h"

// Mac Video Log
log_t video_log = LOG_ERR;

// video parameter struct
static video_param_t video_param;

// dummy
int video_arch_cmdline_options_init(void)
{
    return 0;
}

// ---------- VICE Video Resources ----------

/* tell all canvases to reconfigure after setting new video_param resources */
static void video_reconfigure(int sizeAffected)
{
    int numCanvases = [theVICEMachine getNumCanvases];
    int i;
    
    for(i=0;i<numCanvases;i++) {
        video_canvas_t *canvas = [theVICEMachine getCanvasForId:i];

        NSData *data = [NSData dataWithBytes:&canvas length:sizeof(video_canvas_t *)];

        if(sizeAffected) {
            int width = canvas->width;
            int height = canvas->height;
            [[theVICEMachine app] resizeCanvas:data withSize:NSMakeSize(width,height)];
        } else {
            // call UI thread to reconfigure canvas
            [[theVICEMachine app] reconfigureCanvas:data];
        }
    }
}

static int set_sync_draw_mode(int val, void *param)
{
    switch (val) {
        case SYNC_DRAW_OFF:
        case SYNC_DRAW_NEAREST:
        case SYNC_DRAW_BLEND:
            break;
        default:
            return -1;
    }
    
    if (val != video_param.sync_draw_mode) {
        video_param.sync_draw_mode = val;
        video_reconfigure(0);
    }
    return 0;
}

static int set_sync_draw_buffers(int val, void *param)
{
    if (val < 1) {
        val = 0;
    } else if (val > 16) {
        val = 16;
    }

    if (val != video_param.sync_draw_buffers) {            
        video_param.sync_draw_buffers = val;
        video_reconfigure(0);
    }
    return 0;
}

static int set_sync_draw_flicker_fix(int value, void *param)
{
    int val = value ? 1 : 0;

    if(val != video_param.sync_draw_flicker_fix) {            
        video_param.sync_draw_flicker_fix = val;
        video_reconfigure(0);
    }
    return 0;
}

static int set_true_pixel_aspect(int value, void *param)
{
    int val = value ? 1 : 0;

    if(val != video_param.true_pixel_aspect) {            
        video_param.true_pixel_aspect = val;
        video_reconfigure(1);
    }
    return 0;
}

static int set_show_key_codes(int value, void *param)
{
    int val = value ? 1 : 0;
    
    if(val != video_param.show_key_codes) {
        video_param.show_key_codes = val;
        video_reconfigure(0);
    }
    return 0;
}

static resource_int_t resources_int[] =
{
    { "SyncDrawMode", SYNC_DRAW_OFF, RES_EVENT_NO, NULL,
       &video_param.sync_draw_mode, set_sync_draw_mode, NULL },
    { "SyncDrawBuffers", 0, RES_EVENT_NO, NULL,
       &video_param.sync_draw_buffers, set_sync_draw_buffers, NULL },
    { "SyncDrawFlickerFix", 0, RES_EVENT_NO, NULL,
       &video_param.sync_draw_flicker_fix, set_sync_draw_flicker_fix, NULL },
    { "TrueAspectRatio", 1, RES_EVENT_NO, NULL,
       &video_param.true_pixel_aspect, set_true_pixel_aspect, NULL },
    { "ShowKeyCodes", 0, RES_EVENT_NO, NULL,
       &video_param.show_key_codes, set_show_key_codes, NULL },
    RESOURCE_INT_LIST_END
};

int video_arch_resources_init(void)
{
    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
}

// ---------- VICE Video Command Line ----------

static const cmdline_option_t cmdline_options[] = {
    { "-syncdrawmode", SET_RESOURCE, 1,
      NULL, NULL, "SyncDrawMode", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-2>", N_("Enable draw synchronization to vertical blank") },
    { "-syncdrawbuffers", SET_RESOURCE, 1,
      NULL, NULL, "SyncDrawBuffers", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-8>", N_("Set number of buffers used for sync draw (0=use default)") },
    { "-syncdrawflickerfix", SET_RESOURCE, 0,
      NULL, NULL, "SyncDrawFlickerFix", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Enable flicker fixing in sync draw") },
    { "+syncdrawflickerfix", SET_RESOURCE, 0,
      NULL, NULL, "SyncDrawFlickerFix", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Disable flicker fixing in sync draw") },
    { "-trueaspect", SET_RESOURCE, 0,
      NULL, NULL, "TrueAspectRatio", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Enable true aspect ratio") },
    { "+trueaspect", SET_RESOURCE, 0,
      NULL, NULL, "TrueAspectRatio", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Disable true aspect ratio") },
    CMDLINE_LIST_END
};

int video_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

// --------- init/exit MacVICE video ----------

int video_init(void)
{
    if (video_log == LOG_ERR) {
        video_log = log_open("MacVideo");
    }
    return 0;
}

void video_shutdown(void)
{
    if (video_log != LOG_ERR) {
        log_close(video_log);
    }
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    canvas->pitch = 0;
    canvas->window = nil;
    canvas->view = nil;
    canvas->canvasId = 0;
    
    canvas->video_param = &video_param;
    canvas->video_draw_buffer_callback = NULL;
}

// ---------- VICE Canvas ----------

video_canvas_t *video_canvas_create(struct video_canvas_s *canvas,
                                    unsigned int *width,
                                    unsigned int *height,
                                    int mapped)
{	
	// visible width/height seem to be the only sensible values here...
	struct draw_buffer_s *db = canvas->draw_buffer;
    int w = db->visible_width;
    int h = db->visible_height;

    w *= canvas->videoconfig->scalex;
    h *= canvas->videoconfig->scaley;

	// this actually sets the canvas_physical_width/height in canvas->draw_buffer (?!)
	*width = w;
	*height = h;
	canvas->width = w;
	canvas->height = h;
	
    // encapsulate canvas ptr
    video_canvas_t *canvasPtr = canvas;
    NSData *data = [NSData dataWithBytes:&canvasPtr length:sizeof(video_canvas_t *)];

    // register canvas in machine controller and return Id for it
    canvas->canvasId = [theVICEMachine registerCanvas:canvas];

    // call UI thread to create canvas
    [[theVICEMachine app] createCanvas:data withSize:NSMakeSize(w,h)];

    // init rendering
    video_canvas_set_palette(canvas,canvas->palette);

    // re-post all required notifications for new window
    [[theVICEMachine machineNotifier] notifyNewWindow];

    return canvas;
}

void video_canvas_destroy(video_canvas_t *canvas)
{
    // encapsulate canvas ptr
    video_canvas_t *canvasPtr = canvas;
    NSData *data = [NSData dataWithBytes:&canvasPtr length:sizeof(video_canvas_t *)];

    // call UI thread to destroy canvas
    [[theVICEMachine app] destroyCanvas:data];
}

char video_canvas_can_resize(video_canvas_t *canvas)
{
    return 1;
}

// VICE wants to change the size of the canvas -> adapt View
void video_canvas_resize(video_canvas_t * canvas, char resize_canvas)
{
	struct draw_buffer_s *db = canvas->draw_buffer;	
    int width = db->visible_width;
    int height = db->visible_height;

    width *= canvas->videoconfig->scalex;
    height *= canvas->videoconfig->scaley;

	canvas->width = width;
	canvas->height = height;

    // encapsulate canvas ptr
    video_canvas_t *canvasPtr = canvas;
    NSData *data = [NSData dataWithBytes:&canvasPtr length:sizeof(video_canvas_t *)];

    // call UI thread to resize canvas
    [[theVICEMachine app] resizeCanvas:data withSize:NSMakeSize(width, height)];
}

extern int vsync_frame_counter;

void video_canvas_refresh(video_canvas_t *canvas,
                          unsigned int xs, unsigned int ys,
                          unsigned int xi, unsigned int yi,
                          unsigned int w, unsigned int h)
{
    xi *= canvas->videoconfig->scalex;
    w *= canvas->videoconfig->scalex;

    yi *= canvas->videoconfig->scaley;
    h *= canvas->videoconfig->scaley;

    w = MIN(w, canvas->width - xi);
    h = MIN(h, canvas->height - yi);

    // get drawing buffer
    VICEGLView *view = canvas->view;
    uint8_t *buffer = [view beginMachineDraw:vsync_frame_counter];
    if(buffer == NULL) {
        return;
    }
        
    // draw into buffer
    video_canvas_render(canvas, buffer,
                        w, h, xs, ys, xi, yi, 
                        canvas->pitch, canvas->depth);

    // notify end drawing
    [view endMachineDraw];
}

// ----- Palette Stuff -----

int video_canvas_set_palette(video_canvas_t *c, palette_t *p)
{
    int i;
    int rshift = 0;
    int rbits = 0;
    int gshift = 0;
    int gbits = 0;
    int bshift = 0;
    int bbits = 0;
    uint32_t rmask = 0;
    uint32_t gmask = 0;
    uint32_t bmask = 0;

    if (p == NULL) {
        return 0;	/* no palette, nothing to do */
    }

    c->palette = p;
    
    // set 32bit palette
    for (i = 0; i < p->num_entries; i++)
    {
        uint32_t col;

        rbits = 0; rshift = 16; rmask = 0xff;
        gbits = 0; gshift = 8; gmask = 0xff;
        bbits = 0; bshift = 0; bmask = 0xff;

        col = (p->entries[i].red   >> rbits) << rshift
            | (p->entries[i].green >> gbits) << gshift
            | (p->entries[i].blue  >> bbits) << bshift
            | 0xff000000; // alpha

        video_render_setphysicalcolor(c->videoconfig, i, col, c->depth);
    }

    // set colors for palemu
    for (i = 0; i < 256; i++)
    {
        video_render_setrawrgb(i,
                               ((i & (rmask << rbits)) >> rbits) << rshift,
                               ((i & (gmask << gbits)) >> gbits) << gshift,
                               ((i & (bmask << bbits)) >> bbits) << bshift);
    }
    video_render_setrawalpha(0xff000000);
    video_render_initraw(c->videoconfig);

    return 0;
}

// ----- Color Stuff -----

int uicolor_alloc_color(unsigned int red, unsigned int green,
                        unsigned int blue, unsigned long *color_pixel,
                        uint8_t *pixel_return)
{
    return 0;
}

void uicolor_convert_color_table(unsigned int colnr, uint8_t *data,
                                 long color_pixel, void *c)
{
}

void uicolor_free_color(unsigned int red, unsigned int green,
                        unsigned int blue, unsigned long color_pixel)
{
}
