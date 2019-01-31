/*
 * driveview.m - DriveView
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

#import "driveview.h"
#import "vicenotifications.h"
#import "viceapplication.h"
#import "viceappcontroller.h"

static const char *button_texts[] = {
     "\xe2\x8f\x8f",  /* eject */
     "\xe2\x8f\x8e"   /* attach */
};

static NSString *help_texts[] = {
    @"Eject Disk Image",
    @"Attach Disk Image"
};

@implementation DriveView

- (id)initWithFrame:(NSRect)frame driveNumber:(int)drive
{
    self = [super initWithFrame:frame];
    if (self==nil)
        return nil;
    
    driveNumber = drive;
    activeLedColor = nil;
    inactiveLedColor = [NSColor blackColor];
    [inactiveLedColor retain];
    ejectEnabled = NO;
    
    // calc layout
    float fw = NSWidth(frame);
    float fh = NSHeight(frame);
    
    float bw = fh / 2.0;

    NSRect trackRect = NSMakeRect(0,0,fw - bw * 3,bw);
    NSRect ledRect   = NSMakeRect(fw-bw,0,bw,bw);
    NSRect imageRect = NSMakeRect(0,bw-4,fw,bw);

    NSFont *font = [NSFont labelFontOfSize:10];
    
    // text box for track display
    trackText = [[NSTextField alloc] initWithFrame:trackRect];
    [trackText setDrawsBackground:NO];
    [trackText setAutoresizingMask:NSViewWidthSizable];
    [trackText setEditable:NO];
    [trackText setBordered:NO];
    [trackText setAlignment:NSLeftTextAlignment];
    [self addSubview:trackText];
    
    // color field for drive led
    driveLed = [[NSColorWell alloc] initWithFrame:ledRect];
    [driveLed setEnabled:NO];
    [driveLed setBordered:YES];
    [self addSubview:driveLed];

    // buttons
    int i;
    float x = fw - bw * 3;
    for (i=0;i<2;i++) {
        buttons[i] = [[NSButton alloc] initWithFrame:NSMakeRect(x,0,bw,bw)];
        [buttons[i] setFont:[NSFont fontWithName:@"Apple Symbols" size:14]];    
        [buttons[i] setTag:i];
        [buttons[i] setTitle:
            [NSString stringWithCString:button_texts[i]   
                encoding:NSUTF8StringEncoding]];
        [buttons[i] setToolTip:help_texts[i]];
    
        [buttons[i] setTarget:self];
        [buttons[i] setAction:@selector(buttonPressed:)];    
        [self addSubview:buttons[i]];

        x += bw;
    }

    // image name text field
    imageText = [[NSTextField alloc] initWithFrame:imageRect];
    [imageText setFont:font];
    [imageText setDrawsBackground:NO];
    [imageText setAutoresizingMask:NSViewWidthSizable];
    [imageText setEditable:NO];
    [imageText setBordered:NO];
    [imageText setAlignment:NSLeftTextAlignment];
    [[imageText cell] setLineBreakMode:NSLineBreakByTruncatingHead];
    [self addSubview:imageText];

    // register drag & drop
    [self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];

    // register notifications
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayLed:)
                                                 name:VICEDisplayDriveLedNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayImage:)
                                                 name:VICEDisplayDriveImageNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayTrack:)
                                                 name:VICEDisplayDriveTrackNotification
                                               object:nil];

    [self updateImage:@""];
    return self;
}

- (void)dealloc
{
    [driveLed release];
    [trackText release];
    [imageText release];
    
    [activeLedColor release];
    [inactiveLedColor release];
    
    [super dealloc];
}

- (void)initLedColor:(NSColor*)color
{
    if (activeLedColor==nil) {
        activeLedColor = color;
        [activeLedColor retain];
    }
}

- (void)displayLed:(NSNotification*)notification
{
    NSDictionary * dict = [notification userInfo];
    int drive = [[dict objectForKey:@"drive"] intValue];
    
    if (drive == driveNumber) {
        BOOL active = [[dict objectForKey:@"active"] boolValue];
        NSColor *color;
        if (active && (activeLedColor!=nil)) {
            color = activeLedColor;
        } else {
            color = inactiveLedColor;
        }
        [driveLed setColor:color];
    }
}

- (void)updateImage:(NSString *)image
{
    if ([image length]==0) {
        [imageText setStringValue:@"<no disk image>"];
        // toggle eject button
        [buttons[0] setEnabled:NO];
        ejectEnabled = NO;
    } else {
        [imageText setStringValue:image];
        // toggle eject button
        [buttons[0] setEnabled:YES];
        ejectEnabled = YES;
    }    
}

- (void)displayImage:(NSNotification*)notification
{
    NSDictionary * dict = [notification userInfo];
    int drive = [[dict objectForKey:@"drive"] intValue];
    
    if (drive == driveNumber) {
        NSString *image = [dict objectForKey:@"image"];
        [self updateImage:image];
    }
}

- (void)displayTrack:(NSNotification*)notification
{
    NSDictionary * dict = [notification userInfo];
    int drive = [[dict objectForKey:@"drive"] intValue];
    int curDriveBase = [[dict objectForKey:@"drive_base"] intValue];

    // store in object
    driveBase = curDriveBase;

    if (drive == driveNumber)
    {
        float track = [[dict objectForKey:@"track"] floatValue];
        NSString * s = [NSString stringWithFormat:@"%d: %.1f", drive + driveBase, track];
        [trackText setStringValue:s];
    }
}

- (void)buttonPressed:(id)sender
{
    int command = [sender tag];
    if (command==0) {
        // eject disk image
        [[VICEApplication theAppController] detachDiskImage:self];
    } else if (command==1) {
        // attach disk image
        [[VICEApplication theAppController] attachDiskImage:self];
    }
}

- (NSInteger)tag
{
    // callback from button pressed
    return driveNumber + driveBase;
}

- (void)setEnabled:(BOOL)on
{
    if (on == NO) {
        [buttons[0] setEnabled:NO];
        [buttons[1] setEnabled:NO];
    } else {
        [buttons[0] setEnabled:ejectEnabled];
        [buttons[1] setEnabled:YES];
    }
}

// ----- Drag & Drop -----

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    if (NSDragOperationGeneric & [sender draggingSourceOperationMask])
        return NSDragOperationLink;
    else
        return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    id files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    NSString * path = [files objectAtIndex:0];
    return [[VICEApplication theMachineController] attachDiskImage:driveNumber + driveBase
                                                              path:path];
}


@end
