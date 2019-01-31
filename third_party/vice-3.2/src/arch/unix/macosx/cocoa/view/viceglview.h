/*
 * viceglview.h - VICEGLView
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

#include "video.h"
#include "videoparam.h"

#import <Cocoa/Cocoa.h>
#import <CoreVideo/CVDisplayLink.h>

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

struct video_param_s;

#define MOUSE_HIDE_DELAY 4
#define MOUSE_IS_HIDDEN  -1
#define MOUSE_IS_SHOWN   -2

#define MAX_BUFFERS     8

#define NUM_KEY_MODIFIERS 4

struct texture_s {
    uint8_t         *buffer;            /* raw data of texture */
    GLuint           bindId;            /* GL ID for binding */
    unsigned long    timeStamp;         /* when the machine wrote into buffer */
    int              frameNo;           /* frame number of emulation */
};
typedef struct texture_s texture_t;

@interface VICEGLView : NSOpenGLView
{
    // Texture
    int         numTextures;            /* how many textures are set up? */
    NSSize      textureSize;            /* size of canvas/texture for machine drawing */
    int         textureByteSize;        /* size of texture in bytes */
    texture_t   texture[MAX_BUFFERS];   /* manage up to MAX_BUFFERS textures */

    // View
    NSSize      viewSize;               /* size of canvas view on screen (might be smaller than GL view) */
    NSPoint     viewOrigin;             /* origin of canvas view inside GL view */
    int         canvasId;               /* the canvas id assigned to this view */
    
    // Keyboard
    unsigned int lastKeyModifierFlags;
    unsigned int modifierKeyCode[NUM_KEY_MODIFIERS];
    unsigned int tempKeyMask;
    int          showKeyCodes;
    
    // Mouse
    BOOL        trackMouse;
    float       mouseXScale;
    float       mouseYScale;
    int         mouseX;
    int         mouseY;
    BOOL        mouseLeftButtonPressed;
    BOOL        mouseRightButtonPressed;
    NSTimer *   mouseHideTimer;
    int         mouseHideInterval;
    BOOL        mouseEmuEnabled;
    BOOL        lightpenEmuEnabled;
    
    // OpenGL
    NSOpenGLContext *glContext;
    CGLContextObj    cglContext;
    BOOL             isOpenGLReady;
    BOOL             postponedReconfigure;
    struct video_param_s video_param;   /* a copy of the current params */

    // DisplayLink
    CVDisplayLinkRef displayLink;
    BOOL             displayLinkEnabled;  /* display link is running */
    BOOL             displayLinkSynced;   /* display link delivers valid refresh rate */
    float            screenRefreshPeriod; /* refresh rate of screen (in ms) */

    // MultiBuffer (size is "numTextures")
    float            machineRefreshPeriod; /* refresh rate of the machine (in ms) */
    BOOL             blendingEnabled;      /* flag to enable blending */
    BOOL             overwriteBuffer;      /* needed to overwrite most recent buffer */
    BOOL             handleFullFrames;     /* do flicker fixing? i.e. handle full frames */
    BOOL             lastWasFullFrame;     /* if last blend was a full frame */

    int              drawPos;           /* position in ring buffer where to write to */ 
    int              displayPos;        /* position in ring buffer where to display from */
    int              numDrawn;          /* number of drawn buffers available */
    
    unsigned long    usToMsFactor;    /* factor to convert us to ms */
    unsigned long    hostToUsFactor;    /* how to convert host time to us */
    unsigned long    displayDelta;      /* delta to convert real time to interpol time for display */
    unsigned long    firstDrawTime;     /* when the first frame was rendered */
    unsigned long    lastDrawTime;      /* when the most recent frame was drawn */
    unsigned long    lastDisplayTime;   /* when the most recent diplay update happened */

    float            blendAlpha;        /* weight of left buffer */

    // Pixel Buffer
    NSOpenGLPixelBuffer *pixelBuffer;
    NSOpenGLContext     *glPixelBufferContext;
    CGLContextObj        cglPixelBufferContext;
    BOOL                 pixelBufferValid;
    GLuint               pixelBufferTextureId;
    
    // Pixel Aspect Ratio
    float           pixelAspectRatio;
}

// ----- interface -----

- (id)initWithFrame:(NSRect)rect;
- (void)dealloc;

// the canvas was reconfigured i.e. new parameters were set
- (void)reconfigureCanvas:(struct video_param_s *)vieo_param;

// the size of the canvas changed -> adapt textures and resources to new size
- (void)resizeCanvas:(NSSize)size;

// get next render buffer for drawing by emu. may return NULL if out of buffers
- (uint8_t *)beginMachineDraw:(int)frameNo;

// end rendering into buffer
- (void)endMachineDraw;

// report current canvas pitch in bytes
- (int)getCanvasPitch;

// report current canvas depth in bits
- (int)getCanvasDepth;

// register the id for this canvas
- (void)setCanvasId:(int)canvasId;

// return the current canvas id assigned to this view
- (int)canvasId;

// set the pixel aspect ratio for this view
- (void)setPixelAspectRatio:(float)par;

// ----- local -----

- (void)initBlend;
- (int)calcBlend;
- (void)toggleBlending:(BOOL)on;

- (BOOL)setupDisplayLink;
- (void)shutdownDisplayLink;
- (float)getDisplayLinkRefreshPeriod;

- (BOOL)setupPixelBufferWithSize:(NSSize)size;
- (void)deletePixelBuffer;

- (void)initTextures;
- (void)deleteAllTextures;
- (void)setupTextures:(int)num withSize:(NSSize)size;
- (void)updateTexture:(int)i;

- (void)mouseMove:(NSPoint)pos;

@end

