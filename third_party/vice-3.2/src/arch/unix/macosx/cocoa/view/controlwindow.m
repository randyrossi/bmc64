/*
 * controlwindow.m - Control Window
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

#include "vice.h"
#include "log.h"
#include "machine.h"

#import <Cocoa/Cocoa.h>
#import "controlwindow.h"
#import "vicenotifications.h"
#import "vicewindow.h"

#define PERIPH_WIDTH            140
#define PERIPH_HEIGHT           22
#define PERIPH_OFFSET           5

/* ============================================================== */


@implementation FlippedView : NSView
- (BOOL)isFlipped
{
    return YES;
}
@end

/* ============================================================== */

@implementation ControlWindow

- (id)initWithContentRect:(NSRect)rect title:(NSString *)title;
{
    unsigned int style = NSUtilityWindowMask | NSTitledWindowMask |
        NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
    self = [super initWithContentRect:rect
                            styleMask:style
                              backing:NSBackingStoreBuffered
                                defer:NO];
    if (self==nil)
        return nil;

    [self setReleasedWhenClosed:NO];
    [self setFloatingPanel:TRUE];

    // keep panel visible if another application activates
    // helps dropping images onto drive slots from finder
    [self setHidesOnDeactivate:NO];
    
    // set title
    [self setTitle:title];
    [self setFrameAutosaveName:[self title]];

    led_color[0] = [[NSColor redColor] retain];
    led_color[1] = [[NSColor greenColor] retain];

    // main view container
    main_view = [[FlippedView alloc]
        initWithFrame:NSMakeRect(0,0,PERIPH_WIDTH,PERIPH_HEIGHT*3)];
    [main_view setAutoresizingMask: (NSViewWidthSizable|NSViewHeightSizable)];

    // create disk views
    int i;
    for (i = 0; i < DRIVE_NUM; i++)
    {
        NSRect rect = NSMakeRect(0, 0, PERIPH_WIDTH, PERIPH_HEIGHT*2);
        drive_view[i] = [[DriveView alloc] initWithFrame:rect driveNumber:i];
        [drive_view[i] setAutoresizingMask: NSViewWidthSizable];
    }

    // create tape view
    tape_view = [[TapeView alloc] initWithFrame:NSMakeRect(0, 0, PERIPH_WIDTH, PERIPH_HEIGHT*3)];
    [tape_view setAutoresizingMask: NSViewWidthSizable];
    [main_view addSubview:tape_view];

    // sound
    sound_view = [[SoundView alloc] initWithFrame:NSMakeRect(0, 0, PERIPH_WIDTH, PERIPH_HEIGHT)];
    [sound_view setAutoresizingMask: NSViewWidthSizable];
    [main_view addSubview:sound_view];

    // status view
    status_view = [[StatusView alloc] initWithFrame:NSMakeRect(0, 0, PERIPH_WIDTH, PERIPH_HEIGHT)];
    [status_view setAutoresizingMask: NSViewWidthSizable];
    [main_view addSubview:status_view];

    [self setContentView:main_view];

    // react on
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(enableDriveStatus:)
                                                 name:VICEEnableDriveStatusNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(enableTapeStatus:)
                                                 name:VICEEnableTapeStatusNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(monitorStateChanged:)
                                                 name:VICEMonitorStateNotification
                                               object:nil];
    return self;
}

- (void)dealloc
{
    int i;
    [led_color[0] release];
    [led_color[1] release];

    for (i = 0; i < DRIVE_NUM; i++)
    {
        [drive_view[i] release];
    }

    [tape_view release];
    [super dealloc];
}

- (BOOL)canBecomeMainWindow
{
    return NO;
}

- (void)enableDriveStatus:(NSNotification*)notification
{
    NSRect frameRect = [self frame];
    NSRect boundsRect = [[self contentView] bounds];
    
    // setup drive views
    NSDictionary * dict = [notification userInfo];
    int drive_led_color = [[dict objectForKey:@"drive_led_color"] intValue];
    int enable = [[dict objectForKey:@"enabled_drives"] intValue];

    int i, count = 0;
    float width = NSWidth([[self contentView] bounds]);
    float w = width - 2 * PERIPH_OFFSET;
    float y = PERIPH_OFFSET;
    float x = PERIPH_OFFSET;
    float total_height = PERIPH_OFFSET;
    for (i = 0; i < DRIVE_NUM; i++)
    {
        int flag = 1<<i;
        if (enable & flag)
        {
            int colorIndex = (drive_led_color & flag) != 0 ? 1 : 0; 
            [[self contentView] addSubview:drive_view[i]];
            [drive_view[i] initLedColor:led_color[colorIndex]];
            [drive_view[i] setFrame:NSMakeRect(x, y, w, PERIPH_HEIGHT*2)];
            count++;
            y += PERIPH_HEIGHT*2;
            total_height += PERIPH_HEIGHT*2;
        }
        else
        {
            [drive_view[i] removeFromSuperview];
        }
    }

    // reposition tape display and rec/play field
    [tape_view setFrame:NSMakeRect(x, y, w, PERIPH_HEIGHT*3)];
    y += PERIPH_HEIGHT * 3;
    [sound_view setFrame:NSMakeRect(x, y, w, PERIPH_HEIGHT)];
    y += PERIPH_HEIGHT;
    [status_view setFrame:NSMakeRect(x, y, w, PERIPH_HEIGHT)];
    
    [[self contentView] setNeedsDisplay:YES];
    total_height += PERIPH_HEIGHT*5;
    total_height += 2;
    
    // adjust window frame to stay on same top
    float old_top = NSMinY(frameRect) + NSHeight(frameRect);
    float title_h = NSHeight(frameRect) - NSHeight(boundsRect); 
    float hf = total_height + title_h;
    float yf = old_top - hf;
    [self setFrame:NSMakeRect(NSMinX(frameRect),
                              yf,
                              NSWidth(frameRect),
                              hf) 
           display:TRUE
           animate:TRUE];
}

- (void)enableTapeStatus:(NSNotification*)notification
{
    BOOL enable = [[[notification userInfo] objectForKey:@"enable"] intValue];
    [tape_view setEnabled:enable];
    [[self contentView] setNeedsDisplay:YES];
}

// adjust resizing to be fixed
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)proposedFrameSize
{
    if (![[self contentView] inLiveResize]) {
        return proposedFrameSize;
    }
    
    float height = NSHeight([self frame]);
    float width  = proposedFrameSize.width;
    float min_width = PERIPH_WIDTH + 2 * PERIPH_OFFSET;
    if (width < min_width)
        width = min_width;
    return NSMakeSize(width,height);
}

// monitor state changed
- (void)monitorStateChanged:(NSNotification *)notification
{
    int state = [[[notification userInfo] objectForKey:@"state"] intValue];
    switch(state) {
        case VICEMonitorStateOn:
            [self setEnabled:NO];
            break;
        case VICEMonitorStateOff:
            [self setEnabled:YES];
            break;
        default:
            break;
    }
}

- (void)setEnabled:(BOOL)on
{
    int i;
    
    [tape_view setEnabled:on];
    for (i=0;i<DRIVE_NUM;i++) {
        [drive_view[i] setEnabled:on];
    }
    [sound_view setEnabled:on];
}

@end
