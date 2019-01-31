/*
 * netplaycontrolwindowcontroller.m - netplay
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#import "netplaycontrolwindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

#include "network.h"

@implementation NetplayControlWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"NetplayControl"];
    [self registerForResourceUpdate:@selector(updateResources:)];
    
    flags[0] = NETWORK_CONTROL_KEYB;
    flags[1] = NETWORK_CONTROL_JOY1;
    flags[2] = NETWORK_CONTROL_JOY2;
    flags[3] = NETWORK_CONTROL_DEVC;
    flags[4] = NETWORK_CONTROL_RSRC;
    
    flags[5] = NETWORK_CONTROL_KEYB << NETWORK_CONTROL_CLIENTOFFSET;
    flags[6] = NETWORK_CONTROL_JOY1 << NETWORK_CONTROL_CLIENTOFFSET;
    flags[7] = NETWORK_CONTROL_JOY2 << NETWORK_CONTROL_CLIENTOFFSET;
    flags[8] = NETWORK_CONTROL_DEVC << NETWORK_CONTROL_CLIENTOFFSET;
    flags[9] = NETWORK_CONTROL_RSRC << NETWORK_CONTROL_CLIENTOFFSET;
    
    return self;
}

-(void)dealloc
{
    [super dealloc];
}

-(void)setButtonState
{
    int mode = [[VICEApplication theMachineController] getNetplayMode];
    if (mode == 0) {
        [startServerButton setEnabled:TRUE];
        [connectToButton setEnabled:TRUE];
        [disconnectButton setEnabled:FALSE];
        [infoTextField setStringValue:@""];
    } else {
        [startServerButton setEnabled:FALSE];
        [connectToButton setEnabled:FALSE];
        [disconnectButton setEnabled:TRUE];
        [infoTextField setStringValue:(mode == 3 ? @"Client" : @"Server")];
    }
}

-(void)windowDidLoad
{
    control[0] = control00;
    control[1] = control01;
    control[2] = control02;
    control[3] = control03;
    control[4] = control04;
    
    control[5] = control05;
    control[6] = control06;
    control[7] = control07;
    control[8] = control08;
    control[9] = control09;
    
    [self updateResources:nil];
    [self setButtonState];
    [super windowDidLoad];
}

-(void)updateResources:(NSNotification *)notification
{
    int ctl = [self getIntResource:@"NetworkControl"];
    int i;
    for (i = 0; i < 10; i++) {
        BOOL on = ((ctl & flags[i]) == flags[i]);
        [control[i] setState:on];
    }
    
    NSString *name = [self getStringResource:@"NetworkServerName"];
    int port = [self getIntResource:@"NetworkServerPort"];
    [serverNameTextField setStringValue:name];
    [serverPortTextField setIntValue:port];
}

// ----- Actions -----

-(void)setServerNameOnly
{
    NSString *name = [serverNameTextField stringValue];
    [self setStringResource:@"NetworkServerName" toValue:name];    
}

-(void)setServerPortOnly
{
    int port = [serverPortTextField intValue];
    [self setIntResource:@"NetworkServerPort" toValue:port];    
}

-(IBAction)setServerName:(id)sender
{
    [self setServerNameOnly];
    [self updateResources:nil];
}

-(IBAction)setServerPort:(id)sender
{
    [self setServerPortOnly];
    [self updateResources:nil];
}

-(IBAction)connectTo:(id)sender
{
    [self setServerNameOnly];
    [self setServerPortOnly];
    
    [[VICEApplication theMachineController] connectNetplayClient];
    [self setButtonState];
}

-(IBAction)startServerButton:(id)sender
{
    [self setServerNameOnly];
    [self setServerPortOnly];

    [[VICEApplication theMachineController] startNetplayServer];
    [self setButtonState];
}

-(IBAction)disconnect:(id)sender
{
    [[VICEApplication theMachineController] disconnectNetplay];
    [self setButtonState];
}

-(IBAction)changedControl:(id)sender
{
    int ctl = 0;
    int i;
    for (i = 0; i < 10; i++) {
        if ([control[i] state] == NSOnState) {
            ctl |= flags[i];
        }
    }
    [self setIntResource:@"NetworkControl" toValue:ctl];
    [self updateResources:nil];
}

@end
