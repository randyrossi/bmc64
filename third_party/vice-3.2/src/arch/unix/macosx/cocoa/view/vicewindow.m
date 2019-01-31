/*
 * vicewindow.m - VICEWindow
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

#import "vicewindow.h"
#import "viceapplication.h"

@implementation VICEWindow

- (id)initWithContentRect:(NSRect)rect title:(NSString *)title canvasSize:(NSSize)size pixelAspectRatio:(float)par
{    
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];

    // store initial canvas size
    unsigned int canvas_width = size.width;
    unsigned int canvas_height = size.height;
    original_canvas_size = size;

    // set window style
    unsigned int style =
        NSTitledWindowMask | NSClosableWindowMask |
        NSMiniaturizableWindowMask | NSResizableWindowMask;
    if ([def boolForKey:@"Textured Windows"])
        style |= NSTexturedBackgroundWindowMask;
    
    // store pixel aspect ratio
    pixelAspectRatio = par;
    
    // create window with desired size...
    // Note: size might get overwritten by user prefs
    self = [super initWithContentRect:rect
                            styleMask:style
                              backing:NSBackingStoreBuffered
                                defer:NO];
    if (self==nil)
        return nil;

    // setup window
    [self setTitle:title];
    [self setFrameAutosaveName:title];
    [self setContentMinSize:NSMakeSize(canvas_width * par / 2.0f, canvas_height / 2.0f)];

    // now size could have changed due to prefences size
    // so determine current canvas size
    NSRect bounds = [[self contentView] bounds];
    float cw = NSWidth(bounds);
    float ch = NSHeight(bounds);
    
    // read scale from prefs
    scale_x = [def floatForKey:[title stringByAppendingString:@"ScaleX"]];
    scale_y = [def floatForKey:[title stringByAppendingString:@"ScaleY"]];
    if(scale_x == 0.0f)
        scale_x = 1.0f;
    if(scale_y == 0.0f)
        scale_y = 1.0f;

    //NSLog(@"%@: canvas %u x %u bounds %g x %g  scale %g x %g",title,
    //    canvas_width, canvas_height, cw,ch,scale_x,scale_y);

    // the container box for the canvas
    rect = NSMakeRect(0, 0, cw, ch);
    canvasContainer = [[NSBox alloc] initWithFrame:rect];
    [canvasContainer setContentViewMargins:NSMakeSize(0, 0)];
    [canvasContainer setTitlePosition:NSNoTitle];
    [canvasContainer setBorderType:NSNoBorder]; // NSNoBorder/NSLineBorder/NSGrooveBorder
    [canvasContainer setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
    [[self contentView] addSubview:canvasContainer];
    
    // canvasView: the OpenGL render canvas
    rect = NSMakeRect(0, 0, cw, ch);
    canvasView = [[VICEGLView alloc] initWithFrame:rect];
    [canvasView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
    [canvasView resizeCanvas:NSMakeSize(canvas_width, canvas_height)];
    [canvasContainer addSubview:canvasView];

    // propagate pixel aspect ratio
    [canvasView setPixelAspectRatio:pixelAspectRatio];

    isFullscreen = false;
    return self;
}

- (void)close
{
    // disable fullscreen on close
    if (isFullscreen)
        [self toggleFullscreen:nil];
        
    [super close];
    
    // store scale to prefs
    NSUserDefaults *def = [NSUserDefaults standardUserDefaults];
    NSString *title = [self title];
    [def setFloat:scale_x forKey:[title stringByAppendingString:@"ScaleX"]];
    [def setFloat:scale_y forKey:[title stringByAppendingString:@"ScaleY"]];
}

- (void)dealloc
{
    [canvasView release];
    [canvasContainer release];
    [super dealloc];
}

- (VICEGLView *)getVICEGLView
{
    return canvasView;
}

- (int)canvasId
{
    return [canvasView canvasId];
}

- (void)resizeWindowToFactorX:(float)fx andY:(float)fy
{
    NSRect f = [self frame];
    NSSize s = [[self contentView] bounds].size;
    
    f.size.width  = original_canvas_size.width * fx * pixelAspectRatio;
    f.size.height += original_canvas_size.height * fy - s.height;
    f.origin.y    += s.height - original_canvas_size.height * fy;
    [self setFrame:f display:YES];
}

- (void)resizeCanvas:(NSSize)size pixelAspectRatio:(float)par
{
    // do not resize if same size
    if ((original_canvas_size.width  == size.width) && 
        (original_canvas_size.height == size.height) &&
        (pixelAspectRatio == par)) {
      return;
    }
    
    original_canvas_size = size;
    pixelAspectRatio = par;
    [canvasView setPixelAspectRatio:par];
    [canvasView resizeCanvas:size];
    
    //NSLog(@"resize: canvas %g x %g scale %g x %g",size.width,size.height,scale_x,scale_y);

    [self setContentMinSize:NSMakeSize(size.width * par / 2.0f, size.height / 2.0f)];
    [self resizeWindowToFactorX:scale_x andY:scale_y];
}

// Note: resize Window here... not Canvas!
- (void)resizeCanvasToMultipleSize:(id)sender
{
    int factor = 1;
    if (sender!=nil)
        factor = [sender tag];

    scale_x = factor;
    scale_y = factor;
    [self resizeWindowToFactorX:factor andY:factor];
}

- (BOOL)windowShouldClose:(id)sender
{
    // tell application to terminate
    [NSApp terminate:nil];

    return NO;
}

// adjust resizing to be proportional
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)proposedFrameSize
{
    UInt32 modifierMask = [[self currentEvent] modifierFlags];

    float titleHeight = NSHeight([self frame]) - NSHeight([[self contentView] bounds]);
    float contentHeight = proposedFrameSize.height - titleHeight;
    float contentWidth  = proposedFrameSize.width;

    float canvasWidthPAR = original_canvas_size.width * pixelAspectRatio;
    float canvasHeight = original_canvas_size.height;

    // SHIFT allows all sizes unconstraint
    if (modifierMask & NSShiftKeyMask ||
        ![[self contentView] inLiveResize])
    {
        scale_x = contentWidth  / canvasWidthPAR;
        scale_y = contentHeight / canvasHeight;
        
        return proposedFrameSize;
    }

    // ALT locks to multiple size x1, x2, ...
    if (modifierMask & NSAlternateKeyMask)
    {
        float scale = floor(contentHeight / canvasHeight + 0.5);
        if (scale < 1.0)
            scale = 1.0;
        contentWidth  = scale * canvasWidthPAR;
        contentHeight = scale * canvasHeight;
        
        scale_x = scale;
        scale_y = scale;
        
        return NSMakeSize(contentWidth, contentHeight + titleHeight);
    }

    // arbitrary but proportional scale
    float scale = contentHeight / canvasHeight;
    contentWidth  = scale * canvasWidthPAR;

    scale_x = scale;
    scale_y = scale;
    
    return NSMakeSize(contentWidth, contentHeight + titleHeight);
}

// toggle fullscreen
- (void)toggleFullscreen:(id)sender
{
    // fade out display
    CGDisplayFadeReservationToken displayFadeReservation;
    CGAcquireDisplayFadeReservation (
        kCGMaxDisplayReservationInterval,
        &displayFadeReservation
        );
    CGDisplayFade (
        displayFadeReservation,
        0.5,						// 0.5 seconds
        kCGDisplayBlendNormal,		// starting state
        kCGDisplayBlendSolidColor,	// ending state
        0.0, 0.0, 0.0,				// black
        TRUE						// wait for completion
        );

    // reattach canvasView to my canvasContainer or attach to fullscreen window
    if (isFullscreen) {
        [canvasContainer setContentView:canvasView];
        [fullscreenWindow close];
        fullscreenWindow = nil;
        [self makeKeyAndOrderFront:nil];
        isFullscreen = FALSE;
    } else {
        [self orderOut:nil];
        fullscreenWindow = [[FullscreenWindow alloc] init];
        [fullscreenWindow setCanvasId:[canvasView canvasId]];
        [fullscreenWindow setToggler:self];
        [fullscreenWindow setContentView:canvasView];
        [fullscreenWindow makeKeyAndOrderFront:self];
        isFullscreen = TRUE;
    }

    // fade in
    CGDisplayFade (
        displayFadeReservation,
        0.5,						// 0.5seconds
        kCGDisplayBlendSolidColor,	// starting state
        kCGDisplayBlendNormal,		// ending state
        0.0, 0.0, 0.0,				// black
        FALSE						// don't wait for completion
        );
    CGReleaseDisplayFadeReservation (displayFadeReservation);
}

// ----- copy & paste support -----

-(IBAction)copy:(id)sender
{
    NSString *data = [[VICEApplication theMachineController] readScreenOutput];
    if (data) {
        NSPasteboard *pb = [NSPasteboard generalPasteboard];
        [pb declareTypes:[NSArray arrayWithObject:NSStringPboardType]
            owner:self];
        [pb setString:data forType:NSStringPboardType];
    } else {
        NSBeep();
    }
}

-(IBAction)paste:(id)sender
{
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    NSString *type = [pb availableTypeFromArray:
                        [NSArray arrayWithObject:NSStringPboardType]];
    if (type) {
        NSString *value = [pb stringForType:NSStringPboardType];
        // type string on keyboard
        [[VICEApplication theMachineController] typeStringOnKeyboard:value toPetscii:YES];
    } else {
        NSBeep();
    }
}

@end
