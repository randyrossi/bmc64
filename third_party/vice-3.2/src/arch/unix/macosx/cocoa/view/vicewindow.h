/*
 * vicewindow.h - VICEWindow
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

//some Cocoa headers give compile errors if DEBUG is defined without a value
#ifdef DEBUG
#undef DEBUG
#define DEBUG 1
#endif
#import <Cocoa/Cocoa.h>
#import "viceglview.h"
#import "fullscreenwindow.h"

@interface VICEWindow : NSWindow <FullscreenToggling>
{
    NSBox *canvasContainer;
    VICEGLView *canvasView;
    
    NSSize original_canvas_size;
    BOOL isFullscreen;
    FullscreenWindow *fullscreenWindow;
    
    float scale_x;
    float scale_y;
    
    float pixelAspectRatio;
}

// create the window and setup canvas structure
- (id)initWithContentRect:(NSRect)rect title:(NSString *)title canvasSize:(NSSize)size pixelAspectRatio:(float)par;

// return the GLView for render access
- (VICEGLView *)getVICEGLView;
// resize canvas
- (void)resizeCanvas:(NSSize)size pixelAspectRatio:(float)par;

// return canvas id
- (int)canvasId;

// close window -> terminate app
- (BOOL)windowShouldClose:(id)sender;

// toggle fullscreen of canvas
- (void)toggleFullscreen:(id)sender;
// resize to multiple size taken from [sender tag]
- (void)resizeCanvasToMultipleSize:(id)sender;

// ----- copy & paste support -----
// copy
-(IBAction)copy:(id)sender;
// paste
-(IBAction)paste:(id)sender;

@end

