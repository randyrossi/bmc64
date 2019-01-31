/*
 * soundview.m - SoundView
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

#import "soundview.h"
#import "vicenotifications.h"
#import "viceapplication.h"
#import "viceappcontroller.h"

/* Unfortunately, the speaker symbols in the "Apple Symbols" font don't have
unicode code points. Now I use a RTF glyph overwrite to get the characters... */

/* a "small" RTF document that contains Glyph ID 1043 of "Apple Symbols" font */ 
static const char *minVolStr = "{\\rtf1\\ansi\\ansicpg1252\\cocoartf949\\cocoasubrtf460\n"
"{\\fonttbl\\f0\\fnil\\fcharset0 AppleSymbols;}\n"
"\\f0\\fs36 \\cf0 {{}{\\*\\glid1043 A}A}}\n";

/* a "small" RTF document that contains Glyph ID 1042 of "Apple Symbols" font */ 
static const char *maxVolStr = "{\\rtf1\\ansi\\ansicpg1252\\cocoartf949\\cocoasubrtf460\n"
"{\\fonttbl\\f0\\fnil\\fcharset0 AppleSymbols;}\n"
"\\f0\\fs36 \\cf0 {{}{\\*\\glid1042 A}A}}\n";

@implementation SoundView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self==nil)
        return nil;
    
    // calculate width
    float w = NSWidth(frame);
    float h = NSHeight(frame);
    float bw = h;

    NSFont *font = [NSFont labelFontOfSize:10];

    // min volume
    minVolume = [[NSTextField alloc] initWithFrame:NSMakeRect(0,-4,bw,h)];
    NSData *rtfMinVolString = [NSData dataWithBytes:minVolStr length:strlen(minVolStr)];
    [minVolume setAttributedStringValue:[[NSAttributedString alloc] initWithRTF:rtfMinVolString documentAttributes:NULL]];
    [minVolume setDrawsBackground:NO];
    [minVolume setEditable:NO];
    [minVolume setBordered:NO];
    [minVolume setAlignment:NSCenterTextAlignment];
    [self addSubview:minVolume];
    
    // max volume
    maxVolume = [[NSTextField alloc] initWithFrame:NSMakeRect(w-1*bw,-4,bw,h)];
    NSData *rtfMaxVolString = [NSData dataWithBytes:maxVolStr length:strlen(maxVolStr)];
    [maxVolume setAttributedStringValue:[[NSAttributedString alloc] initWithRTF:rtfMaxVolString documentAttributes:NULL]];
    [maxVolume setDrawsBackground:NO];
    [maxVolume setEditable:NO];
    [maxVolume setBordered:NO];
    [maxVolume setAlignment:NSCenterTextAlignment];
    [self addSubview:maxVolume];

    // volume slider
    volumeSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(bw,-2,w-bw*2,h)];
    [volumeSlider setMinValue:0];
    [volumeSlider setMaxValue:100];
    [volumeSlider setAction:@selector(volumeControl:)];
    [volumeSlider setTarget:self];
    [self addSubview:volumeSlider];

    // register for volume change notifications
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(changeVolume:)
                                                 name:VICEChangeVolumeNotification
                                               object:nil];

    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)volumeControl:(id)sender
{
    int volume = [sender intValue];
    [[VICEApplication theAppController] setIntResource:@"SoundVolume" toValue:volume];
}

- (void)changeVolume:(id)notification
{
    int volume = [[[notification userInfo] objectForKey:@"volume"] intValue];
    [volumeSlider setIntValue:volume];
}

- (void)setEnabled:(BOOL)on
{
    [volumeSlider setEnabled:on];
}

@end
