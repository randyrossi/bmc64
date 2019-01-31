/*
 * fullscreenwindow.m - FullscreenWindow
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

#import "fullscreenwindow.h"
#import "vicewindow.h"

#ifndef MAC_OS_X_VERSION_10_12
// for SetSystemUIMode:
#include <QuickTime/QuickTime.h>
#endif

@implementation FullscreenWindow

-(id)init
{
    self = [super initWithContentRect:[[NSScreen mainScreen] frame]
                            styleMask:NSBorderlessWindowMask
                              backing:NSBackingStoreBuffered
                                defer:NO
                               screen:[NSScreen mainScreen]];
    if (self==nil)
        return nil;
                                       
    [self setBackgroundColor:[NSColor blackColor]];
    [self setTitle:@"VICE Fullscreen"];
    [self setReleasedWhenClosed:YES];
    [self setCanHide:NO];

    [[NSNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector(setAppNonActive:)
           name: NSApplicationDidResignActiveNotification
         object: NSApp];
    
    [[NSNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector(setAppActive:)
           name: NSApplicationDidBecomeActiveNotification
         object: NSApp];

    [self setLevel:NSFloatingWindowLevel];
    return self;
}

-(void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    
    [super dealloc];
}

-(void)close
{
    [super close];
}

-(BOOL)canBecomeKeyWindow
{
    return YES;
}

-(void)becomeKeyWindow
{
    [super becomeKeyWindow];
#ifndef MAC_OS_X_VERSION_10_12
    SetSystemUIMode( kUIModeAllHidden, kUIOptionAutoShowMenuBar);
#else
    [NSApp setPresentationOptions:(NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar)];
#endif
}

-(void)resignKeyWindow
{
#ifndef MAC_OS_X_VERSION_10_12
    SetSystemUIMode( kUIModeNormal, 0);
#else
    [NSApp setPresentationOptions:NSApplicationPresentationDefault];
#endif
    [super resignKeyWindow];
}

// application became active again
-(void)setAppActive:(id)data
{
    // show fullscreen window again
    [self makeKeyAndOrderFront:nil];
}

// application is not active anymore
-(void)setAppNonActive:(id)data
{
    // hide full screen window
    [self orderOut:nil];
}

-(void)toggleFullscreen:(id)sender
{
    // report to VICEWindow to close fullscreen
    [toggler toggleFullscreen:self];
}

-(void)setToggler:(id<FullscreenToggling>)t
{
    toggler = t;
}

-(void)setCanvasId:(int)id
{
    canvasId = id;
}

-(int)canvasId
{
    return canvasId;
}

@end
