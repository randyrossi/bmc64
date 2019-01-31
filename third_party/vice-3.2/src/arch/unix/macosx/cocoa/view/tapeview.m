/*
 * tapeview.m - TapeView
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

#import "tapeview.h"
#import "vicenotifications.h"
#import "viceapplication.h"
#import "viceappcontroller.h"

static const char *button_texts[] = {
    "\xe2\x96\xa0",  /* 0: stop    */
    "\xe2\x96\xb6",  /* 1: play    */
    "\xe2\x96\xb6\xe2\x96\xb6",      /* 2: forward */
    "\xe2\x97\x80\xe2\x97\x80",      /* 3: rewind  */
    "\xe2\x97\x89",  /* 4: record  */
    "R",             /* 5: reset */
    "C",             /* 6: reset counter */
    "\xe2\x8f\x8f",  /* 7: eject */
    "\xe2\x8f\x8e"   /* 8: attach */
};

static const char *status_texts[] =
{
    "\xe2\x96\xa1", "\xe2\x96\xa0",                           /* stop    */
    "\xe2\x96\xb7", "\xe2\x96\xb6",                             /* play    */
    "\xe2\x96\xb7\xe2\x96\xb7", "\xe2\x96\xb6\xe2\x96\xb6",     /* forward */
    "\xe2\x97\x81\xe2\x97\x81", "\xe2\x97\x80\xe2\x97\x80",     /* rewind  */
    "\xe2\x97\x8b", "\xe2\x97\x8f"                              /* record  */
};

static NSString *help_texts[] = {
    @"Stop Datasette",
    @"Press Play on Tape",
    @"Fast Forwart",
    @"Rewind",
    @"Record",
    @"Reset Datasette",
    @"Reset Counter",
    @"Eject Tape Image",
    @"Attach Tape Image"
};

@implementation TapeView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self==nil)
        return nil;
    
    // calculate width
    float w = NSWidth(frame);
    float bw = w / 6.0;
    float sw = bw;
    float cw = bw * 3;
    float h = NSHeight(frame) / 3.0;

    NSFont *font = [NSFont labelFontOfSize:10];

    // text box for track display
    tape_image = [[NSTextField alloc] initWithFrame:NSMakeRect(0,2*h-4,w,h)];
    [tape_image setFont:font];
    [tape_image setDrawsBackground:NO];
    [tape_image setAutoresizingMask:NSViewWidthSizable];
    [tape_image setEditable:NO];
    [tape_image setBordered:NO];
    [tape_image setAlignment:NSLeftTextAlignment];
    [[tape_image cell] setLineBreakMode:NSLineBreakByTruncatingHead];
    [self addSubview:tape_image];

    // counter display
    tape_counter = [[NSTextField alloc]
        initWithFrame:NSMakeRect(0 , h, cw, h)];
    [tape_counter setDrawsBackground:NO];
    [tape_counter setAlignment:NSLeftTextAlignment];
    [tape_counter setEditable:NO];
    [tape_counter setBordered:NO];
    [self addSubview:tape_counter];

    // status display
    tape_status = [[NSTextField alloc]
        initWithFrame:NSMakeRect(w-bw, h-4, sw, h)];
    [tape_status setFont:[NSFont fontWithName:@"Apple Symbols" size:16.0]];    
    [tape_status setDrawsBackground:NO];
    [tape_status setAlignment:NSCenterTextAlignment];
    [tape_status setEditable:NO];
    [tape_status setBordered:NO];
    [self addSubview:tape_status];

    // build control buttons
    int i;
    float x = 0.0;
    float y = 0.0;
    for (i=0;i<9;i++) {
        float font_size = 14.0;
        // make ff and fr buttons smaller
        if ((i==2)||(i==3)) {
            font_size = 10;
        }

        buttons[i] = [[NSButton alloc] initWithFrame:NSMakeRect(x,y,bw,h)];
        [buttons[i] setFont:[NSFont fontWithName:@"Apple Symbols" size:font_size]];    
        [buttons[i] setTag:i];
        [buttons[i] setTitle:
            [NSString stringWithCString:button_texts[i]   
                encoding:NSUTF8StringEncoding]];
        [buttons[i] setToolTip:help_texts[i]];
    
        [buttons[i] setTarget:self];
        [buttons[i] setAction:@selector(controlDatasette:)];    
        [self addSubview:buttons[i]];

        if (i!=5) {
            x += bw;
        } else {
            // reset counter button
            x = bw*2;
            y = h;
        }
    }

    // preset value
    tape_control_status = 0;
    tape_motor_status = 0;
    tapeControlEnabled = NO;
    tapeEjectEnabled = NO;
    [self updateImage:@""];
    [self updateCounter];
    [self updateTapeStatus];
    [self setTapeControlEnabled:false];
        
    // allow drop of images
    [self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayImage:)
                                                 name:VICEDisplayTapeImageNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayControlStatus:)
                                                 name:VICEDisplayTapeControlStatusNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayMotorStatus:)
                                                 name:VICEDisplayTapeMotorStatusNotification
                                               object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(displayCounter:)
                                                 name:VICEDisplayTapeCounterNotification
                                               object:nil];

    return self;
}

- (void)dealloc
{
    [tape_counter release];
    [tape_status release];

    [super dealloc];
}

- (void)updateImage:(NSString*)image
{
    if ([image length]==0) {
        [tape_image setStringValue:@"<no tape image>"];
        // toggle eject button
        [buttons[7] setEnabled:NO];
        tapeEjectEnabled = NO;
        [self setTapeControlEnabled:NO];
    } else {
        [tape_image setStringValue:image];
        // toggle eject button
        [buttons[7] setEnabled:YES];
        tapeEjectEnabled = YES;
        
        // TODO: enable controls only for TAP
        [self setTapeControlEnabled:YES];
    }
}

- (void)displayImage:(NSNotification*)notification
{
    NSDictionary * dict = [notification userInfo];
    [self updateImage:[dict objectForKey:@"image"]];
}

- (void)updateCounter
{
    NSString * s = [NSString stringWithFormat:@"T: %03d", counter];
    [tape_counter setStringValue:s];
}

- (void)displayCounter:(NSNotification*)notification
{
    counter = [[[notification userInfo] objectForKey:@"counter"] intValue];
    [self updateCounter];
}

- (void)updateTapeStatus
{
    NSString * s = [NSString stringWithUTF8String:
                        status_texts[tape_control_status*2+tape_motor_status]];
    [tape_status setStringValue:s];
}

- (void)displayControlStatus:(NSNotification*)notification
{
    tape_control_status = [[[notification userInfo] objectForKey:@"status"] intValue];
    [self updateTapeStatus];
}

- (void)displayMotorStatus:(NSNotification*)notification
{
    tape_motor_status = [[[notification userInfo] objectForKey:@"status"] intValue];
    [self updateTapeStatus];
}

- (void)setTapeControlEnabled:(BOOL)do_enable
{
    tapeControlEnabled = do_enable;
    
    [self updateCounter];
    [self updateTapeStatus];
        
    // toggle button
    int i;
    for (i=0;i<7;i++) {
        [buttons[i] setEnabled:do_enable];
    }
}

- (void)setEnabled:(BOOL)on
{
    int i;
    
    if (on==NO) {
        for (i=0;i<9;i++) {
            [buttons[i] setEnabled:NO];
        }
    } else {
        for (i=0;i<7;i++) {
            [buttons[i] setEnabled:tapeControlEnabled];
        }
        [buttons[7] setEnabled:tapeEjectEnabled];
        [buttons[8] setEnabled:YES];
    }
}

- (void)controlDatasette:(id)sender
{
    int command = [sender tag];
    
    // attach tape image
    if (command==7) {
        [[VICEApplication theAppController] detachTapeImage:self];
    } 
    // eject tape image
    else if (command==8) {
        [[VICEApplication theAppController] attachTapeImage:self];
    } 
    // datasette command 0..6
    else {
        [[VICEApplication theMachineController] controlDatasette:command];
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
    return [[VICEApplication theMachineController] attachTapeImage:path];
}

@end
