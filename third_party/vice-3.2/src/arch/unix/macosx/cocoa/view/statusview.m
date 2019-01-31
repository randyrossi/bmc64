/*
 * statusview.m - Status bar view
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

#import "vicenotifications.h"
#import "statusview.h"

#define STATUS_HEIGHT 22
#define JOY_WIDTH   22

@implementation StatusView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self==nil)
        return nil;

    [self setContentViewMargins:NSMakeSize(0, 0)];
    [self setTitlePosition:NSNoTitle];
    [self setBorderType:NSNoBorder]; // NSNoBorder/NSLineBorder/NSGrooveBorder
    [self setAutoresizingMask: (NSViewWidthSizable | NSViewMaxYMargin)];

    int speedWidth = NSWidth(frame)-(2*JOY_WIDTH);
    int h = NSHeight(frame);
    
    // speed display
    speedView = [[NSTextField alloc] initWithFrame:NSMakeRect(0, -6, speedWidth, h)];
    [speedView setDrawsBackground:NO];
    [speedView setEditable:NO];
    [speedView setBordered:NO];
    [speedView setAutoresizingMask: NSViewWidthSizable];
    NSFont *font = [NSFont labelFontOfSize:10];
    [speedView setFont:font];
    [[speedView cell] setLineBreakMode:NSLineBreakByTruncatingTail];
    [self addSubview:speedView];

    // joystick 1
    joystickView1 = [[JoystickView alloc] initWithFrame:NSMakeRect(speedWidth,0,JOY_WIDTH,h)];
    [joystickView1 setAutoresizingMask: NSViewMinXMargin];
    [self addSubview:joystickView1];

    // joystick 2
    joystickView2 = [[JoystickView alloc] initWithFrame:NSMakeRect(speedWidth+JOY_WIDTH,0,JOY_WIDTH,h)];
    [joystickView2 setAutoresizingMask: NSViewMinXMargin];
    [self addSubview:joystickView2];

    // register notifcations
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displaySpeed:)
                                                 name:VICEDisplaySpeedNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayPause:)
                                                 name:VICETogglePauseNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayJoystick:)
                                                 name:VICEDisplayJoystickNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayMonitor:)
                                                 name:VICEMonitorStateNotification
                                               object:nil];
    return self;
}

- (void)dealloc
{
    [speedView release];
    [joystickView1 release];
    [joystickView2 release];

    [super dealloc];
}

// ---------- Status Display ----------

- (void)displaySpeed:(NSNotification*)notification
{
    NSDictionary *dict = [notification userInfo];
    NSString *s = [NSString stringWithFormat:@"%.1f%%, %.1f FPS",
                          [[dict objectForKey:@"speed"] floatValue],
                          [[dict objectForKey:@"frame_rate"] floatValue]];
    BOOL isWarp = [[dict objectForKey:@"warp_enabled"] boolValue];

    [speedView setStringValue:s];
    if (isWarp) {
        [speedView setTextColor: [NSColor redColor]];
    } else {
        [speedView setTextColor: [NSColor blackColor]];
    }
}

- (void)displayPause:(NSNotification*)notification
{
    BOOL pauseFlag = [[notification object] boolValue];
    if (pauseFlag)
        [speedView setStringValue:@"PAUSE"];
    else
        [speedView setStringValue:@"RESUME"];
}

- (void)displayMonitor:(NSNotification *)notification
{
    int state = [[[notification userInfo] objectForKey:@"state"] intValue];
    switch(state) {
        case VICEMonitorStateOn:
        case VICEMonitorStateResume:
            [speedView setStringValue:@"MONITOR"];
            break;
        case VICEMonitorStateOff:
        case VICEMonitorStateSuspend:
            [speedView setStringValue:@"EXIT MON"];
            break;
        default:
            break;
    }
}

- (void)displayJoystick:(NSNotification *)notification
{
    NSDictionary *dict = [notification userInfo];
    int joyVal1 = [[dict objectForKey:@"joy1"] intValue];
    int joyVal2 = [[dict objectForKey:@"joy2"] intValue];
    
    [joystickView1 setJoyValue:joyVal1];
    [joystickView2 setJoyValue:joyVal2];
}

@end


