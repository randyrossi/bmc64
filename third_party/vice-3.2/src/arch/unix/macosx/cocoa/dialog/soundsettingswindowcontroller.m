/*
 * soundsettingswindowcontroller.m - SoundSettings dialog controller
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

#import "soundsettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

@implementation SoundSettingsWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"SoundSettings"];
    if (!self) {
        return self;
    }

    [self registerForResourceUpdate:@selector(updateResources:)];

    return self;
}

-(void)windowDidLoad
{
    [self updateResources:nil];
    [super windowDidLoad];
}

-(void)updateResources:(NSNotification *)notification
{
    int sampleRate = [self getIntResource:@"SoundSampleRate"];
    int bufferSize = [self getIntResource:@"SoundBufferSize"];
    int fragmentSize = [self getIntResource:@"SoundFragmentSize"];
    int speedAdjustment = [self getIntResource:@"SoundSpeedAdjustment"];
    [sampleRateButton selectItemWithTag:sampleRate];
    [bufferSizeButton selectItemWithTag:bufferSize];
    [fragmentSizeButton selectItemWithTag:fragmentSize];
    [speedAdjustmentButton selectItemWithTag:speedAdjustment];
}

// ----- Actions -----

-(IBAction)changeSampleRate:(id)sender
{
    int sampleRate = [[sampleRateButton selectedItem] tag];
    [self setIntResource:@"SoundSampleRate" 
                 toValue:sampleRate];
}

-(IBAction)changeBufferSize:(id)sender
{
    int bufferSize = [[bufferSizeButton selectedItem] tag];
    [self setIntResource:@"SoundBufferSize" 
                 toValue:bufferSize];
}

-(IBAction)changeSpeedAdjustment:(id)sender
{
    int speedAdjustment = [[speedAdjustmentButton selectedItem] tag];
    [self setIntResource:@"SoundSpeedAdjustment" 
                 toValue:speedAdjustment];
}

-(IBAction)changeFragmentSize:(id)sender
{
    int fragmentSize = [[fragmentSizeButton selectedItem] tag];
    [self setIntResource:@"SoundFragmentSize" 
                 toValue:fragmentSize];
}

@end
